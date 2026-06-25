#include "DialogueSystem.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    NPCName = TEXT("NPC");
    InteractionRadius = 300.0f;
    bPlayerInRange = false;
    CurrentSequenceIndex = -1;
    CurrentLineIndex = 0;
    bDialogueActive = false;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueComponent::TriggerDialogue(const FString& SequenceID)
{
    for (int32 i = 0; i < DialogueSequences.Num(); ++i)
    {
        if (DialogueSequences[i].SequenceID == SequenceID)
        {
            // If already triggered once and flagged, skip
            if (DialogueSequences[i].bTriggeredOnce)
            {
                return;
            }
            CurrentSequenceIndex = i;
            CurrentLineIndex = 0;
            bDialogueActive = true;
            DialogueSequences[i].bTriggeredOnce = true;
            UE_LOG(LogTemp, Log, TEXT("[Dialogue] Triggered sequence '%s' for NPC '%s'"),
                *SequenceID, *NPCName);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[Dialogue] Sequence '%s' not found for NPC '%s'"),
        *SequenceID, *NPCName);
}

void UNarr_DialogueComponent::SetPlayerInRange(bool bInRange)
{
    bPlayerInRange = bInRange;
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (!bDialogueActive || CurrentSequenceIndex < 0 ||
        CurrentSequenceIndex >= DialogueSequences.Num())
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueSequence& Seq = DialogueSequences[CurrentSequenceIndex];
    if (CurrentLineIndex < Seq.Lines.Num())
    {
        return Seq.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

void UNarr_DialogueComponent::AdvanceLine()
{
    if (!bDialogueActive || CurrentSequenceIndex < 0 ||
        CurrentSequenceIndex >= DialogueSequences.Num())
    {
        return;
    }

    const FNarr_DialogueSequence& Seq = DialogueSequences[CurrentSequenceIndex];
    CurrentLineIndex++;

    if (CurrentLineIndex >= Seq.Lines.Num())
    {
        // End of sequence
        bDialogueActive = false;
        CurrentSequenceIndex = -1;
        CurrentLineIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("[Dialogue] Sequence ended for NPC '%s'"), *NPCName);
    }
}

bool UNarr_DialogueComponent::HasActiveDialogue() const
{
    return bDialogueActive;
}

// ============================================================
// ANarr_DialogueTriggerActor
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;

    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueComponent>(
        TEXT("DialogueComponent"));

    TriggerDialogueID = TEXT("DefaultDialogue");
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();
}

void ANarr_DialogueTriggerActor::OnPlayerEnterRange()
{
    if (DialogueComponent)
    {
        DialogueComponent->SetPlayerInRange(true);
        DialogueComponent->TriggerDialogue(TriggerDialogueID);
        UE_LOG(LogTemp, Log, TEXT("[DialogueTrigger] Player entered range of '%s'"),
            *GetActorLabel());
    }
}

void ANarr_DialogueTriggerActor::OnPlayerExitRange()
{
    if (DialogueComponent)
    {
        DialogueComponent->SetPlayerInRange(false);
        UE_LOG(LogTemp, Log, TEXT("[DialogueTrigger] Player exited range of '%s'"),
            *GetActorLabel());
    }
}
