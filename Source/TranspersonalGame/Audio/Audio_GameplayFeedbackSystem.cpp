#include "Audio_GameplayFeedbackSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AAudio_GameplayFeedbackSystem::AAudio_GameplayFeedbackSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    
    GlobalVolume = 1.0f;
    MaxSimultaneousSounds = 5.0f;
    
    // Set as root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AAudio_GameplayFeedbackSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    InitializeDefaultFeedbackEvents();
}

void AAudio_GameplayFeedbackSystem::InitializeAudioComponents()
{
    // Create audio components for simultaneous sound playback
    for (int32 i = 0; i < MaxSimultaneousSounds; i++)
    {
        FString ComponentName = FString::Printf(TEXT("AudioComponent_%d"), i);
        UAudioComponent* AudioComp = CreateDefaultSubobject<UAudioComponent>(*ComponentName);
        
        if (AudioComp)
        {
            AudioComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
            AudioComp->bAutoActivate = false;
            AudioComp->SetVolumeMultiplier(GlobalVolume);
            AudioComponents.Add(AudioComp);
        }
    }
}

void AAudio_GameplayFeedbackSystem::InitializeDefaultFeedbackEvents()
{
    FeedbackEvents.Empty();
    
    // Player Damaged feedback
    FAudio_FeedbackData DamageData;
    DamageData.EventType = EAudio_GameplayEvent::PlayerDamaged;
    DamageData.Volume = 0.8f;
    DamageData.Pitch = 1.0f;
    DamageData.bShouldLoop = false;
    DamageData.FadeInTime = 0.1f;
    DamageData.FadeOutTime = 0.3f;
    FeedbackEvents.Add(DamageData);
    
    // Dinosaur Nearby feedback
    FAudio_FeedbackData DinosaurData;
    DinosaurData.EventType = EAudio_GameplayEvent::DinosaurNearby;
    DinosaurData.Volume = 0.7f;
    DinosaurData.Pitch = 0.9f;
    DinosaurData.bShouldLoop = true;
    DinosaurData.FadeInTime = 1.0f;
    DinosaurData.FadeOutTime = 2.0f;
    FeedbackEvents.Add(DinosaurData);
    
    // Item Crafted feedback
    FAudio_FeedbackData CraftedData;
    CraftedData.EventType = EAudio_GameplayEvent::ItemCrafted;
    CraftedData.Volume = 0.6f;
    CraftedData.Pitch = 1.2f;
    CraftedData.bShouldLoop = false;
    CraftedData.FadeInTime = 0.2f;
    CraftedData.FadeOutTime = 0.5f;
    FeedbackEvents.Add(CraftedData);
    
    // Food Consumed feedback
    FAudio_FeedbackData FoodData;
    FoodData.EventType = EAudio_GameplayEvent::FoodConsumed;
    FoodData.Volume = 0.5f;
    FoodData.Pitch = 1.1f;
    FoodData.bShouldLoop = false;
    FoodData.FadeInTime = 0.3f;
    FoodData.FadeOutTime = 0.4f;
    FeedbackEvents.Add(FoodData);
    
    // Shelter Entered feedback
    FAudio_FeedbackData ShelterData;
    ShelterData.EventType = EAudio_GameplayEvent::ShelterEntered;
    ShelterData.Volume = 0.4f;
    ShelterData.Pitch = 0.8f;
    ShelterData.bShouldLoop = false;
    ShelterData.FadeInTime = 1.5f;
    ShelterData.FadeOutTime = 1.0f;
    FeedbackEvents.Add(ShelterData);
    
    // Danger Zone feedback
    FAudio_FeedbackData DangerData;
    DangerData.EventType = EAudio_GameplayEvent::DangerZoneEntered;
    DangerData.Volume = 0.9f;
    DangerData.Pitch = 0.7f;
    DangerData.bShouldLoop = true;
    DangerData.FadeInTime = 2.0f;
    DangerData.FadeOutTime = 3.0f;
    FeedbackEvents.Add(DangerData);
    
    // Safe Zone feedback
    FAudio_FeedbackData SafeData;
    SafeData.EventType = EAudio_GameplayEvent::SafeZoneEntered;
    SafeData.Volume = 0.6f;
    SafeData.Pitch = 1.3f;
    SafeData.bShouldLoop = false;
    SafeData.FadeInTime = 1.0f;
    SafeData.FadeOutTime = 2.0f;
    FeedbackEvents.Add(SafeData);
    
    // Night Falling feedback
    FAudio_FeedbackData NightData;
    NightData.EventType = EAudio_GameplayEvent::NightFalling;
    NightData.Volume = 0.7f;
    NightData.Pitch = 0.6f;
    NightData.bShouldLoop = false;
    NightData.FadeInTime = 3.0f;
    NightData.FadeOutTime = 1.0f;
    FeedbackEvents.Add(NightData);
    
    // Day Breaking feedback
    FAudio_FeedbackData DayData;
    DayData.EventType = EAudio_GameplayEvent::DayBreaking;
    DayData.Volume = 0.8f;
    DayData.Pitch = 1.4f;
    DayData.bShouldLoop = false;
    DayData.FadeInTime = 2.0f;
    DayData.FadeOutTime = 1.5f;
    FeedbackEvents.Add(DayData);
}

