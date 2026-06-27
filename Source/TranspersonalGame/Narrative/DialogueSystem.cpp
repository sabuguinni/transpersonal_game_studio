#include "DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

// ============================================================
// ANarr_DialogueTriggerActor — Implementation
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DialogueZone = ENarr_DialogueZone::None;
    TriggerRadius = 300.0f;
    bOneShot = true;
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    bAutoAdvance = true;
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
}

void ANarr_DialogueTriggerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ENarr_DialogueState::Playing && bAutoAdvance)
    {
        LineTimer -= DeltaTime;
        if (LineTimer <= 0.0f)
        {
            AdvanceDialogue();
        }
    }
}

void ANarr_DialogueTriggerActor::TriggerDialogue()
{
    if (bOneShot && DialogueTree.bHasBeenTriggered)
    {
        return;
    }

    if (DialogueTree.Lines.Num() == 0)
    {
        return;
    }

    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Playing;
    DialogueTree.bHasBeenTriggered = true;

    // Set timer for first line
    const FNarr_DialogueLine& FirstLine = DialogueTree.Lines[0];
    LineTimer = FirstLine.DisplayDuration;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue triggered for zone: %d | Speaker: %s"),
        (int32)DialogueZone, *FirstLine.SpeakerName);
}

void ANarr_DialogueTriggerActor::AdvanceDialogue()
{
    if (CurrentState != ENarr_DialogueState::Playing)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueTree.Lines.Num())
    {
        // Dialogue complete
        CurrentState = ENarr_DialogueState::Completed;
        CurrentLineIndex = DialogueTree.Lines.Num() - 1;
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue completed for zone: %d"), (int32)DialogueZone);
        return;
    }

    const FNarr_DialogueLine& NextLine = DialogueTree.Lines[CurrentLineIndex];
    LineTimer = NextLine.DisplayDuration;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue line %d: %s — %s"),
        CurrentLineIndex, *NextLine.SpeakerName, *NextLine.DialogueText.ToString());
}

void ANarr_DialogueTriggerActor::ResetDialogue()
{
    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Idle;
    LineTimer = 0.0f;
    DialogueTree.bHasBeenTriggered = false;
}

FNarr_DialogueLine ANarr_DialogueTriggerActor::GetCurrentLine() const
{
    if (DialogueTree.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueTree.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool ANarr_DialogueTriggerActor::IsDialogueActive() const
{
    return CurrentState == ENarr_DialogueState::Playing;
}

// ============================================================
// ANarr_DialogueManager — Implementation
// ============================================================

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
    ActiveZone = ENarr_DialogueZone::None;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogueTrees();
}

void ANarr_DialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Remove existing tree for this zone if present
    AllDialogueTrees.RemoveAll([&Tree](const FNarr_DialogueTree& Existing)
    {
        return Existing.Zone == Tree.Zone;
    });
    AllDialogueTrees.Add(Tree);
    UE_LOG(LogTemp, Log, TEXT("[Narrative] Registered dialogue tree for zone: %d with %d lines"),
        (int32)Tree.Zone, Tree.Lines.Num());
}

FNarr_DialogueTree ANarr_DialogueManager::GetDialogueTreeForZone(ENarr_DialogueZone Zone) const
{
    for (const FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.Zone == Zone)
        {
            return Tree;
        }
    }
    return FNarr_DialogueTree();
}

void ANarr_DialogueManager::MarkZoneTriggered(ENarr_DialogueZone Zone)
{
    for (FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.Zone == Zone)
        {
            Tree.bHasBeenTriggered = true;
            ActiveZone = Zone;
            UE_LOG(LogTemp, Log, TEXT("[Narrative] Zone triggered: %d"), (int32)Zone);
            return;
        }
    }
}

bool ANarr_DialogueManager::HasZoneBeenTriggered(ENarr_DialogueZone Zone) const
{
    for (const FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.Zone == Zone)
        {
            return Tree.bHasBeenTriggered;
        }
    }
    return false;
}

void ANarr_DialogueManager::UpdateObjectiveProgress(const FString& ObjectiveID, int32 Delta)
{
    for (FNarr_QuestObjective& Obj : TutorialObjectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + Delta, 0, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[Narrative] Objective COMPLETED: %s"), *ObjectiveID);
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("[Narrative] Objective progress: %s [%d/%d]"),
                    *ObjectiveID, Obj.CurrentCount, Obj.RequiredCount);
            }
            return;
        }
    }
}

