// DialogueSystem.cpp
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260622_002
// Implements tree-based NPC dialogue for prehistoric survival game

#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    NPCRole = ENarr_NPCRole::Elder;
    NPCName = TEXT("Elder");
    InteractionRadius = 300.0f;
    bIsInDialogue = false;
    CurrentLineIndex = 0;
    ActiveTreeIndex = INDEX_NONE;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UNarr_DialogueComponent::InitializeDefaultDialogues()
{
    // Only populate if no designer-authored trees exist
    if (DialogueTrees.Num() > 0)
    {
        return;
    }

    // Default dialogue tree based on NPC role
    FNarr_DialogueTree DefaultTree;

    switch (NPCRole)
    {
        case ENarr_NPCRole::Elder:
        {
            DefaultTree.DialogueID = FName("Elder_Greeting");
            DefaultTree.State = ENarr_DialogueState::Idle;
            DefaultTree.LinkedQuestID = FName("Quest_HuntTRex");

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = NPCName;
            Line1.LineText = TEXT("Stranger. You carry yourself like someone who has faced the great beasts and lived.");
            Line1.DisplayDuration = 5.0f;
            DefaultTree.Lines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = NPCName;
            Line2.LineText = TEXT("The predator with the terrible jaws — it has taken three of our hunters this season. We cannot move camp. Our children cannot sleep.");
            Line2.DisplayDuration = 6.0f;
            DefaultTree.Lines.Add(Line2);

            FNarr_DialogueLine Line3;
            Line3.SpeakerName = NPCName;
            Line3.LineText = TEXT("Track it to its territory. Drive it away from our water source. Do this, and our tribe shares fire, food, and shelter with you.");
            Line3.DisplayDuration = 6.0f;
            Line3.bRequiresPlayerResponse = true;
            DefaultTree.Lines.Add(Line3);

            break;
        }

        case ENarr_NPCRole::Hunter:
        {
            DefaultTree.DialogueID = FName("Hunter_Warning");
            DefaultTree.State = ENarr_DialogueState::Idle;

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = NPCName;
            Line1.LineText = TEXT("Do not go east. The pack hunters move in groups of three. They flank. They wait. They are not stupid.");
            Line1.DisplayDuration = 5.0f;
            DefaultTree.Lines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = NPCName;
            Line2.LineText = TEXT("If you must go — make noise. Lots of noise. Confuse their formation. Then run for the river. They hate deep water.");
            Line2.DisplayDuration = 5.0f;
            DefaultTree.Lines.Add(Line2);

            break;
        }

        case ENarr_NPCRole::Scout:
        {
            DefaultTree.DialogueID = FName("Scout_Report");
            DefaultTree.State = ENarr_DialogueState::Idle;

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = NPCName;
            Line1.LineText = TEXT("I tracked the herd migration north. The long-necks are moving. That means the big predators follow.");
            Line1.DisplayDuration = 5.0f;
            DefaultTree.Lines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = NPCName;
            Line2.LineText = TEXT("Three days. Maybe four. Then this valley will not be safe for any of us.");
            Line2.DisplayDuration = 4.0f;
            DefaultTree.Lines.Add(Line2);

            break;
        }

        case ENarr_NPCRole::Merchant:
        {
            DefaultTree.DialogueID = FName("Merchant_Trade");
            DefaultTree.State = ENarr_DialogueState::Idle;

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = NPCName;
            Line1.LineText = TEXT("I have flint tools, dried meat, and rope made from river reeds. What do you carry?");
            Line1.DisplayDuration = 4.0f;
            Line1.bRequiresPlayerResponse = true;
            DefaultTree.Lines.Add(Line1);

            break;
        }

        default:
        {
            DefaultTree.DialogueID = FName("Generic_Greeting");
            DefaultTree.State = ENarr_DialogueState::Idle;

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = NPCName;
            Line1.LineText = TEXT("Stay close to the fire at night. The darkness here has teeth.");
            Line1.DisplayDuration = 4.0f;
            DefaultTree.Lines.Add(Line1);

            break;
        }
    }

    DialogueTrees.Add(DefaultTree);
}

bool UNarr_DialogueComponent::StartDialogue(FName DialogueID)
{
    if (bIsInDialogue)
    {
        return false;
    }

    for (int32 i = 0; i < DialogueTrees.Num(); ++i)
    {
        if (DialogueTrees[i].DialogueID == DialogueID)
        {
            if (DialogueTrees[i].State == ENarr_DialogueState::Locked)
            {
                return false;
            }

            ActiveTreeIndex = i;
            CurrentLineIndex = 0;
            bIsInDialogue = true;
            DialogueTrees[i].State = ENarr_DialogueState::Active;
            return true;
        }
    }

    return false;
}

bool UNarr_DialogueComponent::AdvanceLine()
{
    if (!bIsInDialogue || ActiveTreeIndex == INDEX_NONE)
    {
        return false;
    }

    const FNarr_DialogueTree& ActiveTree = DialogueTrees[ActiveTreeIndex];
    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveTree.Lines.Num())
    {
        EndDialogue();
        return false;
    }

    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    if (ActiveTreeIndex != INDEX_NONE && ActiveTreeIndex < DialogueTrees.Num())
    {
        DialogueTrees[ActiveTreeIndex].State = ENarr_DialogueState::Completed;
    }

    bIsInDialogue = false;
    CurrentLineIndex = 0;
    ActiveTreeIndex = INDEX_NONE;
}

FString UNarr_DialogueComponent::GetCurrentLineText() const
{
    if (!bIsInDialogue || ActiveTreeIndex == INDEX_NONE)
    {
        return FString();
    }

    const FNarr_DialogueTree& ActiveTree = DialogueTrees[ActiveTreeIndex];
    if (CurrentLineIndex >= 0 && CurrentLineIndex < ActiveTree.Lines.Num())
    {
        return ActiveTree.Lines[CurrentLineIndex].LineText;
    }

    return FString();
}

FString UNarr_DialogueComponent::GetCurrentSpeakerName() const
{
    if (!bIsInDialogue || ActiveTreeIndex == INDEX_NONE)
    {
        return FString();
    }

    const FNarr_DialogueTree& ActiveTree = DialogueTrees[ActiveTreeIndex];
    if (CurrentLineIndex >= 0 && CurrentLineIndex < ActiveTree.Lines.Num())
    {
        return ActiveTree.Lines[CurrentLineIndex].SpeakerName;
    }

    return NPCName;
}

bool UNarr_DialogueComponent::IsPlayerInRange(AActor* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    const float DistSq = FVector::DistSquared(Player->GetActorLocation(), GetOwner()->GetActorLocation());
    return DistSq <= FMath::Square(InteractionRadius);
}

void UNarr_DialogueComponent::MarkDialogueCompleted(FName DialogueID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            Tree.State = ENarr_DialogueState::Completed;
            return;
        }
    }
}
