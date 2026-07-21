#include "Narr_NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"

UNarr_NarrativeManager::UNarr_NarrativeManager()
{
    QuestDataTable = nullptr;
    DialogueDataTable = nullptr;
    bInDialogue = false;
}

void UNarr_NarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing prehistoric survival narrative system"));
    
    LoadQuestData();
    LoadDialogueData();
    InitializeDefaultQuests();
    InitializeDefaultDialogues();
    
    // Initialize story flags for prehistoric survival
    StoryFlags.Add(TEXT("FirstHunt"), false);
    StoryFlags.Add(TEXT("MetElder"), false);
    StoryFlags.Add(TEXT("LearnedCrafting"), false);
    StoryFlags.Add(TEXT("SurvivedFirstNight"), false);
    StoryFlags.Add(TEXT("EncounteredPredator"), false);
    StoryFlags.Add(TEXT("FoundWaterSource"), false);
    StoryFlags.Add(TEXT("BuiltShelter"), false);
    StoryFlags.Add(TEXT("JoinedTribe"), false);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialization complete"));
}

void UNarr_NarrativeManager::Deinitialize()
{
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    StoryFlags.Empty();
    bInDialogue = false;
    
    Super::Deinitialize();
}

void UNarr_NarrativeManager::StartQuest(const FString& QuestID)
{
    FNarr_Quest* Quest = FindQuest(QuestID);
    if (Quest && Quest->State == ENarr_QuestState::Inactive)
    {
        Quest->State = ENarr_QuestState::Active;
        ActiveQuests.Add(*Quest);
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started quest '%s'"), *QuestID);
        
        // Trigger story event for quest start
        TriggerStoryEvent(FString::Printf(TEXT("QuestStarted_%s"), *QuestID));
    }
}

void UNarr_NarrativeManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].State = ENarr_QuestState::Completed;
            CompletedQuests.Add(ActiveQuests[i]);
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Completed quest '%s'"), *QuestID);
            
            // Trigger story event for quest completion
            TriggerStoryEvent(FString::Printf(TEXT("QuestCompleted_%s"), *QuestID));
            break;
        }
    }
}

void UNarr_NarrativeManager::FailQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].State = ENarr_QuestState::Failed;
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Failed quest '%s'"), *QuestID);
            break;
        }
    }
}

void UNarr_NarrativeManager::UpdateQuestObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    FNarr_Quest* Quest = FindQuest(QuestID);
    if (Quest && Quest->State == ENarr_QuestState::Active)
    {
        for (FNarr_QuestObjective& Objective : Quest->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.CurrentProgress = FMath::Min(Progress, Objective.TargetProgress);
                
                if (Objective.CurrentProgress >= Objective.TargetProgress)
                {
                    Objective.State = ENarr_QuestState::Completed;
                    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Completed objective '%s' for quest '%s'"), *ObjectiveID, *QuestID);
                }
                
                // Check if all objectives are complete
                bool bAllComplete = true;
                for (const FNarr_QuestObjective& CheckObjective : Quest->Objectives)
                {
                    if (CheckObjective.State != ENarr_QuestState::Completed)
                    {
                        bAllComplete = false;
                        break;
                    }
                }
                
                if (bAllComplete)
                {
                    CompleteQuest(QuestID);
                }
                break;
            }
        }
    }
}

bool UNarr_NarrativeManager::IsQuestActive(const FString& QuestID) const
{
    for (const FNarr_Quest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return true;
        }
    }
    return false;
}

bool UNarr_NarrativeManager::IsQuestCompleted(const FString& QuestID) const
{
    for (const FNarr_Quest& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return true;
        }
    }
    return false;
}

TArray<FNarr_Quest> UNarr_NarrativeManager::GetActiveQuests() const
{
    return ActiveQuests;
}

void UNarr_NarrativeManager::StartDialogue(const FString& DialogueID)
{
    FNarr_DialogueEntry* Dialogue = FindDialogue(DialogueID);
    if (Dialogue)
    {
        CurrentDialogue = *Dialogue;
        bInDialogue = true;
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue '%s' with '%s'"), *DialogueID, *Dialogue->SpeakerName);
    }
}

void UNarr_NarrativeManager::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (bInDialogue && CurrentDialogue.Choices.IsValidIndex(ChoiceIndex))
    {
        const FNarr_DialogueChoice& Choice = CurrentDialogue.Choices[ChoiceIndex];
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Selected choice '%s'"), *Choice.ChoiceText);
        
        if (!Choice.NextDialogueID.IsEmpty())
        {
            StartDialogue(Choice.NextDialogueID);
        }
        else
        {
            EndDialogue();
        }
    }
}

