#include "DialogueSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem.cpp
// Cycle: PROD_CYCLE_AUTO_20260620_001
// Full implementation — zero stubs
// ============================================================

// Voice line URLs generated this cycle (ElevenLabs via TTS tool):
// QuestNarrator_FirstTools: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916736095_QuestNarrator_FirstTools.mp3
// TribalElder_DinoWarning:  https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916738386_TribalElder_DinoWarning.mp3
// TribalElder_RaptorTactic: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916746449_TribalElder_RaptorTactic.mp3
// QuestNarrator_WaterFirst: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916749037_QuestNarrator_WaterFirst.mp3

UDialogueSystemComponent::UDialogueSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ActiveSequenceIndex = -1;
    ActiveLineIndex = 0;
}

void UDialogueSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultSequences();
}

void UDialogueSystemComponent::InitializeDefaultSequences()
{
    // --------------------------------------------------------
    // SEQUENCE 1: Quest Start — "First Tools"
    // Trigger: OnQuestStart
    // --------------------------------------------------------
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("FirstTools_QuestStart");
        Seq.TriggerType = ENarr_DialogueTriggerType::OnQuestStart;
        Seq.bPlayOnce = true;
        Seq.bHasPlayed = false;

        FNarr_DialogueLine Line1;
        Line1.Speaker = ENarr_DialogueSpeaker::QuestNarrator;
        Line1.LineText = FText::FromString(TEXT("Your first tools are ready. A stone axe, carved from the land itself. But do not celebrate yet — the night brings hunters far more dangerous than hunger. Move fast. Find shelter before the sun falls."));
        Line1.DisplayDuration = 6.0f;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916736095_QuestNarrator_FirstTools.mp3");
        Line1.bBlockPlayerInput = false;
        Seq.Lines.Add(Line1);

        RegisteredSequences.Add(Seq);
    }

    // --------------------------------------------------------
    // SEQUENCE 2: Water Quest — "Follow the River"
    // Trigger: OnWaterFound
    // --------------------------------------------------------
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("WaterQuest_RiverFound");
        Seq.TriggerType = ENarr_DialogueTriggerType::OnWaterFound;
        Seq.bPlayOnce = true;
        Seq.bHasPlayed = false;

        FNarr_DialogueLine Line1;
        Line1.Speaker = ENarr_DialogueSpeaker::QuestNarrator;
        Line1.LineText = FText::FromString(TEXT("Water. You need water before anything else. A man without tools can still fight. A man without water is already dead. Follow the sound of the river — it will lead you to everything you need to survive."));
        Line1.DisplayDuration = 6.0f;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916749037_QuestNarrator_WaterFirst.mp3");
        Line1.bBlockPlayerInput = false;
        Seq.Lines.Add(Line1);

        RegisteredSequences.Add(Seq);
    }

    // --------------------------------------------------------
    // SEQUENCE 3: Brachiosaurus Warning
    // Trigger: OnDinoNearby
    // --------------------------------------------------------
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("DinoWarning_Brachio");
        Seq.TriggerType = ENarr_DialogueTriggerType::OnDinoNearby;
        Seq.bPlayOnce = false; // Can repeat per encounter
        Seq.bHasPlayed = false;

        FNarr_DialogueLine Line1;
        Line1.Speaker = ENarr_DialogueSpeaker::TribalElder;
        Line1.LineText = FText::FromString(TEXT("The large one moves slowly, but do not mistake that for safety. A Brachiosaurus that feels cornered will crush you without even noticing. Keep your distance. Watch. Learn their patterns before you act."));
        Line1.DisplayDuration = 6.0f;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916738386_TribalElder_DinoWarning.mp3");
        Line1.bBlockPlayerInput = false;
        Seq.Lines.Add(Line1);

        RegisteredSequences.Add(Seq);
    }

    // --------------------------------------------------------
    // SEQUENCE 4: Raptor Encounter Tactic
    // Trigger: OnDinoNearby (Raptor-specific, handled by caller)
    // --------------------------------------------------------
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("DinoWarning_Raptor");
        Seq.TriggerType = ENarr_DialogueTriggerType::OnDinoNearby;
        Seq.bPlayOnce = false;
        Seq.bHasPlayed = false;

        FNarr_DialogueLine Line1;
        Line1.Speaker = ENarr_DialogueSpeaker::TribalElder;
        Line1.LineText = FText::FromString(TEXT("Three raptors. They never hunt alone. If you see one, the other two are already behind you. Drop what you are carrying and climb. They cannot follow you up the rocks."));
        Line1.DisplayDuration = 5.0f;
        Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916746449_TribalElder_RaptorTactic.mp3");
        Line1.bBlockPlayerInput = false;
        Seq.Lines.Add(Line1);

        RegisteredSequences.Add(Seq);
    }

    // --------------------------------------------------------
    // SEQUENCE 5: Nightfall Warning
    // Trigger: OnNightfall
    // --------------------------------------------------------
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("Nightfall_Warning");
        Seq.TriggerType = ENarr_DialogueTriggerType::OnNightfall;
        Seq.bPlayOnce = true;
        Seq.bHasPlayed = false;

        FNarr_DialogueLine Line1;
        Line1.Speaker = ENarr_DialogueSpeaker::QuestNarrator;
        Line1.LineText = FText::FromString(TEXT("The sun is falling. The predators that sleep in the heat of day are waking now. You have minutes, not hours. Find high ground or a cave entrance. Do not be in the open when darkness comes."));
        Line1.DisplayDuration = 6.0f;
        Line1.AudioURL = TEXT(""); // No audio this cycle — Audio Agent #16 to wire
        Line1.bBlockPlayerInput = false;
        Seq.Lines.Add(Line1);

        RegisteredSequences.Add(Seq);
    }

    // --------------------------------------------------------
    // SEQUENCE 6: Player Damaged — survival bark
    // Trigger: OnPlayerDamaged
    // --------------------------------------------------------
    {
        FNarr_DialogueSequence Seq;
        Seq.SequenceID = FName("PlayerDamaged_Bark");
        Seq.TriggerType = ENarr_DialogueTriggerType::OnPlayerDamaged;
        Seq.bPlayOnce = false;
        Seq.bHasPlayed = false;

        FNarr_DialogueLine Line1;
        Line1.Speaker = ENarr_DialogueSpeaker::AmbientBark;
        Line1.LineText = FText::FromString(TEXT("You are bleeding. Find water to clean the wound, or infection will finish what the predator started."));
        Line1.DisplayDuration = 4.0f;
        Line1.AudioURL = TEXT("");
        Line1.bBlockPlayerInput = false;
        Seq.Lines.Add(Line1);

        RegisteredSequences.Add(Seq);
    }
}

