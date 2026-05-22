#include "NarrativeDialogueManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5 seconds
    
    // Initialize default values
    bIsDialogueActive = false;
    DialogueTimer = 0.0f;
    ProximityCheckRadius = 2000.0f;
    PlayerPawn = nullptr;
    DialogueAudioComponent = nullptr;
    CurrentDialogueID = TEXT("");
}

void UNarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player pawn
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Create audio component for dialogue playback
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(0.8f);
    }
    
    // Initialize default dialogue entries and narrative events
    InitializeDefaultDialogue();
    InitializeDefaultNarrativeEvents();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager initialized with %d dialogue entries and %d narrative events"), 
           DialogueDatabase.Num(), NarrativeEvents.Num());
}

void UNarrativeDialogueManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update dialogue timer if dialogue is active
    if (bIsDialogueActive)
    {
        UpdateDialogueTimer(DeltaTime);
    }
    
    // Check for proximity-based narrative triggers
    CheckProximityTriggers();
    
    // Process any queued narrative events
    ProcessNarrativeQueue();
}

void UNarrativeDialogueManager::TriggerDialogue(const FString& DialogueID)
{
    FNarr_DialogueEntry* Entry = FindDialogueEntry(DialogueID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue entry not found: %s"), *DialogueID);
        return;
    }
    
    // Stop any current dialogue
    StopCurrentDialogue();
    
    // Start new dialogue
    CurrentDialogueID = DialogueID;
    bIsDialogueActive = true;
    DialogueTimer = Entry->Duration;
    
    // Display dialogue text (would integrate with UI system)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *Entry->SpeakerName, *Entry->DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, Entry->Duration, FColor::Cyan, DisplayText);
    }
    
    // Play audio if available (would integrate with audio system)
    if (DialogueAudioComponent && !Entry->AudioPath.IsEmpty())
    {
        // Load and play audio file (placeholder for actual audio loading)
        UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *Entry->AudioPath);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Triggered dialogue: %s - %s"), *Entry->SpeakerName, *Entry->DialogueText);
}

void UNarrativeDialogueManager::StopCurrentDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    bIsDialogueActive = false;
    DialogueTimer = 0.0f;
    CurrentDialogueID = TEXT("");
    
    // Stop audio playback
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Stopped current dialogue"));
}

bool UNarrativeDialogueManager::IsDialogueActive() const
{
    return bIsDialogueActive;
}

FNarr_DialogueEntry UNarrativeDialogueManager::GetDialogueEntry(const FString& DialogueID)
{
    FNarr_DialogueEntry* Entry = FindDialogueEntry(DialogueID);
    if (Entry)
    {
        return *Entry;
    }
    
    // Return empty entry if not found
    return FNarr_DialogueEntry();
}

void UNarrativeDialogueManager::TriggerNarrativeEvent(const FString& EventID)
{
    FNarr_NarrativeEvent* Event = FindNarrativeEvent(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative event not found: %s"), *EventID);
        return;
    }
    
    if (Event->bIsTriggered)
    {
        UE_LOG(LogTemp, Log, TEXT("Narrative event already triggered: %s"), *EventID);
        return;
    }
    
    // Mark event as triggered
    Event->bIsTriggered = true;
    
    // Play dialogue sequence
    for (const FNarr_DialogueEntry& DialogueEntry : Event->DialogueSequence)
    {
        TriggerDialogue(DialogueEntry.DialogueID);
        // In a real implementation, we'd queue these with proper timing
        break; // For now, just play the first one
    }
    
    UE_LOG(LogTemp, Log, TEXT("Triggered narrative event: %s"), *EventID);
}

void UNarrativeDialogueManager::CheckProximityTriggers()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.bIsTriggered)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Event.TriggerLocation);
        if (Distance <= Event.TriggerRadius)
        {
            TriggerNarrativeEvent(Event.EventID);
        }
    }
}

void UNarrativeDialogueManager::RegisterNarrativeEvent(const FNarr_NarrativeEvent& NewEvent)
{
    NarrativeEvents.Add(NewEvent);
    UE_LOG(LogTemp, Log, TEXT("Registered narrative event: %s"), *NewEvent.EventID);
}

void UNarrativeDialogueManager::OnQuestStarted(const FString& QuestID)
{
    FString DialogueID = FString::Printf(TEXT("quest_start_%s"), *QuestID);
    TriggerDialogue(DialogueID);
    UE_LOG(LogTemp, Log, TEXT("Quest started narrative trigger: %s"), *QuestID);
}

