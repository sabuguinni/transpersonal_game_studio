#include "NarrativeManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UNarrativeManager::UNarrativeManager()
{
    bNarrativeEnabled = true;
    NarrativeVolume = 0.8f;
    GlobalNarrativeCooldown = 15.0f;
    MaxConcurrentNarrations = 2;
    LastNarrativeTime = 0.0f;
    CurrentBiome = EEng_BiomeType::Savanna;
    CurrentNarrationComponent = nullptr;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing dynamic narrative system"));
    
    // Initialize default narrative triggers
    InitializeDefaultTriggers();
    
    // Set up update timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            NarrativeUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UNarrativeManager::CleanupFinishedNarrations),
            1.0f,
            true
        );
        
        World->GetTimerManager().SetTimer(
            CooldownUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UNarrativeManager::UpdateNarrativeCooldowns, 1.0f),
            1.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialization complete"));
}

void UNarrativeManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Shutting down narrative system"));
    
    // Clean up timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(NarrativeUpdateTimer);
        World->GetTimerManager().ClearTimer(CooldownUpdateTimer);
    }
    
    // Stop all active narrations
    StopCurrentNarration();
    
    // Clear audio components
    for (UAudioComponent* AudioComp : NarrativeAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    NarrativeAudioComponents.Empty();
    CurrentNarrationComponent = nullptr;
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FVector& Location)
{
    if (!bNarrativeEnabled)
    {
        return;
    }
    
    if (!CanTriggerNarrative(EventType))
    {
        UE_LOG(LogTemp, Verbose, TEXT("NarrativeManager: Event %d is on cooldown"), (int32)EventType);
        return;
    }
    
    // Find the best trigger for this event
    FNarr_NarrativeTrigger* BestTrigger = FindBestTrigger(EventType);
    if (!BestTrigger)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: No trigger found for event %d"), (int32)EventType);
        return;
    }
    
    // Play the narration
    PlayNarration(*BestTrigger);
    
    // Update cooldowns
    EventCooldowns.Add(EventType, GetWorld()->GetTimeSeconds());
    LastNarrativeTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggered narrative event %d at location %s"), 
           (int32)EventType, *Location.ToString());
}

void UNarrativeManager::RegisterNarrativeTrigger(const FNarr_NarrativeTrigger& NewTrigger)
{
    RegisteredTriggers.Add(NewTrigger);
    
    // Create corresponding active narrative entry
    FNarr_ActiveNarrative ActiveNarrative;
    ActiveNarrative.Trigger = NewTrigger;
    ActiveNarrative.bIsActive = true;
    ActiveNarratives.Add(ActiveNarrative);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered new narrative trigger for event %d"), 
           (int32)NewTrigger.EventType);
}

void UNarrativeManager::PlayNarration(const FNarr_NarrativeTrigger& Trigger)
{
    if (!bNarrativeEnabled)
    {
        return;
    }
    
    // Get available audio component
    UAudioComponent* AudioComp = GetAvailableAudioComponent();
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: No available audio component for narration"));
        return;
    }
    
    // Set audio properties
    AudioComp->SetVolumeMultiplier(NarrativeVolume);
    
    // If we have an audio cue, play it
    if (Trigger.AudioCue.IsValid())
    {
        USoundCue* SoundCue = Trigger.AudioCue.LoadSynchronous();
        if (SoundCue)
        {
            AudioComp->SetSound(SoundCue);
            AudioComp->Play();
            CurrentNarrationComponent = AudioComp;
            
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Playing audio narration: %s"), 
                   *Trigger.DialogueText);
        }
    }
    else
    {
        // Log the text narration for debugging
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Text narration: %s"), *Trigger.DialogueText);
        
        // Display on screen for development
        if (GEngine)
        {
            FString NarratorName;
            switch (Trigger.NarratorType)
            {
                case ENarr_NarratorType::TacticalNarrator:
                    NarratorName = TEXT("Tactical");
                    break;
                case ENarr_NarratorType::FieldResearcher:
                    NarratorName = TEXT("Researcher");
                    break;
                case ENarr_NarratorType::EmergencyNarrator:
                    NarratorName = TEXT("Emergency");
                    break;
                case ENarr_NarratorType::StoryNarrator:
                    NarratorName = TEXT("Story");
                    break;
                case ENarr_NarratorType::SurvivalGuide:
                    NarratorName = TEXT("Survival");
                    break;
            }
            
            FString DisplayText = FString::Printf(TEXT("[%s] %s"), *NarratorName, *Trigger.DialogueText);
            GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, DisplayText);
        }
    }
}

