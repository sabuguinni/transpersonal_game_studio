#include "DialogueSystem.h"
#include "Engine/World.h"

ANarr_DialogueActor::ANarr_DialogueActor()
    : InteractionRadius(300.0f)
    , bIsInteractable(true)
    , CurrentLineIndex(0)
{
    PrimaryActorTick.bCanEverTick = false;

    // Default TribalLeader dialogue data
    DialogueData.NPCName = TEXT("TribalLeader");
    DialogueData.CurrentState = ENarr_DialogueState::Idle;

    // Greeting line
    FNarr_DialogueLine GreetLine;
    GreetLine.SpeakerName = TEXT("Korg");
    GreetLine.LineText = TEXT("Stranger. You have survived the long dark. Our tribe has watched you from the treeline.");
    GreetLine.TriggerState = ENarr_DialogueState::Greeting;
    DialogueData.Lines.Add(GreetLine);

    // Quest offer line
    FNarr_DialogueLine QuestLine;
    QuestLine.SpeakerName = TEXT("Korg");
    QuestLine.LineText = TEXT("If you wish to earn your place among us, prove yourself. Drive the raptors from the eastern valley.");
    QuestLine.TriggerState = ENarr_DialogueState::QuestOffer;
    DialogueData.Lines.Add(QuestLine);

    // Quest active line
    FNarr_DialogueLine ActiveLine;
    ActiveLine.SpeakerName = TEXT("Korg");
    ActiveLine.LineText = TEXT("Three raptors. Fast. Dangerous. They took two of our hunters last moon. Do not underestimate them.");
    ActiveLine.TriggerState = ENarr_DialogueState::QuestActive;
    DialogueData.Lines.Add(ActiveLine);

    // Quest done line
    FNarr_DialogueLine DoneLine;
    DoneLine.SpeakerName = TEXT("Korg");
    DoneLine.LineText = TEXT("You did it. The valley is clear. You have earned your place at our fire. Welcome, hunter.");
    DoneLine.TriggerState = ENarr_DialogueState::QuestDone;
    DialogueData.Lines.Add(DoneLine);
}

void ANarr_DialogueActor::BeginPlay()
{
    Super::BeginPlay();
    CurrentLineIndex = 0;
}

void ANarr_DialogueActor::StartDialogue()
{
    if (!bIsInteractable) return;
    CurrentLineIndex = 0;
    SetDialogueState(ENarr_DialogueState::Greeting);
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Started dialogue with %s"), *DialogueData.NPCName);
}

void ANarr_DialogueActor::AdvanceDialogue()
{
    if (DialogueData.Lines.Num() == 0) return;

    CurrentLineIndex++;
    if (CurrentLineIndex >= DialogueData.Lines.Num())
    {
        EndDialogue();
        return;
    }

    FNarr_DialogueLine& Line = DialogueData.Lines[CurrentLineIndex];
    SetDialogueState(Line.TriggerState);
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: [%s] %s"), *Line.SpeakerName, *Line.LineText);
}

void ANarr_DialogueActor::EndDialogue()
{
    SetDialogueState(ENarr_DialogueState::Farewell);
    CurrentLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Ended dialogue with %s"), *DialogueData.NPCName);
}

FNarr_DialogueLine ANarr_DialogueActor::GetCurrentLine() const
{
    if (DialogueData.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueData.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

void ANarr_DialogueActor::SetDialogueState(ENarr_DialogueState NewState)
{
    DialogueData.CurrentState = NewState;
}