void UNarrativeDialogueManager::OnQuestCompleted(const FString& QuestID)
{
    FString DialogueID = FString::Printf(TEXT("quest_complete_%s"), *QuestID);
    TriggerDialogue(DialogueID);
    UE_LOG(LogTemp, Log, TEXT("Quest completed narrative trigger: %s"), *QuestID);
}

void UNarrativeDialogueManager::OnQuestObjectiveUpdated(const FString& QuestID, const FString& ObjectiveID)
{
    FString DialogueID = FString::Printf(TEXT("quest_objective_%s_%s"), *QuestID, *ObjectiveID);
    TriggerDialogue(DialogueID);
    UE_LOG(LogTemp, Log, TEXT("Quest objective updated narrative trigger: %s - %s"), *QuestID, *ObjectiveID);
}

void UNarrativeDialogueManager::OnPlayerHealthLow()
{
    TriggerDialogue(TEXT("survival_health_low"));
}

void UNarrativeDialogueManager::OnPlayerHungerHigh()
{
    TriggerDialogue(TEXT("survival_hunger_high"));
}

void UNarrativeDialogueManager::OnPlayerThirstHigh()
{
    TriggerDialogue(TEXT("survival_thirst_high"));
}

void UNarrativeDialogueManager::OnDangerDetected(const FString& DangerType)
{
    FString DialogueID = FString::Printf(TEXT("danger_%s"), *DangerType);
    TriggerDialogue(DialogueID);
}

void UNarrativeDialogueManager::OnDinosaurSighted(const FString& DinosaurType, float Distance)
{
    FString DialogueID;
    if (Distance < 1000.0f)
    {
        DialogueID = FString::Printf(TEXT("dino_close_%s"), *DinosaurType);
    }
    else
    {
        DialogueID = FString::Printf(TEXT("dino_distant_%s"), *DinosaurType);
    }
    TriggerDialogue(DialogueID);
}

void UNarrativeDialogueManager::OnDinosaurAggressive(const FString& DinosaurType)
{
    FString DialogueID = FString::Printf(TEXT("dino_aggressive_%s"), *DinosaurType);
    TriggerDialogue(DialogueID);
}

void UNarrativeDialogueManager::OnDinosaurFleeing(const FString& DinosaurType)
{
    FString DialogueID = FString::Printf(TEXT("dino_fleeing_%s"), *DinosaurType);
    TriggerDialogue(DialogueID);
}

void UNarrativeDialogueManager::UpdateDialogueTimer(float DeltaTime)
{
    if (DialogueTimer > 0.0f)
    {
        DialogueTimer -= DeltaTime;
        if (DialogueTimer <= 0.0f)
        {
            StopCurrentDialogue();
        }
    }
}

void UNarrativeDialogueManager::ProcessNarrativeQueue()
{
    // Placeholder for processing queued narrative events
    // In a full implementation, this would handle timed sequences
}

FNarr_DialogueEntry* UNarrativeDialogueManager::FindDialogueEntry(const FString& DialogueID)
{
    for (FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

FNarr_NarrativeEvent* UNarrativeDialogueManager::FindNarrativeEvent(const FString& EventID)
{
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID)
        {
            return &Event;
        }
    }
    return nullptr;
}

