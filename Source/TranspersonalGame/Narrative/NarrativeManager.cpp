#include "NarrativeManager.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

UNarrativeManager::UNarrativeManager()
{
    DialogueDataTable = nullptr;
    CurrentStoryState = TEXT("Prologue_Start");
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultFlags();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized successfully"));
}

void UNarrativeManager::Deinitialize()
{
    NarrativeFlags.Empty();
    StoryHistory.Empty();
    
    Super::Deinitialize();
}

bool UNarrativeManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue table loaded"));
        return false;
    }

    FDialogueEntry* Entry = FindDialogueEntry(DialogueID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue entry not found: %s"), *DialogueID);
        return false;
    }

    // Check if conditions are met
    if (!CheckNarrativeConditions(Entry->RequiredFlags))
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue conditions not met for: %s"), *DialogueID);
        return false;
    }

    // Process the dialogue
    ProcessDialogueFlags(*Entry);
    
    // Broadcast the dialogue event
    OnDialogueTriggered.Broadcast(Entry->CharacterName, Entry->DialogueText);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered dialogue: %s - %s"), *Entry->CharacterName, *Entry->DialogueText.ToString());
    
    return true;
}

void UNarrativeManager::LoadDialogueTable(UDataTable* DialogueTable)
{
    DialogueDataTable = DialogueTable;
    
    if (DialogueDataTable)
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue table loaded with %d entries"), DialogueDataTable->GetRowNames().Num());
    }
}

void UNarrativeManager::SetNarrativeFlag(const FString& FlagName, bool bValue, float NumericValue)
{
    FNarrativeFlag& Flag = NarrativeFlags.FindOrAdd(FlagName);
    Flag.FlagName = FlagName;
    Flag.bIsSet = bValue;
    Flag.Value = NumericValue;
    
    UE_LOG(LogTemp, Log, TEXT("Set narrative flag: %s = %s (%.2f)"), *FlagName, bValue ? TEXT("true") : TEXT("false"), NumericValue);
}

bool UNarrativeManager::GetNarrativeFlag(const FString& FlagName) const
{
    const FNarrativeFlag* Flag = NarrativeFlags.Find(FlagName);
    return Flag ? Flag->bIsSet : false;
}

float UNarrativeManager::GetNarrativeFlagValue(const FString& FlagName) const
{
    const FNarrativeFlag* Flag = NarrativeFlags.Find(FlagName);
    return Flag ? Flag->Value : 0.0f;
}

bool UNarrativeManager::CheckNarrativeConditions(const TArray<FString>& RequiredFlags) const
{
    for (const FString& FlagName : RequiredFlags)
    {
        if (!GetNarrativeFlag(FlagName))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::AdvanceStoryState(const FString& NewState)
{
    if (CurrentStoryState != NewState)
    {
        StoryHistory.Add(CurrentStoryState);
        CurrentStoryState = NewState;
        
        OnNarrativeStateChanged.Broadcast(CurrentStoryState);
        
        UE_LOG(LogTemp, Log, TEXT("Story state advanced to: %s"), *CurrentStoryState);
    }
}

FDialogueEntry* UNarrativeManager::FindDialogueEntry(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        return nullptr;
    }

    FName RowName(*DialogueID);
    return DialogueDataTable->FindRow<FDialogueEntry>(RowName, TEXT("NarrativeManager"));
}

void UNarrativeManager::ProcessDialogueFlags(const FDialogueEntry& Entry)
{
    // Set any flags that this dialogue should trigger
    for (const FString& FlagToSet : Entry.SetFlags)
    {
        SetNarrativeFlag(FlagToSet, true);
    }
}

void UNarrativeManager::InitializeDefaultFlags()
{
    // Initialize core narrative flags
    SetNarrativeFlag(TEXT("GameStarted"), true);
    SetNarrativeFlag(TEXT("TutorialCompleted"), false);
    SetNarrativeFlag(TEXT("FirstVisionSeen"), false);
    SetNarrativeFlag(TEXT("ShamanMet"), false);
    SetNarrativeFlag(TEXT("ElderTrust"), false, 0.0f);
    SetNarrativeFlag(TEXT("ConsciousnessLevel"), false, 1.0f);
    SetNarrativeFlag(TEXT("TribalStanding"), false, 50.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Default narrative flags initialized"));
}