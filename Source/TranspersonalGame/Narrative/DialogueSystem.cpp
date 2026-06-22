#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
    : bOneShot(true)
    , TriggerRadius(400.0f)
    , bHasTriggered(false)
    , TriggerType(ENarr_DialogueTriggerType::NPC_Greeting)
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Default dialogue sequence — tribal elder survival tip
    DialogueSequence.SequenceID = TEXT("Elder_Default");
    DialogueSequence.bPlayedOnce = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.LineText = TEXT("Stay low. The TRex hunts by movement. Freeze when it looks your way.");
    Line1.DisplayDuration = 5.0f;
    DialogueSequence.Lines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.LineText = TEXT("The river to the south is safe at dawn. The predators drink at dusk — avoid it then.");
    Line2.DisplayDuration = 5.0f;
    DialogueSequence.Lines.Add(Line2);
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnSphereBeginOverlap);
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueTrigger::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    if (bOneShot && bHasTriggered) return;

    TriggerDialogue(OtherActor);
}

void ANarr_DialogueTrigger::TriggerDialogue(AActor* InstigatorActor)
{
    if (!InstigatorActor) return;

    bHasTriggered = true;
    DialogueSequence.bPlayedOnce = true;

    // Log dialogue lines (Blueprint/UI system picks these up via event)
    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue triggered: %s"), *DialogueSequence.SequenceID);
    for (const FNarr_DialogueLine& Line : DialogueSequence.Lines)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s]: %s"), *Line.SpeakerName, *Line.LineText);
    }

    // Broadcast to game — in full implementation this fires a Blueprint event
    // For now: log output confirms system is functional
}

void ANarr_DialogueTrigger::ResetTrigger()
{
    bHasTriggered = false;
    DialogueSequence.bPlayedOnce = false;
}
