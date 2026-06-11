#include "NarrativeDialogueSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UNarrativeDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bDialogueActive = false;
    CurrentSpeaker = nullptr;
    
    LoadDialogueData();
    LoadQuestData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem initialized"));
}

void UNarrativeDialogueSystem::StartDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!Speaker)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue: Speaker is null"));
        return;
    }

    if (FNarr_DialogueEntry* FoundDialogue = DialogueDatabase.Find(DialogueID))
    {
        CurrentDialogue = *FoundDialogue;
        CurrentSpeaker = Speaker;
        bDialogueActive = true;
        
        BroadcastDialogueUpdate();
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with speaker: %s"), 
               *DialogueID, *Speaker->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue ID not found: %s"), *DialogueID);
    }
}

void UNarrativeDialogueSystem::EndDialogue()
{
    if (bDialogueActive)
    {
        bDialogueActive = false;
        CurrentSpeaker = nullptr;
        CurrentDialogue = FNarr_DialogueEntry();
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
    }
}

void UNarrativeDialogueSystem::SelectDialogueOption(int32 OptionIndex)
{
    if (!bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active dialogue to select option"));
        return;
    }

    if (CurrentDialogue.ResponseOptions.IsValidIndex(OptionIndex))
    {
        FString SelectedOption = CurrentDialogue.ResponseOptions[OptionIndex];
        UE_LOG(LogTemp, Log, TEXT("Player selected dialogue option: %s"), *SelectedOption);
        
        // Process the selected option (could trigger quest updates, new dialogue, etc.)
        EndDialogue();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid dialogue option index: %d"), OptionIndex);
    }
}

bool UNarrativeDialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueEntry UNarrativeDialogueSystem::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

void UNarrativeDialogueSystem::UpdateQuestObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (TArray<FNarr_QuestObjective>* QuestObjectives = QuestDatabase.Find(QuestID))
    {
        for (FNarr_QuestObjective& Objective : *QuestObjectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.RequiredCount);
                
                if (Objective.CurrentCount >= Objective.RequiredCount)
                {
                    Objective.bIsCompleted = true;
                    UE_LOG(LogTemp, Log, TEXT("Quest objective completed: %s - %s"), 
                           *QuestID, *ObjectiveID);
                }
                
                return;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Objective ID not found: %s in quest: %s"), 
               *ObjectiveID, *QuestID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest ID not found: %s"), *QuestID);
    }
}

bool UNarrativeDialogueSystem::IsQuestCompleted(const FString& QuestID) const
{
    if (const TArray<FNarr_QuestObjective>* QuestObjectives = QuestDatabase.Find(QuestID))
    {
        for (const FNarr_QuestObjective& Objective : *QuestObjectives)
        {
            if (!Objective.bIsCompleted)
            {
                return false;
            }
        }
        return true;
    }
    
    return false;
}

void UNarrativeDialogueSystem::LoadDialogueData()
{
    // Sample survival-focused dialogue entries
    FNarr_DialogueEntry HunterGreeting;
    HunterGreeting.SpeakerName = TEXT("Tribal Hunter");
    HunterGreeting.DialogueText = TEXT("The hunting grounds are dangerous today. Large predator tracks near the river.");
    HunterGreeting.AudioDuration = 4.5f;
    HunterGreeting.ResponseOptions.Add(TEXT("Tell me about the tracks"));
    HunterGreeting.ResponseOptions.Add(TEXT("I can handle myself"));
    HunterGreeting.ResponseOptions.Add(TEXT("Where should I hunt instead?"));
    DialogueDatabase.Add(TEXT("hunter_greeting"), HunterGreeting);

    FNarr_DialogueEntry CrafterAdvice;
    CrafterAdvice.SpeakerName = TEXT("Tool Maker");
    CrafterAdvice.DialogueText = TEXT("Sharp stones make better spear tips. Look for flint near the rocky cliffs.");
    CrafterAdvice.AudioDuration = 3.8f;
    CrafterAdvice.ResponseOptions.Add(TEXT("Show me how to knap flint"));
    CrafterAdvice.ResponseOptions.Add(TEXT("Where are these cliffs?"));
    DialogueDatabase.Add(TEXT("crafter_advice"), CrafterAdvice);

    FNarr_DialogueEntry ScoutWarning;
    ScoutWarning.SpeakerName = TEXT("Territory Scout");
    ScoutWarning.DialogueText = TEXT("Raptor pack claimed the eastern valley. Three adults, possibly more. Avoid that area.");
    ScoutWarning.AudioDuration = 5.2f;
    ScoutWarning.ResponseOptions.Add(TEXT("How do you know their numbers?"));
    ScoutWarning.ResponseOptions.Add(TEXT("Any safe routes east?"));
    DialogueDatabase.Add(TEXT("scout_warning"), ScoutWarning);

    UE_LOG(LogTemp, Log, TEXT("Loaded %d dialogue entries"), DialogueDatabase.Num());
}

void UNarrativeDialogueSystem::LoadQuestData()
{
    // Sample survival quest: First Hunt
    TArray<FNarr_QuestObjective> FirstHuntObjectives;
    
    FNarr_QuestObjective CraftSpear;
    CraftSpear.ObjectiveID = TEXT("craft_spear");
    CraftSpear.Description = TEXT("Craft a wooden spear for hunting");
    CraftSpear.RequiredCount = 1;
    FirstHuntObjectives.Add(CraftSpear);
    
    FNarr_QuestObjective HuntSmallGame;
    HuntSmallGame.ObjectiveID = TEXT("hunt_small_game");
    HuntSmallGame.Description = TEXT("Hunt 3 small creatures for food");
    HuntSmallGame.RequiredCount = 3;
    FirstHuntObjectives.Add(HuntSmallGame);
    
    QuestDatabase.Add(TEXT("first_hunt"), FirstHuntObjectives);

    // Sample survival quest: Territory Mapping
    TArray<FNarr_QuestObjective> TerritoryObjectives;
    
    FNarr_QuestObjective ExploreRiver;
    ExploreRiver.ObjectiveID = TEXT("explore_river");
    ExploreRiver.Description = TEXT("Follow the river to find water sources");
    ExploreRiver.RequiredCount = 1;
    TerritoryObjectives.Add(ExploreRiver);
    
    FNarr_QuestObjective MarkDangerZones;
    MarkDangerZones.ObjectiveID = TEXT("mark_danger_zones");
    MarkDangerZones.Description = TEXT("Identify 5 predator territories");
    MarkDangerZones.RequiredCount = 5;
    TerritoryObjectives.Add(MarkDangerZones);
    
    QuestDatabase.Add(TEXT("territory_mapping"), TerritoryObjectives);

    UE_LOG(LogTemp, Log, TEXT("Loaded %d quest entries"), QuestDatabase.Num());
}

void UNarrativeDialogueSystem::BroadcastDialogueUpdate()
{
    // This would broadcast to UI systems in a full implementation
    UE_LOG(LogTemp, Log, TEXT("Broadcasting dialogue update: %s"), 
           *CurrentDialogue.DialogueText);
}