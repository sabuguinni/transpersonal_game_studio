#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    // Initialize narrative context
    NarrativeContext = FNarr_NarrativeContext();
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Initializing narrative system"));
    
    // Initialize story events and descriptions
    InitializeStoryEvents();
    InitializeDescriptions();
    
    // Set initial story phase
    NarrativeContext.CurrentPhase = ENarr_StoryPhase::Arrival;
    NarrativeContext.ThreatLevel = ENarr_ThreatLevel::Safe;
    NarrativeContext.CurrentBiome = TEXT("Unknown Territory");
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Narrative system initialized successfully"));
}

void UNarr_StoryManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Shutting down narrative system"));
    
    // Clear story events
    StoryEvents.Empty();
    PhaseDescriptions.Empty();
    ThreatDescriptions.Empty();
    
    Super::Deinitialize();
}

bool UNarr_StoryManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UNarr_StoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase != NarrativeContext.CurrentPhase)
    {
        ENarr_StoryPhase OldPhase = NarrativeContext.CurrentPhase;
        NarrativeContext.CurrentPhase = NewPhase;
        
        UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Story phase advanced from %d to %d"), 
               (int32)OldPhase, (int32)NewPhase);
        
        // Trigger phase change event
        FNarr_StoryEvent PhaseEvent;
        PhaseEvent.EventID = FString::Printf(TEXT("phase_change_%d"), (int32)NewPhase);
        PhaseEvent.EventName = TEXT("Story Phase Change");
        PhaseEvent.Description = FString::Printf(TEXT("Advanced to phase: %s"), 
                                               *PhaseDescriptions.FindRef(NewPhase));
        PhaseEvent.RequiredPhase = NewPhase;
        PhaseEvent.bIsCompleted = true;
        PhaseEvent.Timestamp = GetWorld()->GetTimeSeconds();
        
        BroadcastStoryEvent(PhaseEvent);
    }
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    // Find the event
    FNarr_StoryEvent* FoundEvent = StoryEvents.FindByPredicate([&EventID](const FNarr_StoryEvent& Event)
    {
        return Event.EventID == EventID;
    });
    
    if (FoundEvent && !FoundEvent->bIsCompleted)
    {
        FoundEvent->bIsCompleted = true;
        FoundEvent->Timestamp = GetWorld()->GetTimeSeconds();
        
        // Add to completed events
        NarrativeContext.CompletedEvents.AddUnique(EventID);
        
        UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Story event triggered: %s"), *EventID);
        
        BroadcastStoryEvent(*FoundEvent);
    }
}

void UNarr_StoryManager::UpdateThreatLevel(ENarr_ThreatLevel NewThreatLevel)
{
    if (NewThreatLevel != NarrativeContext.ThreatLevel)
    {
        ENarr_ThreatLevel OldThreatLevel = NarrativeContext.ThreatLevel;
        NarrativeContext.ThreatLevel = NewThreatLevel;
        
        UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Threat level changed from %d to %d"), 
               (int32)OldThreatLevel, (int32)NewThreatLevel);
    }
}

void UNarr_StoryManager::RecordDinosaurEncounter(const FString& DinosaurType)
{
    NarrativeContext.DinosaurEncounters++;
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Dinosaur encounter recorded: %s (Total: %d)"), 
           *DinosaurType, NarrativeContext.DinosaurEncounters);
    
    // Update threat level based on encounter
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Raptor")))
    {
        UpdateThreatLevel(ENarr_ThreatLevel::Deadly);
    }
    else if (DinosaurType.Contains(TEXT("Brachio")))
    {
        UpdateThreatLevel(ENarr_ThreatLevel::Cautious);
    }
    
    // Trigger encounter event
    FString EventID = FString::Printf(TEXT("encounter_%s_%d"), *DinosaurType, NarrativeContext.DinosaurEncounters);
    TriggerStoryEvent(EventID);
}

bool UNarr_StoryManager::IsEventCompleted(const FString& EventID) const
{
    return NarrativeContext.CompletedEvents.Contains(EventID);
}

FString UNarr_StoryManager::GetContextualNarration() const
{
    return GenerateContextualNarration();
}