void UDialogueSystemComponent::TriggerDialogue(FName SequenceID)
{
    for (int32 i = 0; i < RegisteredSequences.Num(); ++i)
    {
        FNarr_DialogueSequence& Seq = RegisteredSequences[i];
        if (Seq.SequenceID == SequenceID)
        {
            if (Seq.bPlayOnce && Seq.bHasPlayed)
            {
                return; // Already played, skip
            }
            ActiveSequenceIndex = i;
            ActiveLineIndex = 0;
            Seq.bHasPlayed = true;
            PlayLine(i, 0);
            return;
        }
    }
}

void UDialogueSystemComponent::TriggerByType(ENarr_DialogueTriggerType TriggerType)
{
    for (int32 i = 0; i < RegisteredSequences.Num(); ++i)
    {
        FNarr_DialogueSequence& Seq = RegisteredSequences[i];
        if (Seq.TriggerType == TriggerType)
        {
            if (Seq.bPlayOnce && Seq.bHasPlayed)
            {
                continue;
            }
            // Play first matching unplayed sequence
            ActiveSequenceIndex = i;
            ActiveLineIndex = 0;
            Seq.bHasPlayed = true;
            PlayLine(i, 0);
            return;
        }
    }
}

void UDialogueSystemComponent::RegisterSequence(FNarr_DialogueSequence Sequence)
{
    // Check for duplicate ID
    for (const FNarr_DialogueSequence& Existing : RegisteredSequences)
    {
        if (Existing.SequenceID == Sequence.SequenceID)
        {
            return; // Already registered
        }
    }
    RegisteredSequences.Add(Sequence);
}

FNarr_DialogueLine UDialogueSystemComponent::GetCurrentLine() const
{
    if (ActiveSequenceIndex < 0 || ActiveSequenceIndex >= RegisteredSequences.Num())
    {
        return FNarr_DialogueLine();
    }
    const FNarr_DialogueSequence& Seq = RegisteredSequences[ActiveSequenceIndex];
    if (ActiveLineIndex < 0 || ActiveLineIndex >= Seq.Lines.Num())
    {
        return FNarr_DialogueLine();
    }
    return Seq.Lines[ActiveLineIndex];
}

bool UDialogueSystemComponent::IsDialogueActive() const
{
    return ActiveSequenceIndex >= 0;
}

void UDialogueSystemComponent::AdvanceLine()
{
    if (ActiveSequenceIndex < 0 || ActiveSequenceIndex >= RegisteredSequences.Num())
    {
        return;
    }

    // Clear existing timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LineTimerHandle);
    }

    const FNarr_DialogueSequence& Seq = RegisteredSequences[ActiveSequenceIndex];
    int32 NextLine = ActiveLineIndex + 1;

    if (NextLine >= Seq.Lines.Num())
    {
        // Sequence complete
        int32 CompletedIdx = ActiveSequenceIndex;
        ActiveSequenceIndex = -1;
        ActiveLineIndex = 0;
        OnDialogueSequenceEnded.Broadcast();
    }
    else
    {
        ActiveLineIndex = NextLine;
        PlayLine(ActiveSequenceIndex, NextLine);
    }
}

void UDialogueSystemComponent::SkipSequence()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LineTimerHandle);
    }
    ActiveSequenceIndex = -1;
    ActiveLineIndex = 0;
    OnDialogueSequenceEnded.Broadcast();
}

void UDialogueSystemComponent::PlayLine(int32 SequenceIdx, int32 LineIdx)
{
    if (SequenceIdx < 0 || SequenceIdx >= RegisteredSequences.Num())
    {
        return;
    }
    const FNarr_DialogueSequence& Seq = RegisteredSequences[SequenceIdx];
    if (LineIdx < 0 || LineIdx >= Seq.Lines.Num())
    {
        return;
    }

    const FNarr_DialogueLine& Line = Seq.Lines[LineIdx];

    // Broadcast to HUD
    OnDialogueLineChanged.Broadcast(Line);

    // Auto-advance after duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            LineTimerHandle,
            this,
            &UDialogueSystemComponent::OnLineTimerExpired,
            Line.DisplayDuration,
            false
        );
    }
}

void UDialogueSystemComponent::OnLineTimerExpired()
{
    AdvanceLine();
}
