// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "AdaptiveMusicManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

AAdaptiveMusicManager::AAdaptiveMusicManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for music updates

    // Initialize default emotional state
    CurrentEmotionalState = EEmotionalState::Calm;
    TargetEmotionalState = EEmotionalState::Calm;
    EmotionalTransitionSpeed = 1.0f;
    EmotionalIntensity = 0.5f;

    // Initialize environmental awareness
    CurrentEnvironmentalZone = EEnvironmentalZone::Forest;
    CurrentTimeOfDay = ETimeOfDay::Morning;
    WeatherIntensity = 0.0f;

    // Initialize creature proximity
    NearestPredatorDistance = 10000.0f;
    NearestCreatureDistance = 10000.0f;
    VisibleCreatureCount = 0;

    // Initialize music settings
    MasterVolume = 0.7f;
    bMusicEnabled = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AAdaptiveMusicManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveMusicManager: Beginning play - initializing emotional audio journey"));

    // Initialize audio component pool
    AudioComponentPool.Reserve(MaxAudioComponents);
    for (int32 i = 0; i < MaxAudioComponents; i++)
    {
        UAudioComponent* AudioComp = CreateDefaultSubobject<UAudioComponent>(*FString::Printf(TEXT("AudioLayer_%d"), i));
        if (AudioComp)
        {
            AudioComp->SetupAttachment(RootComponent);
            AudioComp->bAutoActivate = false;
            AudioComponentPool.Add(AudioComp);
        }
    }

    // Start context analysis timer
    GetWorld()->GetTimerManager().SetTimer(
        ContextAnalysisTimer,
        this,
        &AAdaptiveMusicManager::CalculateEmotionalStateFromContext,
        2.0f, // Every 2 seconds
        true
    );

    // Start music update timer
    GetWorld()->GetTimerManager().SetTimer(
        MusicUpdateTimer,
        this,
        &AAdaptiveMusicManager::UpdateMusicLayers,
        0.5f, // Every 0.5 seconds
        true
    );

    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Audio system initialized with %d audio layers"), MaxAudioComponents);
}

void AAdaptiveMusicManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth transition between emotional states
    if (CurrentEmotionalState != TargetEmotionalState)
    {
        TransitionToTargetState();
    }

    // Update layer volumes based on current state
    UpdateLayerVolumes();
}

void AAdaptiveMusicManager::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (NewState == TargetEmotionalState)
    {
        return; // Already transitioning to this state
    }

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveMusicManager: Emotional transition from %d to %d over %.1f seconds"), 
           (int32)CurrentEmotionalState, (int32)NewState, TransitionTime);

    TargetEmotionalState = NewState;
    EmotionalTransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
    CurrentTransitionProgress = 0.0f;

    // Trigger immediate music layer update
    UpdateMusicLayers();
}

void AAdaptiveMusicManager::ModifyEmotionalIntensity(float IntensityDelta, float TransitionTime)
{
    float NewIntensity = FMath::Clamp(EmotionalIntensity + IntensityDelta, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Emotional intensity changing from %.2f to %.2f"), 
           EmotionalIntensity, NewIntensity);

    EmotionalIntensity = NewIntensity;
    
    // Update music layers to reflect new intensity
    UpdateMusicLayers();
}

void AAdaptiveMusicManager::TriggerEmotionalMoment(EEmotionalState MomentaryState, float Duration)
{
    if (bIsInEmotionalMoment)
    {
        // Cancel previous emotional moment
        GetWorld()->GetTimerManager().ClearTimer(EmotionalMomentTimer);
    }

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveMusicManager: Triggering emotional moment - %d for %.1f seconds"), 
           (int32)MomentaryState, Duration);

    bIsInEmotionalMoment = true;
    PreMomentState = TargetEmotionalState;
    
    SetEmotionalState(MomentaryState, 1.0f); // Quick transition to momentary state

    // Set timer to return to previous state
    GetWorld()->GetTimerManager().SetTimer(
        EmotionalMomentTimer,
        [this]()
        {
            bIsInEmotionalMoment = false;
            SetEmotionalState(PreMomentState, 3.0f); // Slower return transition
        },
        Duration,
        false
    );
}

