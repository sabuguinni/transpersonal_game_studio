#include "NarrativeDialogueSystem.h"

ANarrativeDialogueSystem::ANarrativeDialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    ActiveOutcome = ENarr_QuestOutcome::None;
    CurrentLineIndex = 0;
    bDialoguePlaying = false;
    LineTimer = 0.0f;

    PopulateDefaultDialogue();
}

void ANarrativeDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
}

void ANarrativeDialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDialoguePlaying || ActiveSequence.Num() == 0)
        return;

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        AdvanceToNextLine();
    }
}

// ============================================================
// Core dialogue trigger
// ============================================================

void ANarrativeDialogueSystem::TriggerDialogue(ENarr_QuestOutcome Outcome, ENarr_StampedeCause Cause)
{
    FNarr_DialogueSet* Set = FindDialogueSet(Outcome, Cause);

    // Fallback: try Unknown cause if specific cause not found
    if (!Set)
    {
        Set = FindDialogueSet(Outcome, ENarr_StampedeCause::Unknown);
    }

    if (!Set || Set->Lines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("NarrativeDialogueSystem: No dialogue found for Outcome=%d Cause=%d"),
            (int32)Outcome, (int32)Cause);
        return;
    }

    // Start sequence
    ActiveSequence = Set->Lines;
    ActiveOutcome = Outcome;
    CurrentLineIndex = 0;
    bDialoguePlaying = true;

    // Fire first line immediately
    const FNarr_DialogueLine& FirstLine = ActiveSequence[0];
    LineTimer = FirstLine.AudioDurationSeconds + 1.5f; // 1.5s pause between lines
    OnDialogueTriggered.Broadcast(FirstLine, Outcome);

    UE_LOG(LogTemp, Log,
        TEXT("NarrativeDialogueSystem: Starting dialogue sequence — %d lines, Outcome=%d"),
        ActiveSequence.Num(), (int32)Outcome);
}

void ANarrativeDialogueSystem::TriggerDialogueByOutcome(ENarr_QuestOutcome Outcome)
{
    TriggerDialogue(Outcome, ENarr_StampedeCause::Unknown);
}

