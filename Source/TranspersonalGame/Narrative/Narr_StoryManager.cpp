#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    CurrentDialogueID = TEXT("");
    bIsDialoguePlaying = false;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("StoryManager: Initializing narrative system"));
    
    InitializeStoryEvents();
    LoadDialogueDatabase();
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            Event.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("StoryManager: Event triggered - %s"), *Event.EventTitle);
            
            // Trigger related dialogue if exists
            if (DialogueDatabase.Contains(EventID))
            {
                PlayDialogue(EventID);
            }
            break;
        }
    }
}

bool UNarr_StoryManager::IsStoryEventCompleted(const FString& EventID)
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

void UNarr_StoryManager::PlayDialogue(const FString& DialogueID)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        CurrentDialogueID = DialogueID;
        bIsDialoguePlaying = true;
        
        const FNarr_DialogueLine& Line = DialogueDatabase[DialogueID];
        UE_LOG(LogTemp, Warning, TEXT("StoryManager: Playing dialogue - %s: %s"), 
               *Line.SpeakerName, *Line.DialogueText);
        
        // Schedule dialogue end
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            bIsDialoguePlaying = false;
            CurrentDialogueID = TEXT("");
        }, Line.Duration, false);
    }
}

void UNarr_StoryManager::RegisterStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    StoryEvents.Add(NewEvent);
    UE_LOG(LogTemp, Warning, TEXT("StoryManager: Registered event - %s"), *NewEvent.EventTitle);
}

TArray<FNarr_StoryEvent> UNarr_StoryManager::GetActiveStoryEvents()
{
    TArray<FNarr_StoryEvent> ActiveEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted)
        {
            ActiveEvents.Add(Event);
        }
    }
    
    // Sort by priority
    ActiveEvents.Sort([](const FNarr_StoryEvent& A, const FNarr_StoryEvent& B)
    {
        return A.Priority > B.Priority;
    });
    
    return ActiveEvents;
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    // Initialize core survival story events
    FNarr_StoryEvent FirstEncounter;
    FirstEncounter.EventID = TEXT("first_dinosaur_encounter");
    FirstEncounter.EventTitle = TEXT("First Predator Encounter");
    FirstEncounter.EventDescription = TEXT("Player encounters their first dangerous dinosaur");
    FirstEncounter.Priority = 100;
    StoryEvents.Add(FirstEncounter);
    
    FNarr_StoryEvent FirstCraft;
    FirstCraft.EventID = TEXT("first_tool_crafted");
    FirstCraft.EventTitle = TEXT("First Tool Crafted");
    FirstCraft.EventDescription = TEXT("Player crafts their first survival tool");
    FirstCraft.Priority = 90;
    StoryEvents.Add(FirstCraft);
    
    FNarr_StoryEvent FirstShelter;
    FirstShelter.EventID = TEXT("first_shelter_built");
    FirstShelter.EventTitle = TEXT("First Shelter Built");
    FirstShelter.EventDescription = TEXT("Player builds their first shelter for protection");
    FirstShelter.Priority = 80;
    StoryEvents.Add(FirstShelter);
    
    FNarr_StoryEvent FirstHunt;
    FirstHunt.EventID = TEXT("first_successful_hunt");
    FirstHunt.EventTitle = TEXT("First Successful Hunt");
    FirstHunt.EventDescription = TEXT("Player successfully hunts their first prey");
    FirstHunt.Priority = 70;
    StoryEvents.Add(FirstHunt);
    
    UE_LOG(LogTemp, Warning, TEXT("StoryManager: Initialized %d story events"), StoryEvents.Num());
}

void UNarr_StoryManager::LoadDialogueDatabase()
{
    // Initialize core survival dialogue lines
    FNarr_DialogueLine PredatorWarning;
    PredatorWarning.SpeakerName = TEXT("Narrator");
    PredatorWarning.DialogueText = TEXT("Danger approaches. The massive predator has caught your scent.");
    PredatorWarning.Duration = 4.0f;
    DialogueDatabase.Add(TEXT("first_dinosaur_encounter"), PredatorWarning);
    
    FNarr_DialogueLine CraftSuccess;
    CraftSuccess.SpeakerName = TEXT("Narrator");
    CraftSuccess.DialogueText = TEXT("Your hands shape the stone. A tool is born from necessity.");
    CraftSuccess.Duration = 3.5f;
    DialogueDatabase.Add(TEXT("first_tool_crafted"), CraftSuccess);
    
    FNarr_DialogueLine ShelterComplete;
    ShelterComplete.SpeakerName = TEXT("Narrator");
    ShelterComplete.DialogueText = TEXT("Shelter from the storm. Safety carved from the wilderness.");
    ShelterComplete.Duration = 3.0f;
    DialogueDatabase.Add(TEXT("first_shelter_built"), ShelterComplete);
    
    FNarr_DialogueLine HuntSuccess;
    HuntSuccess.SpeakerName = TEXT("Narrator");
    HuntSuccess.DialogueText = TEXT("The hunt is complete. Survival demands such choices.");
    HuntSuccess.Duration = 3.5f;
    DialogueDatabase.Add(TEXT("first_successful_hunt"), HuntSuccess);
    
    UE_LOG(LogTemp, Warning, TEXT("StoryManager: Loaded %d dialogue entries"), DialogueDatabase.Num());
}