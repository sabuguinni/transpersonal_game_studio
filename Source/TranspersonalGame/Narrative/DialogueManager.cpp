#include "DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "TimerManager.h"

UDialogueManager::UDialogueManager()
{
    DialogueVolume = 1.0f;
    bSubtitlesEnabled = true;
    bInternalMonologueEnabled = true;
    MaxQueueSize = 10.0f;
    DialogueFadeTime = 0.5f;
    CurrentDialogueStartTime = 0.0f;
    bIsInitialized = false;
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initializing..."));
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        // Main dialogue audio component
        DialogueAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (DialogueAudioComponent)
        {
            DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
            DialogueAudioComponent->OnAudioFinished.AddDynamic(this, &UDialogueManager::OnDialogueAudioFinished);
        }
        
        // Internal monologue audio component (separate for mixing control)
        InternalMonologueAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (InternalMonologueAudioComponent)
        {
            InternalMonologueAudioComponent->SetVolumeMultiplier(DialogueVolume * 0.8f); // Slightly quieter
        }
    }
    
    LoadDialogueData();
    
    // Set up dialogue processing timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueProcessingTimer, this, &UDialogueManager::ProcessDialogueQueue, 0.1f, true);
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initialization complete"));
}

void UDialogueManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Deinitializing..."));
    
    StopAllDialogue();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueProcessingTimer);
        World->GetTimerManager().ClearTimer(SubtitleTimer);
    }
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    if (InternalMonologueAudioComponent)
    {
        InternalMonologueAudioComponent->Stop();
        InternalMonologueAudioComponent = nullptr;
    }
    
    DialogueQueue.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UDialogueManager::Tick(float DeltaTime)
{
    // Update emotional context duration
    if (CurrentEmotionalContext.Duration >= 0.0f)
    {
        CurrentEmotionalContext.Duration += DeltaTime;
    }
    
    // Check for emotional state transitions
    if (CurrentEmotionalContext.bIsTransitioning)
    {
        // Handle transition logic here if needed
    }
}

bool UDialogueManager::TriggerDialogue(FName DialogueID, AActor* Speaker, const FEmotionalContext& EmotionalContext)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Attempted to trigger dialogue before initialization"));
        return false;
    }
    
    FDialogueEntry* DialogueEntry = FindDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Dialogue entry not found: %s"), *DialogueID.ToString());
        return false;
    }
    
    // Get current gameplay tags from game state
    FGameplayTagContainer CurrentTags;
    // TODO: Get actual current tags from game state
    
    if (!ValidateDialogueConditions(*DialogueEntry, CurrentTags))
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Dialogue conditions not met: %s"), *DialogueID.ToString());
        return false;
    }
    
    // Check if we can interrupt current dialogue
    if (IsDialoguePlaying())
    {
        if (!CurrentDialogue.DialogueData.bCanInterrupt && 
            DialogueEntry->Priority <= CurrentDialogue.DialogueData.Priority)
        {
            // Queue the dialogue instead
            QueueDialogue(DialogueID, Speaker, EmotionalContext);
            return true;
        }
        else
        {
            // Stop current dialogue to play higher priority one
            StopCurrentDialogue(true);
        }
    }
    
    // Create queue entry
    FDialogueQueueEntry QueueEntry;
    QueueEntry.DialogueData = *DialogueEntry;
    QueueEntry.Speaker = Speaker;
    QueueEntry.QueueTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Apply emotional context override
    if (EmotionalContext.PrimaryState != EEmotionalState::Calm || EmotionalContext.Intensity != EEmotionalIntensity::Moderate)
    {
        QueueEntry.DialogueData.EmotionalContext = EmotionalContext;
    }
    
    // Start playing immediately
    StartDialoguePlayback(QueueEntry);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Triggered dialogue: %s"), *DialogueID.ToString());
    return true;
}

void UDialogueManager::QueueDialogue(FName DialogueID, AActor* Speaker, const FEmotionalContext& EmotionalContext)
{
    if (DialogueQueue.Num() >= MaxQueueSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Dialogue queue is full, dropping oldest entry"));
        DialogueQueue.RemoveAt(0);
    }
    
    FDialogueEntry* DialogueEntry = FindDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot queue unknown dialogue: %s"), *DialogueID.ToString());
        return;
    }
    
    FDialogueQueueEntry QueueEntry;
    QueueEntry.DialogueData = *DialogueEntry;
    QueueEntry.Speaker = Speaker;
    QueueEntry.QueueTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Apply emotional context override
    if (EmotionalContext.PrimaryState != EEmotionalState::Calm || EmotionalContext.Intensity != EEmotionalIntensity::Moderate)
    {
        QueueEntry.DialogueData.EmotionalContext = EmotionalContext;
    }
    
    DialogueQueue.Add(QueueEntry);
    SortDialogueQueue();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Queued dialogue: %s"), *DialogueID.ToString());
}