void AAdaptiveMusicManager::UpdateEnvironmentalZone(EEnvironmentalZone NewZone)
{
    if (CurrentEnvironmentalZone != NewZone)
    {
        UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Environmental zone changed from %d to %d"), 
               (int32)CurrentEnvironmentalZone, (int32)NewZone);

        CurrentEnvironmentalZone = NewZone;
        CalculateEmotionalStateFromContext();
    }
}

void AAdaptiveMusicManager::UpdateTimeOfDay(ETimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Time of day changed from %d to %d"), 
               (int32)CurrentTimeOfDay, (int32)NewTimeOfDay);

        CurrentTimeOfDay = NewTimeOfDay;
        CalculateEmotionalStateFromContext();
    }
}

void AAdaptiveMusicManager::UpdateWeatherIntensity(float NewWeatherIntensity)
{
    WeatherIntensity = FMath::Clamp(NewWeatherIntensity, 0.0f, 1.0f);
    
    if (WeatherIntensity > 0.7f)
    {
        // Intense weather can trigger tension or fear
        ModifyEmotionalIntensity(0.3f, 2.0f);
    }
}

void AAdaptiveMusicManager::UpdateCreatureProximity(float PredatorDistance, float CreatureDistance, int32 CreatureCount)
{
    NearestPredatorDistance = PredatorDistance;
    NearestCreatureDistance = CreatureDistance;
    VisibleCreatureCount = CreatureCount;

    // Trigger immediate emotional response if predator is very close
    if (PredatorDistance < 500.0f && CurrentEmotionalState != EEmotionalState::Fear)
    {
        TriggerEmotionalMoment(EEmotionalState::Fear, 15.0f);
    }
    else if (PredatorDistance < 1000.0f && CurrentEmotionalState == EEmotionalState::Calm)
    {
        SetEmotionalState(EEmotionalState::Tension, 2.0f);
    }
}

void AAdaptiveMusicManager::OnPredatorSpotted(float Distance)
{
    UE_LOG(LogTemp, Warning, TEXT("AdaptiveMusicManager: Predator spotted at distance %.1f - triggering fear response"), Distance);

    if (Distance < 300.0f)
    {
        TriggerEmotionalMoment(EEmotionalState::Fear, 20.0f);
    }
    else if (Distance < 800.0f)
    {
        SetEmotionalState(EEmotionalState::Tension, 1.5f);
    }
}

void AAdaptiveMusicManager::OnPredatorLost()
{
    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Predator lost - returning to calmer state"));

    if (CurrentEmotionalState == EEmotionalState::Fear || CurrentEmotionalState == EEmotionalState::Tension)
    {
        SetEmotionalState(EEmotionalState::Calm, 5.0f);
    }
}

void AAdaptiveMusicManager::SetMusicEnabled(bool bEnabled)
{
    bMusicEnabled = bEnabled;
    
    if (!bEnabled)
    {
        FadeToSilence(3.0f);
    }
    else
    {
        FadeFromSilence(3.0f);
    }
}

void AAdaptiveMusicManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateLayerVolumes();
}

void AAdaptiveMusicManager::FadeToSilence(float FadeTime)
{
    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Fading to silence over %.1f seconds"), FadeTime);

    for (UAudioComponent* AudioComp : ActiveAudioLayers)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->FadeOut(FadeTime, 0.0f);
        }
    }
}

void AAdaptiveMusicManager::FadeFromSilence(float FadeTime)
{
    UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Fading from silence over %.1f seconds"), FadeTime);

    UpdateMusicLayers(); // Restart appropriate layers
}

void AAdaptiveMusicManager::UpdateMusicLayers()
{
    if (!bMusicEnabled)
    {
        return;
    }

    // Get current music configuration
    FAdaptiveMusicConfig* CurrentConfig = MusicConfigurations.Find(CurrentEmotionalState);
    if (!CurrentConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveMusicManager: No music configuration found for emotional state %d"), (int32)CurrentEmotionalState);
        return;
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("AdaptiveMusicManager: Updating music layers for emotional state %d with %d layers"), 
           (int32)CurrentEmotionalState, CurrentConfig->MusicLayers.Num());

    // Update existing layers based on current emotional state
    // This would be expanded with actual sound asset references
    UpdateLayerVolumes();
}

void AAdaptiveMusicManager::TransitionToTargetState()
{
    CurrentTransitionProgress += EmotionalTransitionSpeed * GetWorld()->GetDeltaSeconds();
    
    if (CurrentTransitionProgress >= 1.0f)
    {
        CurrentEmotionalState = TargetEmotionalState;
        CurrentTransitionProgress = 1.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AdaptiveMusicManager: Emotional transition complete - now in state %d"), (int32)CurrentEmotionalState);
    }
}

