#include "DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UDialogueManager::UDialogueManager()
{
    bIsDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentSequenceID = TEXT("");
    CurrentDialogueIndex = 0;
    DialogueAudioComponent = nullptr;
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initializing narrative dialogue system"));
    
    // Initialize default dialogues
    InitializeDefaultDialogues();
    
    // Create audio component for dialogue playback
    if (UWorld* World = GetWorld())
    {
        if (AActor* WorldActor = World->GetFirstPlayerController())
        {
            DialogueAudioComponent = NewObject<UAudioComponent>(WorldActor);
            if (DialogueAudioComponent)
            {
                DialogueAudioComponent->RegisterComponent();
                UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Audio component created"));
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initialization complete"));
}

void UDialogueManager::Deinitialize()
{
    StopCurrentDialogue();
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UDialogueManager::StartDialogueSequence(const FString& SequenceID)
{
    if (SequenceID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot start dialogue - empty sequence ID"));
        return;
    }
    
    if (DialogueSequences.Contains(SequenceID))
    {
        const FNarr_DialogueSequence& Sequence = DialogueSequences[SequenceID];
        
        // Check if sequence is repeatable or not completed
        if (Sequence.bIsRepeatable || !CompletedSequences.Contains(SequenceID))
        {
            CurrentSequenceID = SequenceID;
            CurrentDialogueIndex = 0;
            bIsDialogueActive = true;
            
            if (Sequence.DialogueEntries.Num() > 0)
            {
                CurrentDialogueEntry = Sequence.DialogueEntries[0];
                CurrentDialogueID = CurrentDialogueEntry.DialogueID;
                
                // Trigger dialogue started event
                OnDialogueStarted.Broadcast(CurrentDialogueID, CurrentDialogueEntry.DialogueText);
                
                // Play audio if available
                if (!CurrentDialogueEntry.AudioPath.IsEmpty())
                {
                    PlayDialogueAudio(CurrentDialogueEntry.AudioPath);
                }
                
                // Set timer for auto-advance
                if (UWorld* World = GetWorld())
                {
                    World->GetTimerManager().SetTimer(
                        DialogueTimerHandle,
                        this,
                        &UDialogueManager::ProcessNextDialogue,
                        CurrentDialogueEntry.DisplayDuration,
                        false
                    );
                }
                
                UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Started sequence '%s' with dialogue '%s'"), 
                       *SequenceID, *CurrentDialogueID);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Sequence '%s' already completed and not repeatable"), *SequenceID);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Sequence '%s' not found"), *SequenceID);
    }
}

void UDialogueManager::PlayDialogueEntry(const FString& DialogueID)
{
    if (DialogueID.IsEmpty()) return;
    
    // Find dialogue entry in current sequence
    if (DialogueSequences.Contains(CurrentSequenceID))
    {
        const FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceID];
        
        for (int32 i = 0; i < Sequence.DialogueEntries.Num(); i++)
        {
            if (Sequence.DialogueEntries[i].DialogueID == DialogueID)
            {
                CurrentDialogueEntry = Sequence.DialogueEntries[i];
                CurrentDialogueID = DialogueID;
                CurrentDialogueIndex = i;
                
                OnDialogueStarted.Broadcast(CurrentDialogueID, CurrentDialogueEntry.DialogueText);
                
                if (!CurrentDialogueEntry.AudioPath.IsEmpty())
                {
                    PlayDialogueAudio(CurrentDialogueEntry.AudioPath);
                }
                
                UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Playing dialogue '%s'"), *DialogueID);
                break;
            }
        }
    }
}

void UDialogueManager::SelectDialogueResponse(int32 ResponseIndex)
{
    if (!bIsDialogueActive || CurrentDialogueEntry.ResponseOptions.Num() == 0)
    {
        return;
    }
    
    if (ResponseIndex >= 0 && ResponseIndex < CurrentDialogueEntry.NextDialogueIDs.Num())
    {
        FString NextDialogueID = CurrentDialogueEntry.NextDialogueIDs[ResponseIndex];
        
        OnDialogueResponseSelected.Broadcast(CurrentDialogueID, ResponseIndex, NextDialogueID);
        
        if (!NextDialogueID.IsEmpty())
        {
            PlayDialogueEntry(NextDialogueID);
        }
        else
        {
            CompleteDialogueSequence();
        }
        
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Selected response %d, next dialogue: '%s'"), 
               ResponseIndex, *NextDialogueID);
    }
}

void UDialogueManager::StopCurrentDialogue()
{
    if (bIsDialogueActive)
    {
        bIsDialogueActive = false;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }
        
        StopDialogueAudio();
        
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Stopped current dialogue"));
    }
}

