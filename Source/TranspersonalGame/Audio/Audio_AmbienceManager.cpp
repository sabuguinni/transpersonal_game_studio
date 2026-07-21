#include "Audio_AmbienceManager.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AAudio_AmbienceManager::AAudio_AmbienceManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create audio components for layered ambience
    ForestAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ForestAudio"));
    ForestAudioComponent->SetupAttachment(RootComponent);
    ForestAudioComponent->bAutoActivate = false;

    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    WindAudioComponent->SetupAttachment(RootComponent);
    WindAudioComponent->bAutoActivate = false;

    WildlifeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WildlifeAudio"));
    WildlifeAudioComponent->SetupAttachment(RootComponent);
    WildlifeAudioComponent->bAutoActivate = false;

    // Initialize default settings
    CurrentAmbienceType = EAudio_AmbienceType::Forest;
    TargetAmbienceType = EAudio_AmbienceType::Forest;
    TransitionDuration = 5.0f;
    PlayerProximityRadius = 1500.0f;
    bEnableTimeOfDayVariation = true;
    DayVolumeMultiplier = 1.0f;
    NightVolumeMultiplier = 0.7f;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    CurrentMasterVolume = 1.0f;
    CachedPlayerPawn = nullptr;
}

void AAudio_AmbienceManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeAmbienceLayers();
    
    // Cache player pawn reference
    CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    
    // Start with forest ambience
    SetAmbienceType(CurrentAmbienceType);
}

void AAudio_AmbienceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdatePlayerProximity();
    
    if (bIsTransitioning)
    {
        ProcessAmbienceTransition(DeltaTime);
    }
}

void AAudio_AmbienceManager::SetAmbienceType(EAudio_AmbienceType NewType)
{
    if (CurrentAmbienceType == NewType && !bIsTransitioning)
    {
        return;
    }

    CurrentAmbienceType = NewType;
    TargetAmbienceType = NewType;
    bIsTransitioning = false;

    // Apply ambience settings immediately
    if (AmbienceLayers.Contains(NewType))
    {
        const FAudio_AmbienceLayer& Layer = AmbienceLayers[NewType];
        
        if (Layer.SoundAsset)
        {
            ForestAudioComponent->SetSound(Layer.SoundAsset);
            ForestAudioComponent->SetVolumeMultiplier(Layer.Volume * CurrentMasterVolume);
            ForestAudioComponent->Play();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_AmbienceManager: Set ambience to %d"), (int32)NewType);
}

void AAudio_AmbienceManager::FadeToAmbience(EAudio_AmbienceType NewType, float FadeDuration)
{
    if (CurrentAmbienceType == NewType)
    {
        return;
    }

    TargetAmbienceType = NewType;
    TransitionDuration = FadeDuration;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("Audio_AmbienceManager: Fading to ambience %d over %.1fs"), (int32)NewType, FadeDuration);
}

void AAudio_AmbienceManager::SetMasterVolume(float Volume)
{
    CurrentMasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    if (ForestAudioComponent && ForestAudioComponent->IsPlaying())
    {
        ForestAudioComponent->SetVolumeMultiplier(ForestAudioComponent->VolumeMultiplier * CurrentMasterVolume);
    }
    
    if (WindAudioComponent && WindAudioComponent->IsPlaying())
    {
        WindAudioComponent->SetVolumeMultiplier(WindAudioComponent->VolumeMultiplier * CurrentMasterVolume);
    }
    
    if (WildlifeAudioComponent && WildlifeAudioComponent->IsPlaying())
    {
        WildlifeAudioComponent->SetVolumeMultiplier(WildlifeAudioComponent->VolumeMultiplier * CurrentMasterVolume);
    }
}

void AAudio_AmbienceManager::UpdateTimeOfDayVolume(float TimeOfDay)
{
    if (!bEnableTimeOfDayVariation)
    {
        return;
    }

    // TimeOfDay expected to be 0.0 (midnight) to 1.0 (next midnight)
    // Day is roughly 0.25 to 0.75, night is 0.0-0.25 and 0.75-1.0
    float VolumeMultiplier = 1.0f;
    
    if (TimeOfDay >= 0.25f && TimeOfDay <= 0.75f)
    {
        // Day time
        VolumeMultiplier = DayVolumeMultiplier;
    }
    else
    {
        // Night time
        VolumeMultiplier = NightVolumeMultiplier;
    }

    SetMasterVolume(VolumeMultiplier);
}

void AAudio_AmbienceManager::PlayOneShot(USoundBase* Sound, FVector Location, float Volume)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, Volume * CurrentMasterVolume);
    }
}

void AAudio_AmbienceManager::UpdatePlayerProximity()
{
    if (!CachedPlayerPawn)
    {
        CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        return;
    }

    float Distance = FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
    float Attenuation = CalculateDistanceAttenuation(Distance);
    
    // Apply distance-based volume attenuation
    if (ForestAudioComponent && ForestAudioComponent->IsPlaying())
    {
        ForestAudioComponent->SetVolumeMultiplier(ForestAudioComponent->VolumeMultiplier * Attenuation);
    }
}

