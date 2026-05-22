#include "NarrativeDialogueManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    DialogueDataTable = nullptr;
}

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadDialogueData();
    
    // Initialize default context
    CurrentContext.CurrentBiome = ENarr_BiomeType::Savana;
    CurrentContext.TimeOfDay = ENarr_TimeOfDay::Day;
    CurrentContext.ThreatLevel = 0.0f;
    CurrentContext.SurvivalDays = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager initialized"));
}

void UNarrativeDialogueManager::Deinitialize()
{
    PlayedDialogues.Empty();
    Super::Deinitialize();
}

void UNarrativeDialogueManager::LoadDialogueData()
{
    // In a real implementation, this would load from a DataTable asset
    // For now, we'll create some default entries programmatically
    UE_LOG(LogTemp, Warning, TEXT("Loading dialogue data..."));
}

void UNarrativeDialogueManager::TriggerDialogue(const FString& DialogueID, const FNarr_NarrativeContext& Context)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue data table found"));
        return;
    }

    FNarr_DialogueEntry* Entry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT(""));
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue entry not found: %s"), *DialogueID);
        return;
    }

    if (ShouldPlayDialogue(*Entry, Context))
    {
        OnDialogueTriggered.Broadcast(*Entry, Context);
        
        if (!Entry->bIsRepeatable)
        {
            PlayedDialogues.AddUnique(DialogueID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Triggered dialogue: %s - %s"), *Entry->SpeakerName.ToString(), *Entry->DialogueText.ToString());
    }
}

void UNarrativeDialogueManager::TriggerContextualDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_NarrativeContext& Context)
{
    TArray<FNarr_DialogueEntry> ContextualDialogues = GetContextualDialogues(TriggerType, Context);
    
    if (ContextualDialogues.Num() == 0)
    {
        return;
    }

    // Sort by priority and select the highest priority dialogue
    ContextualDialogues.Sort([&](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B) {
        float PriorityA = CalculateDialoguePriority(A, Context);
        float PriorityB = CalculateDialoguePriority(B, Context);
        return PriorityA > PriorityB;
    });

    const FNarr_DialogueEntry& SelectedDialogue = ContextualDialogues[0];
    TriggerDialogue(SelectedDialogue.DialogueID, Context);
}

void UNarrativeDialogueManager::UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext)
{
    FNarr_NarrativeContext PreviousContext = CurrentContext;
    CurrentContext = NewContext;

    // Check for context changes that should trigger dialogue
    if (PreviousContext.CurrentBiome != NewContext.CurrentBiome)
    {
        TriggerContextualDialogue(ENarr_DialogueTrigger::BiomeEnter, NewContext);
    }

    if (PreviousContext.ThreatLevel < 0.5f && NewContext.ThreatLevel >= 0.5f)
    {
        TriggerContextualDialogue(ENarr_DialogueTrigger::ThreatDetected, NewContext);
    }

    if (PreviousContext.TimeOfDay != NewContext.TimeOfDay)
    {
        TriggerContextualDialogue(ENarr_DialogueTrigger::TimeChange, NewContext);
    }
}

FNarr_DialogueEntry UNarrativeDialogueManager::GetDialogueEntry(const FString& DialogueID) const
{
    if (!DialogueDataTable)
    {
        return FNarr_DialogueEntry();
    }

    FNarr_DialogueEntry* Entry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT(""));
    if (Entry)
    {
        return *Entry;
    }

    return FNarr_DialogueEntry();
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueManager::GetContextualDialogues(ENarr_DialogueTrigger TriggerType, const FNarr_NarrativeContext& Context) const
{
    TArray<FNarr_DialogueEntry> Result;
    
    if (!DialogueDataTable)
    {
        return Result;
    }

    // Get all dialogue entries
    TArray<FNarr_DialogueEntry*> AllEntries;
    DialogueDataTable->GetAllRows<FNarr_DialogueEntry>(TEXT(""), AllEntries);

    for (FNarr_DialogueEntry* Entry : AllEntries)
    {
        if (Entry && Entry->TriggerType == TriggerType && ShouldPlayDialogue(*Entry, Context))
        {
            Result.Add(*Entry);
        }
    }

    return Result;
}

bool UNarrativeDialogueManager::ShouldPlayDialogue(const FNarr_DialogueEntry& Entry, const FNarr_NarrativeContext& Context) const
{
    // Check if dialogue was already played and is not repeatable
    if (!Entry.bIsRepeatable && PlayedDialogues.Contains(Entry.DialogueID))
    {
        return false;
    }

    // Add contextual checks here based on biome, threat level, etc.
    // For now, allow all dialogues
    return true;
}

float UNarrativeDialogueManager::CalculateDialoguePriority(const FNarr_DialogueEntry& Entry, const FNarr_NarrativeContext& Context) const
{
    float Priority = Entry.Priority;

    // Increase priority based on context relevance
    if (Context.ThreatLevel > 0.7f && Entry.DialogueID.Contains(TEXT("threat")))
    {
        Priority += 2.0f;
    }

    if (Context.SurvivalDays == 0 && Entry.DialogueID.Contains(TEXT("tutorial")))
    {
        Priority += 1.5f;
    }

    return Priority;
}