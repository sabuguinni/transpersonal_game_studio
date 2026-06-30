// NarrativeDialogueManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation: dialogue trees, phase-gated triggers, quest integration
#include "NarrativeDialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 2Hz — proximity checks
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void UNarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    LoadMigrationQuestDialogue();
    LoadCampDefenseDialogue();
}

// ─── TickComponent ────────────────────────────────────────────────────────────

void UNarrativeDialogueManager::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    AccumulatedTime += DeltaTime;
}

// ─── RegisterDialogueTree ─────────────────────────────────────────────────────

void UNarrativeDialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Replace if exists
    for (int32 i = 0; i < DialogueTrees.Num(); ++i)
    {
        if (DialogueTrees[i].TreeID == Tree.TreeID)
        {
            DialogueTrees[i] = Tree;
            return;
        }
    }
    DialogueTrees.Add(Tree);
}

// ─── TriggerDialogueLine ──────────────────────────────────────────────────────

bool UNarrativeDialogueManager::TriggerDialogueLine(FName TreeID, int32 LineIndex)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree) return false;
    if (!Tree->Lines.IsValidIndex(LineIndex)) return false;

    FNarr_DialogueLine& Line = Tree->Lines[LineIndex];

    // Phase gate check
    if (Line.RequiredPhase != ENarr_DialoguePhase::NotStarted &&
        Line.RequiredPhase != CurrentPhase)
    {
        return false;
    }

    // Cooldown check
    float* LastTime = LastTriggerTimestamps.Find(TreeID);
    if (LastTime && (AccumulatedTime - *LastTime) < Line.CooldownSeconds)
    {
        return false;
    }

    // Already played this session (one-shot lines)
    if (Line.bPlayedThisSession && Line.CooldownSeconds <= 0.0f)
    {
        return false;
    }

    Line.bPlayedThisSession = true;
    LastTriggerTimestamps.Add(TreeID, AccumulatedTime);

    OnDialogueLineTriggered.Broadcast(TreeID, LineIndex);
    return true;
}

// ─── AdvanceDialogueTree ──────────────────────────────────────────────────────

void UNarrativeDialogueManager::AdvanceDialogueTree(FName TreeID)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree) return;

    Tree->CurrentLineIndex++;

    if (Tree->CurrentLineIndex >= Tree->Lines.Num())
    {
        Tree->bIsActive = false;
        OnDialogueTreeCompleted.Broadcast(TreeID);
    }
    else
    {
        TriggerDialogueLine(TreeID, Tree->CurrentLineIndex);
    }
}

// ─── SetNarrativePhase ────────────────────────────────────────────────────────

void UNarrativeDialogueManager::SetNarrativePhase(ENarr_DialoguePhase NewPhase)
{
    if (CurrentPhase == NewPhase) return;
    CurrentPhase = NewPhase;
    TriggerPhaseGatedLines(NewPhase);
}

// ─── TriggerPhaseGatedLines ───────────────────────────────────────────────────

void UNarrativeDialogueManager::TriggerPhaseGatedLines(ENarr_DialoguePhase Phase)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (!Tree.bIsActive) continue;
        for (int32 i = 0; i < Tree.Lines.Num(); ++i)
        {
            if (Tree.Lines[i].RequiredPhase == Phase &&
                !Tree.Lines[i].bPlayedThisSession)
            {
                TriggerDialogueLine(Tree.TreeID, i);
                OnPhaseGateUnlocked.Broadcast(Phase, Tree.TreeID);
                break; // one line per tree per phase unlock
            }
        }
    }
}

// ─── GetCurrentLine ───────────────────────────────────────────────────────────

FNarr_DialogueLine UNarrativeDialogueManager::GetCurrentLine(FName TreeID) const
{
    const FNarr_DialogueTree* Tree = FindTreeConst(TreeID);
    if (!Tree || !Tree->Lines.IsValidIndex(Tree->CurrentLineIndex))
    {
        return FNarr_DialogueLine();
    }
    return Tree->Lines[Tree->CurrentLineIndex];
}

// ─── IsTreeComplete ───────────────────────────────────────────────────────────

bool UNarrativeDialogueManager::IsTreeComplete(FName TreeID) const
{
    const FNarr_DialogueTree* Tree = FindTreeConst(TreeID);
    if (!Tree) return false;
    return !Tree->bIsActive && Tree->CurrentLineIndex >= Tree->Lines.Num();
}

// ─── ResetTree ────────────────────────────────────────────────────────────────

void UNarrativeDialogueManager::ResetTree(FName TreeID)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree) return;
    Tree->CurrentLineIndex = 0;
    Tree->bIsActive = true;
    for (FNarr_DialogueLine& Line : Tree->Lines)
    {
        Line.bPlayedThisSession = false;
    }
}

// ─── LoadMigrationQuestDialogue ───────────────────────────────────────────────
// 4 narrative beats: Discovery → Journey → Danger → Arrival
// Voice URLs from ElevenLabs TTS (Agent #15 production assets)