TArray<FString> UNarr_StoryManager::GetAvailableDialogueOptions() const
{
    TArray<FString> Options;
    
    // Generate context-appropriate dialogue options
    switch (NarrativeContext.CurrentPhase)
    {
        case ENarr_StoryPhase::Arrival:
            Options.Add(TEXT("Where am I? This place looks ancient..."));
            Options.Add(TEXT("I need to find shelter and water."));
            break;
            
        case ENarr_StoryPhase::FirstContact:
            Options.Add(TEXT("That creature... it's massive!"));
            Options.Add(TEXT("I need to be very careful here."));
            break;
            
        case ENarr_StoryPhase::Exploration:
            Options.Add(TEXT("This ecosystem is incredible."));
            Options.Add(TEXT("I should document these species."));
            break;
            
        case ENarr_StoryPhase::Survival:
            Options.Add(TEXT("Every day is a struggle here."));
            Options.Add(TEXT("I'm learning to adapt."));
            break;
            
        case ENarr_StoryPhase::Adaptation:
            Options.Add(TEXT("I'm starting to understand this world."));
            Options.Add(TEXT("The patterns are becoming clear."));
            break;
            
        case ENarr_StoryPhase::Mastery:
            Options.Add(TEXT("I've become part of this ecosystem."));
            Options.Add(TEXT("This is my world now."));
            break;
    }
    
    return Options;
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    StoryEvents.Empty();
    
    // Arrival phase events
    FNarr_StoryEvent ArrivalEvent;
    ArrivalEvent.EventID = TEXT("first_awakening");
    ArrivalEvent.EventName = TEXT("First Awakening");
    ArrivalEvent.Description = TEXT("The researcher awakens in an unknown prehistoric world");
    ArrivalEvent.RequiredPhase = ENarr_StoryPhase::Arrival;
    StoryEvents.Add(ArrivalEvent);
    
    // First Contact events
    FNarr_StoryEvent FirstSightEvent;
    FirstSightEvent.EventID = TEXT("first_dinosaur_sighting");
    FirstSightEvent.EventName = TEXT("First Dinosaur Sighting");
    FirstSightEvent.Description = TEXT("First encounter with a living dinosaur");
    FirstSightEvent.RequiredPhase = ENarr_StoryPhase::FirstContact;
    StoryEvents.Add(FirstSightEvent);
    
    // Exploration events
    FNarr_StoryEvent BiomeDiscovery;
    BiomeDiscovery.EventID = TEXT("biome_discovery");
    BiomeDiscovery.EventName = TEXT("Biome Discovery");
    BiomeDiscovery.Description = TEXT("Discovery of a new biome type");
    BiomeDiscovery.RequiredPhase = ENarr_StoryPhase::Exploration;
    StoryEvents.Add(BiomeDiscovery);
    
    // Survival events
    FNarr_StoryEvent FirstCraft;
    FirstCraft.EventID = TEXT("first_tool_craft");
    FirstCraft.EventName = TEXT("First Tool Crafted");
    FirstCraft.Description = TEXT("Successfully crafted the first survival tool");
    FirstCraft.RequiredPhase = ENarr_StoryPhase::Survival;
    StoryEvents.Add(FirstCraft);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Initialized %d story events"), StoryEvents.Num());
}

void UNarr_StoryManager::InitializeDescriptions()
{
    PhaseDescriptions.Empty();
    PhaseDescriptions.Add(ENarr_StoryPhase::Arrival, TEXT("Lost in Time"));
    PhaseDescriptions.Add(ENarr_StoryPhase::FirstContact, TEXT("Ancient Encounters"));
    PhaseDescriptions.Add(ENarr_StoryPhase::Exploration, TEXT("Discovering the Past"));
    PhaseDescriptions.Add(ENarr_StoryPhase::Survival, TEXT("Fighting to Live"));
    PhaseDescriptions.Add(ENarr_StoryPhase::Adaptation, TEXT("Learning the Rules"));
    PhaseDescriptions.Add(ENarr_StoryPhase::Mastery, TEXT("Master of the Ancient World"));
    
    ThreatDescriptions.Empty();
    ThreatDescriptions.Add(ENarr_ThreatLevel::Safe, TEXT("Peaceful surroundings"));
    ThreatDescriptions.Add(ENarr_ThreatLevel::Cautious, TEXT("Potential danger nearby"));
    ThreatDescriptions.Add(ENarr_ThreatLevel::Dangerous, TEXT("Active threats present"));
    ThreatDescriptions.Add(ENarr_ThreatLevel::Deadly, TEXT("Extreme danger - immediate threat"));
    ThreatDescriptions.Add(ENarr_ThreatLevel::Extreme, TEXT("Life-threatening situation"));
}

void UNarr_StoryManager::BroadcastStoryEvent(const FNarr_StoryEvent& Event)
{
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryManager: Broadcasting story event: %s - %s"), 
           *Event.EventName, *Event.Description);
    
    // In a full implementation, this would broadcast to UI systems, audio systems, etc.
    // For now, we just log the event
}

FString UNarr_StoryManager::GenerateContextualNarration() const
{
    FString Narration;
    
    // Base narration on current context
    switch (NarrativeContext.CurrentPhase)
    {
        case ENarr_StoryPhase::Arrival:
            Narration = TEXT("You find yourself in an alien landscape, where every sound could signal danger.");
            break;
            
        case ENarr_StoryPhase::FirstContact:
            Narration = TEXT("The ground trembles beneath massive footsteps. You are no longer alone.");
            break;
            
        case ENarr_StoryPhase::Exploration:
            Narration = TEXT("Each step reveals wonders and terrors from a world lost to time.");
            break;
            
        case ENarr_StoryPhase::Survival:
            Narration = TEXT("Every resource matters. Every decision could be your last.");
            break;
            
        case ENarr_StoryPhase::Adaptation:
            Narration = TEXT("You begin to understand the rhythms of this ancient world.");
            break;
            
        case ENarr_StoryPhase::Mastery:
            Narration = TEXT("You have become a master of survival in the age of giants.");
            break;
    }
    
    // Add threat level context
    switch (NarrativeContext.ThreatLevel)
    {
        case ENarr_ThreatLevel::Deadly:
            Narration += TEXT(" Danger lurks in every shadow.");
            break;
            
        case ENarr_ThreatLevel::Extreme:
            Narration += TEXT(" Death stalks your every move.");
            break;
            
        default:
            break;
    }
    
    return Narration;
}