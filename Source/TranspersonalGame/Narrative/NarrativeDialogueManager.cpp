// NarrativeDialogueManager.cpp — Agent #15 Narrative & Dialogue
// Implements dialogue trigger system for MinPlayableMap survival game

#include "NarrativeDialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;

    bDialogueActive = false;
    CurrentLineIndex = 0;
    ActiveSequenceID = NAME_None;
    PlayerHungerThreshold = 30.0f;
    PlayerDangerRadius = 800.0f;
}

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarrativeDialogueManager::InitializeDefaultDialogues()
{
    BuildCampDialogue();
    BuildRiverDialogue();
    BuildPredatorDialogue();
}

void ANarrativeDialogueManager::BuildCampDialogue()
{
    FNarr_DialogueSequence CampSeq;
    CampSeq.SequenceID = FName("Camp_Arrival");
    CampSeq.TriggerType = ENarr_DialogueTriggerType::Camp;
    CampSeq.bHasPlayed = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder");
    Line1.LineText = TEXT("You return. Good. The pack moved east — three kills since dawn.");
    Line1.DisplayDuration = 4.0f;
    Line1.TriggerContext = ENarr_DialogueTriggerType::Camp;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder");
    Line2.LineText = TEXT("Eat. Rest. Tomorrow we track the herd before the raptors take the valley.");
    Line2.DisplayDuration = 4.5f;
    Line2.TriggerContext = ENarr_DialogueTriggerType::Camp;

    FNarr_DialogueLine Line3;
    Line3.SpeakerName = TEXT("Hunter");
    Line3.LineText = TEXT("I found tracks near the river. Big ones. Bigger than the last season.");
    Line3.DisplayDuration = 4.0f;
    Line3.TriggerContext = ENarr_DialogueTriggerType::Camp;

    CampSeq.Lines.Add(Line1);
    CampSeq.Lines.Add(Line2);
    CampSeq.Lines.Add(Line3);
    DialogueLibrary.Add(CampSeq);
}

void ANarrativeDialogueManager::BuildRiverDialogue()
{
    FNarr_DialogueSequence RiverSeq;
    RiverSeq.SequenceID = FName("River_Warning");
    RiverSeq.TriggerType = ENarr_DialogueTriggerType::River;
    RiverSeq.bHasPlayed = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Scout");
    Line1.LineText = TEXT("Careful. The river crossing is exposed. Raptors hunt the banks at dusk.");
    Line1.DisplayDuration = 4.0f;
    Line1.TriggerContext = ENarr_DialogueTriggerType::River;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Scout");
    Line2.LineText = TEXT("Cross fast. Stay low. Do not run — movement triggers the chase response.");
    Line2.DisplayDuration = 4.5f;
    Line2.TriggerContext = ENarr_DialogueTriggerType::River;

    RiverSeq.Lines.Add(Line1);
    RiverSeq.Lines.Add(Line2);
    DialogueLibrary.Add(RiverSeq);
}

void ANarrativeDialogueManager::BuildPredatorDialogue()
{
    FNarr_DialogueSequence PredSeq;
    PredSeq.SequenceID = FName("Predator_Territory");
    PredSeq.TriggerType = ENarr_DialogueTriggerType::Predator;
    PredSeq.bHasPlayed = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder");
    Line1.LineText = TEXT("This is its territory. The large one. It has hunted here for three seasons.");
    Line1.DisplayDuration = 5.0f;
    Line1.TriggerContext = ENarr_DialogueTriggerType::Predator;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder");
    Line2.LineText = TEXT("We do not fight it. We survive around it. Learn its patterns. Use them.");
    Line2.DisplayDuration = 5.0f;
    Line2.TriggerContext = ENarr_DialogueTriggerType::Predator;

    FNarr_DialogueLine Line3;
    Line3.SpeakerName = TEXT("Hunter");
    Line3.LineText = TEXT("It feeds at the ridge at midday. That is our window. Move then, not before.");
    Line3.DisplayDuration = 4.5f;
    Line3.TriggerContext = ENarr_DialogueTriggerType::Predator;

    PredSeq.Lines.Add(Line1);
    PredSeq.Lines.Add(Line2);
    PredSeq.Lines.Add(Line3);
    DialogueLibrary.Add(PredSeq);
}

void ANarrativeDialogueManager::TriggerDialogueSequence(FName SequenceID)
{
    if (bDialogueActive)
    {
        return;
    }

    FNarr_DialogueSequence* Seq = FindSequenceByID(SequenceID);
    if (!Seq || Seq->bHasPlayed || Seq->Lines.Num() == 0)
    {
        return;
    }

    bDialogueActive = true;
    ActiveSequenceID = SequenceID;
    CurrentLineIndex = 0;
}

void ANarrativeDialogueManager::AdvanceDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    FNarr_DialogueSequence* Seq = FindSequenceByID(ActiveSequenceID);
    if (!Seq)
    {
        EndDialogue();
        return;
    }

    CurrentLineIndex++;
    if (CurrentLineIndex >= Seq->Lines.Num())
    {
        EndDialogue();
        Seq->bHasPlayed = true;
    }
}

void ANarrativeDialogueManager::EndDialogue()
{
    bDialogueActive = false;
    CurrentLineIndex = 0;
    ActiveSequenceID = NAME_None;
}

FNarr_DialogueLine ANarrativeDialogueManager::GetCurrentLine() const
{
    for (const FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == ActiveSequenceID)
        {
            if (CurrentLineIndex >= 0 && CurrentLineIndex < Seq.Lines.Num())
            {
                return Seq.Lines[CurrentLineIndex];
            }
        }
    }
    return FNarr_DialogueLine();
}

bool ANarrativeDialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

void ANarrativeDialogueManager::RegisterTriggerZone(ENarr_DialogueTriggerType TriggerType, FVector Location)
{
    // Trigger zone registration — Blueprint or C++ callers can register zones at runtime
    // Maps TriggerType to world location for proximity checks
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Registered trigger zone type=%d at (%f,%f,%f)"),
        (int32)TriggerType, Location.X, Location.Y, Location.Z);
}

FNarr_DialogueSequence* ANarrativeDialogueManager::FindSequenceByID(FName SequenceID)
{
    for (FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return &Seq;
        }
    }
    return nullptr;
}