void UNarrativeDialogueManager::LoadMigrationQuestDialogue()
{
    FNarr_DialogueTree MigTree;
    MigTree.TreeID = FName("Migration_Elder");
    MigTree.OwnerRole = ENarr_NPCRole::Elder;
    MigTree.bIsActive = true;

    // Beat 1 — Discovery (Introduction phase)
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "The herd moves at dawn. Four days east, past the canyon where the river bends. "
            "If we miss them, we face the dry season alone. I have seen what that does to a tribe.");
        Line.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782820412984_QuestNPC_Elder_MigrationBriefi.mp3";
        Line.SpeakerRole = ENarr_NPCRole::Elder;
        Line.TriggerType = ENarr_DialogueTrigger::OnFirstMeeting;
        Line.RequiredPhase = ENarr_DialoguePhase::Introduction;
        Line.CooldownSeconds = 0.0f; // one-shot
        MigTree.Lines.Add(Line);
    }

    // Beat 2 — Journey (QuestBriefing phase)
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "Keep the canyon wall on your left. The herd leaves tracks you cannot miss — "
            "ground torn up, trees stripped bare. Follow the destruction.");
        Line.AudioURL = "";
        Line.SpeakerRole = ENarr_NPCRole::Elder;
        Line.TriggerType = ENarr_DialogueTrigger::OnQuestPhase;
        Line.RequiredPhase = ENarr_DialoguePhase::QuestBriefing;
        Line.CooldownSeconds = 120.0f;
        MigTree.Lines.Add(Line);
    }

    // Beat 3 — Danger (CrisisPoint phase)
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "The river is low. Lower than my father ever saw it. "
            "Something upstream is wrong. We need to find out what before the herd arrives, "
            "or the crossing will be a slaughter.");
        Line.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782820437243_QuestNPC_Elder_RiverWarning.mp3";
        Line.SpeakerRole = ENarr_NPCRole::Elder;
        Line.TriggerType = ENarr_DialogueTrigger::OnQuestPhase;
        Line.RequiredPhase = ENarr_DialoguePhase::CrisisPoint;
        Line.CooldownSeconds = 0.0f;
        MigTree.Lines.Add(Line);
    }

    // Beat 4 — Arrival (Resolution phase)
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "You crossed the canyon alone. No weapon, no fire, no tribe. "
            "And you are still breathing. That means something. The elders will want to hear your story.");
        Line.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782820435268_QuestNPC_Hunter_PlayerArrival.mp3";
        Line.SpeakerRole = ENarr_NPCRole::Hunter;
        Line.TriggerType = ENarr_DialogueTrigger::OnQuestPhase;
        Line.RequiredPhase = ENarr_DialoguePhase::Resolution;
        Line.CooldownSeconds = 0.0f;
        MigTree.Lines.Add(Line);
    }

    RegisterDialogueTree(MigTree);
}

// ─── LoadCampDefenseDialogue ──────────────────────────────────────────────────
// 3 dramatic beats: Warning → Waves → Final Stand
// Escalating tension: calm briefing → urgent → desperate

void UNarrativeDialogueManager::LoadCampDefenseDialogue()
{
    FNarr_DialogueTree DefTree;
    DefTree.TreeID = FName("Defense_Scout");
    DefTree.OwnerRole = ENarr_NPCRole::Scout;
    DefTree.bIsActive = true;

    // Beat 1 — Warning (Introduction phase)
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "They came from the north last season. Three of them. We lost two hunters before we drove them back. "
            "This time we build the barrier higher. This time nobody dies.");
        Line.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782820428506_QuestNPC_Scout_CampDefenseBrie.mp3";
        Line.SpeakerRole = ENarr_NPCRole::Scout;
        Line.TriggerType = ENarr_DialogueTrigger::OnFirstMeeting;
        Line.RequiredPhase = ENarr_DialoguePhase::Introduction;
        Line.CooldownSeconds = 0.0f;
        DefTree.Lines.Add(Line);
    }

    // Beat 2 — First Wave (MidJourney phase — reused as "wave active")
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "Danger! A predator stalks the camp. Gather your weapons and drive it away before it takes one of our own!");
        Line.AudioURL = ""; // Previous cycle audio from Agent #14
        Line.SpeakerRole = ENarr_NPCRole::Scout;
        Line.TriggerType = ENarr_DialogueTrigger::OnDangerLevel;
        Line.RequiredPhase = ENarr_DialoguePhase::MidJourney;
        Line.CooldownSeconds = 30.0f;
        DefTree.Lines.Add(Line);
    }

    // Beat 3 — Final Stand (CrisisPoint phase)
    {
        FNarr_DialogueLine Line;
        Line.LineText = FText::FromString(
            "The barrier is down! Fall back to the fire circle — they will not cross the flames. "
            "Hold the line until dawn. We survive this together or not at all.");
        Line.AudioURL = "";
        Line.SpeakerRole = ENarr_NPCRole::TribalLeader;
        Line.TriggerType = ENarr_DialogueTrigger::OnDangerLevel;
        Line.RequiredPhase = ENarr_DialoguePhase::CrisisPoint;
        Line.CooldownSeconds = 0.0f;
        DefTree.Lines.Add(Line);
    }

    RegisterDialogueTree(DefTree);
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

FNarr_DialogueTree* UNarrativeDialogueManager::FindTree(FName TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID) return &Tree;
    }
    return nullptr;
}

const FNarr_DialogueTree* UNarrativeDialogueManager::FindTreeConst(FName TreeID) const
{
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID) return &Tree;
    }
    return nullptr;
}
