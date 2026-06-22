// DialogueSystem.cpp
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260622_001
// Prehistoric survival dialogue system: proximity triggers, sequence management,
// auto-advance timer, and built-in narrative lines for MinPlayableMap NPCs.

#include "DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UDialogueSystem::UDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    ProximityTriggerRadius = 300.0f;
    ActiveSequenceIndex = -1;
    ActiveLineIndex = -1;
    bDialogueActive = false;
}

void UDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultSequences();
}

// ============================================================
// InitializeDefaultSequences
// Populates the built-in prehistoric survival dialogue lines.
// These map to the 6 quest zones placed by Agent #14.
// ============================================================
void UDialogueSystem::InitializeDefaultSequences()
{
    // ---- SEQUENCE 1: Elder Warning (Camp — TRex sighting) ----
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("ElderWarning");
        Seq.TriggerType = ENarr_DialogueTriggerType::Proximity;
        Seq.bPlayOnce = true;

        FNarr_DialogueLine L1;
        L1.Speaker = ENarr_DialogueSpeaker::TribalLeader;
        L1.LineText = TEXT("Stay close to the camp tonight. The great lizard was spotted near the river crossing at dusk.");
        L1.DisplayDuration = 5.0f;
        L1.LinkedQuestTag = FName("QuestZone_CampDefense");
        Seq.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.Speaker = ENarr_DialogueSpeaker::TribalLeader;
        L2.LineText = TEXT("We move at first light — north, toward the high ground where the herd was seen. Bring your spear.");
        L2.DisplayDuration = 5.0f;
        L2.LinkedQuestTag = FName("QuestZone_HerdMigration");
        Seq.Lines.Add(L2);

        FNarr_DialogueLine L3;
        L3.Speaker = ENarr_DialogueSpeaker::Player;
        L3.LineText = TEXT("...");
        L3.DisplayDuration = 1.5f;
        Seq.Lines.Add(L3);

        FNarr_DialogueLine L4;
        L4.Speaker = ENarr_DialogueSpeaker::TribalLeader;
        L4.LineText = TEXT("Do not look it in the eye. Move slow. If it charges — scatter and meet at the high rocks.");
        L4.DisplayDuration = 5.0f;
        Seq.Lines.Add(L4);

        DialogueSequences.Add(Seq);
    }

    // ---- SEQUENCE 2: Scout Report (Raptor Patrol Zone) ----
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("ScoutReport");
        Seq.TriggerType = ENarr_DialogueTriggerType::Proximity;
        Seq.bPlayOnce = true;

        FNarr_DialogueLine L1;
        L1.Speaker = ENarr_DialogueSpeaker::Scout;
        L1.LineText = TEXT("The raptor pack has been moving through the eastern forest since midday. Three of them — maybe four.");
        L1.DisplayDuration = 5.0f;
        L1.LinkedQuestTag = FName("QuestZone_RaptorPatrol");
        Seq.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.Speaker = ENarr_DialogueSpeaker::Scout;
        L2.LineText = TEXT("Stay low. Move between the rocks. They hunt by movement — freeze if one looks your way.");
        L2.DisplayDuration = 5.0f;
        Seq.Lines.Add(L2);

        FNarr_DialogueLine L3;
        L3.Speaker = ENarr_DialogueSpeaker::Scout;
        L3.LineText = TEXT("I marked their patrol path with broken branches. Follow the marks and you can get through.");
        L3.DisplayDuration = 4.5f;
        Seq.Lines.Add(L3);

        DialogueSequences.Add(Seq);
    }

    // ---- SEQUENCE 3: Water Discovery ----
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("WaterDiscovery");
        Seq.TriggerType = ENarr_DialogueTriggerType::FirstSighting;
        Seq.bPlayOnce = true;

        FNarr_DialogueLine L1;
        L1.Speaker = ENarr_DialogueSpeaker::Narrator;
        L1.LineText = TEXT("A stream cuts through the valley floor. The water is clear — it has not been fouled by the great lizards upstream.");
        L1.DisplayDuration = 5.0f;
        L1.LinkedQuestTag = FName("QuestZone_WaterSource");
        Seq.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.Speaker = ENarr_DialogueSpeaker::Narrator;
        L2.LineText = TEXT("The camp needs this. Mark the path back — the tribe cannot survive another dry day.");
        L2.DisplayDuration = 4.5f;
        Seq.Lines.Add(L2);

        DialogueSequences.Add(Seq);
    }

    // ---- SEQUENCE 4: Cave Entrance ----
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("CaveEntrance");
        Seq.TriggerType = ENarr_DialogueTriggerType::Proximity;
        Seq.bPlayOnce = true;

        FNarr_DialogueLine L1;
        L1.Speaker = ENarr_DialogueSpeaker::Narrator;
        L1.LineText = TEXT("The cave mouth is wide enough for a man to enter. Claw marks on the stone — something large has been here recently.");
        L1.DisplayDuration = 5.5f;
        L1.LinkedQuestTag = FName("QuestZone_CaveEntrance");
        Seq.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.Speaker = ENarr_DialogueSpeaker::Narrator;
        L2.LineText = TEXT("But the rock inside is dry. Flat. Defensible. If the cave is empty — this could shelter the whole tribe through the cold season.");
        L2.DisplayDuration = 5.5f;
        Seq.Lines.Add(L2);

        DialogueSequences.Add(Seq);
    }

    // ---- SEQUENCE 5: Herd Migration Sighting ----
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("HerdSighting");
        Seq.TriggerType = ENarr_DialogueTriggerType::FirstSighting;
        Seq.bPlayOnce = true;

        FNarr_DialogueLine L1;
        L1.Speaker = ENarr_DialogueSpeaker::Narrator;
        L1.LineText = TEXT("Hundreds of them — moving north in a column that stretches to the horizon. The ground shakes with each step.");
        L1.DisplayDuration = 5.0f;
        L1.LinkedQuestTag = FName("QuestZone_HerdMigration");
        Seq.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.Speaker = ENarr_DialogueSpeaker::Narrator;
        L2.LineText = TEXT("Follow the herd. Where they go, there is water and open ground. And where there is a herd — there are predators.");
        L2.DisplayDuration = 5.5f;
        Seq.Lines.Add(L2);

        DialogueSequences.Add(Seq);
    }

    // ---- SEQUENCE 6: Camp Defense Alert ----
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("CampDefenseAlert");
        Seq.TriggerType = ENarr_DialogueTriggerType::QuestStart;
        Seq.bPlayOnce = false; // Can repeat — each attack is a new event

        FNarr_DialogueLine L1;
        L1.Speaker = ENarr_DialogueSpeaker::TribalLeader;
        L1.LineText = TEXT("It is coming! Get to the fire — they fear the flame. Hold the perimeter!");
        L1.DisplayDuration = 4.0f;
        L1.LinkedQuestTag = FName("QuestZone_CampDefense");
        Seq.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.Speaker = ENarr_DialogueSpeaker::TribalLeader;
        L2.LineText = TEXT("Drive it back — we cannot lose the camp. Everything we have is here!");
        L2.DisplayDuration = 4.0f;
        Seq.Lines.Add(L2);

        DialogueSequences.Add(Seq);
    }
}

