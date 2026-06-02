#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    RootComponent = PrimaryAudioComponent;
    PrimaryAudioComponent->bAutoActivate = false;
    PrimaryAudioComponent->VolumeMultiplier = 0.0f;

    SecondaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAudioComponent"));
    SecondaryAudioComponent->SetupAttachment(RootComponent);
    SecondaryAudioComponent->bAutoActivate = false;
    SecondaryAudioComponent->VolumeMultiplier = 0.0f;

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Forest;
    TargetBiome = EAudio_BiomeType::Forest;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    BiomeDetectionRadius = 1000.0f;
    MasterVolume = 1.0f;
    bEnableRandomSounds = true;
    RandomSoundTimer = 0.0f;
    bPrimaryActive = true;
    FadeTimer = 0.0f;
    FadeDuration = 2.0f;

    // Initialize default biome settings
    FAudio_BiomeAudioSettings ForestSettings;
    ForestSettings.BaseVolume = 0.6f;
    ForestSettings.FadeInTime = 3.0f;
    ForestSettings.FadeOutTime = 2.0f;
    ForestSettings.RandomSoundInterval = 12.0f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Forest, ForestSettings);

    FAudio_BiomeAudioSettings PlainsSettings;
    PlainsSettings.BaseVolume = 0.4f;
    PlainsSettings.FadeInTime = 2.5f;
    PlainsSettings.FadeOutTime = 2.5f;
    PlainsSettings.RandomSoundInterval = 18.0f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Plains, PlainsSettings);

    FAudio_BiomeAudioSettings CaveSettings;
    CaveSettings.BaseVolume = 0.7f;
    CaveSettings.FadeInTime = 1.5f;
    CaveSettings.FadeOutTime = 1.5f;
    CaveSettings.RandomSoundInterval = 25.0f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Cave, CaveSettings);
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with current biome audio
    SetBiome(CurrentBiome);
}

void AAudio_EnvironmentalSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update biome transitions
    if (bIsTransitioning)
    {
        UpdateBiomeTransition(DeltaTime);
    }

    // Update random sounds
    if (bEnableRandomSounds)
    {
        UpdateRandomSounds(DeltaTime);
    }

    // Auto-detect biome changes
    EAudio_BiomeType DetectedBiome = DetectCurrentBiome();
    if (DetectedBiome != CurrentBiome && !bIsTransitioning)
    {
        SetBiome(DetectedBiome);
    }
}

void AAudio_EnvironmentalSoundManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome && !bIsTransitioning)
    {
        return;
    }

    StartBiomeTransition(NewBiome);
}

void AAudio_EnvironmentalSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->SetVolumeMultiplier(GetCurrentVolume());
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->SetVolumeMultiplier(GetCurrentVolume());
    }
}

void AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound()
{
    if (!BiomeAudioSettings.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];
    if (Settings.RandomSounds.Num() == 0)
    {
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, Settings.RandomSounds.Num() - 1);
    USoundWave* RandomSound = Settings.RandomSounds[RandomIndex];
    
    if (RandomSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            RandomSound,
            GetActorLocation(),
            MasterVolume * 0.8f
        );
    }
}

EAudio_BiomeType AAudio_EnvironmentalSoundManager::DetectCurrentBiome()
{
    // Simple biome detection based on player location
    // In a real implementation, this would query the world generation system
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return CurrentBiome;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float DistanceFromOrigin = FVector::Dist(PlayerLocation, FVector::ZeroVector);

    // Simple distance-based biome detection
    if (PlayerLocation.Z < -200.0f)
    {
        return EAudio_BiomeType::Cave;
    }
    else if (DistanceFromOrigin > 5000.0f)
    {
        return EAudio_BiomeType::Desert;
    }
    else if (PlayerLocation.Z > 1000.0f)
    {
        return EAudio_BiomeType::Mountains;
    }
    else if (DistanceFromOrigin > 3000.0f)
    {
        return EAudio_BiomeType::Plains;
    }
    else
    {
        return EAudio_BiomeType::Forest;
    }
}

float AAudio_EnvironmentalSoundManager::GetCurrentVolume() const
{
    if (!BiomeAudioSettings.Contains(CurrentBiome))
    {
        return MasterVolume * 0.5f;
    }

    const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];
    return MasterVolume * Settings.BaseVolume;
}

void AAudio_EnvironmentalSoundManager::UpdateBiomeTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }

    FadeTimer += DeltaTime;
    TransitionProgress = FadeTimer / FadeDuration;

    if (TransitionProgress >= 1.0f)
    {
        CompleteBiomeTransition();
        return;
    }

    // Update volumes during transition
    UAudioComponent* FadingOut = bPrimaryActive ? PrimaryAudioComponent : SecondaryAudioComponent;
    UAudioComponent* FadingIn = bPrimaryActive ? SecondaryAudioComponent : PrimaryAudioComponent;

    if (FadingOut)
    {
        float FadeOutVolume = (1.0f - TransitionProgress) * GetCurrentVolume();
        FadingOut->SetVolumeMultiplier(FadeOutVolume);
    }

    if (FadingIn)
    {
        float FadeInVolume = TransitionProgress * GetCurrentVolume();
        FadingIn->SetVolumeMultiplier(FadeInVolume);
    }
}

void AAudio_EnvironmentalSoundManager::StartBiomeTransition(EAudio_BiomeType NewBiome)
{
    if (!BiomeAudioSettings.Contains(NewBiome))
    {
        return;
    }

    TargetBiome = NewBiome;
    bIsTransitioning = true;
    FadeTimer = 0.0f;

    const FAudio_BiomeAudioSettings& NewSettings = BiomeAudioSettings[NewBiome];
    FadeDuration = NewSettings.FadeInTime;

    // Start playing new biome sound on inactive component
    UAudioComponent* NewComponent = bPrimaryActive ? SecondaryAudioComponent : PrimaryAudioComponent;
    
    if (NewComponent && NewSettings.AmbientSoundCue)
    {
        NewComponent->SetSound(NewSettings.AmbientSoundCue);
        NewComponent->SetVolumeMultiplier(0.0f);
        NewComponent->Play();
    }
}

void AAudio_EnvironmentalSoundManager::CompleteBiomeTransition()
{
    bIsTransitioning = false;
    TransitionProgress = 1.0f;
    CurrentBiome = TargetBiome;

    // Stop old component and activate new one
    UAudioComponent* OldComponent = bPrimaryActive ? PrimaryAudioComponent : SecondaryAudioComponent;
    UAudioComponent* NewComponent = bPrimaryActive ? SecondaryAudioComponent : PrimaryAudioComponent;

    if (OldComponent)
    {
        OldComponent->Stop();
        OldComponent->SetVolumeMultiplier(0.0f);
    }

    if (NewComponent)
    {
        NewComponent->SetVolumeMultiplier(GetCurrentVolume());
    }

    // Swap active component
    bPrimaryActive = !bPrimaryActive;

    // Reset random sound timer
    RandomSoundTimer = 0.0f;
}

void AAudio_EnvironmentalSoundManager::UpdateRandomSounds(float DeltaTime)
{
    if (!BiomeAudioSettings.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];
    RandomSoundTimer += DeltaTime;

    if (RandomSoundTimer >= Settings.RandomSoundInterval)
    {
        PlayRandomBiomeSound();
        RandomSoundTimer = 0.0f;
        
        // Add some randomness to the interval
        RandomSoundTimer -= FMath::RandRange(0.0f, Settings.RandomSoundInterval * 0.3f);
    }
}