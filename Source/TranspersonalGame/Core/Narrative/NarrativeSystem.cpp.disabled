#include "NarrativeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NarrativeActor.h"

UNarrativeSystem::UNarrativeSystem()
{
    CurrentStoryPhase = ENarr_StoryPhase::Awakening;
    CurrentDialogueState = ENarr_DialogueState::Inactive;
    CurrentDialogueIndex = 0;
}

void UNarrativeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Initializing narrative subsystem"));
    
    InitializeStoryEvents();
    
    // Set initial story phase
    CurrentStoryPhase = ENarr_StoryPhase::Awakening;
    CurrentDialogueState = ENarr_DialogueState::Inactive;
    CurrentDialogueIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Initialization complete"));
}

void UNarrativeSystem::Deinitialize()
{
    // Clean up any active timers
    if (UWorld* World = GetWorld())
    {
        if (FTimerManager* TimerManager = &World->GetTimerManager())
        {
            TimerManager->ClearTimer(DialogueTimerHandle);
        }
    }
    
    // Clear registered actors
    RegisteredNarrativeActors.Empty();
    
    Super::Deinitialize();
}

void UNarrativeSystem::InitializeStoryEvents()
{
    StoryEvents.Empty();
    
    // Awakening phase events
    FNarr_StoryEvent FirstSteps;
    FirstSteps.EventID = TEXT("first_steps");
    FirstSteps.EventTitle = FText::FromString(TEXT("First Steps"));
    FirstSteps.EventDescription = FText::FromString(TEXT("The player takes their first steps in the prehistoric world"));
    FirstSteps.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstSteps.bIsCompleted = false;
    StoryEvents.Add(FirstSteps);
    
    FNarr_StoryEvent FirstTool;
    FirstTool.EventID = TEXT("first_tool");
    FirstTool.EventTitle = FText::FromString(TEXT("First Tool"));
    FirstTool.EventDescription = FText::FromString(TEXT("The player crafts their first primitive tool"));
    FirstTool.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstTool.bIsCompleted = false;
    StoryEvents.Add(FirstTool);
    
    // Discovery phase events
    FNarr_StoryEvent FirstDinosaur;
    FirstDinosaur.EventID = TEXT("first_dinosaur");
    FirstDinosaur.EventTitle = FText::FromString(TEXT("First Encounter"));
    FirstDinosaur.EventDescription = FText::FromString(TEXT("The player encounters their first dinosaur"));
    FirstDinosaur.RequiredPhase = ENarr_StoryPhase::Discovery;
    FirstDinosaur.Prerequisites.Add(TEXT("first_steps"));
    FirstDinosaur.bIsCompleted = false;
    StoryEvents.Add(FirstDinosaur);
    
    // Adaptation phase events
    FNarr_StoryEvent FirstShelter;
    FirstShelter.EventID = TEXT("first_shelter");
    FirstShelter.EventTitle = FText::FromString(TEXT("Safe Haven"));
    FirstShelter.EventDescription = FText::FromString(TEXT("The player builds their first shelter"));
    FirstShelter.RequiredPhase = ENarr_StoryPhase::Adaptation;
    FirstShelter.Prerequisites.Add(TEXT("first_tool"));
    FirstShelter.bIsCompleted = false;
    StoryEvents.Add(FirstShelter);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Initialized %d story events"), StoryEvents.Num());
}

void UNarrativeSystem::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase != CurrentStoryPhase)
    {
        ENarr_StoryPhase OldPhase = CurrentStoryPhase;
        CurrentStoryPhase = NewPhase;
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Story phase advanced from %d to %d"), 
               static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));
        
        BroadcastStoryPhaseChange(NewPhase);
    }
}

void UNarrativeSystem::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            // Check if prerequisites are met
            bool bPrerequisitesMet = true;
            for (const FString& Prerequisite : Event.Prerequisites)
            {
                if (!IsStoryEventCompleted(Prerequisite))
                {
                    bPrerequisitesMet = false;
                    break;
                }
            }
            
            if (bPrerequisitesMet && CurrentStoryPhase >= Event.RequiredPhase)
            {
                Event.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Story event '%s' completed"), *EventID);
                
                // Trigger any associated dialogue or effects
                OnStoryEventCompleted(Event);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("NarrativeSystem: Cannot complete event '%s' - prerequisites not met or wrong phase"), *EventID);
            }
            break;
        }
    }
}

bool UNarrativeSystem::IsStoryEventCompleted(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsCompleted;
        }
    }
    return false;
}

void UNarrativeSystem::StartDialogue(const TArray<FNarr_DialogueLine>& DialogueLines)
{
    if (CurrentDialogueState != ENarr_DialogueState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeSystem: Cannot start dialogue - already in progress"));
        return;
    }
    
    CurrentDialogue = DialogueLines;
    CurrentDialogueIndex = 0;
    CurrentDialogueState = ENarr_DialogueState::WaitingForInput;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Started dialogue with %d lines"), DialogueLines.Num());
    
    // Auto-play first line
    PlayNextDialogueLine();
}

void UNarrativeSystem::PlayNextDialogueLine()
{
    if (CurrentDialogueState == ENarr_DialogueState::Inactive || 
        CurrentDialogueIndex >= CurrentDialogue.Num())
    {
        EndDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentDialogue[CurrentDialogueIndex];
    CurrentDialogueState = ENarr_DialogueState::Playing;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Playing dialogue line %d: %s - %s"), 
           CurrentDialogueIndex, *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
    
    // Set timer for dialogue duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &UNarrativeSystem::ProcessDialogueTimer,
            CurrentLine.Duration,
            false
        );
    }
}

