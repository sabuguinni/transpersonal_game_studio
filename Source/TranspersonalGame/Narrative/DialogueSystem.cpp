// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Implementation of proximity-based dialogue trigger system

#include "DialogueSystem.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
    : bOneShot(true)
    , TriggerRadius(500.0f)
    , bHasBeenTriggered(false)
    , CurrentLineIndex(0)
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(TriggerRadius, TriggerRadius, 200.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerVolume->SetGenerateOverlapEvents(true);

    TriggerType = ENarr_DialogueTriggerType::DinosaurEncounter;

    // Default dialogue lines for a T-Rex encounter
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Kael");
    Line1.LineText = TEXT("T-Rex tracks — fresh. Move downwind. Now.");
    Line1.DisplayDuration = 4.0f;
    Line1.TriggerType = ENarr_DialogueTriggerType::DinosaurEncounter;
    DialogueLines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Mara");
    Line2.LineText = TEXT("Stay low. It hunts by movement. Don't run.");
    Line2.DisplayDuration = 4.5f;
    Line2.TriggerType = ENarr_DialogueTriggerType::DinosaurEncounter;
    DialogueLines.Add(Line2);
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(
            this, &ANarr_DialogueTriggerActor::OnTriggerBeginOverlap
        );
    }
}

void ANarr_DialogueTriggerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueTriggerActor::OnTriggerBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player characters
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Check if this is the locally controlled player
    if (!PlayerChar->IsPlayerControlled()) return;

    ActivateDialogue(OtherActor);
}

void ANarr_DialogueTriggerActor::ActivateDialogue(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    if (bOneShot && bHasBeenTriggered) return;
    if (DialogueLines.Num() == 0) return;

    bHasBeenTriggered = true;
    CurrentLineIndex = 0;

    // Log the dialogue activation for debugging
    UE_LOG(LogTemp, Display, TEXT("[Narrative] Dialogue triggered: %s — %s"),
        *DialogueLines[0].SpeakerName,
        *DialogueLines[0].LineText);
}

void ANarr_DialogueTriggerActor::ResetTrigger()
{
    bHasBeenTriggered = false;
    CurrentLineIndex = 0;
}

FNarr_DialogueLine ANarr_DialogueTriggerActor::GetCurrentLine() const
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueLines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}
