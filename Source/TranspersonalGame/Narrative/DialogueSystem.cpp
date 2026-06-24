// DialogueSystem.cpp
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260624_005
// Implements: UNarr_DialogueComponent, ANarr_DialogueTrigger, UNarr_StoryManager
// 3 starter quest dialogue trees: FirstHunt, GreatLizard, SurviveNight

#include "DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bDialogueActive = false;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueComponent::StartDialogue()
{
    if (DialogueTree.Lines.Num() == 0) return;
    DialogueTree.CurrentLineIndex = 0;
    DialogueTree.bCompleted = false;
    bDialogueActive = true;
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!bDialogueActive) return false;
    if (DialogueTree.Lines.Num() == 0) return false;

    const FNarr_DialogueLine& Current = DialogueTree.Lines[DialogueTree.CurrentLineIndex];
    int32 NextIdx = Current.NextLineIndex;

    if (NextIdx < 0 || NextIdx >= DialogueTree.Lines.Num())
    {
        // End of conversation
        bDialogueActive = false;
        DialogueTree.bCompleted = true;
        return false;
    }

    DialogueTree.CurrentLineIndex = NextIdx;
    return true;
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (DialogueTree.Lines.Num() == 0 || DialogueTree.CurrentLineIndex < 0)
    {
        return FNarr_DialogueLine();
    }
    if (DialogueTree.CurrentLineIndex >= DialogueTree.Lines.Num())
    {
        return FNarr_DialogueLine();
    }
    return DialogueTree.Lines[DialogueTree.CurrentLineIndex];
}

bool UNarr_DialogueComponent::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueComponent::ResetDialogue()
{
    bDialogueActive = false;
    DialogueTree.CurrentLineIndex = 0;
    DialogueTree.bCompleted = false;
}

// ============================================================
// ANarr_DialogueTrigger
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    bHasFired = false;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void ANarr_DialogueTrigger::ActivateTrigger()
{
    if (bOneShot && bHasFired) return;
    bHasFired = true;
    // Blueprint/Audio system picks up NarrationText and AudioURL
    // to play the voice line and display subtitle
}

bool ANarr_DialogueTrigger::HasFired() const
{
    return bHasFired;
}

// ============================================================
// UNarr_StoryManager
// ============================================================

UNarr_StoryManager::UNarr_StoryManager()
{
}

void UNarr_StoryManager::InitializeStoryBeats()
{
    StoryBeats.Empty();

    // --- Beat 1: First Hunt ---
    {
        FNarr_StoryBeat Beat;
        Beat.QuestID = ENarr_QuestID::FirstHunt;
        Beat.BeatTitle = TEXT("First Hunt");
        Beat.BeatDescription = FText::FromString(
            TEXT("The Hunter has spotted three raptors on the eastern ridge. "
                 "They move in a coordinated pack — one flanks left, one right, one charges. "
                 "Use the rocks as cover. Craft a spear before engaging. "
                 "Survive the encounter to prove you belong in this tribe."));
        Beat.RequiredCraftedItem = TEXT("Spear");
        Beat.bUnlocked = true; // Available from the start
        Beat.bCompleted = false;
        StoryBeats.Add(Beat);
    }

    // --- Beat 2: The Great Lizard ---
    {
        FNarr_StoryBeat Beat;
        Beat.QuestID = ENarr_QuestID::GreatLizard;
        Beat.BeatTitle = TEXT("The Great Lizard");
        Beat.BeatDescription = FText::FromString(
            TEXT("Tracks near the river crossing. Enormous. The Elder says it is the great lizard — "
                 "the one the tribe does not name near children. "
                 "Scout its territory without being seen. "
                 "It hunts by movement. Freeze when it looks your way. "
                 "Do not engage — you are not ready. Yet."));
        Beat.RequiredCraftedItem = TEXT("StoneAxe");
        Beat.bUnlocked = false; // Unlocks after FirstHunt
        Beat.bCompleted = false;
        StoryBeats.Add(Beat);
    }

    // --- Beat 3: Survive the Night ---
    {
        FNarr_StoryBeat Beat;
        Beat.QuestID = ENarr_QuestID::SurviveNight;
        Beat.BeatTitle = TEXT("Survive the Night");
        Beat.BeatDescription = FText::FromString(
            TEXT("Night falls fast in this land. Predators grow bolder in the dark. "
                 "Craft a campfire before sundown. Gather three sticks from the forest floor. "
                 "The fire will keep the smaller hunters away. "
                 "Stay close to it until dawn. Then we move."));
        Beat.RequiredCraftedItem = TEXT("Campfire");
        Beat.bUnlocked = true; // Available from the start (parallel to FirstHunt)
        Beat.bCompleted = false;
        StoryBeats.Add(Beat);
    }
}

bool UNarr_StoryManager::UnlockBeat(ENarr_QuestID QuestID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.QuestID == QuestID)
        {
            Beat.bUnlocked = true;
            return true;
        }
    }
    return false;
}

bool UNarr_StoryManager::CompleteBeat(ENarr_QuestID QuestID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.QuestID == QuestID && Beat.bUnlocked)
        {
            Beat.bCompleted = true;

            // Chain unlock: FirstHunt completion unlocks GreatLizard
            if (QuestID == ENarr_QuestID::FirstHunt)
            {
                UnlockBeat(ENarr_QuestID::GreatLizard);
            }
            return true;
        }
    }
    return false;
}

FNarr_StoryBeat UNarr_StoryManager::GetBeat(ENarr_QuestID QuestID) const
{
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.QuestID == QuestID)
        {
            return Beat;
        }
    }
    return FNarr_StoryBeat();
}

int32 UNarr_StoryManager::GetCompletedBeatCount() const
{
    int32 Count = 0;
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.bCompleted) Count++;
    }
    return Count;
}
