#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentLineIndex = 0;
    bIsDialogueActive = false;
    DefaultDisplayDuration = 4.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    CreateSurvivalDialogues();
    CreateDangerWarnings();
    CreateQuestBriefings();
}

void UNarr_DialogueSystem::CreateSurvivalDialogues()
{
    // Survival Tips Sequence
    FNarr_DialogueSequence SurvivalTips;
    SurvivalTips.SequenceID = TEXT("survival_tips");
    SurvivalTips.bIsRepeatable = true;
    SurvivalTips.Priority = 1;

    FNarr_DialogueLine Tip1;
    Tip1.SpeakerName = TEXT("Survival Guide");
    Tip1.DialogueText = TEXT("Stay near water sources, but beware - predators hunt at watering holes.");
    Tip1.DialogueType = ENarr_DialogueType::SurvivalTip;
    Tip1.DisplayDuration = 5.0f;
    SurvivalTips.DialogueLines.Add(Tip1);

    FNarr_DialogueLine Tip2;
    Tip2.SpeakerName = TEXT("Survival Guide");
    Tip2.DialogueText = TEXT("Craft stone tools before venturing into dangerous territory.");
    Tip2.DialogueType = ENarr_DialogueType::SurvivalTip;
    Tip2.DisplayDuration = 4.0f;
    SurvivalTips.DialogueLines.Add(Tip2);

    FNarr_DialogueLine Tip3;
    Tip3.SpeakerName = TEXT("Survival Guide");
    Tip3.DialogueText = TEXT("High ground provides safety and better visibility of approaching threats.");
    Tip3.DialogueType = ENarr_DialogueType::SurvivalTip;
    Tip3.DisplayDuration = 4.5f;
    SurvivalTips.DialogueLines.Add(Tip3);

    DialogueSequences.Add(SurvivalTips);
}

void UNarr_DialogueSystem::CreateDangerWarnings()
{
    // Danger Warning Sequence
    FNarr_DialogueSequence DangerWarnings;
    DangerWarnings.SequenceID = TEXT("danger_warnings");
    DangerWarnings.bIsRepeatable = true;
    DangerWarnings.Priority = 3;

    FNarr_DialogueLine Warning1;
    Warning1.SpeakerName = TEXT("Alert System");
    Warning1.DialogueText = TEXT("Predator scent detected! Move to safety immediately!");
    Warning1.DialogueType = ENarr_DialogueType::DangerWarning;
    Warning1.DisplayDuration = 3.0f;
    DangerWarnings.DialogueLines.Add(Warning1);

    FNarr_DialogueLine Warning2;
    Warning2.SpeakerName = TEXT("Alert System");
    Warning2.DialogueText = TEXT("Pack hunters in the area. Avoid open ground and seek elevated positions.");
    Warning2.DialogueType = ENarr_DialogueType::DangerWarning;
    Warning2.DisplayDuration = 4.0f;
    DangerWarnings.DialogueLines.Add(Warning2);

    FNarr_DialogueLine Warning3;
    Warning3.SpeakerName = TEXT("Alert System");
    Warning3.DialogueText = TEXT("Large predator approaching from the south. Take cover now!");
    Warning3.DialogueType = ENarr_DialogueType::DangerWarning;
    Warning3.DisplayDuration = 3.5f;
    DangerWarnings.DialogueLines.Add(Warning3);

    DialogueSequences.Add(DangerWarnings);
}

