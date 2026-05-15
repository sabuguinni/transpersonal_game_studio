#include "NarrativeManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ANarrativeManager::ANarrativeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create audio component for voice playback
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    RootComponent = VoiceAudioComponent;
    
    // Initialize narrative state
    CurrentStoryState = ENarr_StoryState::Introduction;
    DialogueTimer = 0.0f;
    bIsPlayingDialogue = false;
    StoryProgressionTimer = 0.0f;
    CurrentDialogueIndex = 0;
    CurrentStoryEvent = nullptr;
    
    // Set up audio component
    VoiceAudioComponent->bAutoActivate = false;
    VoiceAudioComponent->SetVolumeMultiplier(1.0f);
}

void ANarrativeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize story events when the game starts
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story system initialized"));
}

void ANarrativeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update dialogue timer
    if (bIsPlayingDialogue)
    {
        DialogueTimer -= DeltaTime;
        if (DialogueTimer <= 0.0f)
        {
            bIsPlayingDialogue = false;
            OnDialogueFinished();
            
            // Check if there are more dialogue lines in current event
            if (CurrentStoryEvent && CurrentDialogueIndex < CurrentStoryEvent->DialogueLines.Num() - 1)
            {
                CurrentDialogueIndex++;
                PlayDialogueLine(CurrentStoryEvent->DialogueLines[CurrentDialogueIndex]);
            }
            else
            {
                CurrentStoryEvent = nullptr;
                CurrentDialogueIndex = 0;
            }
        }
    }
    
    // Check for story event triggers
    CheckPlayerProximityToEvents();
    
    // Update story progression timer
    StoryProgressionTimer += DeltaTime;
}

void ANarrativeManager::TriggerStoryEvent(const FString& EventName)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventName == EventName && !Event.bIsTriggered)
        {
            // Check if we're in the right story state
            if (Event.RequiredState == CurrentStoryState || Event.RequiredState == ENarr_StoryState::Introduction)
            {
                Event.bIsTriggered = true;
                CurrentStoryEvent = &Event;
                CurrentDialogueIndex = 0;
                
                if (Event.DialogueLines.Num() > 0)
                {
                    PlayDialogueLine(Event.DialogueLines[0]);
                }
                
                UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Triggered story event: %s"), *EventName);
                break;
            }
        }
    }
}

void ANarrativeManager::AdvanceStoryState()
{
    ENarr_StoryState OldState = CurrentStoryState;
    
    switch (CurrentStoryState)
    {
        case ENarr_StoryState::Introduction:
            CurrentStoryState = ENarr_StoryState::FirstHunt;
            break;
        case ENarr_StoryState::FirstHunt:
            CurrentStoryState = ENarr_StoryState::PackEncounter;
            break;
        case ENarr_StoryState::PackEncounter:
            CurrentStoryState = ENarr_StoryState::ResourceScarcity;
            break;
        case ENarr_StoryState::ResourceScarcity:
            CurrentStoryState = ENarr_StoryState::TribalConflict;
            break;
        case ENarr_StoryState::TribalConflict:
            CurrentStoryState = ENarr_StoryState::SeasonalMigration;
            break;
        case ENarr_StoryState::SeasonalMigration:
            CurrentStoryState = ENarr_StoryState::FinalSurvival;
            break;
        case ENarr_StoryState::FinalSurvival:
            // Story complete
            break;
    }
    
    if (OldState != CurrentStoryState)
    {
        OnStoryStateChanged(CurrentStoryState);
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story state advanced to: %d"), (int32)CurrentStoryState);
    }
}

void ANarrativeManager::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    if (bIsPlayingDialogue)
    {
        StopCurrentDialogue();
    }
    
    bIsPlayingDialogue = true;
    DialogueTimer = DialogueLine.Duration;
    
    // Play voice clip if available
    if (DialogueLine.VoiceClip.IsValid())
    {
        VoiceAudioComponent->SetSound(DialogueLine.VoiceClip.LoadSynchronous());
        VoiceAudioComponent->Play();
    }
    
    // Trigger Blueprint event
    OnDialogueStarted(DialogueLine);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing dialogue: %s - %s"), 
           *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
}