// ============================================================
// TriggerDialogueByID
// ============================================================
bool UDialogueSystem::TriggerDialogueByID(FName SequenceID)
{
    int32 Index = FindSequenceIndex(SequenceID);
    if (Index == INDEX_NONE)
    {
        return false;
    }

    FNarr_DialogueSequence& Seq = DialogueSequences[Index];

    // Respect bPlayOnce
    if (Seq.bPlayOnce && Seq.bHasPlayed)
    {
        return false;
    }

    if (Seq.Lines.Num() == 0)
    {
        return false;
    }

    // Stop any existing dialogue
    if (bDialogueActive)
    {
        EndDialogue();
    }

    ActiveSequenceIndex = Index;
    ActiveLineIndex = 0;
    bDialogueActive = true;

    // Schedule auto-advance if duration > 0
    const FNarr_DialogueLine& FirstLine = Seq.Lines[0];
    if (FirstLine.DisplayDuration > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            AutoAdvanceTimerHandle,
            this,
            &UDialogueSystem::OnAutoAdvanceTimer,
            FirstLine.DisplayDuration,
            false
        );
    }

    return true;
}

// ============================================================
// AdvanceDialogue
// ============================================================
bool UDialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive || ActiveSequenceIndex == INDEX_NONE)
    {
        return false;
    }

    // Clear any pending auto-advance timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
    }

    FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];
    ActiveLineIndex++;

    if (ActiveLineIndex >= Seq.Lines.Num())
    {
        // End of sequence
        EndDialogue();
        return false;
    }

    // Schedule auto-advance for next line
    const FNarr_DialogueLine& NextLine = Seq.Lines[ActiveLineIndex];
    if (NextLine.DisplayDuration > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            AutoAdvanceTimerHandle,
            this,
            &UDialogueSystem::OnAutoAdvanceTimer,
            NextLine.DisplayDuration,
            false
        );
    }

    return true;
}

