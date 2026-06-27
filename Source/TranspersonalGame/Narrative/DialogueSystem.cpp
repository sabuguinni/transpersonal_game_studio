// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260627_008

#include "DialogueSystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ── Constructor ──────────────────────────────────────────────────────────────

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root trigger volume
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(TriggerRadius, TriggerRadius, 200.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    TriggerVolume->SetGenerateOverlapEvents(true);

    // Defaults
    NPCName = TEXT("Unknown NPC");
    NPCRole = ENarr_NPCRole::Scout;
    CurrentState = ENarr_DialogueTriggerState::Idle;
    CurrentLineIndex = 0;
    bHasBeenTriggered = false;
    CooldownTimer = 0.0f;
    LineTimer = 0.0f;
    bPlayerInsideTrigger = false;
}

// ── BeginPlay ────────────────────────────────────────────────────────────────

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_DialogueTrigger::OnTriggerBeginOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(
        this, &ANarr_DialogueTrigger::OnTriggerEndOverlap);

    // Update box extent from TriggerRadius
    TriggerVolume->SetBoxExtent(FVector(TriggerRadius, TriggerRadius, 200.0f));
}

// ── Tick ─────────────────────────────────────────────────────────────────────

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CurrentState)
    {
    case ENarr_DialogueTriggerState::Playing:
    {
        LineTimer -= DeltaTime;
        if (LineTimer <= 0.0f)
        {
            AdvanceLine();
        }
        break;
    }
    case ENarr_DialogueTriggerState::Cooldown:
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.0f)
        {
            CurrentState = ENarr_DialogueTriggerState::Idle;
            // If player still inside, re-trigger (for non-one-shot sequences)
            if (bPlayerInsideTrigger && !DialogueSequence.bPlayOnce)
            {
                StartDialogue();
            }
        }
        break;
    }
    default:
        break;
    }
}

// ── StartDialogue ────────────────────────────────────────────────────────────

void ANarr_DialogueTrigger::StartDialogue()
{
    if (DialogueSequence.Lines.Num() == 0)
    {
        return;
    }

    if (DialogueSequence.bPlayOnce && bHasBeenTriggered)
    {
        return;
    }

    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueTriggerState::Playing;
    bHasBeenTriggered = true;

    const FNarr_DialogueLine& FirstLine = DialogueSequence.Lines[0];
    LineTimer = FirstLine.DisplayDuration;

    OnDialogueStarted(FirstLine);
}

// ── AdvanceLine ──────────────────────────────────────────────────────────────

void ANarr_DialogueTrigger::AdvanceLine()
{
    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueSequence.Lines.Num())
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueLine& NextLine = DialogueSequence.Lines[CurrentLineIndex];
    LineTimer = NextLine.DisplayDuration;

    OnDialogueLineChanged(NextLine, CurrentLineIndex);
}

// ── EndDialogue ──────────────────────────────────────────────────────────────

void ANarr_DialogueTrigger::EndDialogue()
{
    CurrentState = ENarr_DialogueTriggerState::Cooldown;
    CooldownTimer = DialogueSequence.CooldownSeconds;
    CurrentLineIndex = 0;

    OnDialogueEnded();
}

// ── IsPlayerInRange ──────────────────────────────────────────────────────────

bool ANarr_DialogueTrigger::IsPlayerInRange() const
{
    return bPlayerInsideTrigger;
}

// ── GetCurrentLine ───────────────────────────────────────────────────────────

FNarr_DialogueLine ANarr_DialogueTrigger::GetCurrentLine() const
{
    if (DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueSequence.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

// ── Overlap Callbacks ────────────────────────────────────────────────────────

void ANarr_DialogueTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Only react to player character
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
    {
        return;
    }

    // Check it's the local player
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character || !Character->IsPlayerControlled())
    {
        return;
    }

    bPlayerInsideTrigger = true;
    CurrentState = ENarr_DialogueTriggerState::PlayerNearby;

    // Auto-start dialogue when player enters
    StartDialogue();
}

void ANarr_DialogueTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character || !Character->IsPlayerControlled())
    {
        return;
    }

    bPlayerInsideTrigger = false;

    // If dialogue is playing, end it when player leaves
    if (CurrentState == ENarr_DialogueTriggerState::Playing)
    {
        EndDialogue();
    }
    else if (CurrentState == ENarr_DialogueTriggerState::PlayerNearby)
    {
        CurrentState = ENarr_DialogueTriggerState::Idle;
    }
}