void UDialogueManager::StopCurrentDialogue(bool bFadeOut)
{
    if (!IsDialoguePlaying())
    {
        return;
    }
    
    UAudioComponent* ActiveComponent = CurrentDialogue.DialogueData.bIsInternalMonologue ? 
        InternalMonologueAudioComponent : DialogueAudioComponent;
    
    if (ActiveComponent && ActiveComponent->IsPlaying())
    {
        if (bFadeOut)
        {
            ActiveComponent->FadeOut(DialogueFadeTime, 0.0f);
        }
        else
        {
            ActiveComponent->Stop();
        }
    }
    
    // Clear subtitle timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SubtitleTimer);
    }
    
    // Broadcast dialogue finished event
    OnDialogueFinished.Broadcast(CurrentDialogue.DialogueData.DialogueID, CurrentDialogue.Speaker);
    
    // Clear current dialogue
    CurrentDialogue = FDialogueQueueEntry();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Stopped current dialogue"));
}

void UDialogueManager::StopAllDialogue()
{
    StopCurrentDialogue(false);
    DialogueQueue.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Stopped all dialogue"));
}

void UDialogueManager::PlayInternalMonologue(FName DialogueID, const FEmotionalContext& EmotionalContext)
{
    if (!bInternalMonologueEnabled)
    {
        return;
    }
    
    FDialogueEntry* DialogueEntry = FindDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Internal monologue entry not found: %s"), *DialogueID.ToString());
        return;
    }
    
    // Force internal monologue flag
    DialogueEntry->bIsInternalMonologue = true;
    
    TriggerDialogue(DialogueID, nullptr, EmotionalContext);
}

bool UDialogueManager::CanTriggerDialogue(FName DialogueID, const FGameplayTagContainer& CurrentTags) const
{
    FDialogueEntry* DialogueEntry = FindDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        return false;
    }
    
    return ValidateDialogueConditions(*DialogueEntry, CurrentTags);
}

float UDialogueManager::GetCurrentDialogueProgress() const
{
    if (!IsDialoguePlaying() || CurrentDialogue.DialogueData.Duration <= 0.0f)
    {
        return 0.0f;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float ElapsedTime = CurrentTime - CurrentDialogueStartTime;
    
    return FMath::Clamp(ElapsedTime / CurrentDialogue.DialogueData.Duration, 0.0f, 1.0f);
}

void UDialogueManager::SetDialogueVolume(float Volume)
{
    DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
    }
    
    if (InternalMonologueAudioComponent)
    {
        InternalMonologueAudioComponent->SetVolumeMultiplier(DialogueVolume * 0.8f);
    }
}

void UDialogueManager::UpdateEmotionalContext(const FEmotionalContext& NewContext)
{
    CurrentEmotionalContext = NewContext;
    CurrentEmotionalContext.Duration = 0.0f; // Reset duration for new context
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Updated emotional context to %d"), (int32)NewContext.PrimaryState);
}

void UDialogueManager::LoadDialogueData()
{
    // Load dialogue data table
    if (DialogueDataTable.IsValid())
    {
        UDataTable* DataTable = DialogueDataTable.LoadSynchronous();
        if (DataTable)
        {
            UE_LOG(LogTemp, Log, TEXT("DialogueManager: Loaded dialogue data table with %d entries"), DataTable->GetRowNames().Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Failed to load dialogue data table"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: No dialogue data table assigned"));
    }
}

FDialogueEntry* UDialogueManager::FindDialogueEntry(FName DialogueID) const
{
    if (!DialogueDataTable.IsValid())
    {
        return nullptr;
    }
    
    UDataTable* DataTable = DialogueDataTable.LoadSynchronous();
    if (!DataTable)
    {
        return nullptr;
    }
    
    return DataTable->FindRow<FDialogueEntry>(DialogueID, TEXT("FindDialogueEntry"));
}

