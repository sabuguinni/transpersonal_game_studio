// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue
// Implementation of dialogue trigger zones for prehistoric survival NPCs

#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;

    TriggerRadius = 350.0f;
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    bDialogueActive = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnPlayerEnterRange);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnPlayerExitRange);
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDialogueActive) return;
    if (DialogueSequence.Lines.Num() == 0) return;
    if (CurrentLineIndex >= DialogueSequence.Lines.Num()) return;

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        AdvanceLineInternal();
    }
}

void ANarr_DialogueTrigger::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    TriggerDialogue(OtherActor);
}

void ANarr_DialogueTrigger::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Dialogue continues even if player walks away — realistic behaviour
    // Only stop if sequence is repeatable and player leaves mid-sequence
    if (DialogueSequence.bRepeatable && bDialogueActive)
    {
        // Let current line finish, then stop
        bDialogueActive = false;
    }
}

void ANarr_DialogueTrigger::TriggerDialogue(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    // Don't replay non-repeatable sequences
    if (DialogueSequence.bHasBeenPlayed && !DialogueSequence.bRepeatable) return;
    if (bDialogueActive) return;
    if (DialogueSequence.Lines.Num() == 0) return;

    bDialogueActive = true;
    CurrentLineIndex = 0;

    // Set timer for first line
    const FNarr_DialogueLine& FirstLine = DialogueSequence.Lines[0];
    LineTimer = FirstLine.DisplayDuration > 0.0f ? FirstLine.DisplayDuration : 4.0f;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue started: %s — '%s'"),
        *DialogueSequence.SequenceID.ToString(),
        *FirstLine.SpeakerName);
}

void ANarr_DialogueTrigger::AdvanceLine()
{
    AdvanceLineInternal();
}

void ANarr_DialogueTrigger::AdvanceLineInternal()
{
    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueSequence.Lines.Num())
    {
        // Sequence complete
        bDialogueActive = false;
        DialogueSequence.bHasBeenPlayed = true;
        CurrentLineIndex = 0;
        LineTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue sequence complete: %s"),
            *DialogueSequence.SequenceID.ToString());
        return;
    }

    const FNarr_DialogueLine& NextLine = DialogueSequence.Lines[CurrentLineIndex];
    LineTimer = NextLine.DisplayDuration > 0.0f ? NextLine.DisplayDuration : 4.0f;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Line %d: %s — '%s'"),
        CurrentLineIndex,
        *NextLine.SpeakerName,
        *NextLine.LineText);
}

bool ANarr_DialogueTrigger::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueLine ANarr_DialogueTrigger::GetCurrentLine() const
{
    if (!bDialogueActive || DialogueSequence.Lines.Num() == 0) return FNarr_DialogueLine();
    if (CurrentLineIndex < 0 || CurrentLineIndex >= DialogueSequence.Lines.Num()) return FNarr_DialogueLine();
    return DialogueSequence.Lines[CurrentLineIndex];
}

void ANarr_DialogueTrigger::ResetDialogue()
{
    bDialogueActive = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    DialogueSequence.bHasBeenPlayed = false;
}
