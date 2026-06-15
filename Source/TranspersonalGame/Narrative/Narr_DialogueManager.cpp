#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    DialogueDataTable = nullptr;
    DialogueAudioComponent = nullptr;
    DefaultAudioVolume = 1.0f;
    bAutoAdvanceDialogue = false;
    AutoAdvanceDelay = 3.0f;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Manager initialized"));
    
    // Initialize conversation state
    CurrentConversation = FNarr_ConversationState();
}

bool UNarr_DialogueManager::StartConversation(const FString& NPCName, const FString& InitialDialogueID)
{
    if (CurrentConversation.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start conversation - another conversation is already active"));
        return false;
    }

    FNarr_DialogueEntry* DialogueEntry = FindDialogueEntry(InitialDialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find dialogue entry with ID: %s"), *InitialDialogueID);
        return false;
    }

    // Initialize conversation state
    CurrentConversation.bIsActive = true;
    CurrentConversation.ConversationPartner = NPCName;
    CurrentConversation.CurrentDialogueID = InitialDialogueID;
    CurrentConversation.ConversationStartTime = GetWorld()->GetTimeSeconds();
    CurrentConversation.DialogueHistory.Empty();

    // Process the initial dialogue
    ProcessDialogueEntry(*DialogueEntry);

    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(DialogueEntry->SpeakerName, DialogueEntry->DialogueText);

    UE_LOG(LogTemp, Log, TEXT("Started conversation with %s using dialogue ID: %s"), *NPCName, *InitialDialogueID);
    return true;
}

void UNarr_DialogueManager::EndConversation()
{
    if (!CurrentConversation.bIsActive)
    {
        return;
    }

    FString ConversationPartner = CurrentConversation.ConversationPartner;

    // Stop any playing audio
    StopDialogueAudio();

    // Clear auto-advance timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(AutoAdvanceTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoAdvanceTimer);
    }

    // Reset conversation state
    CurrentConversation = FNarr_ConversationState();

    // Broadcast dialogue ended event
    OnDialogueEnded.Broadcast(ConversationPartner);

    UE_LOG(LogTemp, Log, TEXT("Ended conversation with %s"), *ConversationPartner);
}

bool UNarr_DialogueManager::SelectResponse(int32 ResponseIndex)
{
    if (!CurrentConversation.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot select response - no active conversation"));
        return false;
    }

    FNarr_DialogueEntry* CurrentEntry = FindDialogueEntry(CurrentConversation.CurrentDialogueID);
    if (!CurrentEntry)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find current dialogue entry"));
        return false;
    }

    if (ResponseIndex < 0 || ResponseIndex >= CurrentEntry->ResponseOptions.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid response index: %d"), ResponseIndex);
        return false;
    }

    if (ResponseIndex >= CurrentEntry->NextDialogueIDs.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("No next dialogue ID for response index: %d"), ResponseIndex);
        return false;
    }

    FString SelectedResponse = CurrentEntry->ResponseOptions[ResponseIndex];
    FString NextDialogueID = CurrentEntry->NextDialogueIDs[ResponseIndex];

    // Update conversation history
    UpdateConversationHistory(CurrentConversation.CurrentDialogueID);

    // Check if this ends the conversation
    bool bEndsConversation = NextDialogueID.IsEmpty() || NextDialogueID.Equals(TEXT("END"));

    // Broadcast response selected event
    OnResponseSelected.Broadcast(SelectedResponse, NextDialogueID, bEndsConversation);

    if (bEndsConversation)
    {
        EndConversation();
        return true;
    }

    // Continue to next dialogue
    CurrentConversation.CurrentDialogueID = NextDialogueID;
    FNarr_DialogueEntry* NextEntry = FindDialogueEntry(NextDialogueID);
    if (NextEntry)
    {
        ProcessDialogueEntry(*NextEntry);
        OnDialogueStarted.Broadcast(NextEntry->SpeakerName, NextEntry->DialogueText);
    }

    UE_LOG(LogTemp, Log, TEXT("Selected response: %s, Next dialogue: %s"), *SelectedResponse, *NextDialogueID);
    return true;
}

