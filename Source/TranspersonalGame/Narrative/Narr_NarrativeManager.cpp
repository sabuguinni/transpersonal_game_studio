#include "Narr_NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UNarr_NarrativeManager::UNarr_NarrativeManager()
{
    EventCooldownTime = 5.0f;
}

void UNarr_NarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Manager initialized with %d events"), NarrativeEvents.Num());
}

void UNarr_NarrativeManager::Deinitialize()
{
    NarrativeEvents.Empty();
    CompletedEvents.Empty();
    EventLastTriggered.Empty();
    
    Super::Deinitialize();
}

void UNarr_NarrativeManager::TriggerNarrativeEvent(const FString& EventID, AActor* SourceActor)
{
    if (!NarrativeEvents.Contains(EventID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative event not found: %s"), *EventID);
        return;
    }

    const FNarr_NarrativeEvent& Event = NarrativeEvents[EventID];
    
    // Check cooldown
    if (EventLastTriggered.Contains(EventID))
    {
        float TimeSinceLastTrigger = GetWorld()->GetTimeSeconds() - EventLastTriggered[EventID];
        if (TimeSinceLastTrigger < EventCooldownTime)
        {
            return;
        }
    }

    // Check if event is repeatable
    if (!Event.bIsRepeatable && CompletedEvents.Contains(EventID))
    {
        return;
    }

    // Check event conditions
    if (!CheckEventConditions(Event))
    {
        return;
    }

    ProcessEventTrigger(Event, SourceActor);
    EventLastTriggered.Add(EventID, GetWorld()->GetTimeSeconds());
}

bool UNarr_NarrativeManager::IsEventAvailable(const FString& EventID) const
{
    if (!NarrativeEvents.Contains(EventID))
    {
        return false;
    }

    const FNarr_NarrativeEvent& Event = NarrativeEvents[EventID];
    
    if (!Event.bIsRepeatable && CompletedEvents.Contains(EventID))
    {
        return false;
    }

    return CheckEventConditions(Event);
}

void UNarr_NarrativeManager::AdvanceStoryProgress(const FString& ObjectiveID)
{
    CurrentStoryProgress.CurrentObjective++;
    
    UE_LOG(LogTemp, Log, TEXT("Story progress advanced: %s (Objective %d)"), 
           *ObjectiveID, CurrentStoryProgress.CurrentObjective);

    // Trigger story advancement event
    TriggerNarrativeEvent(TEXT("StoryAdvancement"));
}

FNarr_StoryProgress UNarr_NarrativeManager::GetCurrentStoryProgress() const
{
    return CurrentStoryProgress;
}

void UNarr_NarrativeManager::RegisterEventCompletion(const FString& EventID)
{
    if (!CompletedEvents.Contains(EventID))
    {
        CompletedEvents.Add(EventID);
        CurrentStoryProgress.CompletedEvents.Add(EventID);
    }
}

TArray<FNarr_NarrativeEvent> UNarr_NarrativeManager::GetAvailableEvents() const
{
    TArray<FNarr_NarrativeEvent> AvailableEvents;
    
    for (const auto& EventPair : NarrativeEvents)
    {
        if (IsEventAvailable(EventPair.Key))
        {
            AvailableEvents.Add(EventPair.Value);
        }
    }

    // Sort by priority
    AvailableEvents.Sort([](const FNarr_NarrativeEvent& A, const FNarr_NarrativeEvent& B) {
        return A.EventPriority > B.EventPriority;
    });

    return AvailableEvents;
}

void UNarr_NarrativeManager::LoadNarrativeData(UDataTable* NarrativeDataTable)
{
    if (!NarrativeDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative data table is null"));
        return;
    }

    TArray<FNarr_NarrativeEvent*> AllRows;
    NarrativeDataTable->GetAllRows<FNarr_NarrativeEvent>(TEXT("LoadNarrativeData"), AllRows);

    for (FNarr_NarrativeEvent* Row : AllRows)
    {
        if (Row)
        {
            NarrativeEvents.Add(Row->EventID, *Row);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d narrative events from data table"), AllRows.Num());
}

void UNarr_NarrativeManager::PlayNarrativeAudio(const FString& AudioClipID)
{
    // Find audio clip and play it
    FString AudioPath = FString::Printf(TEXT("/Game/Audio/Narrative/%s"), *AudioClipID);
    
    USoundCue* AudioClip = LoadObject<USoundCue>(nullptr, *AudioPath);
    if (AudioClip && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), AudioClip);
        UE_LOG(LogTemp, Log, TEXT("Playing narrative audio: %s"), *AudioClipID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio clip not found: %s"), *AudioClipID);
    }
}

void UNarr_NarrativeManager::SetStoryChapter(const FString& ChapterID)
{
    CurrentStoryProgress.ChapterID = ChapterID;
    CurrentStoryProgress.CurrentObjective = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Story chapter set to: %s"), *ChapterID);
}

void UNarr_NarrativeManager::InitializeDefaultEvents()
{
    // Danger Warning Event
    FNarr_NarrativeEvent DangerEvent;
    DangerEvent.EventID = TEXT("DangerWarning");
    DangerEvent.EventTitle = TEXT("Predator Detected");
    DangerEvent.EventDescription = TEXT("A large predator has been detected nearby");
    DangerEvent.EventType = ENarr_NarrativeEventType::Combat_Warning;
    DangerEvent.EventPriority = 10.0f;
    DangerEvent.bIsRepeatable = true;
    DangerEvent.AudioClips.Add(TEXT("DangerWarning_01"));
    NarrativeEvents.Add(DangerEvent.EventID, DangerEvent);

    // Discovery Event
    FNarr_NarrativeEvent DiscoveryEvent;
    DiscoveryEvent.EventID = TEXT("FireDiscovery");
    DiscoveryEvent.EventTitle = TEXT("Fire Discovered");
    DiscoveryEvent.EventDescription = TEXT("Ancient knowledge of fire has been discovered");
    DiscoveryEvent.EventType = ENarr_NarrativeEventType::Discovery;
    DiscoveryEvent.EventPriority = 8.0f;
    DiscoveryEvent.bIsRepeatable = false;
    DiscoveryEvent.AudioClips.Add(TEXT("FireDiscovery_01"));
    NarrativeEvents.Add(DiscoveryEvent.EventID, DiscoveryEvent);

    // Low Health Warning
    FNarr_NarrativeEvent HealthEvent;
    HealthEvent.EventID = TEXT("LowHealth");
    HealthEvent.EventTitle = TEXT("Wounded");
    HealthEvent.EventDescription = TEXT("Health is critically low");
    HealthEvent.EventType = ENarr_NarrativeEventType::LowHealth;
    HealthEvent.EventPriority = 9.0f;
    HealthEvent.bIsRepeatable = true;
    HealthEvent.RequiredConditions.Add(TEXT("Health<25"));
    NarrativeEvents.Add(HealthEvent.EventID, HealthEvent);

    // Hunger Warning
    FNarr_NarrativeEvent HungerEvent;
    HungerEvent.EventID = TEXT("Hunger");
    HungerEvent.EventTitle = TEXT("Starving");
    HungerEvent.EventDescription = TEXT("Food is urgently needed");
    HungerEvent.EventType = ENarr_NarrativeEventType::Hunger;
    HungerEvent.EventPriority = 7.0f;
    HungerEvent.bIsRepeatable = true;
    HungerEvent.RequiredConditions.Add(TEXT("Hunger<20"));
    NarrativeEvents.Add(HungerEvent.EventID, HungerEvent);

    // Area Discovery
    FNarr_NarrativeEvent AreaEvent;
    AreaEvent.EventID = TEXT("NewAreaDiscovered");
    AreaEvent.EventTitle = TEXT("New Territory");
    AreaEvent.EventDescription = TEXT("A new area has been discovered");
    AreaEvent.EventType = ENarr_NarrativeEventType::Discovery;
    AreaEvent.EventPriority = 6.0f;
    AreaEvent.bIsRepeatable = true;
    NarrativeEvents.Add(AreaEvent.EventID, AreaEvent);
}

bool UNarr_NarrativeManager::CheckEventConditions(const FNarr_NarrativeEvent& Event) const
{
    // Basic condition checking - can be expanded with game state
    for (const FString& Condition : Event.RequiredConditions)
    {
        if (Condition.Contains(TEXT("Health<")))
        {
            // Would check actual player health here
            // For now, return true for demonstration
            return true;
        }
        
        if (Condition.Contains(TEXT("Hunger<")))
        {
            // Would check actual player hunger here
            return true;
        }
    }
    
    return true;
}

void UNarr_NarrativeManager::ProcessEventTrigger(const FNarr_NarrativeEvent& Event, AActor* SourceActor)
{
    UE_LOG(LogTemp, Log, TEXT("Narrative Event Triggered: %s - %s"), 
           *Event.EventTitle, *Event.EventDescription);

    // Play audio clips
    for (const FString& AudioClip : Event.AudioClips)
    {
        PlayNarrativeAudio(AudioClip);
    }

    // Mark as completed if not repeatable
    if (!Event.bIsRepeatable)
    {
        RegisterEventCompletion(Event.EventID);
    }

    // Broadcast event to other systems
    if (GetWorld())
    {
        // Could broadcast to UI, quest system, etc.
    }
}