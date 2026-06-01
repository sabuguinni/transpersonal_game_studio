#include "Quest_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

UQuest_DialogueSystem::UQuest_DialogueSystem()
{
    CurrentDialogueState = EQuest_DialogueState::None;
    CurrentNPCName = TEXT("");
    DialogueDataTable = nullptr;
}

void UQuest_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DialogueSystem initialized"));
    
    // Initialize dialogue state
    CurrentDialogueState = EQuest_DialogueState::None;
    CurrentNPCName = TEXT("");
    NPCDialogueMap.Empty();
}

void UQuest_DialogueSystem::Deinitialize()
{
    // Clean up any active dialogue
    if (CurrentDialogueState != EQuest_DialogueState::None)
    {
        EndDialogue();
    }
    
    NPCDialogueMap.Empty();
    DialogueDataTable = nullptr;
    
    Super::Deinitialize();
}

bool UQuest_DialogueSystem::StartDialogue(const FString& NPCName, int32 StartingDialogueID)
{
    if (CurrentDialogueState != EQuest_DialogueState::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - dialogue already active"));
        return false;
    }

    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start dialogue - no dialogue data table loaded"));
        return false;
    }

    FQuest_DialogueNode* DialogueNode = FindDialogueNode(StartingDialogueID);
    if (!DialogueNode)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find dialogue node with ID: %d"), StartingDialogueID);
        return false;
    }

    // Start dialogue
    CurrentDialogueState = EQuest_DialogueState::Active;
    CurrentNPCName = NPCName;
    CurrentDialogueNode = *DialogueNode;

    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(NPCName, CurrentDialogueNode.DialogueEntry.DialogueText);

    UE_LOG(LogTemp, Log, TEXT("Started dialogue with %s: %s"), 
           *NPCName, 
           *CurrentDialogueNode.DialogueEntry.DialogueText.ToString());

    return true;
}

void UQuest_DialogueSystem::EndDialogue()
{
    if (CurrentDialogueState == EQuest_DialogueState::None)
    {
        return;
    }

    // Clear dialogue timer if active
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }

    // Reset dialogue state
    CurrentDialogueState = EQuest_DialogueState::None;
    CurrentNPCName = TEXT("");
    CurrentDialogueNode = FQuest_DialogueNode();

    // Broadcast dialogue ended event
    OnDialogueEnded.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

void UQuest_DialogueSystem::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (CurrentDialogueState != EQuest_DialogueState::Active)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot select choice - no active dialogue"));
        return;
    }

    if (!CurrentDialogueNode.Choices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid choice index: %d"), ChoiceIndex);
        return;
    }

    FQuest_DialogueChoice& SelectedChoice = CurrentDialogueNode.Choices[ChoiceIndex];
    
    // Broadcast choice selection event
    OnDialogueChoice.Broadcast(ChoiceIndex, SelectedChoice.ChoiceText);

    UE_LOG(LogTemp, Log, TEXT("Player selected choice: %s"), *SelectedChoice.ChoiceText.ToString());

    // Handle choice result
    if (SelectedChoice.bEndsDialogue)
    {
        EndDialogue();
    }
    else if (SelectedChoice.NextDialogueID >= 0)
    {
        ProcessDialogueNode(SelectedChoice.NextDialogueID);
    }
    else
    {
        // No next dialogue specified, end dialogue
        EndDialogue();
    }
}

void UQuest_DialogueSystem::LoadDialogueDataTable(UDataTable* DialogueTable)
{
    DialogueDataTable = DialogueTable;
    
    if (DialogueDataTable)
    {
        BuildNPCDialogueMap();
        UE_LOG(LogTemp, Log, TEXT("Loaded dialogue data table with %d entries"), 
               DialogueDataTable->GetRowNames().Num());
    }
}

bool UQuest_DialogueSystem::HasDialogueForNPC(const FString& NPCName) const
{
    return NPCDialogueMap.Contains(NPCName);
}

void UQuest_DialogueSystem::ProcessDialogueNode(int32 DialogueID)
{
    FQuest_DialogueNode* DialogueNode = FindDialogueNode(DialogueID);
    if (!DialogueNode)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find dialogue node with ID: %d"), DialogueID);
        EndDialogue();
        return;
    }

    CurrentDialogueNode = *DialogueNode;
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue: %s"), 
           *CurrentDialogueNode.DialogueEntry.DialogueText.ToString());

    // If this dialogue doesn't require player response and has no choices, auto-advance
    if (!CurrentDialogueNode.DialogueEntry.bRequiresPlayerResponse && 
        CurrentDialogueNode.Choices.Num() == 0)
    {
        // Set timer to auto-advance dialogue
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                DialogueTimerHandle,
                [this]() { EndDialogue(); },
                CurrentDialogueNode.DialogueEntry.Duration,
                false
            );
        }
    }
}

void UQuest_DialogueSystem::BuildNPCDialogueMap()
{
    NPCDialogueMap.Empty();
    
    if (!DialogueDataTable)
    {
        return;
    }

    // Get all dialogue rows
    TArray<FQuest_DialogueNode*> AllDialogueRows;
    DialogueDataTable->GetAllRows<FQuest_DialogueNode>(TEXT("BuildNPCDialogueMap"), AllDialogueRows);

    // Build map of NPC names to dialogue IDs
    for (FQuest_DialogueNode* DialogueRow : AllDialogueRows)
    {
        if (DialogueRow && !DialogueRow->DialogueEntry.SpeakerName.IsEmpty())
        {
            FString NPCName = DialogueRow->DialogueEntry.SpeakerName;
            
            if (!NPCDialogueMap.Contains(NPCName))
            {
                NPCDialogueMap.Add(NPCName, TArray<int32>());
            }
            
            NPCDialogueMap[NPCName].Add(DialogueRow->DialogueID);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Built NPC dialogue map for %d NPCs"), NPCDialogueMap.Num());
}

FQuest_DialogueNode* UQuest_DialogueSystem::FindDialogueNode(int32 DialogueID)
{
    if (!DialogueDataTable)
    {
        return nullptr;
    }

    // Search through all rows for matching dialogue ID
    TArray<FQuest_DialogueNode*> AllDialogueRows;
    DialogueDataTable->GetAllRows<FQuest_DialogueNode>(TEXT("FindDialogueNode"), AllDialogueRows);

    for (FQuest_DialogueNode* DialogueRow : AllDialogueRows)
    {
        if (DialogueRow && DialogueRow->DialogueID == DialogueID)
        {
            return DialogueRow;
        }
    }

    return nullptr;
}