void AAdaptiveMusicManager::CalculateEmotionalStateFromContext()
{
    if (bIsInEmotionalMoment)
    {
        return; // Don't override emotional moments
    }

    // Calculate optimal emotional state based on current context
    EEmotionalState OptimalState = DetermineOptimalEmotionalState();
    
    if (OptimalState != TargetEmotionalState)
    {
        SetEmotionalState(OptimalState, 4.0f);
    }
}

void AAdaptiveMusicManager::UpdateLayerVolumes()
{
    float EffectiveVolume = MasterVolume * (bMusicEnabled ? 1.0f : 0.0f);
    
    for (UAudioComponent* AudioComp : ActiveAudioLayers)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            // Apply emotional intensity and environmental modifiers
            float LayerVolume = EffectiveVolume * EmotionalIntensity;
            
            // Modify volume based on time of day
            switch (CurrentTimeOfDay)
            {
                case ETimeOfDay::Night:
                case ETimeOfDay::DeepNight:
                    LayerVolume *= 0.7f; // Quieter at night
                    break;
                case ETimeOfDay::Dawn:
                case ETimeOfDay::Dusk:
                    LayerVolume *= 0.85f; // Slightly quieter during transitions
                    break;
                default:
                    break;
            }
            
            AudioComp->SetVolumeMultiplier(LayerVolume);
        }
    }
}

float AAdaptiveMusicManager::CalculateTensionFromProximity() const
{
    float TensionLevel = 0.0f;
    
    // Predator proximity creates immediate tension
    if (NearestPredatorDistance < 2000.0f)
    {
        TensionLevel = FMath::Clamp(1.0f - (NearestPredatorDistance / 2000.0f), 0.0f, 1.0f);
    }
    
    // Multiple creatures increase tension
    if (VisibleCreatureCount > 3)
    {
        TensionLevel += 0.2f * (VisibleCreatureCount - 3);
    }
    
    return FMath::Clamp(TensionLevel, 0.0f, 1.0f);
}

float AAdaptiveMusicManager::CalculateWonderFromEnvironment() const
{
    float WonderLevel = 0.3f; // Base wonder level
    
    // Certain environments inspire more wonder
    switch (CurrentEnvironmentalZone)
    {
        case EEnvironmentalZone::Mountains:
        case EEnvironmentalZone::Coastline:
            WonderLevel += 0.3f;
            break;
        case EEnvironmentalZone::Forest:
        case EEnvironmentalZone::Valley:
            WonderLevel += 0.2f;
            break;
        default:
            break;
    }
    
    // Dawn and dusk increase wonder
    if (CurrentTimeOfDay == ETimeOfDay::Dawn || CurrentTimeOfDay == ETimeOfDay::Dusk)
    {
        WonderLevel += 0.2f;
    }
    
    return FMath::Clamp(WonderLevel, 0.0f, 1.0f);
}

float AAdaptiveMusicManager::CalculateFearFromThreats() const
{
    if (NearestPredatorDistance < 500.0f)
    {
        return 1.0f; // Maximum fear
    }
    
    if (NearestPredatorDistance < 1000.0f)
    {
        return 0.7f; // High fear
    }
    
    // Night time increases base fear level
    if (CurrentTimeOfDay == ETimeOfDay::Night || CurrentTimeOfDay == ETimeOfDay::DeepNight)
    {
        return 0.3f;
    }
    
    return 0.0f;
}

EEmotionalState AAdaptiveMusicManager::DetermineOptimalEmotionalState() const
{
    float TensionLevel = CalculateTensionFromProximity();
    float WonderLevel = CalculateWonderFromEnvironment();
    float FearLevel = CalculateFearFromThreats();
    
    // Fear takes priority
    if (FearLevel > 0.6f)
    {
        return EEmotionalState::Fear;
    }
    
    // Tension is next priority
    if (TensionLevel > 0.4f)
    {
        return EEmotionalState::Tension;
    }
    
    // Wonder during safe moments
    if (WonderLevel > 0.6f && TensionLevel < 0.2f)
    {
        return EEmotionalState::Wonder;
    }
    
    // Default to calm
    return EEmotionalState::Calm;
}