FNarr_DialogueEntry UNarr_DialogueManager::GetCurrentDialogue() const
{
    if (!CurrentConversation.bIsActive)
    {
        return FNarr_DialogueEntry();
    }

    FNarr_DialogueEntry* Entry = const_cast<UNarr_DialogueManager*>(this)->FindDialogueEntry(CurrentConversation.CurrentDialogueID);
    if (Entry)
    {
        return *Entry;
    }

    return FNarr_DialogueEntry();
}

TArray<FString> UNarr_DialogueManager::GetCurrentResponseOptions() const
{
    FNarr_DialogueEntry CurrentEntry = GetCurrentDialogue();
    return CurrentEntry.ResponseOptions;
}

bool UNarr_DialogueManager::IsConversationActive() const
{
    return CurrentConversation.bIsActive;
}

void UNarr_DialogueManager::LoadDialogueData(UDataTable* DialogueTable)
{
    DialogueDataTable = DialogueTable;
    UE_LOG(LogTemp, Log, TEXT("Loaded dialogue data table"));
}

void UNarr_DialogueManager::PlayDialogueAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty())
    {
        return;
    }

    // Stop current audio if playing
    StopDialogueAudio();

    // Load and play the audio asset
    USoundBase* AudioAsset = LoadObject<USoundBase>(nullptr, *AudioPath);
    if (AudioAsset && GetWorld())
    {
        DialogueAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), AudioAsset, DefaultAudioVolume);
        UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not load audio asset: %s"), *AudioPath);
    }
}

void UNarr_DialogueManager::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsValidLowLevel())
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
}

FNarr_DialogueEntry* UNarr_DialogueManager::FindDialogueEntry(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("No dialogue data table loaded"));
        return nullptr;
    }

    FNarr_DialogueEntry* Entry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(*DialogueID, TEXT("FindDialogueEntry"));
    return Entry;
}

void UNarr_DialogueManager::ProcessDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    // Play audio if available
    if (!Entry.AudioAssetPath.IsEmpty())
    {
        PlayDialogueAudio(Entry.AudioAssetPath);
    }

    // Handle quest-related dialogue
    if (Entry.bIsQuestRelated)
    {
        HandleQuestDialogue(Entry);
    }

    // Set up auto-advance if enabled and no response options
    if (bAutoAdvanceDialogue && Entry.ResponseOptions.Num() == 0 && GetWorld())
    {
        float DelayTime = FMath::Max(AutoAdvanceDelay, Entry.AudioDuration);
        GetWorld()->GetTimerManager().SetTimer(AutoAdvanceTimer, this, &UNarr_DialogueManager::AutoAdvanceDialogue, DelayTime, false);
    }
}

void UNarr_DialogueManager::HandleQuestDialogue(const FNarr_DialogueEntry& Entry)
{
    if (!Entry.QuestID.IsEmpty())
    {
        // Quest integration would go here
        // For now, just log the quest interaction
        UE_LOG(LogTemp, Log, TEXT("Quest dialogue triggered for quest: %s"), *Entry.QuestID);
    }
}

void UNarr_DialogueManager::UpdateConversationHistory(const FString& DialogueID)
{
    CurrentConversation.DialogueHistory.Add(DialogueID);
    
    // Keep history manageable - limit to last 20 entries
    if (CurrentConversation.DialogueHistory.Num() > 20)
    {
        CurrentConversation.DialogueHistory.RemoveAt(0);
    }
}

void UNarr_DialogueManager::AutoAdvanceDialogue()
{
    if (CurrentConversation.bIsActive)
    {
        FNarr_DialogueEntry* CurrentEntry = FindDialogueEntry(CurrentConversation.CurrentDialogueID);
        if (CurrentEntry && CurrentEntry->NextDialogueIDs.Num() > 0)
        {
            // Auto-advance to first next dialogue option
            SelectResponse(0);
        }
        else
        {
            // No more dialogue, end conversation
            EndConversation();
        }
    }
}