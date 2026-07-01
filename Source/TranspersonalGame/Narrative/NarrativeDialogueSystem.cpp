// NarrativeDialogueSystem.cpp
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// Full implementation: dialogue trigger, quest dialogue, NPC bark system

#include "NarrativeDialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// UNarr_DialogueComponent — Constructor
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    CurrentConversationID = NAME_None;
    LineDisplayDuration = 4.0f;
    SpeakerRole = ENarr_SpeakerRole::Narrator;
    TriggerType = ENarr_DialogueTriggerType::Proximity;
    TriggerRadius = 300.0f;
    bCanRepeatDialogue = false;
    bHasBeenTriggered = false;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

// ============================================================
// StartConversation — begin a named dialogue sequence
// ============================================================

void UNarr_DialogueComponent::StartConversation(FName ConversationID)
{
    if (bIsDialogueActive)
    {
        return;
    }

    // Find matching conversation in library
    const FNarr_DialogueConversation* Found = nullptr;
    for (const FNarr_DialogueConversation& Conv : DialogueLibrary)
    {
        if (Conv.ConversationID == ConversationID)
        {
            Found = &Conv;
            break;
        }
    }

    if (!Found || Found->Lines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Conversation '%s' not found or empty."), *ConversationID.ToString());
        return;
    }

    if (bHasBeenTriggered && !bCanRepeatDialogue)
    {
        return;
    }

    bIsDialogueActive = true;
    bHasBeenTriggered = true;
    CurrentConversationID = ConversationID;
    CurrentLineIndex = 0;
    ActiveConversation = *Found;

    OnDialogueStarted.Broadcast(ConversationID);
    DisplayCurrentLine();
}

// ============================================================
// AdvanceLine — move to next line or end conversation
// ============================================================

void UNarr_DialogueComponent::AdvanceLine()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveConversation.Lines.Num())
    {
        EndConversation();
        return;
    }

    DisplayCurrentLine();
}

// ============================================================
// EndConversation — clean up and broadcast
// ============================================================

void UNarr_DialogueComponent::EndConversation()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    bIsDialogueActive = false;

    // Clear auto-advance timer if running
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(LineAdvanceTimer);
    }

    OnDialogueEnded.Broadcast(CurrentConversationID);
    CurrentConversationID = NAME_None;
    CurrentLineIndex = 0;
}

// ============================================================
// DisplayCurrentLine — show line and schedule auto-advance
// ============================================================

void UNarr_DialogueComponent::DisplayCurrentLine()
{
    if (!ActiveConversation.Lines.IsValidIndex(CurrentLineIndex))
    {
        EndConversation();
        return;
    }

    const FNarr_DialogueLine& Line = ActiveConversation.Lines[CurrentLineIndex];
    OnLineDisplayed.Broadcast(Line);

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue [%s | %s]: %s"),
        *Line.SpeakerName.ToString(),
        *UEnum::GetValueAsString(Line.SpeakerRole),
        *Line.DialogueText.ToString());

    // Auto-advance after display duration
    float Duration = Line.DisplayDuration > 0.0f ? Line.DisplayDuration : LineDisplayDuration;
    if (GetWorld() && ActiveConversation.bAutoAdvance)
    {
        GetWorld()->GetTimerManager().SetTimer(
            LineAdvanceTimer,
            this,
            &UNarr_DialogueComponent::AdvanceLine,
            Duration,
            false
        );
    }
}

// ============================================================
// TriggerBark — single-line NPC reaction
// ============================================================

void UNarr_DialogueComponent::TriggerBark(ENarr_BarkType BarkType)
{
    // Find a matching bark line in the library
    for (const FNarr_BarkLine& Bark : BarkLibrary)
    {
        if (Bark.BarkType == BarkType)
        {
            OnBarkTriggered.Broadcast(Bark);
            UE_LOG(LogTemp, Log, TEXT("NarrativeBark [%s]: %s"),
                *UEnum::GetValueAsString(BarkType),
                *Bark.BarkText.ToString());
            return;
        }
    }
}

// ============================================================
// GetCurrentLine — safe accessor for UI
// ============================================================

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (bIsDialogueActive && ActiveConversation.Lines.IsValidIndex(CurrentLineIndex))
    {
        return ActiveConversation.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

// ============================================================
// RegisterConversation — runtime registration of new dialogue
// ============================================================

void UNarr_DialogueComponent::RegisterConversation(FNarr_DialogueConversation NewConversation)
{
    // Check for duplicate ID
    for (const FNarr_DialogueConversation& Existing : DialogueLibrary)
    {
        if (Existing.ConversationID == NewConversation.ConversationID)
        {
            UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Conversation '%s' already registered. Skipping."),
                *NewConversation.ConversationID.ToString());
            return;
        }
    }
    DialogueLibrary.Add(NewConversation);
}

// ============================================================
// GetLineCount — utility for UI progress display
// ============================================================

int32 UNarr_DialogueComponent::GetLineCount() const
{
    return ActiveConversation.Lines.Num();
}

int32 UNarr_DialogueComponent::GetCurrentLineIndex() const
{
    return CurrentLineIndex;
}