void UNarrativeManager::SetNarrativeEnabled(bool bEnabled)
{
    bNarrativeEnabled = bEnabled;
    
    if (!bEnabled)
    {
        StopCurrentNarration();
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Narrative system %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UNarrativeManager::TriggerBiomeNarrative(EEng_BiomeType BiomeType, const FVector& PlayerLocation)
{
    if (CurrentBiome != BiomeType)
    {
        CurrentBiome = BiomeType;
        TriggerNarrativeEvent(ENarr_NarrativeEvent::PlayerEntersBiome, PlayerLocation);
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Player entered biome %d"), (int32)BiomeType);
    }
}

void UNarrativeManager::TriggerDinosaurEncounter(EEng_DinosaurSpecies Species, float Distance, EEng_ThreatLevel ThreatLevel)
{
    TriggerNarrativeEvent(ENarr_NarrativeEvent::DinosaurEncounter);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dinosaur encounter - Species: %d, Distance: %.1f, Threat: %d"), 
           (int32)Species, Distance, (int32)ThreatLevel);
}

void UNarrativeManager::TriggerSurvivalNarrative(EEng_SurvivalStat StatType, float StatValue)
{
    if (StatValue < 25.0f) // Critical survival state
    {
        TriggerNarrativeEvent(ENarr_NarrativeEvent::SurvivalCritical);
    }
}

void UNarrativeManager::TriggerQuestNarrative(EEng_QuestType QuestType, EEng_QuestStatus Status)
{
    if (Status == EEng_QuestStatus::Completed)
    {
        TriggerNarrativeEvent(ENarr_NarrativeEvent::QuestCompleted);
    }
}

void UNarrativeManager::StopCurrentNarration()
{
    if (CurrentNarrationComponent && IsValid(CurrentNarrationComponent))
    {
        CurrentNarrationComponent->Stop();
        CurrentNarrationComponent = nullptr;
    }
    
    // Stop all active audio components
    for (UAudioComponent* AudioComp : NarrativeAudioComponents)
    {
        if (IsValid(AudioComp) && AudioComp->IsPlaying())
        {
            AudioComp->Stop();
        }
    }
}

bool UNarrativeManager::IsNarrationPlaying() const
{
    if (CurrentNarrationComponent && IsValid(CurrentNarrationComponent))
    {
        return CurrentNarrationComponent->IsPlaying();
    }
    
    // Check all audio components
    for (UAudioComponent* AudioComp : NarrativeAudioComponents)
    {
        if (IsValid(AudioComp) && AudioComp->IsPlaying())
        {
            return true;
        }
    }
    
    return false;
}

void UNarrativeManager::SetNarrativeVolume(float NewVolume)
{
    NarrativeVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (UAudioComponent* AudioComp : NarrativeAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->SetVolumeMultiplier(NarrativeVolume);
        }
    }
}

bool UNarrativeManager::CanTriggerNarrative(ENarr_NarrativeEvent EventType) const
{
    if (!bNarrativeEnabled)
    {
        return false;
    }
    
    // Check global cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastNarrativeTime < GlobalNarrativeCooldown)
    {
        return false;
    }
    
    // Check event-specific cooldown
    if (const float* LastEventTime = EventCooldowns.Find(EventType))
    {
        // Find the cooldown for this event type
        float EventCooldown = 30.0f; // Default cooldown
        for (const FNarr_NarrativeTrigger& Trigger : RegisteredTriggers)
        {
            if (Trigger.EventType == EventType)
            {
                EventCooldown = Trigger.Cooldown;
                break;
            }
        }
        
        if (CurrentTime - *LastEventTime < EventCooldown)
        {
            return false;
        }
    }
    
    return true;
}

