#include "Audio_TribalAmbientSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_TribalAmbientSystem::UAudio_TribalAmbientSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentActivity = EAudio_TribalActivity::Idle;
    CurrentMood = EAudio_TribalMood::Peaceful;
    TribalPopulation = 10;
    CurrentTimeOfDay = 0.5f; // Noon
    CurrentDangerLevel = 0.0f;
    
    MasterVolume = 1.0f;
    ActivityTransitionTime = 3.0f;
    PopulationVolumeMultiplier = 0.1f;
    MaxAudibleDistance = 2000.0f;
    
    ActivityTransitionTimer = 0.0f;
    TargetActivity = EAudio_TribalActivity::Idle;
    bIsTransitioning = false;

    // Create audio components
    BaseAmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BaseAmbienceComponent"));
    ActivityComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ActivityComponent"));
    CampfireComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CampfireComponent"));
    ConversationComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ConversationComponent"));
    WeatherComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherComponent"));

    if (BaseAmbienceComponent)
    {
        BaseAmbienceComponent->bAutoActivate = true;
        BaseAmbienceComponent->VolumeMultiplier = 0.6f;
    }

    if (ActivityComponent)
    {
        ActivityComponent->bAutoActivate = false;
        ActivityComponent->VolumeMultiplier = 0.8f;
    }

    if (CampfireComponent)
    {
        CampfireComponent->bAutoActivate = true;
        CampfireComponent->VolumeMultiplier = 0.4f;
    }

    if (ConversationComponent)
    {
        ConversationComponent->bAutoActivate = false;
        ConversationComponent->VolumeMultiplier = 0.3f;
    }

    if (WeatherComponent)
    {
        WeatherComponent->bAutoActivate = false;
        WeatherComponent->VolumeMultiplier = 0.5f;
    }
}

void UAudio_TribalAmbientSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize audio layers
    UpdateAudioLayers();
    
    // Start campfire sound if available
    if (CampfireComponent && CampfireCracklingSound)
    {
        CampfireComponent->SetSound(CampfireCracklingSound);
        CampfireComponent->Play();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Ambient System: Initialized with %d population"), TribalPopulation);
}

void UAudio_TribalAmbientSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle activity transitions
    if (bIsTransitioning)
    {
        ActivityTransitionTimer += DeltaTime;
        
        if (ActivityTransitionTimer >= ActivityTransitionTime)
        {
            CurrentActivity = TargetActivity;
            bIsTransitioning = false;
            ActivityTransitionTimer = 0.0f;
            
            UpdateAudioLayers();
            UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Transition completed to activity %d"), (int32)CurrentActivity);
        }
        else
        {
            // Smooth transition between activities
            float TransitionProgress = ActivityTransitionTimer / ActivityTransitionTime;
            UpdateVolumeBasedOnPopulation();
        }
    }
    
    // Update time-based effects
    UpdateTimeOfDayEffects();
}

void UAudio_TribalAmbientSystem::SetTribalActivity(EAudio_TribalActivity NewActivity)
{
    if (NewActivity != CurrentActivity && !bIsTransitioning)
    {
        TargetActivity = NewActivity;
        bIsTransitioning = true;
        ActivityTransitionTimer = 0.0f;
        
        TransitionToActivity(NewActivity);
        UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Starting transition to activity %d"), (int32)NewActivity);
    }
}

void UAudio_TribalAmbientSystem::SetTribalMood(EAudio_TribalMood NewMood)
{
    if (NewMood != CurrentMood)
    {
        CurrentMood = NewMood;
        TransitionToMood(NewMood);
        UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Mood changed to %d"), (int32)NewMood);
    }
}

void UAudio_TribalAmbientSystem::SetTribalPopulation(int32 PopulationCount)
{
    TribalPopulation = FMath::Clamp(PopulationCount, 1, 100);
    UpdateVolumeBasedOnPopulation();
    UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Population updated to %d"), TribalPopulation);
}

void UAudio_TribalAmbientSystem::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    UpdateTimeOfDayEffects();
}

