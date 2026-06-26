#include "NarrativeDialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ============================================================
// ANarr_DialogueTrigger — Constructor
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    TriggerRadius = 500.0f;
    bDebugDraw = false;
    CurrentLineIndex = 0;
    bIsPlaying = false;
    LineTimer = 0.0f;

    // Default dialogue sequence — TRex encounter warning
    DialogueSequence.SequenceID = TEXT("DEFAULT_ENCOUNTER");
    DialogueSequence.TriggerType = ENarr_DialogueTriggerType::DinoEncounter;
    DialogueSequence.bCanRepeat = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Scout Mira");
    Line1.SpeakerRole = ENarr_SpeakerRole::Scout;
    Line1.LineText = TEXT("Danger! Stay low. Do not run — running triggers the hunt.");
    Line1.DisplayDuration = 5.0f;
    DialogueSequence.Lines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Hunter Brak");
    Line2.SpeakerRole = ENarr_SpeakerRole::Hunter;
    Line2.LineText = TEXT("Find cover. A boulder, a tree — anything between you and it.");
    Line2.DisplayDuration = 5.0f;
    DialogueSequence.Lines.Add(Line2);
}

// ============================================================
// BeginPlay
// ============================================================

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnPlayerEnterRadius);
    }
}

// ============================================================
// Tick — advance line timer
// ============================================================

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPlaying && DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        LineTimer -= DeltaTime;
        if (LineTimer <= 0.0f)
        {
            PlayNextLine();
        }
    }

    if (bDebugDraw)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), TriggerRadius, 16,
            FColor::Yellow, false, -1.0f, 0, 2.0f);
    }
}

// ============================================================
// OnPlayerEnterRadius — overlap callback
// ============================================================

void ANarr_DialogueTrigger::OnPlayerEnterRadius(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player characters
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    TriggerDialogue(OtherActor);
}

// ============================================================
// TriggerDialogue — start playback if not already played
// ============================================================

void ANarr_DialogueTrigger::TriggerDialogue(AActor* PlayerActor)
{
    if (DialogueSequence.bHasPlayed && !DialogueSequence.bCanRepeat)
    {
        return;
    }

    if (DialogueSequence.Lines.Num() == 0)
    {
        return;
    }

    CurrentLineIndex = 0;
    bIsPlaying = true;
    DialogueSequence.bHasPlayed = true;

    // Start timer for first line
    if (DialogueSequence.Lines.IsValidIndex(0))
    {
        LineTimer = DialogueSequence.Lines[0].DisplayDuration;
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue started: %s — '%s'"),
            *DialogueSequence.Lines[0].SpeakerName,
            *DialogueSequence.Lines[0].LineText);
    }
}

// ============================================================
// PlayNextLine — advance to next line or end sequence
// ============================================================

void ANarr_DialogueTrigger::PlayNextLine()
{
    CurrentLineIndex++;

    if (!DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        // Sequence complete
        bIsPlaying = false;
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue sequence complete: %s"),
            *DialogueSequence.SequenceID);
        return;
    }

    const FNarr_DialogueLine& Line = DialogueSequence.Lines[CurrentLineIndex];
    LineTimer = Line.DisplayDuration;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Line %d — %s: '%s'"),
        CurrentLineIndex,
        *Line.SpeakerName,
        *Line.LineText);
}

// ============================================================
// IsSequenceComplete
// ============================================================

bool ANarr_DialogueTrigger::IsSequenceComplete() const
{
    return !bIsPlaying && DialogueSequence.bHasPlayed;
}

// ============================================================
// ResetSequence — allow replay
// ============================================================

void ANarr_DialogueTrigger::ResetSequence()
{
    CurrentLineIndex = 0;
    bIsPlaying = false;
    LineTimer = 0.0f;
    DialogueSequence.bHasPlayed = false;
}

// ============================================================
// GetCurrentLine — returns the active dialogue line
// ============================================================

FNarr_DialogueLine ANarr_DialogueTrigger::GetCurrentLine() const
{
    if (DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueSequence.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}