void ANarrativeManager::CheckPlayerProximityToEvents()
{
    // Get player pawn
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Check each story event for proximity triggers
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsTriggered && Event.RequiredState == CurrentStoryState)
        {
            float Distance = FVector::Dist(PlayerLocation, Event.TriggerLocation);
            if (Distance <= Event.TriggerRadius)
            {
                TriggerStoryEvent(Event.EventName);
            }
        }
    }
}

ENarr_StoryState ANarrativeManager::GetCurrentStoryState() const
{
    return CurrentStoryState;
}

void ANarrativeManager::InitializeStoryEvents()
{
    StoryEvents.Empty();
    
    // Introduction Event
    FNarr_StoryEvent IntroEvent;
    IntroEvent.EventName = TEXT("GameIntroduction");
    IntroEvent.RequiredState = ENarr_StoryState::Introduction;
    IntroEvent.TriggerLocation = FVector(0, 0, 0);
    IntroEvent.TriggerRadius = 1000.0f;
    
    FNarr_DialogueLine IntroLine;
    IntroLine.SpeakerName = TEXT("Narrator");
    IntroLine.DialogueText = TEXT("The ancient world awakens. You are alone in a land where giants roam and survival depends on wit, courage, and understanding the rhythms of nature.");
    IntroLine.DialogueType = ENarr_DialogueType::Narration;
    IntroLine.Duration = 8.0f;
    IntroEvent.DialogueLines.Add(IntroLine);
    
    StoryEvents.Add(IntroEvent);
    
    // First Hunt Event
    FNarr_StoryEvent HuntEvent;
    HuntEvent.EventName = TEXT("FirstHuntEncounter");
    HuntEvent.RequiredState = ENarr_StoryState::FirstHunt;
    HuntEvent.TriggerLocation = FVector(2000, 0, 100);
    HuntEvent.TriggerRadius = 800.0f;
    
    FNarr_DialogueLine HuntLine;
    HuntLine.SpeakerName = TEXT("Hunter");
    HuntLine.DialogueText = TEXT("Fresh tracks in the mud. Large herbivore, moving slowly. The hunt begins now - stay downwind and move with patience.");
    HuntLine.DialogueType = ENarr_DialogueType::Instruction;
    HuntLine.Duration = 6.0f;
    HuntEvent.DialogueLines.Add(HuntLine);
    
    StoryEvents.Add(HuntEvent);
    
    // Pack Encounter Event
    FNarr_StoryEvent PackEvent;
    PackEvent.EventName = TEXT("RaptorPackWarning");
    PackEvent.RequiredState = ENarr_StoryState::PackEncounter;
    PackEvent.TriggerLocation = FVector(0, -3000, 100);
    PackEvent.TriggerRadius = 1200.0f;
    
    FNarr_DialogueLine PackLine;
    PackLine.SpeakerName = TEXT("Scout");
    PackLine.DialogueText = TEXT("Pack hunters circle in the distance. They test our defenses, looking for weakness. Show no fear, but prepare for battle.");
    PackLine.DialogueType = ENarr_DialogueType::Warning;
    PackLine.Duration = 7.0f;
    PackEvent.DialogueLines.Add(PackLine);
    
    StoryEvents.Add(PackEvent);
    
    // Resource Scarcity Event
    FNarr_StoryEvent ResourceEvent;
    ResourceEvent.EventName = TEXT("WaterScarcity");
    ResourceEvent.RequiredState = ENarr_StoryState::ResourceScarcity;
    ResourceEvent.TriggerLocation = FVector(3000, 3000, 100);
    ResourceEvent.TriggerRadius = 600.0f;
    
    FNarr_DialogueLine ResourceLine;
    ResourceLine.SpeakerName = TEXT("Elder");
    ResourceLine.DialogueText = TEXT("The streams run low and the earth cracks with thirst. We must venture into dangerous territory to find new water sources.");
    ResourceLine.DialogueType = ENarr_DialogueType::Observation;
    ResourceLine.Duration = 6.5f;
    ResourceEvent.DialogueLines.Add(ResourceLine);
    
    StoryEvents.Add(ResourceEvent);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialized %d story events"), StoryEvents.Num());
}

bool ANarrativeManager::IsDialoguePlaying() const
{
    return bIsPlayingDialogue;
}

void ANarrativeManager::StopCurrentDialogue()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        DialogueTimer = 0.0f;
        VoiceAudioComponent->Stop();
        OnDialogueFinished();
    }
}