void ANarrativeDialogueSystem::StopDialogue()
{
    bDialoguePlaying = false;
    ActiveSequence.Empty();
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

// ============================================================
// State queries
// ============================================================

FText ANarrativeDialogueSystem::GetCurrentSubtitleText() const
{
    if (!bDialoguePlaying || !ActiveSequence.IsValidIndex(CurrentLineIndex))
        return FText::GetEmpty();
    return ActiveSequence[CurrentLineIndex].LineText;
}

FText ANarrativeDialogueSystem::GetCurrentSpeakerName() const
{
    if (!bDialoguePlaying || !ActiveSequence.IsValidIndex(CurrentLineIndex))
        return FText::GetEmpty();
    return ActiveSequence[CurrentLineIndex].SpeakerName;
}

bool ANarrativeDialogueSystem::IsDialoguePlaying() const
{
    return bDialoguePlaying;
}

FString ANarrativeDialogueSystem::GetCurrentLineAudioURL() const
{
    if (!bDialoguePlaying || !ActiveSequence.IsValidIndex(CurrentLineIndex))
        return FString();
    return ActiveSequence[CurrentLineIndex].AudioURL;
}

// ============================================================
// Internal helpers
// ============================================================

void ANarrativeDialogueSystem::AdvanceToNextLine()
{
    CurrentLineIndex++;

    if (!ActiveSequence.IsValidIndex(CurrentLineIndex))
    {
        // Sequence complete
        bDialoguePlaying = false;
        OnDialogueSequenceComplete.Broadcast(ActiveOutcome);
        UE_LOG(LogTemp, Log,
            TEXT("NarrativeDialogueSystem: Dialogue sequence complete for Outcome=%d"),
            (int32)ActiveOutcome);
        return;
    }

    const FNarr_DialogueLine& NextLine = ActiveSequence[CurrentLineIndex];
    LineTimer = NextLine.AudioDurationSeconds + 1.5f;
    OnDialogueTriggered.Broadcast(NextLine, ActiveOutcome);
}

FNarr_DialogueSet* ANarrativeDialogueSystem::FindDialogueSet(
    ENarr_QuestOutcome Outcome, ENarr_StampedeCause Cause)
{
    for (FNarr_DialogueSet& Set : DialogueSets)
    {
        if (Set.Outcome == Outcome && Set.Cause == Cause)
            return &Set;
    }
    return nullptr;
}

// ============================================================
// Default dialogue population — all stampede quest variants
// ============================================================

void ANarrativeDialogueSystem::PopulateDefaultDialogue()
{
    // --------------------------------------------------------
    // OUTCOME: SurvivedFast (<10s) — Cause: Unknown/Any
    // --------------------------------------------------------
    {
        FNarr_DialogueSet FastSet;
        FastSet.Outcome = ENarr_QuestOutcome::SurvivedFast;
        FastSet.Cause = ENarr_StampedeCause::Unknown;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Tribal Elder"));
        Line1.LineText = FText::FromString(
            TEXT("You made it. I watched from the ridge — I thought the herd would take you. "
                 "But you ran smart. You found the high ground. Not many survive their first stampede. "
                 "The tribe will remember this."));
        Line1.SpeakerRole = ENarr_NPCRole::TribalElder;
        Line1.TriggerOutcome = ENarr_QuestOutcome::SurvivedFast;
        Line1.TriggerCause = ENarr_StampedeCause::Unknown;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782981562782_TribalElder_StampedeComplete.mp3");
        Line1.AudioDurationSeconds = 13.0f;
        FastSet.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = FText::FromString(TEXT("Chief Hunter"));
        Line2.LineText = FText::FromString(
            TEXT("Fast feet. Good instincts. You read the herd before it broke. "
                 "That is a hunter's eye. Come — I have more to teach you."));
        Line2.SpeakerRole = ENarr_NPCRole::ChiefHunter;
        Line2.TriggerOutcome = ENarr_QuestOutcome::SurvivedFast;
        Line2.TriggerCause = ENarr_StampedeCause::Unknown;
        Line2.AudioDurationSeconds = 10.0f;
        FastSet.Lines.Add(Line2);

        DialogueSets.Add(FastSet);
    }

    // --------------------------------------------------------
    // OUTCOME: SurvivedFast — Cause: Predator
    // --------------------------------------------------------
    {
        FNarr_DialogueSet FastPredSet;
        FastPredSet.Outcome = ENarr_QuestOutcome::SurvivedFast;
        FastPredSet.Cause = ENarr_StampedeCause::Predator;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Tracker"));
        Line1.LineText = FText::FromString(
            TEXT("The herd ran from a predator — you saw it before I did. "
                 "That T-Rex was downwind. The herd smelled it first. "
                 "You survived because you moved when they moved. Smart."));
        Line1.SpeakerRole = ENarr_NPCRole::Tracker;
        Line1.TriggerOutcome = ENarr_QuestOutcome::SurvivedFast;
        Line1.TriggerCause = ENarr_StampedeCause::Predator;
        Line1.AudioDurationSeconds = 12.0f;
        FastPredSet.Lines.Add(Line1);

        DialogueSets.Add(FastPredSet);
    }

    // --------------------------------------------------------
    // OUTCOME: SurvivedBarely (10-20s) — Cause: Unknown/Any
    // --------------------------------------------------------
    {
        FNarr_DialogueSet BarelySet;
        BarelySet.Outcome = ENarr_QuestOutcome::SurvivedBarely;
        BarelySet.Cause = ENarr_StampedeCause::Unknown;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Tribal Elder"));
        Line1.LineText = FText::FromString(
            TEXT("The herd passed through here three days ago. See these tracks? Deep — a full-grown Brachiosaurus. "
                 "But look here — something spooked them. They ran hard, broke trees, crushed everything. "
                 "Something big was hunting them. Stay alert."));
        Line1.SpeakerRole = ENarr_NPCRole::TribalElder;
        Line1.TriggerOutcome = ENarr_QuestOutcome::SurvivedBarely;
        Line1.TriggerCause = ENarr_StampedeCause::Unknown;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782981547329_TribalElder_QuestReact.mp3");
        Line1.AudioDurationSeconds = 16.0f;
        BarelySet.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = FText::FromString(TEXT("Scout Runner"));
        Line2.LineText = FText::FromString(
            TEXT("You cut it close. Another few seconds and the herd would have taken you. "
                 "Next time — do not wait to see which way they turn. "
                 "When the ground shakes, you are already moving."));
        Line2.SpeakerRole = ENarr_NPCRole::ScoutRunner;
        Line2.TriggerOutcome = ENarr_QuestOutcome::SurvivedBarely;
        Line2.TriggerCause = ENarr_StampedeCause::Unknown;
        Line2.AudioDurationSeconds = 11.0f;
        BarelySet.Lines.Add(Line2);

        DialogueSets.Add(BarelySet);
    }

    // --------------------------------------------------------
    // OUTCOME: SurvivedBarely — Cause: Earthquake
    // --------------------------------------------------------
    {
        FNarr_DialogueSet BarelyEqSet;
        BarelyEqSet.Outcome = ENarr_QuestOutcome::SurvivedBarely;
        BarelyEqSet.Cause = ENarr_StampedeCause::Earthquake;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Camp Builder"));
        Line1.LineText = FText::FromString(
            TEXT("The earth moved and the herd panicked. You cannot predict the ground shaking — "
                 "but you can read the animals. When they stop eating and look up all at once, "
                 "something is wrong. Get to high ground before they start running."));
        Line1.SpeakerRole = ENarr_NPCRole::CampBuilder;
        Line1.TriggerOutcome = ENarr_QuestOutcome::SurvivedBarely;
        Line1.TriggerCause = ENarr_StampedeCause::Earthquake;
        Line1.AudioDurationSeconds = 13.0f;
        BarelyEqSet.Lines.Add(Line1);

        DialogueSets.Add(BarelyEqSet);
    }

    // --------------------------------------------------------
    // OUTCOME: SurvivedBarely — Cause: Fire
    // --------------------------------------------------------
    {
        FNarr_DialogueSet BarelyFireSet;
        BarelyFireSet.Outcome = ENarr_QuestOutcome::SurvivedBarely;
        BarelyFireSet.Cause = ENarr_StampedeCause::Fire;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Chief Hunter"));
        Line1.LineText = FText::FromString(
            TEXT("Fire drives them mad. They cannot think — only run from the smoke. "
                 "If you smell burning grass, you have maybe two minutes before the herd moves. "
                 "Use that time. Do not waste it watching the flames."));
        Line1.SpeakerRole = ENarr_NPCRole::ChiefHunter;
        Line1.TriggerOutcome = ENarr_QuestOutcome::SurvivedBarely;
        Line1.TriggerCause = ENarr_StampedeCause::Fire;
        Line1.AudioDurationSeconds = 12.0f;
        BarelyFireSet.Lines.Add(Line1);

        DialogueSets.Add(BarelyFireSet);
    }

    // --------------------------------------------------------
    // OUTCOME: Failed — Cause: Unknown/Any
    // --------------------------------------------------------
    {
        FNarr_DialogueSet FailedSet;
        FailedSet.Outcome = ENarr_QuestOutcome::Failed;
        FailedSet.Cause = ENarr_StampedeCause::Unknown;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Chief Hunter"));
        Line1.LineText = FText::FromString(
            TEXT("Too slow. You hesitated when the herd broke. "
                 "Next time — when the ground shakes, you do not think. You run. "
                 "Always run to high ground. Always. "
                 "The valley is a death trap when the great beasts move."));
        Line1.SpeakerRole = ENarr_NPCRole::ChiefHunter;
        Line1.TriggerOutcome = ENarr_QuestOutcome::Failed;
        Line1.TriggerCause = ENarr_StampedeCause::Unknown;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782981574400_ChiefHunter_StampedeFail.mp3");
        Line1.AudioDurationSeconds = 14.0f;
        FailedSet.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = FText::FromString(TEXT("Tribal Elder"));
        Line2.LineText = FText::FromString(
            TEXT("The ground shook and I froze. The herd was everywhere — I could not see the sky. "
                 "I ran but my legs would not move fast enough. I fell. I thought — this is how I die. "
                 "Then I saw the rocks above me and I climbed. I did not stop climbing until the thunder was behind me."));
        Line2.SpeakerRole = ENarr_NPCRole::TribalElder;
        Line2.TriggerOutcome = ENarr_QuestOutcome::Failed;
        Line2.TriggerCause = ENarr_StampedeCause::Unknown;
        Line2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782981571907_PlayerNarrator_StampedeFailed.mp3");
        Line2.AudioDurationSeconds = 18.0f;
        FailedSet.Lines.Add(Line2);

        DialogueSets.Add(FailedSet);
    }

    // --------------------------------------------------------
    // OUTCOME: Failed — Cause: Predator (T-Rex triggered stampede)
    // --------------------------------------------------------
    {
        FNarr_DialogueSet FailedPredSet;
        FailedPredSet.Outcome = ENarr_QuestOutcome::Failed;
        FailedPredSet.Cause = ENarr_StampedeCause::Predator;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = FText::FromString(TEXT("Tracker"));
        Line1.LineText = FText::FromString(
            TEXT("A T-Rex drove the herd and you were caught in the middle. "
                 "That is the worst place to be. The predator does not care about you — "
                 "but the herd will crush anything in its path. "
                 "Remember: the predator comes from one direction. Run the other way."));
        Line1.SpeakerRole = ENarr_NPCRole::Tracker;
        Line1.TriggerOutcome = ENarr_QuestOutcome::Failed;
        Line1.TriggerCause = ENarr_StampedeCause::Predator;
        Line1.AudioDurationSeconds = 14.0f;
        FailedPredSet.Lines.Add(Line1);

        DialogueSets.Add(FailedPredSet);
    }
}
