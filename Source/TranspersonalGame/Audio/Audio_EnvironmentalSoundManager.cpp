#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.5f);

    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    WindAudioComponent->SetupAttachment(RootComponent);
    WindAudioComponent->bAutoActivate = false;
    WindAudioComponent->SetVolumeMultiplier(0.3f);

    CreatureAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CreatureAudio"));
    CreatureAudioComponent->SetupAttachment(RootComponent);
    CreatureAudioComponent->bAutoActivate = false;
    CreatureAudioComponent->SetVolumeMultiplier(0.4f);

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Savana;
    MasterVolume = 1.0f;
    WeatherIntensity = 0.0f;
    BiomeTransitionSpeed = 2.0f;
    PlayerLocation = FVector::ZeroVector;

    // Initialize biome audio data with default values
    FAudio_BiomeAudioData SavanaData;
    SavanaData.BaseVolume = 0.6f;
    SavanaData.FadeDistance = 8000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Savana, SavanaData);

    FAudio_BiomeAudioData PantanoData;
    PantanoData.BaseVolume = 0.7f;
    PantanoData.FadeDistance = 6000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Pantano, PantanoData);

    FAudio_BiomeAudioData FlorestaData;
    FlorestaData.BaseVolume = 0.8f;
    FlorestaData.FadeDistance = 5000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Floresta, FlorestaData);

    FAudio_BiomeAudioData DesertoData;
    DesertoData.BaseVolume = 0.4f;
    DesertoData.FadeDistance = 10000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Deserto, DesertoData);

    FAudio_BiomeAudioData MontanhaData;
    MontanhaData.BaseVolume = 0.5f;
    MontanhaData.FadeDistance = 7000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Montanha, MontanhaData);
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with current biome audio
    TransitionToBiomeAudio();
    
    // Set up periodic audio level updates
    GetWorldTimerManager().SetTimer(TransitionTimerHandle, this, 
        &AAudio_EnvironmentalSoundManager::UpdateAudioLevels, 0.1f, true);
}

void AAudio_EnvironmentalSoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(TransitionTimerHandle);
    Super::EndPlay(EndPlayReason);
}

void AAudio_EnvironmentalSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle biome transition
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime * BiomeTransitionSpeed;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
        }
        UpdateAudioLevels();
    }
}

void AAudio_EnvironmentalSoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        bIsTransitioning = true;
        TransitionProgress = 0.0f;
        TransitionToBiomeAudio();
    }
}

void AAudio_EnvironmentalSoundManager::UpdatePlayerLocation(FVector NewPlayerLocation)
{
    PlayerLocation = NewPlayerLocation;
    
    // Determine biome based on location (using coordinates from memory)
    EAudio_BiomeType DetectedBiome = EAudio_BiomeType::Savana; // Default
    
    if (PlayerLocation.X < -40000.0f && PlayerLocation.Y < -40000.0f)
    {
        DetectedBiome = EAudio_BiomeType::Pantano; // SW
    }
    else if (PlayerLocation.X < -40000.0f && PlayerLocation.Y > 35000.0f)
    {
        DetectedBiome = EAudio_BiomeType::Floresta; // NW
    }
    else if (PlayerLocation.X > 50000.0f)
    {
        DetectedBiome = EAudio_BiomeType::Deserto; // E
    }
    else if (PlayerLocation.X > 35000.0f && PlayerLocation.Y > 45000.0f)
    {
        DetectedBiome = EAudio_BiomeType::Montanha; // NE
    }
    
    SetCurrentBiome(DetectedBiome);
}

void AAudio_EnvironmentalSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioLevels();
}

void AAudio_EnvironmentalSoundManager::SetWeatherIntensity(float Intensity)
{
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateAudioLevels();
}

void AAudio_EnvironmentalSoundManager::TransitionToBiomeAudio()
{
    if (!BiomeAudioMap.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioData& BiomeData = BiomeAudioMap[CurrentBiome];
    
    // Load and set biome-specific sounds (placeholder implementation)
    // In a full implementation, these would load actual sound assets
    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
    
    if (WindAudioComponent && WeatherIntensity > 0.1f && !WindAudioComponent->IsPlaying())
    {
        WindAudioComponent->Play();
    }
    
    if (CreatureAudioComponent && !CreatureAudioComponent->IsPlaying())
    {
        CreatureAudioComponent->Play();
    }
}

void AAudio_EnvironmentalSoundManager::UpdateAudioLevels()
{
    if (!BiomeAudioMap.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioData& BiomeData = BiomeAudioMap[CurrentBiome];
    float BiomeVolume = BiomeData.BaseVolume;
    
    // Apply master volume and transition
    float FinalVolume = BiomeVolume * MasterVolume;
    if (bIsTransitioning)
    {
        FinalVolume *= TransitionProgress;
    }
    
    // Update component volumes
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
    }
    
    if (WindAudioComponent)
    {
        float WindVolume = FinalVolume * WeatherIntensity;
        WindAudioComponent->SetVolumeMultiplier(WindVolume);
    }
    
    if (CreatureAudioComponent)
    {
        float CreatureVolume = FinalVolume * 0.7f; // Slightly lower for ambience
        CreatureAudioComponent->SetVolumeMultiplier(CreatureVolume);
    }
}