#include "Audio_EnvironmentalManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AAudio_EnvironmentalManager::AAudio_EnvironmentalManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create audio components
    PrimaryAmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAmbientAudio"));
    PrimaryAmbientAudio->SetupAttachment(RootComponent);
    PrimaryAmbientAudio->bAutoActivate = false;

    SecondaryAmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAmbientAudio"));
    SecondaryAmbientAudio->SetupAttachment(RootComponent);
    SecondaryAmbientAudio->bAutoActivate = false;

    EventAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EventAudio"));
    EventAudio->SetupAttachment(RootComponent);
    EventAudio->bAutoActivate = false;

    // Initialize settings
    CurrentEnvironment = FAudio_EnvironmentSettings();
    ActiveEnvironment = EAudio_EnvironmentType::Forest;
    CurrentTimeOfDay = EAudio_TimeOfDay::Morning;
    CurrentIntensity = 1.0f;
    bIsTransitioning = false;
    bInTransition = false;
    TransitionTimer = 0.0f;
    UpdateInterval = 2.0f;
    PlayerProximityRadius = 3000.0f;
}

void AAudio_EnvironmentalManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeTimeBasedSettings();
    
    // Start update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, 
                                             &AAudio_EnvironmentalManager::UpdateEnvironmentalAudio, 
                                             UpdateInterval, true);
        
        GetWorld()->GetTimerManager().SetTimer(ProximityTimerHandle, this,
                                             &AAudio_EnvironmentalManager::CheckPlayerProximity,
                                             1.0f, true);
    }

    SetupRandomEventTimer();
    UpdateAmbientLayers();

    UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalManager initialized with environment: %d"), 
           (int32)ActiveEnvironment);
}

void AAudio_EnvironmentalManager::InitializeTimeBasedSettings()
{
    // Initialize default time-based settings if empty
    if (TimeBasedSettings.Num() == 0)
    {
        for (int32 i = 0; i < 6; ++i)
        {
            FAudio_TimeBasedSettings TimeSettings;
            TimeSettings.TimeOfDay = (EAudio_TimeOfDay)i;
            
            switch (TimeSettings.TimeOfDay)
            {
                case EAudio_TimeOfDay::Dawn:
                    TimeSettings.VolumeModifier = 0.6f;
                    TimeSettings.PitchModifier = 0.9f;
                    break;
                case EAudio_TimeOfDay::Morning:
                    TimeSettings.VolumeModifier = 1.0f;
                    TimeSettings.PitchModifier = 1.0f;
                    break;
                case EAudio_TimeOfDay::Midday:
                    TimeSettings.VolumeModifier = 0.8f;
                    TimeSettings.PitchModifier = 1.1f;
                    break;
                case EAudio_TimeOfDay::Afternoon:
                    TimeSettings.VolumeModifier = 0.9f;
                    TimeSettings.PitchModifier = 1.0f;
                    break;
                case EAudio_TimeOfDay::Dusk:
                    TimeSettings.VolumeModifier = 0.7f;
                    TimeSettings.PitchModifier = 0.95f;
                    break;
                case EAudio_TimeOfDay::Night:
                    TimeSettings.VolumeModifier = 0.5f;
                    TimeSettings.PitchModifier = 0.85f;
                    break;
            }
            
            TimeBasedSettings.Add(TimeSettings);
        }
    }
}

void AAudio_EnvironmentalManager::SetEnvironmentType(EAudio_EnvironmentType NewEnvironment)
{
    if (NewEnvironment != ActiveEnvironment)
    {
        EAudio_EnvironmentType OldEnvironment = ActiveEnvironment;
        StartEnvironmentTransition(NewEnvironment);
        OnEnvironmentChanged(OldEnvironment, NewEnvironment);
        
        UE_LOG(LogTemp, Log, TEXT("Environment changed from %d to %d"), 
               (int32)OldEnvironment, (int32)NewEnvironment);
    }
}

void AAudio_EnvironmentalManager::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        EAudio_TimeOfDay OldTime = CurrentTimeOfDay;
        CurrentTimeOfDay = NewTimeOfDay;
        ApplyTimeBasedModifiers();
        OnTimeOfDayChanged(OldTime, NewTimeOfDay);
        
        UE_LOG(LogTemp, Log, TEXT("Time of day changed from %d to %d"), 
               (int32)OldTime, (int32)NewTimeOfDay);
    }
}

void AAudio_EnvironmentalManager::StartEnvironmentTransition(EAudio_EnvironmentType TargetEnv)
{
    TargetEnvironment = TargetEnv;
    bInTransition = true;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
}

void AAudio_EnvironmentalManager::SetEnvironmentIntensity(float NewIntensity)
{
    CurrentIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UpdateAmbientLayers();
}

void AAudio_EnvironmentalManager::TriggerEnvironmentEvent()
{
    if (EventAudio && CurrentEnvironment.RandomEvents.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CurrentEnvironment.RandomEvents.Num() - 1);
        USoundBase* EventSound = CurrentEnvironment.RandomEvents[RandomIndex];
        
        if (EventSound)
        {
            EventAudio->SetSound(EventSound);
            EventAudio->SetVolumeMultiplier(CurrentEnvironment.BaseVolume * CurrentIntensity);
            EventAudio->Play();
            
            UE_LOG(LogTemp, Log, TEXT("Triggered environment event: %s"), 
                   EventSound ? *EventSound->GetName() : TEXT("None"));
        }
    }
}