void UDialogueManager::TriggerQuestDialogue(ENarr_QuestType QuestType, const FVector& Location)
{
    if (QuestDialogueMap.Contains(QuestType))
    {
        const TArray<FString>& DialogueSequences = QuestDialogueMap[QuestType];
        
        if (DialogueSequences.Num() > 0)
        {
            // Select highest priority available sequence
            FString BestSequenceID;
            int32 HighestPriority = 0;
            
            for (const FString& SequenceID : DialogueSequences)
            {
                if (this->DialogueSequences.Contains(SequenceID))
                {
                    const FNarr_DialogueSequence& Sequence = this->DialogueSequences[SequenceID];
                    
                    if (Sequence.bIsRepeatable || !CompletedSequences.Contains(SequenceID))
                    {
                        if (Sequence.Priority > HighestPriority)
                        {
                            HighestPriority = Sequence.Priority;
                            BestSequenceID = SequenceID;
                        }
                    }
                }
            }
            
            if (!BestSequenceID.IsEmpty())
            {
                StartDialogueSequence(BestSequenceID);
                UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Triggered quest dialogue for type %d at location %s"), 
                       (int32)QuestType, *Location.ToString());
            }
        }
    }
}

void UDialogueManager::RegisterQuestCompletion(ENarr_QuestType QuestType, bool bSuccess)
{
    FString CompletionDialogueID = bSuccess ? TEXT("quest_success_") : TEXT("quest_failure_");
    CompletionDialogueID += FString::FromInt((int32)QuestType);
    
    if (DialogueSequences.Contains(CompletionDialogueID))
    {
        StartDialogueSequence(CompletionDialogueID);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Registered quest completion - Type: %d, Success: %s"), 
           (int32)QuestType, bSuccess ? TEXT("true") : TEXT("false"));
}

void UDialogueManager::LoadDialogueData()
{
    // Load dialogue data from data table or config file
    // For now, use default initialization
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Loaded %d dialogue sequences"), DialogueSequences.Num());
}

void UDialogueManager::AddDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    if (!Sequence.SequenceID.IsEmpty())
    {
        DialogueSequences.Add(Sequence.SequenceID, Sequence);
        
        // Add to quest dialogue mapping
        if (!QuestDialogueMap.Contains(Sequence.TriggerQuestType))
        {
            QuestDialogueMap.Add(Sequence.TriggerQuestType, TArray<FString>());
        }
        QuestDialogueMap[Sequence.TriggerQuestType].Add(Sequence.SequenceID);
        
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Added dialogue sequence '%s'"), *Sequence.SequenceID);
    }
}

FNarr_DialogueSequence UDialogueManager::GetDialogueSequence(const FString& SequenceID) const
{
    if (DialogueSequences.Contains(SequenceID))
    {
        return DialogueSequences[SequenceID];
    }
    
    return FNarr_DialogueSequence();
}

void UDialogueManager::PlayDialogueAudio(const FString& AudioPath)
{
    if (DialogueAudioComponent && !AudioPath.IsEmpty())
    {
        // Try to load sound asset
        USoundBase* Sound = LoadObject<USoundBase>(nullptr, *AudioPath);
        
        if (Sound)
        {
            DialogueAudioComponent->SetSound(Sound);
            DialogueAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Playing audio '%s'"), *AudioPath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Could not load audio '%s'"), *AudioPath);
        }
    }
}

void UDialogueManager::StopDialogueAudio()
{
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->Stop();
    }
}

void UDialogueManager::ProcessNextDialogue()
{
    if (!bIsDialogueActive || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceID];
    
    // Check if there are response options (player choice required)
    if (CurrentDialogueEntry.ResponseOptions.Num() > 0)
    {
        // Wait for player response
        return;
    }
    
    // Auto-advance to next dialogue
    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex < Sequence.DialogueEntries.Num())
    {
        CurrentDialogueEntry = Sequence.DialogueEntries[CurrentDialogueIndex];
        CurrentDialogueID = CurrentDialogueEntry.DialogueID;
        
        OnDialogueStarted.Broadcast(CurrentDialogueID, CurrentDialogueEntry.DialogueText);
        
        if (!CurrentDialogueEntry.AudioPath.IsEmpty())
        {
            PlayDialogueAudio(CurrentDialogueEntry.AudioPath);
        }
        
        // Set timer for next dialogue
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                DialogueTimerHandle,
                this,
                &UDialogueManager::ProcessNextDialogue,
                CurrentDialogueEntry.DisplayDuration,
                false
            );
        }
    }
    else
    {
        CompleteDialogueSequence();
    }
}

