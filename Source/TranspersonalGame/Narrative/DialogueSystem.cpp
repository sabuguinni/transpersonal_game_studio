#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

ANarr_DialogueZone::ANarr_DialogueZone()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(400.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
    EditorIcon->SetupAttachment(RootComponent);

    // Default dialogue sequence — river crossing warning
    DialogueSequence.SequenceID = TEXT("DefaultDialogue");
    DialogueSequence.TriggerType = ENarr_DialogueTriggerType::ProximityEnter;
    DialogueSequence.bPlayOnce = true;
    DialogueSequence.bHasPlayed = false;

    FNarr_DialogueLine DefaultLine;
    DefaultLine.SpeakerName = TEXT("Tribal Leader");
    DefaultLine.SpeakerRole = ENarr_SpeakerRole::TribalLeader;
    DefaultLine.DialogueText = TEXT("The river crossing is flooded. Move west.");
    DefaultLine.DisplayDuration = 5.0f;
    DefaultLine.bBlockPlayerMovement = false;
    DialogueSequence.Lines.Add(DefaultLine);
}

void ANarr_DialogueZone::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueZone::OnSphereBeginOverlap);
    }
}

void ANarr_DialogueZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Debug sphere visualization in editor
    if (bShowDebugSphere)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), TriggerRadius, 16,
            FColor::Yellow, false, -1.0f, 0, 2.0f);
    }

    // Advance dialogue line timer
    if (bIsPlaying && DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        LineTimer += DeltaTime;
        float Duration = DialogueSequence.Lines[CurrentLineIndex].DisplayDuration;
        if (LineTimer >= Duration)
        {
            LineTimer = 0.0f;
            CurrentLineIndex++;
            if (CurrentLineIndex >= DialogueSequence.Lines.Num())
            {
                bIsPlaying = false;
                DialogueSequence.bHasPlayed = true;
                CurrentLineIndex = 0;
            }
        }
    }
}

void ANarr_DialogueZone::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    TriggerDialogue();
}

void ANarr_DialogueZone::TriggerDialogue()
{
    // Respect play-once flag
    if (DialogueSequence.bPlayOnce && DialogueSequence.bHasPlayed) return;
    if (DialogueSequence.Lines.Num() == 0) return;
    if (bIsPlaying) return;

    bIsPlaying = true;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[DialogueZone] Triggered: %s — Speaker: %s — Lines: %d"),
        *DialogueSequence.SequenceID,
        *GetCurrentSpeakerName(),
        DialogueSequence.Lines.Num());
}

bool ANarr_DialogueZone::HasPlayerEntered() const
{
    return bIsPlaying || DialogueSequence.bHasPlayed;
}

void ANarr_DialogueZone::ResetDialogue()
{
    DialogueSequence.bHasPlayed = false;
    bIsPlaying = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

FString ANarr_DialogueZone::GetCurrentSpeakerName() const
{
    if (DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueSequence.Lines[CurrentLineIndex].SpeakerName;
    }
    return TEXT("Unknown");
}

int32 ANarr_DialogueZone::GetLineCount() const
{
    return DialogueSequence.Lines.Num();
}
