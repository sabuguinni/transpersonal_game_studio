#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

// ============================================================
// ANarr_DialogueTrigger — Constructor
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from editable property
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }

    // Bind overlap event
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(
        this,
        [](UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
           bool bFromSweep, const FHitResult& SweepResult)
        {
            // Overlap handled via OnPlayerEnterTrigger
        }
    );
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueTrigger::OnPlayerEnterTrigger(AActor* OverlappingActor)
{
    if (!OverlappingActor)
    {
        return;
    }

    // Only fire for player character
    ACharacter* Character = Cast<ACharacter>(OverlappingActor);
    if (!Character)
    {
        return;
    }

    // Already triggered and not repeatable — skip
    if (bTriggered && !DialogueSequence.bRepeatable)
    {
        return;
    }

    FireDialogue();
}

void ANarr_DialogueTrigger::FireDialogue()
{
    if (DialogueSequence.bHasPlayed && !DialogueSequence.bRepeatable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueTrigger [%s]: Sequence '%s' already played and is not repeatable."),
            *GetActorLabel(), *DialogueSequence.SequenceID.ToString());
        return;
    }

    bTriggered = true;
    DialogueSequence.bHasPlayed = true;

    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger [%s]: Firing sequence '%s' with %d lines. Speaker: %d"),
        *GetActorLabel(),
        *DialogueSequence.SequenceID.ToString(),
        DialogueSequence.Lines.Num(),
        (int32)SpeakerType);

    // Log each line for debug (in full game, this would drive UI + audio)
    for (int32 i = 0; i < DialogueSequence.Lines.Num(); ++i)
    {
        const FNarr_DialogueLine& Line = DialogueSequence.Lines[i];
        UE_LOG(LogTemp, Log, TEXT("  [%d] %s: \"%s\" (pause=%.1fs)"),
            i,
            *Line.SpeakerID,
            *Line.LineText,
            Line.PauseDuration);
    }
}

void ANarr_DialogueTrigger::ResetTrigger()
{
    bTriggered = false;
    DialogueSequence.bHasPlayed = false;
    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger [%s]: Reset — sequence '%s' can play again."),
        *GetActorLabel(), *DialogueSequence.SequenceID.ToString());
}

// ============================================================
// UNarr_DialogueManager
// ============================================================

UNarr_DialogueManager::UNarr_DialogueManager()
{
    // Default constructor — no world context needed at construction
}

void UNarr_DialogueManager::RegisterTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (!Trigger)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Attempted to register null trigger."));
        return;
    }

    if (!RegisteredTriggers.Contains(Trigger))
    {
        RegisteredTriggers.Add(Trigger);
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Registered trigger '%s'. Total: %d"),
            *Trigger->GetActorLabel(), RegisteredTriggers.Num());
    }
}

void UNarr_DialogueManager::UnregisterTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (!Trigger)
    {
        return;
    }

    int32 Removed = RegisteredTriggers.Remove(Trigger);
    if (Removed > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Unregistered trigger '%s'. Total: %d"),
            *Trigger->GetActorLabel(), RegisteredTriggers.Num());
    }
}

TArray<ANarr_DialogueTrigger*> UNarr_DialogueManager::GetAllTriggers() const
{
    return RegisteredTriggers;
}

bool UNarr_DialogueManager::HasSequencePlayed(FName SequenceID) const
{
    return PlayedSequenceIDs.Contains(SequenceID);
}

void UNarr_DialogueManager::MarkSequencePlayed(FName SequenceID)
{
    PlayedSequenceIDs.Add(SequenceID);
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Marked sequence '%s' as played. Total played: %d"),
        *SequenceID.ToString(), PlayedSequenceIDs.Num());
}

void UNarr_DialogueManager::ResetAllSequences()
{
    int32 Count = PlayedSequenceIDs.Num();
    PlayedSequenceIDs.Empty();

    // Reset all registered triggers too
    for (ANarr_DialogueTrigger* Trigger : RegisteredTriggers)
    {
        if (Trigger)
        {
            Trigger->ResetTrigger();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Reset %d played sequences and %d triggers."),
        Count, RegisteredTriggers.Num());
}
