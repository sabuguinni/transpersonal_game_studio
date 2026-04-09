#include "DialogueComponent.h"
#include "NarrativeManager.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Sound/DialogueWave.h"
#include "Sound/DialogueVoice.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    CharacterName = TEXT("Unknown Character");
    DefaultLineDuration = 3.0f;
    bAutoAdvanceDialogue = true;
    AutoAdvanceDelay = 0.5f;
    CurrentState = EDialogueState::Idle;
    CurrentLineIndex = 0;
    CurrentListener = nullptr;
    CurrentAudioComponent = nullptr;
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Load dialogue from data table if available
    LoadDialogueFromDataTable();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Initialized for %s"), *CharacterName);
}

void UDialogueComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopDialogueSequence();
    Super::EndPlay(EndPlayReason);
}

void UDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle dialogue state updates if needed
    if (CurrentState == EDialogueState::Playing)
    {
        // Additional processing during dialogue playback
    }
}

void UDialogueComponent::StartDialogueSequence(FGameplayTag SequenceTag, AActor* Listener)
{
    if (!SequenceTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Invalid sequence tag"));
        return;
    }
    
    if (!CanPlaySequence(SequenceTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Cannot play sequence %s"), *SequenceTag.ToString());
        return;
    }
    
    // Stop any current dialogue
    if (CurrentState != EDialogueState::Idle)
    {
        StopDialogueSequence();
    }
    
    // Find the dialogue sequence
    const FDialogueSequence* Sequence = DialogueSequences.Find(SequenceTag);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Sequence %s not found or empty"), *SequenceTag.ToString());
        return;
    }
    
    // Initialize dialogue state
    CurrentSequenceTag = SequenceTag;
    CurrentLineIndex = 0;
    CurrentListener = Listener;
    CurrentState = EDialogueState::Playing;
    
    // Enable ticking for dialogue updates
    SetComponentTickEnabled(true);
    
    // Start the first line
    ProcessCurrentLine();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Started sequence %s for %s"), 
           *SequenceTag.ToString(), *CharacterName);
}

void UDialogueComponent::StopDialogueSequence()
{
    if (CurrentState == EDialogueState::Idle)
    {
        return;
    }
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LineTimerHandle);
        World->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
    }
    
    // Stop audio
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent = nullptr;
    }
    
    // Reset state
    CurrentState = EDialogueState::Idle;
    CurrentSequenceTag = FGameplayTag::EmptyTag;
    CurrentLineIndex = 0;
    CurrentListener = nullptr;
    
    // Disable ticking
    SetComponentTickEnabled(false);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Stopped dialogue for %s"), *CharacterName);
}

void UDialogueComponent::PauseDialogue()
{
    if (CurrentState == EDialogueState::Playing)
    {
        CurrentState = EDialogueState::Paused;
        
        if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
        {
            CurrentAudioComponent->SetPaused(true);
        }
        
        // Pause timers
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().PauseTimer(LineTimerHandle);
            World->GetTimerManager().PauseTimer(AutoAdvanceTimerHandle);
        }
    }
}

void UDialogueComponent::ResumeDialogue()
{
    if (CurrentState == EDialogueState::Paused)
    {
        CurrentState = EDialogueState::Playing;
        
        if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
        {
            CurrentAudioComponent->SetPaused(false);
        }
        
        // Resume timers
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().UnPauseTimer(LineTimerHandle);
            World->GetTimerManager().UnPauseTimer(AutoAdvanceTimerHandle);
        }
    }
}

void UDialogueComponent::AdvanceDialogue()
{
    if (CurrentState == EDialogueState::Playing || CurrentState == EDialogueState::WaitingForInput)
    {
        CompleteCurrentLine();
        AdvanceToNextLine();
    }
}

void UDialogueComponent::PlayDialogueLine(const FDialogueLine& DialogueLine, AActor* Listener)
{
    if (DialogueLine.DialogueText.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Empty dialogue text"));
        return;
    }
    
    // Stop any current audio
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
    }
    
    // Play audio if available
    if (DialogueLine.DialogueWave)
    {
        CurrentAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            DialogueLine.DialogueWave,
            GetOwner()->GetActorLocation()
        );
        
        if (CurrentAudioComponent)
        {
            CurrentAudioComponent->OnAudioFinished.AddDynamic(this, &UDialogueComponent::OnAudioComponentFinished);
        }
    }
    
    // Broadcast dialogue line
    FString SpeakerName = DialogueLine.SpeakerName.IsEmpty() ? CharacterName : DialogueLine.SpeakerName;
    OnDialogueLineSpoken.Broadcast(SpeakerName, DialogueLine.DialogueText, DialogueLine.DisplayDuration);
    
    // Trigger any events associated with this line
    TriggerLineEvents(DialogueLine);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: %s says: %s"), *SpeakerName, *DialogueLine.DialogueText);
}