void AAudio_GameplayFeedbackSystem::TriggerGameplayFeedback(EAudio_GameplayEvent EventType)
{
    // Check if event is already active and shouldn't repeat
    if (IsEventActive(EventType))
    {
        FAudio_FeedbackData* FeedbackData = GetFeedbackData(EventType);
        if (FeedbackData && !FeedbackData->bShouldLoop)
        {
            return; // Don't retrigger non-looping sounds
        }
    }
    
    FAudio_FeedbackData* FeedbackData = GetFeedbackData(EventType);
    if (!FeedbackData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No feedback data found for event type"));
        return;
    }
    
    UAudioComponent* AudioComp = GetAvailableAudioComponent();
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available audio component for feedback"));
        return;
    }
    
    PlayFeedbackSound(*FeedbackData, AudioComp);
    
    // Add to active events if not already present
    if (!ActiveEvents.Contains(EventType))
    {
        ActiveEvents.Add(EventType);
    }
}

void AAudio_GameplayFeedbackSystem::StopGameplayFeedback(EAudio_GameplayEvent EventType)
{
    // Find and stop audio components playing this event type
    for (UAudioComponent* AudioComp : AudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            FAudio_FeedbackData* FeedbackData = GetFeedbackData(EventType);
            if (FeedbackData)
            {
                AudioComp->FadeOut(FeedbackData->FadeOutTime, 0.0f);
            }
            else
            {
                AudioComp->Stop();
            }
        }
    }
    
    // Remove from active events
    ActiveEvents.Remove(EventType);
}

void AAudio_GameplayFeedbackSystem::StopAllFeedback()
{
    for (UAudioComponent* AudioComp : AudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->FadeOut(0.5f, 0.0f);
        }
    }
    
    ActiveEvents.Empty();
}

void AAudio_GameplayFeedbackSystem::SetGlobalVolume(float NewVolume)
{
    GlobalVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    for (UAudioComponent* AudioComp : AudioComponents)
    {
        if (AudioComp)
        {
            AudioComp->SetVolumeMultiplier(GlobalVolume);
        }
    }
}

bool AAudio_GameplayFeedbackSystem::IsEventActive(EAudio_GameplayEvent EventType) const
{
    return ActiveEvents.Contains(EventType);
}

UAudioComponent* AAudio_GameplayFeedbackSystem::GetAvailableAudioComponent()
{
    // Find an audio component that's not currently playing
    for (UAudioComponent* AudioComp : AudioComponents)
    {
        if (AudioComp && !AudioComp->IsPlaying())
        {
            return AudioComp;
        }
    }
    
    // If all are busy, return the first one (will interrupt)
    return AudioComponents.Num() > 0 ? AudioComponents[0] : nullptr;
}

FAudio_FeedbackData* AAudio_GameplayFeedbackSystem::GetFeedbackData(EAudio_GameplayEvent EventType)
{
    for (FAudio_FeedbackData& Data : FeedbackEvents)
    {
        if (Data.EventType == EventType)
        {
            return &Data;
        }
    }
    return nullptr;
}

void AAudio_GameplayFeedbackSystem::PlayFeedbackSound(const FAudio_FeedbackData& FeedbackData, UAudioComponent* AudioComp)
{
    if (!AudioComp)
    {
        return;
    }
    
    // Load and set the sound cue
    if (!FeedbackData.FeedbackSound.IsNull())
    {
        USoundCue* SoundCue = FeedbackData.FeedbackSound.LoadSynchronous();
        if (SoundCue)
        {
            AudioComp->SetSound(SoundCue);
            AudioComp->SetVolumeMultiplier(FeedbackData.Volume * GlobalVolume);
            AudioComp->SetPitchMultiplier(FeedbackData.Pitch);
            
            if (FeedbackData.bShouldLoop)
            {
                AudioComp->Play();
            }
            else
            {
                AudioComp->Play();
            }
            
            // Apply fade in if specified
            if (FeedbackData.FadeInTime > 0.0f)
            {
                AudioComp->FadeIn(FeedbackData.FadeInTime, FeedbackData.Volume * GlobalVolume);
            }
        }
    }
}