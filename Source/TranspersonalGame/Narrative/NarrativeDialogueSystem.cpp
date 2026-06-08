#include "NarrativeDialogueSystem.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"

UNarrativeDialogueSystem::UNarrativeDialogueSystem()
{
    bDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentEntryIndex = 0;
    DialogueDataTable = nullptr;
}

void UNarrativeDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem initialized"));
    
    // Initialize dialogue system
    bDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentEntryIndex = 0;
}

bool UNarrativeDialogueSystem::StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active. Ending current dialogue."));
        EndDialogue();
    }

    if (!Speaker || !Listener)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Speaker or Listener for dialogue"));
        return false;
    }

    if (LoadDialogueTree(DialogueID))
    {
        bDialogueActive = true;
        CurrentDialogueID = DialogueID;
        CurrentEntryIndex = 0;
        CurrentSpeaker = Speaker;
        CurrentListener = Listener;

        UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *DialogueID);
        
        // Trigger dialogue start event
        TriggerDialogueEvent(TEXT("DialogueStart"));
        
        return true;
    }

    return false;
}

void UNarrativeDialogueSystem::EndDialogue()
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Ending dialogue: %s"), *CurrentDialogueID);
        
        // Trigger dialogue end event
        TriggerDialogueEvent(TEXT("DialogueEnd"));
        
        bDialogueActive = false;
        CurrentDialogueID = TEXT("");
        CurrentEntryIndex = 0;
        CurrentDialogueTree = FNarr_DialogueTree();
        CurrentSpeaker.Reset();
        CurrentListener.Reset();
    }
}

bool UNarrativeDialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive || CurrentDialogueTree.DialogueEntries.Num() == 0)
    {
        return false;
    }

    CurrentEntryIndex++;
    
    if (CurrentEntryIndex >= CurrentDialogueTree.DialogueEntries.Num())
    {
        // Check for next dialogue in tree
        if (CurrentDialogueTree.NextDialogueIDs.Num() > 0)
        {
            // Auto-advance to first next dialogue
            FString NextDialogueID = CurrentDialogueTree.NextDialogueIDs[0];
            return StartDialogue(NextDialogueID, CurrentSpeaker.Get(), CurrentListener.Get());
        }
        else
        {
            // End dialogue
            EndDialogue();
            return false;
        }
    }

    // Play audio for current entry
    FNarr_DialogueEntry CurrentEntry = GetCurrentDialogueEntry();
    if (!CurrentEntry.AudioPath.IsEmpty())
    {
        PlayDialogueAudio(CurrentEntry.AudioPath);
    }

    return true;
}

FNarr_DialogueEntry UNarrativeDialogueSystem::GetCurrentDialogueEntry() const
{
    if (bDialogueActive && CurrentDialogueTree.DialogueEntries.IsValidIndex(CurrentEntryIndex))
    {
        return CurrentDialogueTree.DialogueEntries[CurrentEntryIndex];
    }

    return FNarr_DialogueEntry();
}

bool UNarrativeDialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarrativeDialogueSystem::RegisterDialogueTable(UDataTable* DialogueTable)
{
    if (DialogueTable)
    {
        DialogueDataTable = DialogueTable;
        UE_LOG(LogTemp, Log, TEXT("Registered dialogue data table"));
    }
}

TArray<FString> UNarrativeDialogueSystem::GetAvailableChoices() const
{
    TArray<FString> Choices;
    
    if (bDialogueActive)
    {
        // Get choices from next dialogue IDs
        for (const FString& NextID : CurrentDialogueTree.NextDialogueIDs)
        {
            Choices.Add(NextID);
        }
    }
    
    return Choices;
}

void UNarrativeDialogueSystem::SelectChoice(int32 ChoiceIndex)
{
    if (bDialogueActive && CurrentDialogueTree.NextDialogueIDs.IsValidIndex(ChoiceIndex))
    {
        FString SelectedDialogueID = CurrentDialogueTree.NextDialogueIDs[ChoiceIndex];
        StartDialogue(SelectedDialogueID, CurrentSpeaker.Get(), CurrentListener.Get());
    }
}

bool UNarrativeDialogueSystem::LoadDialogueTree(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("No dialogue data table registered"));
        return false;
    }

    FNarr_DialogueTree* FoundDialogue = DialogueDataTable->FindRow<FNarr_DialogueTree>(FName(*DialogueID), TEXT(""));
    
    if (FoundDialogue)
    {
        CurrentDialogueTree = *FoundDialogue;
        UE_LOG(LogTemp, Log, TEXT("Loaded dialogue tree: %s with %d entries"), *DialogueID, CurrentDialogueTree.DialogueEntries.Num());
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("Failed to find dialogue: %s"), *DialogueID);
    return false;
}

void UNarrativeDialogueSystem::PlayDialogueAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty() || !CurrentSpeaker.IsValid())
    {
        return;
    }

    // Load and play audio
    USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *AudioPath);
    if (DialogueSound && CurrentSpeaker.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DialogueSound,
            CurrentSpeaker->GetActorLocation(),
            1.0f,
            1.0f
        );
        
        UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioPath);
    }
}

void UNarrativeDialogueSystem::TriggerDialogueEvent(const FString& EventName)
{
    UE_LOG(LogTemp, Log, TEXT("Dialogue Event: %s"), *EventName);
    
    // Can be extended to trigger Blueprint events or other systems
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("Dialogue Event: %s"), *EventName));
    }
}