void UDialogueManager::CompleteDialogueSequence()
{
    if (!CurrentSequenceID.IsEmpty())
    {
        CompletedSequences.AddUnique(CurrentSequenceID);
        OnDialogueCompleted.Broadcast(CurrentSequenceID);
        
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Completed dialogue sequence '%s'"), *CurrentSequenceID);
    }
    
    bIsDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentSequenceID = TEXT("");
    CurrentDialogueIndex = 0;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    StopDialogueAudio();
}

void UDialogueManager::InitializeDefaultDialogues()
{
    // Introduction sequence
    FNarr_DialogueSequence IntroSequence;
    IntroSequence.SequenceID = TEXT("intro_sequence");
    IntroSequence.TriggerQuestType = ENarr_QuestType::Exploration;
    IntroSequence.bIsRepeatable = false;
    IntroSequence.Priority = 10;
    
    FNarr_DialogueEntry IntroEntry;
    IntroEntry.DialogueID = TEXT("intro_001");
    IntroEntry.SpeakerName = TEXT("Narrator");
    IntroEntry.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators and the promise of discovery."));
    IntroEntry.Context = ENarr_DialogueContext::Narration;
    IntroEntry.DisplayDuration = 8.0f;
    IntroSequence.DialogueEntries.Add(IntroEntry);
    
    AddDialogueSequence(IntroSequence);
    
    // Predator warning sequence
    FNarr_DialogueSequence PredatorSequence;
    PredatorSequence.SequenceID = TEXT("predator_warning");
    PredatorSequence.TriggerQuestType = ENarr_QuestType::Survival;
    PredatorSequence.bIsRepeatable = true;
    PredatorSequence.Priority = 8;
    
    FNarr_DialogueEntry PredatorEntry;
    PredatorEntry.DialogueID = TEXT("predator_001");
    PredatorEntry.SpeakerName = TEXT("Survival Guide");
    PredatorEntry.DialogueText = FText::FromString(TEXT("Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey."));
    PredatorEntry.Context = ENarr_DialogueContext::Warning;
    PredatorEntry.DisplayDuration = 6.0f;
    PredatorSequence.DialogueEntries.Add(PredatorEntry);
    
    AddDialogueSequence(PredatorSequence);
    
    // Migration sequence
    FNarr_DialogueSequence MigrationSequence;
    MigrationSequence.SequenceID = TEXT("migration_guide");
    MigrationSequence.TriggerQuestType = ENarr_QuestType::Observation;
    MigrationSequence.bIsRepeatable = true;
    MigrationSequence.Priority = 6;
    
    FNarr_DialogueEntry MigrationEntry;
    MigrationEntry.DialogueID = TEXT("migration_001");
    MigrationEntry.SpeakerName = TEXT("Field Guide");
    MigrationEntry.DialogueText = FText::FromString(TEXT("The great migration begins at dawn. Follow the herbivore trails to witness one of nature's most spectacular events."));
    MigrationEntry.Context = ENarr_DialogueContext::Information;
    MigrationEntry.DisplayDuration = 7.0f;
    MigrationSequence.DialogueEntries.Add(MigrationEntry);
    
    AddDialogueSequence(MigrationSequence);
    
    // Territorial warning sequence
    FNarr_DialogueSequence TerritorialSequence;
    TerritorialSequence.SequenceID = TEXT("territorial_warning");
    TerritorialSequence.TriggerQuestType = ENarr_QuestType::TerritorialDefense;
    TerritorialSequence.bIsRepeatable = true;
    TerritorialSequence.Priority = 7;
    
    FNarr_DialogueEntry TerritorialEntry;
    TerritorialEntry.DialogueID = TEXT("territorial_001");
    TerritorialEntry.SpeakerName = TEXT("Territorial Alert");
    TerritorialEntry.DialogueText = FText::FromString(TEXT("Territory claimed. The alpha has marked this hunting ground. Approach with extreme caution - territorial disputes are often deadly."));
    TerritorialEntry.Context = ENarr_DialogueContext::Warning;
    TerritorialEntry.DisplayDuration = 8.0f;
    TerritorialSequence.DialogueEntries.Add(TerritorialEntry);
    
    AddDialogueSequence(TerritorialSequence);
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initialized %d default dialogue sequences"), DialogueSequences.Num());
}