FNarr_NarrativeTrigger* UNarrativeManager::FindBestTrigger(ENarr_NarrativeEvent EventType, ENarr_NarratorType PreferredNarrator)
{
    FNarr_NarrativeTrigger* BestTrigger = nullptr;
    float BestPriority = -1.0f;
    
    for (FNarr_NarrativeTrigger& Trigger : RegisteredTriggers)
    {
        if (Trigger.EventType != EventType)
        {
            continue;
        }
        
        // Check if this trigger is still active
        bool bTriggerActive = true;
        for (const FNarr_ActiveNarrative& ActiveNarrative : ActiveNarratives)
        {
            if (ActiveNarrative.Trigger.EventType == EventType &&
                ActiveNarrative.Trigger.NarratorType == Trigger.NarratorType)
            {
                if (Trigger.bIsOneShot && ActiveNarrative.TriggerCount > 0)
                {
                    bTriggerActive = false;
                    break;
                }
            }
        }
        
        if (!bTriggerActive)
        {
            continue;
        }
        
        // Calculate priority (prefer the preferred narrator)
        float Priority = Trigger.Priority;
        if (Trigger.NarratorType == PreferredNarrator)
        {
            Priority += 10.0f;
        }
        
        if (Priority > BestPriority)
        {
            BestPriority = Priority;
            BestTrigger = &Trigger;
        }
    }
    
    return BestTrigger;
}

void UNarrativeManager::UpdateNarrativeCooldowns(float DeltaTime)
{
    // This is called by timer, DeltaTime is not used but kept for consistency
    // Actual cooldown checking is done in CanTriggerNarrative using world time
}

void UNarrativeManager::CleanupFinishedNarrations()
{
    // Remove finished audio components
    for (int32 i = NarrativeAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = NarrativeAudioComponents[i];
        if (!IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            if (AudioComp == CurrentNarrationComponent)
            {
                CurrentNarrationComponent = nullptr;
            }
            NarrativeAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* UNarrativeManager::GetAvailableAudioComponent()
{
    // First, try to find an existing non-playing component
    for (UAudioComponent* AudioComp : NarrativeAudioComponents)
    {
        if (IsValid(AudioComp) && !AudioComp->IsPlaying())
        {
            return AudioComp;
        }
    }
    
    // If we haven't reached the limit, create a new one
    if (NarrativeAudioComponents.Num() < MaxConcurrentNarrations)
    {
        if (UWorld* World = GetWorld())
        {
            UAudioComponent* NewAudioComp = UGameplayStatics::SpawnSound2D(
                World, nullptr, NarrativeVolume, 1.0f, 0.0f, nullptr, false, false
            );
            
            if (NewAudioComp)
            {
                NarrativeAudioComponents.Add(NewAudioComp);
                return NewAudioComp;
            }
        }
    }
    
    return nullptr;
}

void UNarrativeManager::InitializeDefaultTriggers()
{
    // Biome entry narratives
    FNarr_NarrativeTrigger BiomeSwampTrigger;
    BiomeSwampTrigger.EventType = ENarr_NarrativeEvent::PlayerEntersBiome;
    BiomeSwampTrigger.NarratorType = ENarr_NarratorType::FieldResearcher;
    BiomeSwampTrigger.DialogueText = TEXT("Entering swampland. High humidity detected. Watch for aquatic predators and unstable ground.");
    BiomeSwampTrigger.Priority = 5.0f;
    BiomeSwampTrigger.Cooldown = 60.0f;
    RegisterNarrativeTrigger(BiomeSwampTrigger);
    
    // Dinosaur encounter narratives
    FNarr_NarrativeTrigger TRexEncounterTrigger;
    TRexEncounterTrigger.EventType = ENarr_NarrativeEvent::DinosaurEncounter;
    TRexEncounterTrigger.NarratorType = ENarr_NarratorType::TacticalNarrator;
    TRexEncounterTrigger.DialogueText = TEXT("Massive predator detected. T-Rex in your vicinity. Do not run unless you have clear escape route.");
    TRexEncounterTrigger.Priority = 10.0f;
    TRexEncounterTrigger.Cooldown = 45.0f;
    RegisterNarrativeTrigger(TRexEncounterTrigger);
    
    // Survival critical narratives
    FNarr_NarrativeTrigger SurvivalCriticalTrigger;
    SurvivalCriticalTrigger.EventType = ENarr_NarrativeEvent::SurvivalCritical;
    SurvivalCriticalTrigger.NarratorType = ENarr_NarratorType::EmergencyNarrator;
    SurvivalCriticalTrigger.DialogueText = TEXT("Warning: Critical survival status detected. Immediate action required to prevent system failure.");
    SurvivalCriticalTrigger.Priority = 15.0f;
    SurvivalCriticalTrigger.Cooldown = 30.0f;
    RegisterNarrativeTrigger(SurvivalCriticalTrigger);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized %d default triggers"), RegisteredTriggers.Num());
}