#include "NarrativeDialogueSystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ============================================================
// ANarr_DialogueTriggerActor — Implementation
// Agent #15 — Narrative & Dialogue System
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root trigger box
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetBoxExtent(TriggerExtent);
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetGenerateOverlapEvents(true);
    RootComponent = TriggerBox;
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();

    // Apply configured extent
    if (TriggerBox)
    {
        TriggerBox->SetBoxExtent(TriggerExtent);
        TriggerBox->OnComponentBeginOverlap.AddDynamic(
            this, &ANarr_DialogueTriggerActor::OnPlayerEntered);
    }
}

void ANarr_DialogueTriggerActor::OnPlayerEntered(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Only trigger for player characters
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar)
    {
        return;
    }

    // Respect one-shot flag
    if (bHasFired && !DialogueSequence.bRepeatable)
    {
        return;
    }

    ActivateDialogue();
}

void ANarr_DialogueTriggerActor::ActivateDialogue()
{
    if (DialogueSequence.Lines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("NarrDialogue: Trigger '%s' has no dialogue lines configured."),
               *GetActorLabel());
        return;
    }

    bHasFired = true;

    // Log each line for now — Audio Agent (#16) will hook into this
    UE_LOG(LogTemp, Log,
           TEXT("NarrDialogue: Sequence '%s' started (%d lines)"),
           *DialogueSequence.SequenceID.ToString(),
           DialogueSequence.Lines.Num());

    for (int32 i = 0; i < DialogueSequence.Lines.Num(); ++i)
    {
        const FNarr_DialogueLine& Line = DialogueSequence.Lines[i];
        UE_LOG(LogTemp, Log,
               TEXT("  [%d] %s: \"%s\" (Tone=%d, Duration=%.1fs)"),
               i,
               *Line.SpeakerName.ToString(),
               *Line.LineText.ToString(),
               (int32)Line.Tone,
               Line.DisplayDuration);
    }

    // Broadcast to Blueprint / UI layer via Actor event
    // (Blueprint subclass can override and show subtitles)
    OnActorBeginOverlap.Broadcast(this, this);
}

void ANarr_DialogueTriggerActor::ResetTrigger()
{
    bHasFired = false;
    UE_LOG(LogTemp, Log,
           TEXT("NarrDialogue: Trigger '%s' reset — ready to fire again."),
           *GetActorLabel());
}