void UNarrativeSystem::ProcessDialogueTimer()
{
    if (CurrentDialogueState == ENarr_DialogueState::Playing)
    {
        CurrentDialogueIndex++;
        
        if (CurrentDialogueIndex >= CurrentDialogue.Num())
        {
            EndDialogue();
        }
        else
        {
            const FNarr_DialogueLine& NextLine = CurrentDialogue[CurrentDialogueIndex];
            if (NextLine.bRequiresPlayerResponse)
            {
                CurrentDialogueState = ENarr_DialogueState::WaitingForInput;
                UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Waiting for player input"));
            }
            else
            {
                PlayNextDialogueLine();
            }
        }
    }
}

void UNarrativeSystem::EndDialogue()
{
    CurrentDialogueState = ENarr_DialogueState::Completed;
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Dialogue ended"));
    
    // Reset state after a brief delay
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ResetHandle;
        World->GetTimerManager().SetTimer(
            ResetHandle,
            [this]()
            {
                CurrentDialogueState = ENarr_DialogueState::Inactive;
                CurrentDialogue.Empty();
                CurrentDialogueIndex = 0;
            },
            1.0f,
            false
        );
    }
}

void UNarrativeSystem::RegisterNarrativeActor(ANarrativeActor* Actor)
{
    if (Actor && !RegisteredNarrativeActors.Contains(Actor))
    {
        RegisteredNarrativeActors.Add(Actor);
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Registered narrative actor %s"), *Actor->GetName());
    }
}

void UNarrativeSystem::UnregisterNarrativeActor(ANarrativeActor* Actor)
{
    if (Actor)
    {
        RegisteredNarrativeActors.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Unregistered narrative actor %s"), *Actor->GetName());
    }
}

void UNarrativeSystem::TriggerEnvironmentalNarrative(const FVector& Location, float Radius)
{
    int32 TriggeredCount = 0;
    
    for (TWeakObjectPtr<ANarrativeActor> ActorPtr : RegisteredNarrativeActors)
    {
        if (ANarrativeActor* Actor = ActorPtr.Get())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                Actor->TriggerNarrative();
                TriggeredCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Triggered %d environmental narratives near location %s"), 
           TriggeredCount, *Location.ToString());
}

void UNarrativeSystem::OnPlayerSurvivalEvent(const FString& EventType, float Severity)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Player survival event - %s (Severity: %.2f)"), *EventType, Severity);
    
    // Trigger appropriate story events based on survival situations
    if (EventType == TEXT("LowHealth") && Severity > 0.8f)
    {
        TriggerStoryEvent(TEXT("near_death"));
    }
    else if (EventType == TEXT("Hunger") && Severity > 0.7f)
    {
        TriggerStoryEvent(TEXT("desperate_hunger"));
    }
    else if (EventType == TEXT("FirstFire"))
    {
        TriggerStoryEvent(TEXT("fire_mastery"));
    }
}

void UNarrativeSystem::OnDinosaurEncounter(const FString& DinosaurType, bool bHostile)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Dinosaur encounter - %s (Hostile: %s)"), 
           *DinosaurType, bHostile ? TEXT("Yes") : TEXT("No"));
    
    // First dinosaur encounter
    if (!IsStoryEventCompleted(TEXT("first_dinosaur")))
    {
        TriggerStoryEvent(TEXT("first_dinosaur"));
    }
    
    // Specific dinosaur encounters
    if (DinosaurType == TEXT("TRex") && bHostile)
    {
        TriggerStoryEvent(TEXT("apex_predator_encounter"));
    }
    else if (DinosaurType == TEXT("Raptor") && bHostile)
    {
        TriggerStoryEvent(TEXT("pack_hunter_encounter"));
    }
}

void UNarrativeSystem::OnCraftingAchievement(const FString& ItemCrafted)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Crafting achievement - %s"), *ItemCrafted);
    
    if (ItemCrafted.Contains(TEXT("Tool")) && !IsStoryEventCompleted(TEXT("first_tool")))
    {
        TriggerStoryEvent(TEXT("first_tool"));
    }
    else if (ItemCrafted.Contains(TEXT("Shelter")) && !IsStoryEventCompleted(TEXT("first_shelter")))
    {
        TriggerStoryEvent(TEXT("first_shelter"));
    }
}

void UNarrativeSystem::BroadcastStoryPhaseChange(ENarr_StoryPhase NewPhase)
{
    // Broadcast to other systems that story phase has changed
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Broadcasting story phase change to %d"), static_cast<int32>(NewPhase));
    
    // This could trigger UI updates, quest unlocks, etc.
}

void UNarrativeSystem::OnStoryEventCompleted(const FNarr_StoryEvent& CompletedEvent)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystem: Story event completed - %s"), *CompletedEvent.EventTitle.ToString());
    
    // Create contextual dialogue for the completed event
    TArray<FNarr_DialogueLine> EventDialogue;
    
    if (CompletedEvent.EventID == TEXT("first_steps"))
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Narrator");
        Line.DialogueText = FText::FromString(TEXT("Your journey begins in this ancient world. Every step forward is a victory against the unknown."));
        Line.Duration = 4.0f;
        EventDialogue.Add(Line);
    }
    else if (CompletedEvent.EventID == TEXT("first_tool"))
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Narrator");
        Line.DialogueText = FText::FromString(TEXT("With your first tool, you begin to shape this world to your will. The path of survival starts here."));
        Line.Duration = 4.5f;
        EventDialogue.Add(Line);
    }
    
    if (EventDialogue.Num() > 0)
    {
        StartDialogue(EventDialogue);
    }
}