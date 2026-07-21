#include "Quest_DialogueManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UQuest_DialogueManager::UQuest_DialogueManager()
{
    bDialogueActive = false;
    CurrentSpeaker = TEXT("");
    CurrentLineIndex = 0;
    CurrentPlayer = nullptr;
    DialogueDataTable = nullptr;
}

void UQuest_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadDialogueData();
}

void UQuest_DialogueManager::LoadDialogueData()
{
    // Load dialogue data table from content
    DialogueDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/TranspersonalGame/Data/DT_NPCDialogue"));
    
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: Failed to load dialogue data table"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Dialogue data table loaded successfully"));
    }
}

bool UQuest_DialogueManager::StartDialogue(const FString& NPCName, AActor* Player)
{
    if (!DialogueDataTable || !Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: Cannot start dialogue - missing data table or player"));
        return false;
    }

    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: Dialogue already active"));
        return false;
    }

    // Find NPC dialogue data
    FQuest_NPCDialogue* DialogueData = DialogueDataTable->FindRow<FQuest_NPCDialogue>(FName(*NPCName), TEXT(""));
    
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueManager: No dialogue data found for NPC: %s"), *NPCName);
        return false;
    }

    // Start dialogue
    CurrentDialogue = *DialogueData;
    CurrentSpeaker = NPCName;
    CurrentPlayer = Player;
    CurrentLineIndex = 0;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Started dialogue with %s"), *NPCName);

    // Play first dialogue line
    if (CurrentDialogue.DialogueLines.Num() > 0)
    {
        PlayDialogueLine(CurrentDialogue.DialogueLines[0]);
    }

    return true;
}

void UQuest_DialogueManager::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    bDialogueActive = false;
    CurrentSpeaker = TEXT("");
    CurrentPlayer = nullptr;
    CurrentLineIndex = 0;

    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Dialogue ended"));
}

bool UQuest_DialogueManager::PlayDialogueLine(const FQuest_DialogueLine& DialogueLine)
{
    if (!bDialogueActive)
    {
        return false;
    }

    // Log dialogue text
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: %s says: %s"), *CurrentSpeaker, *DialogueLine.DialogueText);

    // Play audio if available
    if (!DialogueLine.AudioPath.IsEmpty() && CurrentPlayer)
    {
        UWorld* World = CurrentPlayer->GetWorld();
        if (World)
        {
            // Load and play audio
            USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *DialogueLine.AudioPath);
            if (DialogueSound)
            {
                UGameplayStatics::PlaySoundAtLocation(World, DialogueSound, CurrentPlayer->GetActorLocation());
            }
        }
    }

    // Handle quest triggers
    if (DialogueLine.DialogueType == EQuest_DialogueType::QuestGiver && !CurrentDialogue.QuestID.IsEmpty())
    {
        TriggerQuestEvent(CurrentDialogue.QuestID);
    }

    return true;
}

TArray<FString> UQuest_DialogueManager::GetPlayerResponses(const FString& NPCName)
{
    TArray<FString> Responses;

    if (!bDialogueActive || CurrentLineIndex >= CurrentDialogue.DialogueLines.Num())
    {
        return Responses;
    }

    return CurrentDialogue.DialogueLines[CurrentLineIndex].PlayerResponses;
}

void UQuest_DialogueManager::SelectPlayerResponse(int32 ResponseIndex)
{
    if (!bDialogueActive)
    {
        return;
    }

    ProcessDialogueChoice(ResponseIndex);

    // Move to next dialogue line
    CurrentLineIndex++;

    if (CurrentLineIndex >= CurrentDialogue.DialogueLines.Num())
    {
        // End dialogue
        EndDialogue();
    }
    else
    {
        // Play next line
        PlayDialogueLine(CurrentDialogue.DialogueLines[CurrentLineIndex]);
    }
}

void UQuest_DialogueManager::ProcessDialogueChoice(int32 ChoiceIndex)
{
    // Handle player choice consequences
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Player selected choice %d"), ChoiceIndex);

    // This can trigger different quest branches, reputation changes, etc.
    // Implementation depends on specific quest design requirements
}

void UQuest_DialogueManager::TriggerQuestEvent(const FString& QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Triggering quest event for: %s"), *QuestID);

    // Send quest event to quest manager
    // This would integrate with the Quest_ObjectiveManager system
    // For now, just log the event
}