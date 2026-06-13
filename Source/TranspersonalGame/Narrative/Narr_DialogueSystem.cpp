#include "Narr_DialogueSystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TranspersonalGame.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    bDialogueActive = false;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    DialogueDataTable = nullptr;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Narrative Dialogue System initialized"));
    
    bDialogueActive = false;
    CurrentDialogue = FNarr_DialogueEntry();
    RegisteredDialogueActors.Empty();
}

bool UNarr_DialogueSystem::StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (!Speaker || !Listener)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot start dialogue: Invalid speaker or listener"));
        return false;
    }

    if (bDialogueActive)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot start dialogue: Another dialogue is already active"));
        return false;
    }

    if (!LoadDialogueEntry(DialogueID))
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot start dialogue: Failed to load dialogue entry %s"), *DialogueID);
        return false;
    }

    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    bDialogueActive = true;

    UE_LOG(LogTranspersonalGame, Log, TEXT("Started dialogue %s between %s and %s"), 
           *DialogueID, 
           *Speaker->GetName(), 
           *Listener->GetName());

    return true;
}

void UNarr_DialogueSystem::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    UE_LOG(LogTranspersonalGame, Log, TEXT("Ending dialogue"));

    bDialogueActive = false;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    CurrentDialogue = FNarr_DialogueEntry();
}

bool UNarr_DialogueSystem::SelectPlayerChoice(int32 ChoiceIndex)
{
    if (!bDialogueActive)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot select choice: No active dialogue"));
        return false;
    }

    if (CurrentDialogue.DialogueType != ENarr_DialogueType::PlayerChoice)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot select choice: Current dialogue is not a player choice"));
        return false;
    }

    if (ChoiceIndex < 0 || ChoiceIndex >= CurrentDialogue.PlayerChoices.Num())
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot select choice: Invalid choice index %d"), ChoiceIndex);
        return false;
    }

    UE_LOG(LogTranspersonalGame, Log, TEXT("Player selected choice %d: %s"), 
           ChoiceIndex, 
           *CurrentDialogue.PlayerChoices[ChoiceIndex]);

    ProcessDialogueChoice(CurrentDialogue.NextDialogueID);
    return true;
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueSystem::LoadDialogueData(UDataTable* DialogueTable)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot load dialogue data: Invalid data table"));
        return;
    }

    DialogueDataTable = DialogueTable;
    UE_LOG(LogTranspersonalGame, Log, TEXT("Loaded dialogue data table with %d entries"), 
           DialogueTable->GetRowNames().Num());
}

void UNarr_DialogueSystem::RegisterDialogueActor(AActor* Actor, const FString& ActorID)
{
    if (!Actor || ActorID.IsEmpty())
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot register dialogue actor: Invalid actor or ID"));
        return;
    }

    RegisteredDialogueActors.Add(ActorID, Actor);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Registered dialogue actor %s with ID %s"), 
           *Actor->GetName(), 
           *ActorID);
}

bool UNarr_DialogueSystem::LoadDialogueEntry(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot load dialogue entry: No data table loaded"));
        return false;
    }

    FNarr_DialogueEntry* FoundEntry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT("LoadDialogueEntry"));
    
    if (!FoundEntry)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Dialogue entry not found: %s"), *DialogueID);
        return false;
    }

    if (!CheckDialogueRequirements(*FoundEntry))
    {
        UE_LOG(LogTranspersonalGame, Log, TEXT("Dialogue requirements not met for: %s"), *DialogueID);
        return false;
    }

    CurrentDialogue = *FoundEntry;
    return true;
}

void UNarr_DialogueSystem::ProcessDialogueChoice(const FString& NextDialogueID)
{
    if (NextDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }

    if (!LoadDialogueEntry(NextDialogueID))
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Failed to load next dialogue: %s"), *NextDialogueID);
        EndDialogue();
    }
}

bool UNarr_DialogueSystem::CheckDialogueRequirements(const FNarr_DialogueEntry& Entry)
{
    if (!Entry.bRequiresItem)
    {
        return true;
    }

    // TODO: Implement item requirement checking when inventory system is available
    UE_LOG(LogTranspersonalGame, Log, TEXT("Item requirement check not implemented yet: %s"), 
           *Entry.RequiredItemName);
    
    return true; // For now, assume all requirements are met
}