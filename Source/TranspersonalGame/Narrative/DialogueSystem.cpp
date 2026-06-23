// DialogueSystem.cpp
// Narrative & Dialogue Agent #15
// Prehistoric survival NPC dialogue — no spiritual content
// All dialogue is practical: survival, danger, resources, territory

#include "DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    CurrentState = ENarr_DialogueState::Idle;
    NPCRole = ENarr_NPCRole::Survivor;
    InteractionRadius = 200.0f;
    bIsInDialogue = false;
    CurrentLineIndex = 0;
    ActiveTree = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    LoadDefaultDialogueForRole();
}

void UNarr_DialogueComponent::BeginDialogue(ENarr_DialogueState InitialState)
{
    if (bIsInDialogue)
    {
        return;
    }

    CurrentState = InitialState;
    CurrentLineIndex = 0;
    bIsInDialogue = true;
    ActiveTree = FindTreeForState(InitialState);

    if (!ActiveTree || ActiveTree->Lines.Num() == 0)
    {
        // No lines for this state — end immediately
        bIsInDialogue = false;
        ActiveTree = nullptr;
    }
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!bIsInDialogue || !ActiveTree)
    {
        return false;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveTree->Lines.Num())
    {
        // Reached end of tree
        EndDialogue();
        return false;
    }

    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    bIsInDialogue = false;
    CurrentLineIndex = 0;
    ActiveTree = nullptr;
    CurrentState = ENarr_DialogueState::Idle;
}

FString UNarr_DialogueComponent::GetCurrentLineText() const
{
    if (!ActiveTree || !bIsInDialogue)
    {
        return FString(TEXT(""));
    }

    if (CurrentLineIndex >= 0 && CurrentLineIndex < ActiveTree->Lines.Num())
    {
        return ActiveTree->Lines[CurrentLineIndex].LineText;
    }

    return FString(TEXT(""));
}

FString UNarr_DialogueComponent::GetCurrentSpeakerID() const
{
    if (!ActiveTree || !bIsInDialogue)
    {
        return FString(TEXT(""));
    }

    if (CurrentLineIndex >= 0 && CurrentLineIndex < ActiveTree->Lines.Num())
    {
        return ActiveTree->Lines[CurrentLineIndex].SpeakerID;
    }

    return FString(TEXT(""));
}

void UNarr_DialogueComponent::SetDialogueState(ENarr_DialogueState NewState)
{
    if (bIsInDialogue)
    {
        EndDialogue();
    }

    CurrentState = NewState;
    BeginDialogue(NewState);
}

void UNarr_DialogueComponent::LoadDefaultDialogueForRole()
{
    DialogueTrees.Empty();

    switch (NPCRole)
    {
        case ENarr_NPCRole::TribalElder:
            BuildElderDialogue();
            break;
        case ENarr_NPCRole::ScoutHunter:
            BuildScoutDialogue();
            break;
        case ENarr_NPCRole::Gatherer:
            BuildGathererDialogue();
            break;
        default:
            BuildScoutDialogue();
            break;
    }
}

FNarr_DialogueTree* UNarr_DialogueComponent::FindTreeForState(ENarr_DialogueState State)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.Lines.Num() > 0 && Tree.Lines[0].TriggerState == State)
        {
            return &Tree;
        }
    }

    // Fallback: return first tree if available
    if (DialogueTrees.Num() > 0)
    {
        return &DialogueTrees[0];
    }

    return nullptr;
}

void UNarr_DialogueComponent::BuildElderDialogue()
{
    // === GREETING TREE ===
    {
        FNarr_DialogueTree GreetTree;
        GreetTree.TreeID = TEXT("Elder_Greeting");
        GreetTree.NPCRole = ENarr_NPCRole::TribalElder;

        auto MakeLine = [](FString Speaker, FString Text, ENarr_DialogueState State, float Dur) -> FNarr_DialogueLine
        {
            FNarr_DialogueLine L;
            L.SpeakerID = Speaker;
            L.LineText = Text;
            L.TriggerState = State;
            L.DisplayDuration = Dur;
            return L;
        };

        GreetTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("You are still alive. Good. The canyon took three hunters last season."),
            ENarr_DialogueState::Greeting, 4.0f));

        GreetTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("The great lizards move south when the rains come. We move with them — or we starve."),
            ENarr_DialogueState::Greeting, 4.5f));

        GreetTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("Bring me flint from the river bend. Sharp flint. Not the grey stone — the black stone."),
            ENarr_DialogueState::Greeting, 4.0f));

        DialogueTrees.Add(GreetTree);
    }

    // === WARNING TREE ===
    {
        FNarr_DialogueTree WarnTree;
        WarnTree.TreeID = TEXT("Elder_Warning");
        WarnTree.NPCRole = ENarr_NPCRole::TribalElder;

        auto MakeLine = [](FString Speaker, FString Text, ENarr_DialogueState State, float Dur) -> FNarr_DialogueLine
        {
            FNarr_DialogueLine L;
            L.SpeakerID = Speaker;
            L.LineText = Text;
            L.TriggerState = State;
            L.DisplayDuration = Dur;
            return L;
        };

        WarnTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("Do not go east. The big one — the one with the small arms — it hunts there at night."),
            ENarr_DialogueState::Warning, 4.5f));

        WarnTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("I lost my brother to one of those. Thirty seasons ago. The ground shook before we heard it."),
            ENarr_DialogueState::Warning, 5.0f));

        WarnTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("If you must go east — go at midday. It rests in the shade then. Move fast. Stay downwind."),
            ENarr_DialogueState::Warning, 5.0f));

        DialogueTrees.Add(WarnTree);
    }

    // === QUEST GIVE TREE ===
    {
        FNarr_DialogueTree QuestTree;
        QuestTree.TreeID = TEXT("Elder_QuestGive");
        QuestTree.NPCRole = ENarr_NPCRole::TribalElder;

        auto MakeLine = [](FString Speaker, FString Text, ENarr_DialogueState State, float Dur) -> FNarr_DialogueLine
        {
            FNarr_DialogueLine L;
            L.SpeakerID = Speaker;
            L.LineText = Text;
            L.TriggerState = State;
            L.DisplayDuration = Dur;
            return L;
        };

        QuestTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("The river is two days walk north. The herd crosses there — the long-necks."),
            ENarr_DialogueState::QuestGive, 4.0f));

        QuestTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("We need meat. Not lizard meat — the big herd meat. Enough for the cold season."),
            ENarr_DialogueState::QuestGive, 4.5f));

        QuestTree.Lines.Add(MakeLine(
            TEXT("Elder"),
            TEXT("Take the young one with you. He is fast. You are careful. Together you might survive."),
            ENarr_DialogueState::QuestGive, 4.5f));

        DialogueTrees.Add(QuestTree);
    }
}