void UAudio_TribalAmbientSystem::TriggerCraftingSound(FVector Location)
{
    if (ActivityComponent)
    {
        // Play crafting sound at specific location
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ActivitySounds.Contains(EAudio_TribalActivity::Crafting) ? 
                ActivitySounds[EAudio_TribalActivity::Crafting].SoundCue : nullptr,
            Location,
            0.8f
        );
        
        UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Crafting sound triggered at location"));
    }
}

void UAudio_TribalAmbientSystem::TriggerStorytellingSession()
{
    SetTribalActivity(EAudio_TribalActivity::Storytelling);
    
    if (ConversationComponent && DistantConversationSound)
    {
        ConversationComponent->SetSound(DistantConversationSound);
        ConversationComponent->FadeIn(2.0f, 0.5f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Storytelling session started"));
}

void UAudio_TribalAmbientSystem::TriggerHuntingPreparation()
{
    SetTribalActivity(EAudio_TribalActivity::Hunting);
    SetTribalMood(EAudio_TribalMood::Tense);
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Hunting preparation triggered"));
}

void UAudio_TribalAmbientSystem::TriggerCelebrationEvent()
{
    SetTribalActivity(EAudio_TribalActivity::Celebration);
    SetTribalMood(EAudio_TribalMood::Joyful);
    
    if (ActivityComponent && TribalMusicSound)
    {
        ActivityComponent->SetSound(TribalMusicSound);
        ActivityComponent->FadeIn(1.0f, 0.9f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Celebration event triggered"));
}

void UAudio_TribalAmbientSystem::UpdateWeatherConditions(bool bIsRaining, bool bIsStormy, float WindStrength)
{
    if (WeatherComponent)
    {
        if (bIsRaining && RainOnShelterSound)
        {
            WeatherComponent->SetSound(RainOnShelterSound);
            WeatherComponent->SetVolumeMultiplier(bIsStormy ? 0.8f : 0.4f);
            WeatherComponent->Play();
        }
        else if (WindStrength > 0.3f && WindThroughCampSound)
        {
            WeatherComponent->SetSound(WindThroughCampSound);
            WeatherComponent->SetVolumeMultiplier(WindStrength * 0.6f);
            WeatherComponent->Play();
        }
        else
        {
            WeatherComponent->FadeOut(2.0f, 0.0f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Weather updated - Rain: %s, Storm: %s, Wind: %.2f"), 
           bIsRaining ? TEXT("Yes") : TEXT("No"),
           bIsStormy ? TEXT("Yes") : TEXT("No"),
           WindStrength);
}

void UAudio_TribalAmbientSystem::UpdateDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    
    // Adjust tribal mood based on danger
    if (CurrentDangerLevel > 0.7f)
    {
        SetTribalMood(EAudio_TribalMood::Fearful);
    }
    else if (CurrentDangerLevel > 0.4f)
    {
        SetTribalMood(EAudio_TribalMood::Tense);
    }
    else if (CurrentDangerLevel < 0.1f)
    {
        SetTribalMood(EAudio_TribalMood::Peaceful);
    }
    
    // Reduce conversation volume when danger is high
    if (ConversationComponent)
    {
        float SafetyVolume = 1.0f - (CurrentDangerLevel * 0.8f);
        ConversationComponent->SetVolumeMultiplier(SafetyVolume * 0.3f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Audio: Danger level updated to %.2f"), CurrentDangerLevel);
}

void UAudio_TribalAmbientSystem::UpdateAudioLayers()
{
    // Update base ambience based on current state
    if (BaseAmbienceComponent)
    {
        float BaseVolume = MasterVolume * 0.6f;
        BaseAmbienceComponent->SetVolumeMultiplier(BaseVolume);
    }
    
    // Update activity-specific audio
    if (ActivityComponent && ActivitySounds.Contains(CurrentActivity))
    {
        const FAudio_TribalSoundLayer& ActivityLayer = ActivitySounds[CurrentActivity];
        if (ActivityLayer.SoundCue)
        {
            ActivityComponent->SetSound(ActivityLayer.SoundCue);
            ActivityComponent->SetVolumeMultiplier(ActivityLayer.BaseVolume * MasterVolume);
            
            if (!ActivityComponent->IsPlaying())
            {
                ActivityComponent->FadeIn(ActivityLayer.FadeInTime, ActivityLayer.BaseVolume);
            }
        }
    }
    
    UpdateVolumeBasedOnPopulation();
}

void UAudio_TribalAmbientSystem::TransitionToActivity(EAudio_TribalActivity NewActivity)
{
    // Fade out current activity sound
    if (ActivityComponent && ActivityComponent->IsPlaying())
    {
        ActivityComponent->FadeOut(1.0f, 0.0f);
    }
    
    // Prepare new activity sound
    if (ActivitySounds.Contains(NewActivity))
    {
        const FAudio_TribalSoundLayer& NewLayer = ActivitySounds[NewActivity];
        if (NewLayer.SoundCue && ActivityComponent)
        {
            // Set up new sound but don't play yet - will be handled in UpdateAudioLayers
            ActivityComponent->SetSound(NewLayer.SoundCue);
        }
    }
}

void UAudio_TribalAmbientSystem::TransitionToMood(EAudio_TribalMood NewMood)
{
    // Mood affects the overall tone and volume of tribal sounds
    float MoodVolumeModifier = 1.0f;
    
    switch (NewMood)
    {
        case EAudio_TribalMood::Peaceful:
            MoodVolumeModifier = 1.0f;
            break;
        case EAudio_TribalMood::Joyful:
            MoodVolumeModifier = 1.2f;
            break;
        case EAudio_TribalMood::Tense:
            MoodVolumeModifier = 0.7f;
            break;
        case EAudio_TribalMood::Fearful:
            MoodVolumeModifier = 0.4f;
            break;
        case EAudio_TribalMood::Solemn:
            MoodVolumeModifier = 0.6f;
            break;
        case EAudio_TribalMood::Excited:
            MoodVolumeModifier = 1.1f;
            break;
    }
    
    // Apply mood modifier to conversation and activity components
    if (ConversationComponent)
    {
        ConversationComponent->SetVolumeMultiplier(0.3f * MoodVolumeModifier);
    }
    
    if (ActivityComponent)
    {
        float CurrentVolume = ActivityComponent->VolumeMultiplier;
        ActivityComponent->SetVolumeMultiplier(CurrentVolume * MoodVolumeModifier);
    }
}

void UAudio_TribalAmbientSystem::UpdateVolumeBasedOnPopulation()
{
    // More people = more ambient conversation and activity sounds
    float PopulationFactor = FMath::Clamp(TribalPopulation * PopulationVolumeMultiplier, 0.2f, 2.0f);
    
    if (ConversationComponent)
    {
        ConversationComponent->SetVolumeMultiplier(0.3f * PopulationFactor);
    }
    
    if (ActivityComponent)
    {
        float BaseActivityVolume = 0.8f;
        ActivityComponent->SetVolumeMultiplier(BaseActivityVolume * PopulationFactor);
    }
}

void UAudio_TribalAmbientSystem::UpdateTimeOfDayEffects()
{
    // Night time reduces activity and conversation volume
    float TimeBasedVolume = 1.0f;
    
    if (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f) // Night time
    {
        TimeBasedVolume = 0.4f;
        
        // Increase campfire prominence at night
        if (CampfireComponent)
        {
            CampfireComponent->SetVolumeMultiplier(0.6f);
        }
    }
    else // Day time
    {
        TimeBasedVolume = 1.0f;
        
        if (CampfireComponent)
        {
            CampfireComponent->SetVolumeMultiplier(0.4f);
        }
    }
    
    // Apply time-based volume to conversation and activity
    if (ConversationComponent)
    {
        float CurrentConversationVolume = ConversationComponent->VolumeMultiplier;
        ConversationComponent->SetVolumeMultiplier(CurrentConversationVolume * TimeBasedVolume);
    }
    
    if (ActivityComponent)
    {
        float CurrentActivityVolume = ActivityComponent->VolumeMultiplier;
        ActivityComponent->SetVolumeMultiplier(CurrentActivityVolume * TimeBasedVolume);
    }
}

void UAudio_TribalAmbientSystem::UpdateWeatherEffects()
{
    // Weather effects are handled in UpdateWeatherConditions
    // This function can be used for additional weather-based audio processing
}