void UNarrativeDialogueManager::InitializeDefaultDialogue()
{
    // Survival context dialogues
    FNarr_DialogueEntry HealthLowEntry;
    HealthLowEntry.DialogueID = TEXT("survival_health_low");
    HealthLowEntry.SpeakerName = TEXT("Survival Instinct");
    HealthLowEntry.DialogueText = TEXT("Your wounds are severe. Find shelter and tend to your injuries before it's too late.");
    HealthLowEntry.Duration = 4.0f;
    DialogueDatabase.Add(HealthLowEntry);

    FNarr_DialogueEntry HungerHighEntry;
    HungerHighEntry.DialogueID = TEXT("survival_hunger_high");
    HungerHighEntry.SpeakerName = TEXT("Survival Instinct");
    HungerHighEntry.DialogueText = TEXT("Hunger gnaws at your strength. Hunt for food or gather edible plants quickly.");
    HungerHighEntry.Duration = 4.0f;
    DialogueDatabase.Add(HungerHighEntry);

    FNarr_DialogueEntry ThirstHighEntry;
    ThirstHighEntry.DialogueID = TEXT("survival_thirst_high");
    ThirstHighEntry.SpeakerName = TEXT("Survival Instinct");
    ThirstHighEntry.DialogueText = TEXT("Your throat burns with thirst. Locate fresh water before dehydration weakens you further.");
    ThirstHighEntry.Duration = 4.0f;
    DialogueDatabase.Add(ThirstHighEntry);

    // Dinosaur encounter dialogues
    FNarr_DialogueEntry TRexCloseEntry;
    TRexCloseEntry.DialogueID = TEXT("dino_close_trex");
    TRexCloseEntry.SpeakerName = TEXT("Primal Fear");
    TRexCloseEntry.DialogueText = TEXT("The apex predator is near. Move slowly, avoid sudden movements, and pray it hasn't caught your scent.");
    TRexCloseEntry.Duration = 5.0f;
    DialogueDatabase.Add(TRexCloseEntry);

    FNarr_DialogueEntry RaptorAggressiveEntry;
    RaptorAggressiveEntry.DialogueID = TEXT("dino_aggressive_raptor");
    RaptorAggressiveEntry.SpeakerName = TEXT("Tactical Mind");
    RaptorAggressiveEntry.DialogueText = TEXT("Pack hunters closing in. They're testing your defenses. Find high ground or prepare to fight.");
    RaptorAggressiveEntry.Duration = 4.5f;
    DialogueDatabase.Add(RaptorAggressiveEntry);

    // Quest-related dialogues
    FNarr_DialogueEntry QuestHerdStudyEntry;
    QuestHerdStudyEntry.DialogueID = TEXT("quest_start_herd_study");
    QuestHerdStudyEntry.SpeakerName = TEXT("Research Protocol");
    QuestHerdStudyEntry.DialogueText = TEXT("Begin behavioral observation of herbivore herds. Document their migration patterns and social structures.");
    QuestHerdStudyEntry.Duration = 5.0f;
    QuestHerdStudyEntry.bIsQuestRelated = true;
    DialogueDatabase.Add(QuestHerdStudyEntry);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogue entries"), DialogueDatabase.Num());
}

void UNarrativeDialogueManager::InitializeDefaultNarrativeEvents()
{
    // Valley entrance narrative event
    FNarr_NarrativeEvent ValleyEntranceEvent;
    ValleyEntranceEvent.EventID = TEXT("valley_entrance");
    ValleyEntranceEvent.EventDescription = TEXT("Player enters the ancient valley");
    ValleyEntranceEvent.TriggerLocation = FVector(0.0f, 0.0f, 0.0f); // Savana center
    ValleyEntranceEvent.TriggerRadius = 1500.0f;
    
    FNarr_DialogueEntry ValleyDialogue;
    ValleyDialogue.DialogueID = TEXT("valley_entrance_dialogue");
    ValleyDialogue.SpeakerName = TEXT("Ancient Valley");
    ValleyDialogue.DialogueText = TEXT("You stand at the threshold of an ancient world. The air itself whispers of primordial dangers and forgotten secrets.");
    ValleyDialogue.Duration = 6.0f;
    ValleyEntranceEvent.DialogueSequence.Add(ValleyDialogue);
    
    NarrativeEvents.Add(ValleyEntranceEvent);

    // Forest edge narrative event
    FNarr_NarrativeEvent ForestEdgeEvent;
    ForestEdgeEvent.EventID = TEXT("forest_edge");
    ForestEdgeEvent.EventDescription = TEXT("Player approaches the dense forest");
    ForestEdgeEvent.TriggerLocation = FVector(-45000.0f, 40000.0f, 0.0f); // Forest biome
    ForestEdgeEvent.TriggerRadius = 2000.0f;
    
    FNarr_DialogueEntry ForestDialogue;
    ForestDialogue.DialogueID = TEXT("forest_edge_dialogue");
    ForestDialogue.SpeakerName = TEXT("Forest Whisper");
    ForestDialogue.DialogueText = TEXT("The forest canopy blocks the sun, creating a world of shadows. Predators lurk in the undergrowth.");
    ForestDialogue.Duration = 5.0f;
    ForestEdgeEvent.DialogueSequence.Add(ForestDialogue);
    
    NarrativeEvents.Add(ForestEdgeEvent);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default narrative events"), NarrativeEvents.Num());
}