void UDialogueComponent::AddDialogueSequence(const FDialogueSequence& NewSequence)
{
    if (NewSequence.SequenceTag.IsValid())
    {
        DialogueSequences.Add(NewSequence.SequenceTag, NewSequence);
        UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Added dialogue sequence %s"), *NewSequence.SequenceTag.ToString());
    }
}

bool UDialogueComponent::HasDialogueSequence(FGameplayTag SequenceTag) const
{
    return DialogueSequences.Contains(SequenceTag);
}

bool UDialogueComponent::CanPlaySequence(FGameplayTag SequenceTag) const
{
    const FDialogueSequence* Sequence = DialogueSequences.Find(SequenceTag);
    if (!Sequence)
    {
        return false;
    }
    
    // Check prerequisites with narrative manager
    if (UNarrativeManager* NarrativeManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeManager>())
    {
        for (const FGameplayTag& PrereqTag : Sequence->PrerequisiteTags)
        {
            if (!NarrativeManager->IsEventCompleted(PrereqTag))
            {
                return false;
            }
        }
    }
    
    return true;
}

void UDialogueComponent::LoadDialogueFromDataTable()
{
    if (DialogueDataTable)
    {
        TArray<FDialogueSequence*> DialogueData;
        DialogueDataTable->GetAllRows<FDialogueSequence>(TEXT("LoadDialogue"), DialogueData);
        
        for (const FDialogueSequence* Sequence : DialogueData)
        {
            if (Sequence && Sequence->SequenceTag.IsValid())
            {
                AddDialogueSequence(*Sequence);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Loaded %d dialogue sequences from data table"), DialogueData.Num());
    }
}

void UDialogueComponent::ProcessCurrentLine()
{
    const FDialogueSequence* CurrentSequence = DialogueSequences.Find(CurrentSequenceTag);
    if (!CurrentSequence || CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        CompleteSequence();
        return;
    }
    
    const FDialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
    
    // Play the dialogue line
    PlayDialogueLine(CurrentLine, CurrentListener);
    
    // Set up timing for line completion
    float LineDuration = CurrentLine.DisplayDuration > 0 ? CurrentLine.DisplayDuration : DefaultLineDuration;
    
    if (CurrentLine.bWaitForInput)
    {
        CurrentState = EDialogueState::WaitingForInput;
    }
    else
    {
        // Auto-advance after duration
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                LineTimerHandle,
                [this]() { CompleteCurrentLine(); AdvanceToNextLine(); },
                LineDuration,
                false
            );
        }
    }
}

void UDialogueComponent::CompleteCurrentLine()
{
    // Clear line timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LineTimerHandle);
    }
    
    if (CurrentState == EDialogueState::WaitingForInput)
    {
        CurrentState = EDialogueState::Playing;
    }
}

void UDialogueComponent::AdvanceToNextLine()
{
    CurrentLineIndex++;
    
    if (bAutoAdvanceDialogue && AutoAdvanceDelay > 0)
    {
        // Delay before next line
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AutoAdvanceTimerHandle,
                [this]() { ProcessCurrentLine(); },
                AutoAdvanceDelay,
                false
            );
        }
    }
    else
    {
        ProcessCurrentLine();
    }
}

void UDialogueComponent::CompleteSequence()
{
    FGameplayTag CompletedSequence = CurrentSequenceTag;
    
    // Broadcast completion
    OnDialogueSequenceComplete.Broadcast(CompletedSequence);
    
    // Stop dialogue
    StopDialogueSequence();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Completed sequence %s"), *CompletedSequence.ToString());
}

void UDialogueComponent::TriggerLineEvents(const FDialogueLine& Line)
{
    if (UNarrativeManager* NarrativeManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeManager>())
    {
        for (const FGameplayTag& EventTag : Line.TriggeredEvents)
        {
            if (EventTag.IsValid())
            {
                NarrativeManager->TriggerStoryEvent(EventTag, Line.DialogueText);
            }
        }
    }
}

void UDialogueComponent::OnAudioComponentFinished()
{
    // Audio finished playing, handle auto-advance if needed
    if (CurrentState == EDialogueState::Playing && bAutoAdvanceDialogue)
    {
        CompleteCurrentLine();
        AdvanceToNextLine();
    }
}