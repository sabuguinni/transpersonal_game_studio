// NarrativeDialogueSystem.cpp
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// Full implementation of the dialogue and narrative system for prehistoric survival game.

#include "NarrativeDialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_DialogueComponent — Actor Component Implementation
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    InteractionRadius = 250.0f;
    bHasBeenTriggered = false;
    DefaultSpeakerRole = ENarr_SpeakerRole::Survivor;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueComponent::StartDialogue(const FName& ConversationID)
{
    if (bIsDialogueActive)
    {
        return;
    }

    // Find the conversation in the library
    const FNarr_Conversation* Found = ConversationLibrary.FindByPredicate(
        [&ConversationID](const FNarr_Conversation& Conv)
        {
            return Conv.ConversationID == ConversationID;
        }
    );

    if (!Found || Found->Lines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Conversation '%s' not found or empty."), *ConversationID.ToString());
        return;
    }

    ActiveConversation = *Found;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;

    OnDialogueStarted.Broadcast(ConversationID);
    DisplayCurrentLine();
}

void UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveConversation.Lines.Num())
    {
        EndDialogue();
        return;
    }

    DisplayCurrentLine();
}

void UNarr_DialogueComponent::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    bIsDialogueActive = false;
    bHasBeenTriggered = true;

    // Clear any pending auto-advance timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
    }

    OnDialogueEnded.Broadcast(ActiveConversation.ConversationID);
    ActiveConversation = FNarr_Conversation();
    CurrentLineIndex = 0;
}

void UNarr_DialogueComponent::DisplayCurrentLine()
{
    if (!bIsDialogueActive || !ActiveConversation.Lines.IsValidIndex(CurrentLineIndex))
    {
        return;
    }

    const FNarr_DialogueLine& Line = ActiveConversation.Lines[CurrentLineIndex];
    OnLineDisplayed.Broadcast(Line);

    // Auto-advance if duration is set
    if (Line.DisplayDurationSeconds > 0.0f && ActiveConversation.bAutoAdvance)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AutoAdvanceTimerHandle,
                this,
                &UNarr_DialogueComponent::AdvanceDialogue,
                Line.DisplayDurationSeconds,
                false
            );
        }
    }
}

bool UNarr_DialogueComponent::CanTriggerDialogue() const
{
    // One-shot conversations only trigger once
    if (ActiveConversation.bOneShot && bHasBeenTriggered)
    {
        return false;
    }
    return !bIsDialogueActive;
}

