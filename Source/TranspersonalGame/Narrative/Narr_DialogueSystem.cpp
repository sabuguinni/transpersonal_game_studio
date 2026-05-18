#include "Narr_DialogueSystem.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio component for dialogue playback
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    DefaultDialogueVolume = 1.0f;
    bGlobalSubtitlesEnabled = true;
    bIsPlayingDialogue = false;
    CurrentDialogueIndex = 0;
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind audio finished delegate
    if (AudioComponent)
    {
        AudioComponent->OnAudioFinished.AddDynamic(this, &ANarr_DialogueSystem::OnAudioFinished);
        AudioComponent->SetVolumeMultiplier(DefaultDialogueVolume);
    }

    // Initialize story events
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        Event.bHasBeenTriggered = false;
    }
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update dialogue system state
    if (bIsPlayingDialogue && AudioComponent && !AudioComponent->IsPlaying())
    {
        // Check if dialogue finished naturally
        if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
        {
            PlayNextDialogue();
        }
    }
}

void ANarr_DialogueSystem::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            // Check if event can be triggered
            if (Event.bOneTimeOnly && Event.bHasBeenTriggered)
            {
                return;
            }

            // Mark as triggered
            Event.bHasBeenTriggered = true;

            // Play dialogue sequence
            if (Event.DialogueSequence.Num() > 0)
            {
                CurrentDialogueSequence = Event.DialogueSequence;
                CurrentDialogueIndex = 0;
                PlayNextDialogue();
            }

            // Broadcast event
            OnStoryEventTriggered(Event);

            break;
        }
    }
}

void ANarr_DialogueSystem::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    if (bIsPlayingDialogue)
    {
        StopCurrentDialogue();
    }

    CurrentDialogue = DialogueLine;
    bIsPlayingDialogue = true;

    // Play audio if available
    if (DialogueLine.AudioClip && AudioComponent)
    {
        AudioComponent->SetSound(DialogueLine.AudioClip);
        AudioComponent->Play();
    }

    // Set timer for dialogue duration
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &ANarr_DialogueSystem::OnAudioFinished,
            DialogueLine.Duration,
            false
        );
    }

    // Broadcast dialogue started
    OnDialogueStarted(DialogueLine);

    // Log dialogue for debugging
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), 
           *DialogueLine.SpeakerName, 
           *DialogueLine.DialogueText.ToString());
}

void ANarr_DialogueSystem::StopCurrentDialogue()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;

        if (AudioComponent && AudioComponent->IsPlaying())
        {
            AudioComponent->Stop();
        }

        if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }

        OnDialogueFinished(CurrentDialogue);
    }
}

bool ANarr_DialogueSystem::IsDialoguePlaying() const
{
    return bIsPlayingDialogue;
}

void ANarr_DialogueSystem::AddStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    StoryEvents.Add(NewEvent);
}

void ANarr_DialogueSystem::SetSubtitlesEnabled(bool bEnabled)
{
    bGlobalSubtitlesEnabled = bEnabled;
}

void ANarr_DialogueSystem::OnAudioFinished()
{
    if (bIsPlayingDialogue)
    {
        OnDialogueFinished(CurrentDialogue);
        
        if (CurrentDialogueSequence.Num() > 0)
        {
            PlayNextDialogue();
        }
        else
        {
            bIsPlayingDialogue = false;
        }
    }
}

void ANarr_DialogueSystem::PlayNextDialogue()
{
    if (CurrentDialogueIndex < CurrentDialogueSequence.Num())
    {
        PlayDialogueLine(CurrentDialogueSequence[CurrentDialogueIndex]);
        CurrentDialogueIndex++;
    }
    else
    {
        FinishDialogueSequence();
    }
}

void ANarr_DialogueSystem::FinishDialogueSequence()
{
    bIsPlayingDialogue = false;
    CurrentDialogueSequence.Empty();
    CurrentDialogueIndex = 0;

    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence finished"));
}