// NarrativeDialogueSystem.cpp
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// Prehistoric survival game dialogue trigger implementation

#include "NarrativeDialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"

// ============================================================
// ANarr_DialogueTrigger — Constructor
// ============================================================
ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;

    InteractionRadius = 300.0f;
    bTriggerOnce = true;
    bHasTriggered = false;
    CurrentLineIndex = 0;

    TriggerSphere->SetSphereRadius(InteractionRadius);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnSphereOverlap);

    // Default NPC profile — Tribal Elder
    NPCProfile.NPCID = TEXT("elder_default");
    NPCProfile.DisplayName = TEXT("Tribal Elder");
    NPCProfile.Role = ENarr_NPCRole::TribalElder;
    NPCProfile.bHasBeenMet = false;

    // Default dialogue sequence
    FNarr_DialogueSequence DefaultSeq;
    DefaultSeq.SequenceID = TEXT("elder_intro");
    DefaultSeq.bIsRepeatable = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerID = TEXT("elder_default");
    Line1.LineText = TEXT("The eastern valley belongs to the raptors now. Three of our hunters went in at dawn. None came back.");
    Line1.Tone = ENarr_DialogueTone::Warning;
    Line1.DisplayDuration = 5.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerID = TEXT("elder_default");
    Line2.LineText = TEXT("If you want those supplies, you will need to be smarter than they were. Watch the shadows. Raptors flank.");
    Line2.Tone = ENarr_DialogueTone::Cautious;
    Line2.DisplayDuration = 5.5f;

    FNarr_DialogueLine Line3;
    Line3.SpeakerID = TEXT("elder_default");
    Line3.LineText = TEXT("The river crossing is safer. Follow the herbivore herd south — predators avoid the open water.");
    Line3.Tone = ENarr_DialogueTone::Informative;
    Line3.DisplayDuration = 5.0f;

    DefaultSeq.Lines.Add(Line1);
    DefaultSeq.Lines.Add(Line2);
    DefaultSeq.Lines.Add(Line3);

    NPCProfile.DialogueTrees.Add(DefaultSeq);
}

// ============================================================
// BeginPlay
// ============================================================
void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from property
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(InteractionRadius);
    }
}

// ============================================================
// OnSphereOverlap — auto-trigger dialogue on player proximity
// ============================================================
void ANarr_DialogueTrigger::OnSphereOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player-controlled pawns
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn || !Pawn->IsPlayerControlled()) return;

    if (bTriggerOnce && bHasTriggered) return;

    TriggerDialogue(OtherActor);
}

// ============================================================
// TriggerDialogue — begin dialogue sequence
// ============================================================
void ANarr_DialogueTrigger::TriggerDialogue(AActor* Interactor)
{
    if (!Interactor) return;

    bHasTriggered = true;
    CurrentLineIndex = 0;
    NPCProfile.bHasBeenMet = true;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue triggered: NPC=%s Interactor=%s"),
        *NPCProfile.DisplayName,
        *Interactor->GetName());
}

// ============================================================
// GetNextDialogueLine — returns next line in active sequence
// ============================================================
FNarr_DialogueLine ANarr_DialogueTrigger::GetNextDialogueLine()
{
    if (NPCProfile.DialogueTrees.Num() == 0)
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueSequence& ActiveSeq = NPCProfile.DialogueTrees[0];

    if (CurrentLineIndex >= ActiveSeq.Lines.Num())
    {
        // Sequence complete — return empty line
        return FNarr_DialogueLine();
    }

    FNarr_DialogueLine Line = ActiveSeq.Lines[CurrentLineIndex];
    CurrentLineIndex++;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Line %d/%d: %s"),
        CurrentLineIndex,
        ActiveSeq.Lines.Num(),
        *Line.LineText);

    return Line;
}

// ============================================================
// ResetDialogue — allow re-triggering
// ============================================================
void ANarr_DialogueTrigger::ResetDialogue()
{
    bHasTriggered = false;
    CurrentLineIndex = 0;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue reset: NPC=%s"), *NPCProfile.DisplayName);
}
