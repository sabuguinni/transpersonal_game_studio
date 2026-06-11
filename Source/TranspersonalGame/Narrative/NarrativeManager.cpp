#include "NarrativeManager.h"
#include "Engine/Engine.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsDialogueActive = false;
    CurrentNarrationText = TEXT("");
    
    LoadDialogueData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized"));
}

void UNarrativeManager::StartDialogue(const FString& DialogueID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active, cannot start new dialogue: %s"), *DialogueID);
        return;
    }

    // Sample survival dialogue data
    CurrentDialogue.Empty();
    
    if (DialogueID == TEXT("HunterEncounter"))
    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Experienced Hunter");
        Line1.DialogueText = TEXT("Stay low, newcomer. The great beasts hunt these grounds.");
        Line1.DisplayDuration = 4.0f;
        CurrentDialogue.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Experienced Hunter");
        Line2.DialogueText = TEXT("See those tracks? Thunderfoot passed here at dawn. Avoid the river bend.");
        Line2.DisplayDuration = 5.0f;
        CurrentDialogue.Add(Line2);
    }
    else if (DialogueID == TEXT("TribalElder"))
    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Tribal Elder");
        Line1.DialogueText = TEXT("The old ways kept us alive when the earth shook and fire rained from sky.");
        Line1.DisplayDuration = 5.0f;
        CurrentDialogue.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Tribal Elder");
        Line2.DialogueText = TEXT("Learn to read the wind, child. It carries warnings of approaching danger.");
        Line2.DisplayDuration = 4.5f;
        CurrentDialogue.Add(Line2);
    }

    bIsDialogueActive = true;
    
    if (CurrentDialogue.Num() > 0)
    {
        DisplayDialogueLine(CurrentDialogue[0]);
    }
}

void UNarrativeManager::DisplayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    UE_LOG(LogTemp, Log, TEXT("Displaying dialogue - %s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
    
    // In a full implementation, this would trigger UI display
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.DisplayDuration, FColor::Yellow, DisplayText);
    }
}

void UNarrativeManager::CompleteQuestObjective(const FString& ObjectiveID)
{
    for (FNarr_QuestObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentCount++;
            if (Objective.CurrentCount >= Objective.RequiredCount)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("Quest objective completed: %s"), *ObjectiveID);
                
                if (GEngine)
                {
                    FString CompletionText = FString::Printf(TEXT("Objective Complete: %s"), *Objective.Description);
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, CompletionText);
                }
            }
            break;
        }
    }
}

bool UNarrativeManager::IsQuestObjectiveComplete(const FString& ObjectiveID)
{
    for (const FNarr_QuestObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsCompleted;
        }
    }
    return false;
}

void UNarrativeManager::TriggerNarration(const FString& NarrationText, float Duration)
{
    CurrentNarrationText = NarrationText;
    UE_LOG(LogTemp, Log, TEXT("Narration triggered: %s"), *NarrationText);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Cyan, NarrationText);
    }
}

void UNarrativeManager::LoadDialogueData()
{
    // Initialize sample quest objectives for survival gameplay
    ActiveObjectives.Empty();
    
    FNarr_QuestObjective Obj1;
    Obj1.ObjectiveID = TEXT("GatherSticks");
    Obj1.Description = TEXT("Gather 10 sticks for shelter construction");
    Obj1.RequiredCount = 10;
    Obj1.CurrentCount = 0;
    ActiveObjectives.Add(Obj1);
    
    FNarr_QuestObjective Obj2;
    Obj2.ObjectiveID = TEXT("FindWater");
    Obj2.Description = TEXT("Locate a safe water source");
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    ActiveObjectives.Add(Obj2);
    
    FNarr_QuestObjective Obj3;
    Obj3.ObjectiveID = TEXT("AvoidPredators");
    Obj3.Description = TEXT("Survive 3 predator encounters");
    Obj3.RequiredCount = 3;
    Obj3.CurrentCount = 0;
    ActiveObjectives.Add(Obj3);
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d quest objectives"), ActiveObjectives.Num());
}

void UNarrativeManager::ProcessDialogueChoice(int32 ChoiceIndex)
{
    // Handle player dialogue choices
    UE_LOG(LogTemp, Log, TEXT("Player selected dialogue choice: %d"), ChoiceIndex);
    
    // This would advance the dialogue tree based on player choice
    bIsDialogueActive = false;
}