FNarr_DialogueEntry UNarr_NarrativeManager::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

bool UNarr_NarrativeManager::IsInDialogue() const
{
    return bInDialogue;
}

void UNarr_NarrativeManager::EndDialogue()
{
    bInDialogue = false;
    CurrentDialogue = FNarr_DialogueEntry();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ended dialogue"));
}

void UNarr_NarrativeManager::SetStoryFlag(const FString& FlagName, bool bValue)
{
    StoryFlags.Add(FlagName, bValue);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Set story flag '%s' to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
    
    // Trigger story event for flag change
    TriggerStoryEvent(FString::Printf(TEXT("FlagChanged_%s_%s"), *FlagName, bValue ? TEXT("True") : TEXT("False")));
}

bool UNarr_NarrativeManager::GetStoryFlag(const FString& FlagName) const
{
    const bool* Flag = StoryFlags.Find(FlagName);
    return Flag ? *Flag : false;
}

void UNarr_NarrativeManager::TriggerStoryEvent(const FString& EventID)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggered story event '%s'"), *EventID);
    
    // Handle specific prehistoric survival story events
    if (EventID == TEXT("QuestStarted_FirstHunt"))
    {
        SetStoryFlag(TEXT("FirstHunt"), true);
    }
    else if (EventID == TEXT("QuestCompleted_FirstHunt"))
    {
        SetStoryFlag(TEXT("LearnedCrafting"), true);
    }
    else if (EventID == TEXT("FlagChanged_SurvivedFirstNight_True"))
    {
        // Unlock advanced survival quests
        StartQuest(TEXT("BuildShelter"));
    }
}

void UNarr_NarrativeManager::LoadQuestData()
{
    // In a full implementation, this would load from data tables
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Loading quest data"));
}

void UNarr_NarrativeManager::LoadDialogueData()
{
    // In a full implementation, this would load from data tables
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Loading dialogue data"));
}

void UNarr_NarrativeManager::InitializeDefaultQuests()
{
    // Create default prehistoric survival quests
    FNarr_Quest FirstHunt;
    FirstHunt.QuestID = TEXT("FirstHunt");
    FirstHunt.QuestName = TEXT("First Hunt");
    FirstHunt.Description = TEXT("Learn to hunt small prey to survive in the prehistoric wilderness.");
    FirstHunt.State = ENarr_QuestState::Inactive;
    FirstHunt.ExperienceReward = 100;
    
    FNarr_QuestObjective HuntObjective;
    HuntObjective.ObjectiveID = TEXT("KillSmallPrey");
    HuntObjective.Description = TEXT("Hunt 3 small animals");
    HuntObjective.State = ENarr_QuestState::Inactive;
    HuntObjective.TargetProgress = 3;
    FirstHunt.Objectives.Add(HuntObjective);
    
    FNarr_Quest BuildShelter;
    BuildShelter.QuestID = TEXT("BuildShelter");
    BuildShelter.QuestName = TEXT("Build Shelter");
    BuildShelter.Description = TEXT("Construct a basic shelter to protect yourself from predators and weather.");
    BuildShelter.State = ENarr_QuestState::Inactive;
    BuildShelter.ExperienceReward = 200;
    
    FNarr_QuestObjective GatherMaterials;
    GatherMaterials.ObjectiveID = TEXT("GatherWood");
    GatherMaterials.Description = TEXT("Gather 10 pieces of wood");
    GatherMaterials.State = ENarr_QuestState::Inactive;
    GatherMaterials.TargetProgress = 10;
    BuildShelter.Objectives.Add(GatherMaterials);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized default quests"));
}

void UNarr_NarrativeManager::InitializeDefaultDialogues()
{
    // Create default prehistoric survival dialogues
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized default dialogues"));
}

FNarr_Quest* UNarr_NarrativeManager::FindQuest(const FString& QuestID)
{
    for (FNarr_Quest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}

const FNarr_Quest* UNarr_NarrativeManager::FindQuest(const FString& QuestID) const
{
    for (const FNarr_Quest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}

FNarr_DialogueEntry* UNarr_NarrativeManager::FindDialogue(const FString& DialogueID)
{
    // In a full implementation, this would search dialogue data
    return nullptr;
}