void AAudio_AmbienceManager::ProcessAmbienceTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        SetAmbienceType(TargetAmbienceType);
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        return;
    }

    // Fade out current ambience and fade in target
    float FadeOutVolume = 1.0f - TransitionProgress;
    float FadeInVolume = TransitionProgress;

    if (AmbienceLayers.Contains(CurrentAmbienceType))
    {
        const FAudio_AmbienceLayer& CurrentLayer = AmbienceLayers[CurrentAmbienceType];
        ForestAudioComponent->SetVolumeMultiplier(CurrentLayer.Volume * FadeOutVolume * CurrentMasterVolume);
    }

    if (AmbienceLayers.Contains(TargetAmbienceType))
    {
        const FAudio_AmbienceLayer& TargetLayer = AmbienceLayers[TargetAmbienceType];
        if (TargetLayer.SoundAsset && !WindAudioComponent->IsPlaying())
        {
            WindAudioComponent->SetSound(TargetLayer.SoundAsset);
            WindAudioComponent->Play();
        }
        WindAudioComponent->SetVolumeMultiplier(TargetLayer.Volume * FadeInVolume * CurrentMasterVolume);
    }
}

void AAudio_AmbienceManager::InitializeAmbienceLayers()
{
    // Initialize default ambience layers
    FAudio_AmbienceLayer ForestLayer;
    ForestLayer.Volume = 0.6f;
    ForestLayer.FadeInTime = 2.0f;
    ForestLayer.FadeOutTime = 3.0f;
    ForestLayer.bLooping = true;
    ForestLayer.MinDistance = 100.0f;
    ForestLayer.MaxDistance = 2000.0f;
    AmbienceLayers.Add(EAudio_AmbienceType::Forest, ForestLayer);

    FAudio_AmbienceLayer PlainsLayer;
    PlainsLayer.Volume = 0.5f;
    PlainsLayer.FadeInTime = 3.0f;
    PlainsLayer.FadeOutTime = 4.0f;
    PlainsLayer.bLooping = true;
    PlainsLayer.MinDistance = 200.0f;
    PlainsLayer.MaxDistance = 3000.0f;
    AmbienceLayers.Add(EAudio_AmbienceType::Plains, PlainsLayer);

    FAudio_AmbienceLayer CaveLayer;
    CaveLayer.Volume = 0.4f;
    CaveLayer.FadeInTime = 1.5f;
    CaveLayer.FadeOutTime = 2.0f;
    CaveLayer.bLooping = true;
    CaveLayer.MinDistance = 50.0f;
    CaveLayer.MaxDistance = 800.0f;
    AmbienceLayers.Add(EAudio_AmbienceType::Cave, CaveLayer);

    FAudio_AmbienceLayer RiverLayer;
    RiverLayer.Volume = 0.7f;
    RiverLayer.FadeInTime = 2.5f;
    RiverLayer.FadeOutTime = 3.5f;
    RiverLayer.bLooping = true;
    RiverLayer.MinDistance = 150.0f;
    RiverLayer.MaxDistance = 1500.0f;
    AmbienceLayers.Add(EAudio_AmbienceType::River, RiverLayer);

    FAudio_AmbienceLayer MountainLayer;
    MountainLayer.Volume = 0.3f;
    MountainLayer.FadeInTime = 4.0f;
    MountainLayer.FadeOutTime = 5.0f;
    MountainLayer.bLooping = true;
    MountainLayer.MinDistance = 300.0f;
    MountainLayer.MaxDistance = 4000.0f;
    AmbienceLayers.Add(EAudio_AmbienceType::Mountain, MountainLayer);

    FAudio_AmbienceLayer SwampLayer;
    SwampLayer.Volume = 0.8f;
    SwampLayer.FadeInTime = 2.0f;
    SwampLayer.FadeOutTime = 2.5f;
    SwampLayer.bLooping = true;
    SwampLayer.MinDistance = 100.0f;
    SwampLayer.MaxDistance = 1200.0f;
    AmbienceLayers.Add(EAudio_AmbienceType::Swamp, SwampLayer);

    UE_LOG(LogTemp, Log, TEXT("Audio_AmbienceManager: Initialized %d ambience layers"), AmbienceLayers.Num());
}

float AAudio_AmbienceManager::CalculateDistanceAttenuation(float Distance) const
{
    if (Distance <= PlayerProximityRadius)
    {
        return 1.0f;
    }

    float AttenuationDistance = Distance - PlayerProximityRadius;
    float MaxAttenuationDistance = PlayerProximityRadius * 2.0f;
    
    return FMath::Clamp(1.0f - (AttenuationDistance / MaxAttenuationDistance), 0.1f, 1.0f);
}