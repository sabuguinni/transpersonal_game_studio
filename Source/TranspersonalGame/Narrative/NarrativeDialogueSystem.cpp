// NarrativeDialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation of the dialogue system for the "First Tools" tutorial quest arc.

#include "NarrativeDialogueSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// UNarrDialogueLine — per-line playback component
// ─────────────────────────────────────────────────────────────────────────────

UNarrDialogueSystem::UNarrDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsDialoguePlaying = false;
    CurrentLineIndex = 0;
    bAutoAdvance = true;
    AutoAdvanceDelay = 0.5f;
}

void UNarrDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultConversations();
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialise built-in conversations for the "First Tools" quest arc
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::InitializeDefaultConversations()
{
    // ── CONVERSATION 1: Elder intro — quest giver ──────────────────────────
    FNarr_Conversation ElderIntro;
    ElderIntro.ConversationID = FName("elder_intro_first_tools");
    ElderIntro.TriggerType    = ENarr_DialogueTriggerType::QuestStart;
    ElderIntro.bIsRepeatable  = false;

    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName     = FName("Elder");
        Line1.SpeakerRole     = ENarr_SpeakerRole::TribalElder;
        Line1.DialogueText    = TEXT("We lost three hunters last season. Not to the great lizards — to hunger. To cold. To wounds that could have been bound with hide and vine.");
        Line1.AudioAssetPath  = TEXT("tts/1782901171865_Elder_NPC.mp3");
        Line1.DisplayDuration = 5.5f;
        ElderIntro.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName     = FName("Elder");
        Line2.SpeakerRole     = ENarr_SpeakerRole::TribalElder;
        Line2.DialogueText    = TEXT("You will learn to make what you need, or you will not survive the next moon. Start with the axe. Everything else follows from that.");
        Line2.AudioAssetPath  = TEXT("tts/1782901171865_Elder_NPC.mp3");
        Line2.DisplayDuration = 5.0f;
        ElderIntro.Lines.Add(Line2);
    }
    Conversations.Add(ElderIntro.ConversationID, ElderIntro);

    // ── CONVERSATION 2: Crafter teaches recipes ────────────────────────────
    FNarr_Conversation CrafterTeach;
    CrafterTeach.ConversationID = FName("crafter_teach_stone_axe");
    CrafterTeach.TriggerType    = ENarr_DialogueTriggerType::Proximity;
    CrafterTeach.bIsRepeatable  = true;

    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName     = FName("Crafter");
        Line1.SpeakerRole     = ENarr_SpeakerRole::Crafter;
        Line1.DialogueText    = TEXT("Two stones, one stick, a length of vine. That is all you need. Bind the flint to the branch — tight, like your life depends on it. Because out here, it does.");
        Line1.AudioAssetPath  = TEXT("tts/1782901151337_Crafter_NPC.mp3");
        Line1.DisplayDuration = 6.0f;
        CrafterTeach.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName     = FName("Crafter");
        Line2.SpeakerRole     = ENarr_SpeakerRole::Crafter;
        Line2.DialogueText    = TEXT("This axe is your first tool. Your first step from prey to hunter.");
        Line2.AudioAssetPath  = TEXT("tts/1782901151337_Crafter_NPC.mp3");
        Line2.DisplayDuration = 4.0f;
        CrafterTeach.Lines.Add(Line2);
    }
    Conversations.Add(CrafterTeach.ConversationID, CrafterTeach);

    // ── CONVERSATION 3: Scout warns about raptor territory ─────────────────
    FNarr_Conversation ScoutWarning;
    ScoutWarning.ConversationID = FName("scout_raptor_warning");
    ScoutWarning.TriggerType    = ENarr_DialogueTriggerType::Proximity;
    ScoutWarning.bIsRepeatable  = false;

    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName     = FName("Scout");
        Line1.SpeakerRole     = ENarr_SpeakerRole::Scout;
        Line1.DialogueText    = TEXT("Stay low near the eastern ridge. The raptors nest there at dusk — three of them, maybe four. They will not chase you past the river.");
        Line1.AudioAssetPath  = TEXT("tts/1782901181541_Scout_NPC.mp3");
        Line1.DisplayDuration = 5.5f;
        ScoutWarning.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName     = FName("Scout");
        Line2.SpeakerRole     = ENarr_SpeakerRole::Scout;
        Line2.DialogueText    = TEXT("Move fast, stay quiet, and do not run unless you have no choice. Running tells them you are prey.");
        Line2.AudioAssetPath  = TEXT("tts/1782901181541_Scout_NPC.mp3");
        Line2.DisplayDuration = 4.5f;
        ScoutWarning.Lines.Add(Line2);
    }
    Conversations.Add(ScoutWarning.ConversationID, ScoutWarning);

    // ── CONVERSATION 4: Elder quest completion ─────────────────────────────
    FNarr_Conversation ElderComplete;
    ElderComplete.ConversationID = FName("elder_first_tools_complete");
    ElderComplete.TriggerType    = ENarr_DialogueTriggerType::QuestComplete;
    ElderComplete.bIsRepeatable  = false;

    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName     = FName("Elder");
        Line1.SpeakerRole     = ENarr_SpeakerRole::TribalElder;
        Line1.DialogueText    = TEXT("You made it back. And you brought the axe. Good. The tribe needs more than one.");
        Line1.AudioAssetPath  = TEXT("tts/1782901184633_Elder_QuestComplete.mp3");
        Line1.DisplayDuration = 4.5f;
        ElderComplete.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName     = FName("Elder");
        Line2.SpeakerRole     = ENarr_SpeakerRole::TribalElder;
        Line2.DialogueText    = TEXT("Teach the others what you learned. Knowledge shared is survival multiplied. That is how we outlast the great lizards. Not with strength. With memory.");
        Line2.AudioAssetPath  = TEXT("tts/1782901184633_Elder_QuestComplete.mp3");
        Line2.DisplayDuration = 6.0f;
        ElderComplete.Lines.Add(Line2);
    }
    Conversations.Add(ElderComplete.ConversationID, ElderComplete);

    UE_LOG(LogTemp, Log, TEXT("NarrDialogueSystem: Initialised %d conversations"), Conversations.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// StartConversation — begin playback by ID
// ─────────────────────────────────────────────────────────────────────────────
bool UNarrDialogueSystem::StartConversation(FName ConversationID)
{
    if (bIsDialoguePlaying)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrDialogueSystem: Cannot start '%s' — dialogue already playing"), *ConversationID.ToString());
        return false;
    }

    FNarr_Conversation* Conv = Conversations.Find(ConversationID);
    if (!Conv)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrDialogueSystem: Conversation '%s' not found"), *ConversationID.ToString());
        return false;
    }

    if (!Conv->bIsRepeatable && PlayedConversations.Contains(ConversationID))
    {
        UE_LOG(LogTemp, Log, TEXT("NarrDialogueSystem: '%s' already played and is not repeatable"), *ConversationID.ToString());
        return false;
    }

    ActiveConversation = *Conv;
    CurrentLineIndex   = 0;
    bIsDialoguePlaying = true;

    PlayedConversations.Add(ConversationID);
    OnConversationStarted.Broadcast(ConversationID);

    PlayCurrentLine();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// PlayCurrentLine — display current line and schedule advance
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::PlayCurrentLine()
{
    if (!ActiveConversation.Lines.IsValidIndex(CurrentLineIndex))
    {
        EndConversation();
        return;
    }

    const FNarr_DialogueLine& Line = ActiveConversation.Lines[CurrentLineIndex];

    // Broadcast to UI
    OnDialogueLineStarted.Broadcast(Line);

    // Debug display in viewport
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *Line.SpeakerName.ToString(), *Line.DialogueText);
        GEngine->AddOnScreenDebugMessage(
            (uint64)(1000 + CurrentLineIndex),
            Line.DisplayDuration + AutoAdvanceDelay,
            FColor::Yellow,
            DisplayText
        );
    }

    UE_LOG(LogTemp, Log, TEXT("NarrDialogueSystem: [%s] %s"), *Line.SpeakerName.ToString(), *Line.DialogueText);

    // Auto-advance timer
    if (bAutoAdvance && GetWorld())
    {
        FTimerHandle TimerHandle;
        float Delay = Line.DisplayDuration + AutoAdvanceDelay;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            this,
            &UNarrDialogueSystem::AdvanceDialogue,
            Delay,
            false
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AdvanceDialogue — move to next line or end
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::AdvanceDialogue()
{
    if (!bIsDialoguePlaying) return;

    OnDialogueLineEnded.Broadcast(ActiveConversation.Lines[CurrentLineIndex]);
    CurrentLineIndex++;

    if (ActiveConversation.Lines.IsValidIndex(CurrentLineIndex))
    {
        PlayCurrentLine();
    }
    else
    {
        EndConversation();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// EndConversation — clean up and broadcast
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::EndConversation()
{
    bIsDialoguePlaying = false;
    FName CompletedID  = ActiveConversation.ConversationID;
    ActiveConversation = FNarr_Conversation();
    CurrentLineIndex   = 0;

    OnConversationEnded.Broadcast(CompletedID);
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueSystem: Conversation '%s' ended"), *CompletedID.ToString());
}

// ─────────────────────────────────────────────────────────────────────────────
// SkipCurrentLine — player-initiated skip
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::SkipCurrentLine()
{
    if (!bIsDialoguePlaying) return;

    // Cancel auto-advance timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    }

    AdvanceDialogue();
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerConversationByType — called by world events (proximity, combat, etc.)
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::TriggerConversationByType(ENarr_DialogueTriggerType TriggerType)
{
    for (auto& Pair : Conversations)
    {
        FNarr_Conversation& Conv = Pair.Value;
        if (Conv.TriggerType == TriggerType)
        {
            if (Conv.bIsRepeatable || !PlayedConversations.Contains(Conv.ConversationID))
            {
                StartConversation(Conv.ConversationID);
                return; // Play first matching unplayed conversation
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetConversationProgress — returns 0.0-1.0 progress through active conversation
// ─────────────────────────────────────────────────────────────────────────────
float UNarrDialogueSystem::GetConversationProgress() const
{
    if (!bIsDialoguePlaying || ActiveConversation.Lines.Num() == 0)
    {
        return 0.0f;
    }
    return (float)CurrentLineIndex / (float)ActiveConversation.Lines.Num();
}

// ─────────────────────────────────────────────────────────────────────────────
// IsConversationPlayed — check if a conversation has already been seen
// ─────────────────────────────────────────────────────────────────────────────
bool UNarrDialogueSystem::IsConversationPlayed(FName ConversationID) const
{
    return PlayedConversations.Contains(ConversationID);
}

// ─────────────────────────────────────────────────────────────────────────────
// AddConversation — runtime registration (for quest system integration)
// ─────────────────────────────────────────────────────────────────────────────
void UNarrDialogueSystem::AddConversation(const FNarr_Conversation& NewConversation)
{
    Conversations.Add(NewConversation.ConversationID, NewConversation);
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueSystem: Registered conversation '%s'"), *NewConversation.ConversationID.ToString());
}