void ANarr_DialogueManager::InitializeDefaultDialogueTrees()
{
    AllDialogueTrees.Empty();

    // ---- RIVER CROSSING DIALOGUE ----
    {
        FNarr_DialogueTree RiverTree;
        RiverTree.Zone = ENarr_DialogueZone::RiverCrossing;
        RiverTree.LinkedQuestID = TEXT("QUEST_RIVER_CROSSING");

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Elder");
        Line1.DialogueText = FText::FromString(TEXT("The river crossing is dangerous. Triceratops move through at dawn."));
        Line1.DisplayDuration = 6.0f;
        Line1.SpeakerType = ENarr_NPCType::Elder;
        RiverTree.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Elder");
        Line2.DialogueText = FText::FromString(TEXT("Stay low, move slow, and wait for the gap between the adults. The calves follow last."));
        Line2.DisplayDuration = 6.0f;
        Line2.SpeakerType = ENarr_NPCType::Elder;
        RiverTree.Lines.Add(Line2);

        FNarr_DialogueLine Line3;
        Line3.SpeakerName = TEXT("Elder");
        Line3.DialogueText = FText::FromString(TEXT("That is your window. Move fast. Do not hesitate."));
        Line3.DisplayDuration = 5.0f;
        Line3.SpeakerType = ENarr_NPCType::Elder;
        RiverTree.Lines.Add(Line3);

        RegisterDialogueTree(RiverTree);
    }

    // ---- RAPTOR TERRITORY DIALOGUE ----
    {
        FNarr_DialogueTree RaptorTree;
        RaptorTree.Zone = ENarr_DialogueZone::RaptorTerritory;
        RaptorTree.LinkedQuestID = TEXT("QUEST_RAPTOR_TERRITORY");

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Scout");
        Line1.DialogueText = FText::FromString(TEXT("Raptors. Three of them, maybe four. They have been circling this ridge since yesterday."));
        Line1.DisplayDuration = 6.0f;
        Line1.SpeakerType = ENarr_NPCType::Scout;
        RaptorTree.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Scout");
        Line2.DialogueText = FText::FromString(TEXT("Do not run — they chase runners. Find a tree, get high, and wait."));
        Line2.DisplayDuration = 6.0f;
        Line2.SpeakerType = ENarr_NPCType::Scout;
        RaptorTree.Lines.Add(Line2);

        FNarr_DialogueLine Line3;
        Line3.SpeakerName = TEXT("Scout");
        Line3.DialogueText = FText::FromString(TEXT("They lose interest when the prey disappears. We learned this the hard way."));
        Line3.DisplayDuration = 5.0f;
        Line3.SpeakerType = ENarr_NPCType::Scout;
        RaptorTree.Lines.Add(Line3);

        RegisterDialogueTree(RaptorTree);
    }

    // ---- CRAFTING CAMP DIALOGUE ----
    {
        FNarr_DialogueTree CraftTree;
        CraftTree.Zone = ENarr_DialogueZone::CraftingCamp;
        CraftTree.LinkedQuestID = TEXT("QUEST_CRAFTING_TUTORIAL");

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Elder");
        Line1.DialogueText = FText::FromString(TEXT("You want to make a stone axe? Good. You will need it."));
        Line1.DisplayDuration = 5.0f;
        Line1.SpeakerType = ENarr_NPCType::Elder;
        CraftTree.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Elder");
        Line2.DialogueText = FText::FromString(TEXT("Find two flat rocks from the riverbed — the grey ones, not the red. And a dry branch, no longer than your arm."));
        Line2.DisplayDuration = 7.0f;
        Line2.SpeakerType = ENarr_NPCType::Elder;
        CraftTree.Lines.Add(Line2);

        FNarr_DialogueLine Line3;
        Line3.SpeakerName = TEXT("Elder");
        Line3.DialogueText = FText::FromString(TEXT("Bring them back here. I will show you how our people have done this for ten thousand years."));
        Line3.DisplayDuration = 6.0f;
        Line3.SpeakerType = ENarr_NPCType::Elder;
        CraftTree.Lines.Add(Line3);

        RegisterDialogueTree(CraftTree);

        // Tutorial objectives for crafting quest
        FNarr_QuestObjective Obj1;
        Obj1.ObjectiveID = TEXT("COLLECT_ROCKS");
        Obj1.ObjectiveText = FText::FromString(TEXT("Collect 2 rocks from the riverbed"));
        Obj1.HintText = FText::FromString(TEXT("Look for grey flat rocks near the river. Press E to pick up."));
        Obj1.RequiredCount = 2;
        TutorialObjectives.Add(Obj1);

        FNarr_QuestObjective Obj2;
        Obj2.ObjectiveID = TEXT("COLLECT_BRANCH");
        Obj2.ObjectiveText = FText::FromString(TEXT("Find a dry branch"));
        Obj2.HintText = FText::FromString(TEXT("Dead trees drop branches. Look for fallen wood."));
        Obj2.RequiredCount = 1;
        TutorialObjectives.Add(Obj2);

        FNarr_QuestObjective Obj3;
        Obj3.ObjectiveID = TEXT("CRAFT_STONE_AXE");
        Obj3.ObjectiveText = FText::FromString(TEXT("Craft a Stone Axe at the camp"));
        Obj3.HintText = FText::FromString(TEXT("Return to the Elder with your materials. Open crafting with C."));
        Obj3.RequiredCount = 1;
        TutorialObjectives.Add(Obj3);
    }

    // ---- HERD MIGRATION DIALOGUE ----
    {
        FNarr_DialogueTree HerdTree;
        HerdTree.Zone = ENarr_DialogueZone::HerdMigration;
        HerdTree.LinkedQuestID = TEXT("QUEST_HERD_MIGRATION");

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Scout");
        Line1.DialogueText = FText::FromString(TEXT("The herd moves south before the cold season. Hundreds of them — Triceratops, Edmontosaurus, Parasaurolophus."));
        Line1.DisplayDuration = 7.0f;
        Line1.SpeakerType = ENarr_NPCType::Scout;
        HerdTree.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Scout");
        Line2.DialogueText = FText::FromString(TEXT("If you follow the herd, you follow the food. But where the prey gathers, the predators follow."));
        Line2.DisplayDuration = 7.0f;
        Line2.SpeakerType = ENarr_NPCType::Scout;
        HerdTree.Lines.Add(Line2);

        FNarr_DialogueLine Line3;
        Line3.SpeakerName = TEXT("Scout");
        Line3.DialogueText = FText::FromString(TEXT("Keep your spear ready and never sleep without a fire."));
        Line3.DisplayDuration = 5.0f;
        Line3.SpeakerType = ENarr_NPCType::Scout;
        HerdTree.Lines.Add(Line3);

        RegisterDialogueTree(HerdTree);
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] DialogueManager initialized: %d trees loaded"), AllDialogueTrees.Num());
}