void AAudio_EnvironmentalManager::UpdateEnvironmentalAudio()
{
    ApplyTimeBasedModifiers();
    UpdateAmbientLayers();
    
    // Handle environment transitions
    if (bInTransition && CurrentEnvironment.TransitionTime > 0.0f)
    {
        TransitionTimer += UpdateInterval;
        float TransitionAlpha = FMath::Clamp(TransitionTimer / CurrentEnvironment.TransitionTime, 0.0f, 1.0f);
        
        if (TransitionAlpha >= 1.0f)
        {
            ActiveEnvironment = TargetEnvironment;
            bInTransition = false;
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
}

void AAudio_EnvironmentalManager::ApplyTimeBasedModifiers()
{
    for (const FAudio_TimeBasedSettings& TimeSettings : TimeBasedSettings)
    {
        if (TimeSettings.TimeOfDay == CurrentTimeOfDay)
        {
            float FinalVolume = CurrentEnvironment.BaseVolume * TimeSettings.VolumeModifier * CurrentIntensity;
            
            if (PrimaryAmbientAudio)
            {
                PrimaryAmbientAudio->SetVolumeMultiplier(FinalVolume);
                PrimaryAmbientAudio->SetPitchMultiplier(TimeSettings.PitchModifier);
            }
            
            if (SecondaryAmbientAudio)
            {
                SecondaryAmbientAudio->SetVolumeMultiplier(FinalVolume * 0.7f);
                SecondaryAmbientAudio->SetPitchMultiplier(TimeSettings.PitchModifier);
            }
            
            break;
        }
    }
}

void AAudio_EnvironmentalManager::UpdateAmbientLayers()
{
    if (CurrentEnvironment.AmbientSounds.Num() > 0)
    {
        // Primary ambient layer
        if (PrimaryAmbientAudio && CurrentEnvironment.AmbientSounds.IsValidIndex(0))
        {
            USoundBase* PrimarySound = CurrentEnvironment.AmbientSounds[0];
            if (PrimarySound && PrimaryAmbientAudio->GetSound() != PrimarySound)
            {
                PrimaryAmbientAudio->SetSound(PrimarySound);
                if (!PrimaryAmbientAudio->IsPlaying())
                {
                    PrimaryAmbientAudio->Play();
                }
            }
        }
        
        // Secondary ambient layer
        if (SecondaryAmbientAudio && CurrentEnvironment.AmbientSounds.IsValidIndex(1))
        {
            USoundBase* SecondarySound = CurrentEnvironment.AmbientSounds[1];
            if (SecondarySound && SecondaryAmbientAudio->GetSound() != SecondarySound)
            {
                SecondaryAmbientAudio->SetSound(SecondarySound);
                if (!SecondaryAmbientAudio->IsPlaying())
                {
                    SecondaryAmbientAudio->Play();
                }
            }
        }
    }
}

void AAudio_EnvironmentalManager::CheckPlayerProximity()
{
    float DistanceToPlayer = CalculateDistanceToPlayer();
    
    if (DistanceToPlayer <= PlayerProximityRadius)
    {
        // Player is close - full intensity
        SetEnvironmentIntensity(1.0f);
    }
    else if (DistanceToPlayer <= PlayerProximityRadius * 2.0f)
    {
        // Player is medium distance - reduced intensity
        float Alpha = (DistanceToPlayer - PlayerProximityRadius) / PlayerProximityRadius;
        SetEnvironmentIntensity(FMath::Lerp(1.0f, 0.3f, Alpha));
    }
    else
    {
        // Player is far - minimal intensity
        SetEnvironmentIntensity(0.1f);
    }
}

float AAudio_EnvironmentalManager::CalculateDistanceToPlayer()
{
    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            FVector ManagerLocation = GetActorLocation();
            return FVector::Dist(PlayerLocation, ManagerLocation);
        }
    }
    
    return 99999.0f; // Very far if no player found
}

void AAudio_EnvironmentalManager::SetupRandomEventTimer()
{
    if (GetWorld() && CurrentEnvironment.EventFrequency > 0.0f)
    {
        float RandomDelay = FMath::RandRange(CurrentEnvironment.EventFrequency * 0.5f, 
                                           CurrentEnvironment.EventFrequency * 1.5f);
        
        GetWorld()->GetTimerManager().SetTimer(EventTimerHandle, this,
                                             &AAudio_EnvironmentalManager::TriggerRandomEvent,
                                             RandomDelay, false);
    }
}

void AAudio_EnvironmentalManager::TriggerRandomEvent()
{
    TriggerEnvironmentEvent();
    SetupRandomEventTimer(); // Schedule next random event
}

void AAudio_EnvironmentalManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Additional per-frame updates if needed
    if (bInTransition)
    {
        // Smooth transition handling could go here
    }
}