void UNarr_DialogueComponent::AddConversation(const FNarr_Conversation& Conversation)
{
    // Remove existing with same ID first
    ConversationLibrary.RemoveAll([&Conversation](const FNarr_Conversation& Existing)
    {
        return Existing.ConversationID == Conversation.ConversationID;
    });
    ConversationLibrary.Add(Conversation);
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (bIsDialogueActive && ActiveConversation.Lines.IsValidIndex(CurrentLineIndex))
    {
        return ActiveConversation.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

// ============================================================
// UNarr_QuestNarrativeManager — Game Instance Subsystem
// ============================================================

void UNarr_QuestNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Register default lore entries for prehistoric survival
    RegisterDefaultLoreEntries();
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized with %d lore entries."), LoreDatabase.Num());
}

void UNarr_QuestNarrativeManager::Deinitialize()
{
    LoreDatabase.Empty();
    ActiveNarrativeFlags.Empty();
    Super::Deinitialize();
}

void UNarr_QuestNarrativeManager::RegisterLoreEntry(const FNarr_LoreEntry& Entry)
{
    LoreDatabase.Add(Entry.EntryID, Entry);
}

bool UNarr_QuestNarrativeManager::GetLoreEntry(const FName& EntryID, FNarr_LoreEntry& OutEntry) const
{
    if (const FNarr_LoreEntry* Found = LoreDatabase.Find(EntryID))
    {
        OutEntry = *Found;
        return true;
    }
    return false;
}

void UNarr_QuestNarrativeManager::SetNarrativeFlag(const FName& FlagName, bool bValue)
{
    ActiveNarrativeFlags.Add(FlagName, bValue);
    OnNarrativeFlagChanged.Broadcast(FlagName, bValue);
}

bool UNarr_QuestNarrativeManager::GetNarrativeFlag(const FName& FlagName) const
{
    const bool* Found = ActiveNarrativeFlags.Find(FlagName);
    return Found ? *Found : false;
}

void UNarr_QuestNarrativeManager::TriggerNarrativeEvent(const FName& EventID)
{
    OnNarrativeEventTriggered.Broadcast(EventID);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Event triggered — %s"), *EventID.ToString());
}

void UNarr_QuestNarrativeManager::RegisterDefaultLoreEntries()
{
    // Stone Axe lore
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID = FName("Lore_StoneAxe");
        Entry.Title = FText::FromString("The First Axe");
        Entry.BodyText = FText::FromString(
            "A stone axe is not merely a tool — it is the boundary between prey and predator. "
            "Before it, a human is soft and slow. After it, a human can fell a tree, split bone, "
            "and face a threat that would otherwise be a death sentence. The first person who "
            "struck two stones together and saw the edge was not thinking about survival. "
            "They were thinking about tomorrow."
        );
        Entry.UnlockCondition = FName("Crafted_StoneAxe");
        Entry.bIsUnlocked = false;
        LoreDatabase.Add(Entry.EntryID, Entry);
    }

    // Campfire lore
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID = FName("Lore_Campfire");
        Entry.Title = FText::FromString("Fire and Tribe");
        Entry.BodyText = FText::FromString(
            "Fire does three things: it warms, it cooks, and it gathers. Before fire, "
            "humans slept alone in fear. Around fire, they began to talk — to share what "
            "they had seen, what had tried to kill them, where the water was. "
            "The campfire is where tribe begins. It is the first wall ever built."
        );
        Entry.UnlockCondition = FName("Crafted_Campfire");
        Entry.bIsUnlocked = false;
        LoreDatabase.Add(Entry.EntryID, Entry);
    }

    // Raptor lore
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID = FName("Lore_Raptor");
        Entry.Title = FText::FromString("The Pack Hunters");
        Entry.BodyText = FText::FromString(
            "Raptors do not hunt randomly. They test. One will approach from the front "
            "while two more circle wide. They are measuring you — your speed, your reaction, "
            "your fear. If you run, the test is over. You have answered their question. "
            "The only way to survive a raptor encounter is to make them recalculate the cost."
        );
        Entry.UnlockCondition = FName("Survived_RaptorEncounter");
        Entry.bIsUnlocked = false;
        LoreDatabase.Add(Entry.EntryID, Entry);
    }

    // T-Rex lore
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID = FName("Lore_TRex");
        Entry.Title = FText::FromString("The Apex");
        Entry.BodyText = FText::FromString(
            "There is no fighting a Rex. There is only distance, cover, and silence. "
            "It sees movement before it sees shape. It hears heartbeats in the undergrowth. "
            "The hunters who survived a Rex encounter did not fight back — they became "
            "part of the landscape. They became still. They became nothing."
        );
        Entry.UnlockCondition = FName("Survived_TRexEncounter");
        Entry.bIsUnlocked = false;
        LoreDatabase.Add(Entry.EntryID, Entry);
    }

    // Bone Spear lore
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID = FName("Lore_BoneSpear");
        Entry.Title = FText::FromString("Distance is Life");
        Entry.BodyText = FText::FromString(
            "The spear changed everything. Before it, to kill you had to be close enough "
            "to be killed. The spear gave hunters the gift of distance — the ability to "
            "strike without being struck. A bone spear is not elegant. But it is the "
            "difference between a hunt and a suicide."
        );
        Entry.UnlockCondition = FName("Crafted_BoneSpear");
        Entry.bIsUnlocked = false;
        LoreDatabase.Add(Entry.EntryID, Entry);
    }
}