bool UDialogueManager::ValidateDialogueConditions(const FDialogueEntry& Entry, const FGameplayTagContainer& CurrentTags) const
{
    // Check required tags
    if (Entry.RequiredTags.Num() > 0)
    {
        if (!CurrentTags.HasAll(Entry.RequiredTags))
        {
            return false;
        }
    }
    
    // Check blocking tags
    if (Entry.BlockingTags.Num() > 0)
    {
        if (CurrentTags.HasAny(Entry.BlockingTags))
        {
            return false;
        }
    }
    
    return true;
}

void UDialogueManager::ProcessDialogueQueue()
{
    if (DialogueQueue.Num() == 0 || IsDialoguePlaying())
    {
        return;
    }
    
    // Get the highest priority dialogue from queue
    FDialogueQueueEntry NextDialogue = DialogueQueue[0];
    DialogueQueue.RemoveAt(0);
    
    StartDialoguePlayback(NextDialogue);
}

void UDialogueManager::StartDialoguePlayback(FDialogueQueueEntry& QueueEntry)
{
    CurrentDialogue = QueueEntry;
    CurrentDialogue.bIsPlaying = true;
    CurrentDialogueStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Select appropriate audio component
    UAudioComponent* ActiveComponent = CurrentDialogue.DialogueData.bIsInternalMonologue ? 
        InternalMonologueAudioComponent : DialogueAudioComponent;
    
    // Load and play audio
    if (CurrentDialogue.DialogueData.AudioClip.IsValid() && ActiveComponent)
    {
        USoundWave* SoundWave = CurrentDialogue.DialogueData.AudioClip.LoadSynchronous();
        if (SoundWave)
        {
            ActiveComponent->SetSound(SoundWave);
            ActiveComponent->Play();
            
            // Update duration if not set
            if (CurrentDialogue.DialogueData.Duration <= 0.0f)
            {
                CurrentDialogue.DialogueData.Duration = SoundWave->GetDuration();
            }
        }
    }
    
    // Display subtitles
    if (bSubtitlesEnabled)
    {
        FText SubtitleText = CurrentDialogue.DialogueData.SubtitleOverride.IsEmpty() ? 
            CurrentDialogue.DialogueData.DialogueText : CurrentDialogue.DialogueData.SubtitleOverride;
        
        DisplaySubtitle(SubtitleText, CurrentDialogue.DialogueData.Duration, CurrentDialogue.DialogueData.bIsInternalMonologue);
    }
    
    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(CurrentDialogue.DialogueData.DialogueID, CurrentDialogue.Speaker, CurrentDialogue.DialogueData.DialogueText);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Started playback of dialogue: %s"), *CurrentDialogue.DialogueData.DialogueID.ToString());
}

void UDialogueManager::OnDialogueAudioFinished()
{
    if (IsDialoguePlaying())
    {
        // Broadcast dialogue finished event
        OnDialogueFinished.Broadcast(CurrentDialogue.DialogueData.DialogueID, CurrentDialogue.Speaker);
        
        // Check for follow-up dialogues
        if (CurrentDialogue.DialogueData.FollowUpDialogues.Num() > 0)
        {
            // Queue the first follow-up dialogue
            QueueDialogue(CurrentDialogue.DialogueData.FollowUpDialogues[0], CurrentDialogue.Speaker, CurrentDialogue.DialogueData.EmotionalContext);
        }
        
        // Clear current dialogue
        CurrentDialogue = FDialogueQueueEntry();
        
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Audio finished, dialogue complete"));
    }
}

void UDialogueManager::DisplaySubtitle(const FText& SubtitleText, float Duration, bool bIsInternalMonologue)
{
    // Broadcast subtitle event for UI to handle
    OnSubtitleDisplayed.Broadcast(SubtitleText, Duration, bIsInternalMonologue);
    
    // Set timer to clear subtitle
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(SubtitleTimer, [this]()
        {
            OnSubtitleDisplayed.Broadcast(FText::GetEmpty(), 0.0f, false);
        }, Duration, false);
    }
}

void UDialogueManager::SortDialogueQueue()
{
    // Sort by priority (highest first), then by queue time (oldest first)
    DialogueQueue.Sort([](const FDialogueQueueEntry& A, const FDialogueQueueEntry& B)
    {
        if (A.DialogueData.Priority != B.DialogueData.Priority)
        {
            return A.DialogueData.Priority > B.DialogueData.Priority;
        }
        return A.QueueTime < B.QueueTime;
    });
}