void UNarr_DialogueSystem::CreateQuestBriefings()
{
    // Quest Briefing Sequence
    FNarr_DialogueSequence QuestBriefings;
    QuestBriefings.SequenceID = TEXT("quest_briefings");
    QuestBriefings.bIsRepeatable = false;
    QuestBriefings.Priority = 2;

    FNarr_DialogueLine Quest1;
    Quest1.SpeakerName = TEXT("Mission Briefing");
    Quest1.DialogueText = TEXT("Hunt Mission: Eliminate the velociraptor pack threatening our territory.");
    Quest1.DialogueType = ENarr_DialogueType::QuestBriefing;
    Quest1.DisplayDuration = 5.0f;
    QuestBriefings.DialogueLines.Add(Quest1);

    FNarr_DialogueLine Quest2;
    Quest2.SpeakerName = TEXT("Mission Briefing");
    Quest2.DialogueText = TEXT("Gather Mission: Collect stone and wood resources for shelter construction.");
    Quest2.DialogueType = ENarr_DialogueType::QuestBriefing;
    Quest2.DisplayDuration = 4.5f;
    QuestBriefings.DialogueLines.Add(Quest2);

    FNarr_DialogueLine Quest3;
    Quest3.SpeakerName = TEXT("Mission Briefing");
    Quest3.DialogueText = TEXT("Explore Mission: Scout the eastern valley for new resources and threats.");
    Quest3.DialogueType = ENarr_DialogueType::QuestBriefing;
    Quest3.DisplayDuration = 4.5f;
    QuestBriefings.DialogueLines.Add(Quest3);

    DialogueSequences.Add(QuestBriefings);
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            CurrentSequence = Sequence;
            CurrentLineIndex = 0;
            bIsDialogueActive = true;
            
            if (CurrentSequence.DialogueLines.Num() > 0)
            {
                FNarr_DialogueLine CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
                UE_LOG(LogTemp, Warning, TEXT("Dialogue Started: %s - %s"), 
                    *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
            }
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    CurrentSequence = FNarr_DialogueSequence();
}

void UNarr_DialogueSystem::NextDialogueLine()
{
    if (!bIsDialogueActive || CurrentSequence.DialogueLines.Num() == 0)
    {
        return;
    }

    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        if (CurrentSequence.bIsRepeatable)
        {
            CurrentLineIndex = 0;
        }
        else
        {
            StopCurrentDialogue();
            return;
        }
    }

    FNarr_DialogueLine CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    UE_LOG(LogTemp, Warning, TEXT("Next Dialogue: %s - %s"), 
        *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (bIsDialogueActive && CurrentSequence.DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        return CurrentSequence.DialogueLines[CurrentLineIndex];
    }
    
    return FNarr_DialogueLine();
}

void UNarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
}

void UNarr_DialogueSystem::TriggerSurvivalTip(const FString& TipText)
{
    FNarr_DialogueSequence TipSequence;
    TipSequence.SequenceID = TEXT("dynamic_tip");
    TipSequence.bIsRepeatable = false;
    TipSequence.Priority = 1;

    FNarr_DialogueLine Tip;
    Tip.SpeakerName = TEXT("Survival Guide");
    Tip.DialogueText = TipText;
    Tip.DialogueType = ENarr_DialogueType::SurvivalTip;
    Tip.DisplayDuration = 4.0f;
    TipSequence.DialogueLines.Add(Tip);

    CurrentSequence = TipSequence;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
}

void UNarr_DialogueSystem::TriggerDangerWarning(const FString& WarningText)
{
    FNarr_DialogueSequence WarningSequence;
    WarningSequence.SequenceID = TEXT("dynamic_warning");
    WarningSequence.bIsRepeatable = false;
    WarningSequence.Priority = 3;

    FNarr_DialogueLine Warning;
    Warning.SpeakerName = TEXT("Alert System");
    Warning.DialogueText = WarningText;
    Warning.DialogueType = ENarr_DialogueType::DangerWarning;
    Warning.DisplayDuration = 3.0f;
    WarningSequence.DialogueLines.Add(Warning);

    CurrentSequence = WarningSequence;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
}

void UNarr_DialogueSystem::TriggerQuestBriefing(const FString& QuestText)
{
    FNarr_DialogueSequence QuestSequence;
    QuestSequence.SequenceID = TEXT("dynamic_quest");
    QuestSequence.bIsRepeatable = false;
    QuestSequence.Priority = 2;

    FNarr_DialogueLine Quest;
    Quest.SpeakerName = TEXT("Mission Briefing");
    Quest.DialogueText = QuestText;
    Quest.DialogueType = ENarr_DialogueType::QuestBriefing;
    Quest.DisplayDuration = 5.0f;
    QuestSequence.DialogueLines.Add(Quest);

    CurrentSequence = QuestSequence;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
}