void UNarr_DialogueComponent::BuildScoutDialogue()
{
    // === GREETING TREE ===
    {
        FNarr_DialogueTree GreetTree;
        GreetTree.TreeID = TEXT("Scout_Greeting");
        GreetTree.NPCRole = ENarr_NPCRole::ScoutHunter;

        auto MakeLine = [](FString Speaker, FString Text, ENarr_DialogueState State, float Dur) -> FNarr_DialogueLine
        {
            FNarr_DialogueLine L;
            L.SpeakerID = Speaker;
            L.LineText = Text;
            L.TriggerState = State;
            L.DisplayDuration = Dur;
            return L;
        };

        GreetTree.Lines.Add(MakeLine(
            TEXT("Scout"),
            TEXT("Three raptors. North ridge. Moving in a pack — they are hunting together."),
            ENarr_DialogueState::Greeting, 4.0f));

        GreetTree.Lines.Add(MakeLine(
            TEXT("Scout"),
            TEXT("I tracked them for half a day. They are not after us — there is a wounded long-neck near the falls."),
            ENarr_DialogueState::Greeting, 5.0f));

        GreetTree.Lines.Add(MakeLine(
            TEXT("Scout"),
            TEXT("When they finish — we can take what is left. But we need to be quick. And quiet."),
            ENarr_DialogueState::Greeting, 4.5f));

        DialogueTrees.Add(GreetTree);
    }

    // === WARNING TREE ===
    {
        FNarr_DialogueTree WarnTree;
        WarnTree.TreeID = TEXT("Scout_Warning");
        WarnTree.NPCRole = ENarr_NPCRole::ScoutHunter;

        auto MakeLine = [](FString Speaker, FString Text, ENarr_DialogueState State, float Dur) -> FNarr_DialogueLine
        {
            FNarr_DialogueLine L;
            L.SpeakerID = Speaker;
            L.LineText = Text;
            L.TriggerState = State;
            L.DisplayDuration = Dur;
            return L;
        };

        WarnTree.Lines.Add(MakeLine(
            TEXT("Scout"),
            TEXT("Tracks. Fresh. Something big came through here last night."),
            ENarr_DialogueState::Warning, 3.5f));

        WarnTree.Lines.Add(MakeLine(
            TEXT("Scout"),
            TEXT("See how deep the print is? That is not a raptor. That is something much heavier."),
            ENarr_DialogueState::Warning, 4.5f));

        WarnTree.Lines.Add(MakeLine(
            TEXT("Scout"),
            TEXT("We go around. The long way. I am not dying today."),
            ENarr_DialogueState::Warning, 3.5f));

        DialogueTrees.Add(WarnTree);
    }
}

void UNarr_DialogueComponent::BuildGathererDialogue()
{
    // === GREETING TREE ===
    {
        FNarr_DialogueTree GreetTree;
        GreetTree.TreeID = TEXT("Gatherer_Greeting");
        GreetTree.NPCRole = ENarr_NPCRole::Gatherer;

        auto MakeLine = [](FString Speaker, FString Text, ENarr_DialogueState State, float Dur) -> FNarr_DialogueLine
        {
            FNarr_DialogueLine L;
            L.SpeakerID = Speaker;
            L.LineText = Text;
            L.TriggerState = State;
            L.DisplayDuration = Dur;
            return L;
        };

        GreetTree.Lines.Add(MakeLine(
            TEXT("Gatherer"),
            TEXT("The berry bushes near the stream — they are ripe. But the ground is soft. Prints everywhere."),
            ENarr_DialogueState::Greeting, 5.0f));

        GreetTree.Lines.Add(MakeLine(
            TEXT("Gatherer"),
            TEXT("I filled half my bag before I heard it breathing. Did not look back. Just ran."),
            ENarr_DialogueState::Greeting, 4.5f));

        GreetTree.Lines.Add(MakeLine(
            TEXT("Gatherer"),
            TEXT("If you go — take a spear. And do not go alone."),
            ENarr_DialogueState::Greeting, 3.5f));

        DialogueTrees.Add(GreetTree);
    }
}
