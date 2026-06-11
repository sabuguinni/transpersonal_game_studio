#include "NarrativeDialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

UNarrativeDialogueSystem::UNarrativeDialogueSystem()
{
    DialogueDataTable = nullptr;
}

void UNarrativeDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Initializing dialogue system"));
    
    LoadDialogueData();
    
    // Initialize default quest dialogues
    TArray<FNarr_DialogueEntry> FirstHuntDialogues;
    
    FNarr_DialogueEntry Entry1;
    Entry1.DialogueID = TEXT("FirstHunt_Start");
    Entry1.SpeakerName = TEXT("Tribal Elder");
    Entry1.DialogueText = TEXT("The ancient hunting grounds hold many secrets. Follow the river downstream to find the sacred cave.");
    Entry1.DisplayDuration = 5.0f;
    Entry1.QuestID = TEXT("FirstHunt");
    Entry1.Priority = 1;
    FirstHuntDialogues.Add(Entry1);
    
    FNarr_DialogueEntry Entry2;
    Entry2.DialogueID = TEXT("FirstHunt_Warning");
    Entry2.SpeakerName = TEXT("Scout");
    Entry2.DialogueText = TEXT("Danger! Massive footprints in the mud ahead. Take cover and wait for the beast to pass.");
    Entry2.DisplayDuration = 4.0f;
    Entry2.QuestID = TEXT("FirstHunt");
    Entry2.Priority = 2;
    FirstHuntDialogues.Add(Entry2);
    
    RegisterQuestDialogue(TEXT("FirstHunt"), FirstHuntDialogues);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Initialization complete"));
}

void UNarrativeDialogueSystem::Deinitialize()
{
    QuestDialogues.Empty();
    ActiveDialogues.Empty();
    
    Super::Deinitialize();
}

void UNarrativeDialogueSystem::RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& DialogueEntries)
{
    if (QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Cannot register dialogue with empty QuestID"));
        return;
    }
    
    FNarr_QuestDialogue NewQuestDialogue;
    NewQuestDialogue.QuestID = QuestID;
    NewQuestDialogue.DialogueEntries = DialogueEntries;
    NewQuestDialogue.bIsActive = false;
    
    QuestDialogues.Add(QuestID, NewQuestDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Registered %d dialogue entries for quest %s"), 
           DialogueEntries.Num(), *QuestID);
}

void UNarrativeDialogueSystem::TriggerDialogue(const FString& DialogueID, AActor* TriggerActor)
{
    if (DialogueID.IsEmpty())
    {
        return;
    }
    
    // Find dialogue entry by ID
    for (auto& QuestPair : QuestDialogues)
    {
        if (QuestPair.Value.bIsActive)
        {
            for (const FNarr_DialogueEntry& Entry : QuestPair.Value.DialogueEntries)
            {
                if (Entry.DialogueID == DialogueID)
                {
                    // Add to active dialogues if not already present
                    bool bAlreadyActive = false;
                    for (const FNarr_DialogueEntry& ActiveEntry : ActiveDialogues)
                    {
                        if (ActiveEntry.DialogueID == DialogueID)
                        {
                            bAlreadyActive = true;
                            break;
                        }
                    }
                    
                    if (!bAlreadyActive)
                    {
                        ActiveDialogues.Add(Entry);
                        SortDialoguesByPriority();
                        
                        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Triggered dialogue %s"), *DialogueID);
                        
                        // Broadcast dialogue event (can be extended for UI integration)
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, Entry.DisplayDuration, FColor::Yellow,
                                FString::Printf(TEXT("%s: %s"), *Entry.SpeakerName, *Entry.DialogueText));
                        }
                    }
                    return;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Dialogue %s not found or quest not active"), *DialogueID);
}

void UNarrativeDialogueSystem::ActivateQuestDialogues(const FString& QuestID)
{
    if (FNarr_QuestDialogue* QuestDialogue = QuestDialogues.Find(QuestID))
    {
        QuestDialogue->bIsActive = true;
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Activated dialogues for quest %s"), *QuestID);
    }
}

void UNarrativeDialogueSystem::DeactivateQuestDialogues(const FString& QuestID)
{
    if (FNarr_QuestDialogue* QuestDialogue = QuestDialogues.Find(QuestID))
    {
        QuestDialogue->bIsActive = false;
        
        // Remove active dialogues for this quest
        ActiveDialogues.RemoveAll([QuestID](const FNarr_DialogueEntry& Entry)
        {
            return Entry.QuestID == QuestID;
        });
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Deactivated dialogues for quest %s"), *QuestID);
    }
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueSystem::GetActiveDialogues() const
{
    return ActiveDialogues;
}

void UNarrativeDialogueSystem::OnQuestStarted(const FString& QuestID)
{
    ActivateQuestDialogues(QuestID);
}

void UNarrativeDialogueSystem::OnQuestCompleted(const FString& QuestID)
{
    DeactivateQuestDialogues(QuestID);
}

void UNarrativeDialogueSystem::OnQuestFailed(const FString& QuestID)
{
    DeactivateQuestDialogues(QuestID);
}

void UNarrativeDialogueSystem::LoadDialogueData()
{
    // Load dialogue data table if available
    if (DialogueDataTable)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: Loading dialogue data from data table"));
        // Implementation for loading from data table can be added here
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem: No dialogue data table specified, using default dialogues"));
    }
}

void UNarrativeDialogueSystem::SortDialoguesByPriority()
{
    ActiveDialogues.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B)
    {
        return A.Priority < B.Priority;
    });
}

// UNarrativeDialogueTrigger Implementation

UNarrativeDialogueTrigger::UNarrativeDialogueTrigger()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    DialogueID = TEXT("");
    RequiredQuestID = TEXT("");
    bTriggerOnce = true;
    TriggerRadius = 200.0f;
    bHasTriggered = false;
    DialogueSystem = nullptr;
}

void UNarrativeDialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        DialogueSystem = GameInstance->GetSubsystem<UNarrativeDialogueSystem>();
    }
}

void UNarrativeDialogueTrigger::TriggerDialogue(AActor* TriggeringActor)
{
    if (!CanTrigger(TriggeringActor))
    {
        return;
    }
    
    if (DialogueSystem)
    {
        DialogueSystem->TriggerDialogue(DialogueID, TriggeringActor);
        
        if (bTriggerOnce)
        {
            bHasTriggered = true;
        }
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueTrigger: Triggered dialogue %s"), *DialogueID);
    }
}

bool UNarrativeDialogueTrigger::CanTrigger(AActor* TriggeringActor) const
{
    if (!TriggeringActor)
    {
        return false;
    }
    
    if (bTriggerOnce && bHasTriggered)
    {
        return false;
    }
    
    if (DialogueID.IsEmpty())
    {
        return false;
    }
    
    // Check if required quest is active (if specified)
    if (!RequiredQuestID.IsEmpty() && DialogueSystem)
    {
        // Quest validation logic can be added here
        // For now, assume quest is active if RequiredQuestID is not empty
    }
    
    return true;
}