// ============================================================
// EndDialogue
// ============================================================
void UDialogueSystem::EndDialogue()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
    }

    if (ActiveSequenceIndex != INDEX_NONE && ActiveSequenceIndex < DialogueSequences.Num())
    {
        DialogueSequences[ActiveSequenceIndex].bHasPlayed = true;
    }

    bDialogueActive = false;
    ActiveSequenceIndex = INDEX_NONE;
    ActiveLineIndex = INDEX_NONE;
}

// ============================================================
// GetCurrentLine
// ============================================================
bool UDialogueSystem::GetCurrentLine(FNarr_DialogueLine& OutLine) const
{
    if (!bDialogueActive || ActiveSequenceIndex == INDEX_NONE || ActiveLineIndex == INDEX_NONE)
    {
        return false;
    }

    const FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];
    if (ActiveLineIndex < 0 || ActiveLineIndex >= Seq.Lines.Num())
    {
        return false;
    }

    OutLine = Seq.Lines[ActiveLineIndex];
    return true;
}

// ============================================================
// CheckProximityTrigger
// ============================================================
void UDialogueSystem::CheckProximityTrigger(AActor* PlayerActor)
{
    if (!PlayerActor || bDialogueActive)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    float Distance = FVector::Dist(Owner->GetActorLocation(), PlayerActor->GetActorLocation());
    if (Distance > ProximityTriggerRadius)
    {
        return;
    }

    // Find first unplayed proximity sequence
    for (FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.TriggerType == ENarr_DialogueTriggerType::Proximity)
        {
            if (!Seq.bHasPlayed || !Seq.bPlayOnce)
            {
                TriggerDialogueByID(Seq.SequenceID);
                return;
            }
        }
    }
}

// ============================================================
// MarkSequencePlayed / HasSequencePlayed
// ============================================================
void UDialogueSystem::MarkSequencePlayed(FName SequenceID)
{
    int32 Index = FindSequenceIndex(SequenceID);
    if (Index != INDEX_NONE)
    {
        DialogueSequences[Index].bHasPlayed = true;
    }
}

bool UDialogueSystem::HasSequencePlayed(FName SequenceID) const
{
    int32 Index = FindSequenceIndex(SequenceID);
    if (Index == INDEX_NONE)
    {
        return false;
    }
    return DialogueSequences[Index].bHasPlayed;
}

// ============================================================
// Private helpers
// ============================================================
int32 UDialogueSystem::FindSequenceIndex(FName SequenceID) const
{
    for (int32 i = 0; i < DialogueSequences.Num(); ++i)
    {
        if (DialogueSequences[i].SequenceID == SequenceID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UDialogueSystem::OnAutoAdvanceTimer()
{
    AdvanceDialogue();
}
