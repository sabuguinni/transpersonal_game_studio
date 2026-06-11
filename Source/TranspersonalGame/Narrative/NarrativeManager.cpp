#include "NarrativeManager.h"
#include "NarrativeDialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalCharacter.h"
#include "Engine/Engine.h"

ANarrativeManager::ANarrativeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create dialogue system component
    DialogueSystem = CreateDefaultSubobject<UNarrativeDialogueSystem>(TEXT("DialogueSystem"));

    // Create narrator audio component
    NarratorAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarratorAudio"));
    RootComponent = NarratorAudioComponent;

    // Initialize default values
    PlayerDetectionRadius = 1000.0f;
    bNarrativeSystemActive = true;
    PlayerCharacter = nullptr;
    CurrentDialogueTimer = 0.0f;
    CurrentDialogueIndex = -1;
}

void ANarrativeManager::BeginPlay()
{
    Super::BeginPlay();

    // Find player character
    FindPlayerCharacter();

    // Initialize narrative events
    InitializeNarrativeEvents();
}

void ANarrativeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bNarrativeSystemActive)
    {
        CheckPlayerProximity();
        UpdateDialoguePlayback(DeltaTime);
    }
}

void ANarrativeManager::TriggerNarrativeEvent(const FString& EventID)
{
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID)
        {
            if (Event.bIsOneShot && Event.bHasBeenTriggered)
            {
                return; // Already triggered
            }

            Event.bHasBeenTriggered = true;

            // Create dialogue entry for this event
            FNarr_DialogueEntry DialogueEntry;
            DialogueEntry.Speaker = Event.CharacterName;
            DialogueEntry.Text = Event.DialogueText;
            DialogueEntry.DisplayDuration = 4.0f;

            TArray<FNarr_DialogueEntry> SingleDialogue;
            SingleDialogue.Add(DialogueEntry);

            StartDialogueSequence(SingleDialogue);

            // Trigger blueprint event
            OnNarrativeEventTriggered(EventID, Event.DialogueText);

            break;
        }
    }
}

void ANarrativeManager::StartDialogueSequence(const TArray<FNarr_DialogueEntry>& DialogueEntries)
{
    if (DialogueEntries.Num() > 0)
    {
        ActiveDialogue = DialogueEntries;
        CurrentDialogueIndex = 0;
        CurrentDialogueTimer = 0.0f;

        // Start first dialogue
        const FNarr_DialogueEntry& FirstEntry = ActiveDialogue[0];
        OnDialogueStarted(FirstEntry.Speaker, FirstEntry.Text);

        // Play audio if available
        if (FirstEntry.VoiceClip.IsValid())
        {
            USoundCue* SoundCue = FirstEntry.VoiceClip.LoadSynchronous();
            if (SoundCue && NarratorAudioComponent)
            {
                NarratorAudioComponent->SetSound(SoundCue);
                NarratorAudioComponent->Play();
            }
        }
    }
}

void ANarrativeManager::StopCurrentDialogue()
{
    CurrentDialogueIndex = -1;
    CurrentDialogueTimer = 0.0f;
    ActiveDialogue.Empty();

    if (NarratorAudioComponent)
    {
        NarratorAudioComponent->Stop();
    }

    OnDialogueEnded();
}

bool ANarrativeManager::IsDialoguePlaying() const
{
    return CurrentDialogueIndex >= 0 && CurrentDialogueIndex < ActiveDialogue.Num();
}

void ANarrativeManager::CheckPlayerProximity()
{
    if (!PlayerCharacter)
    {
        FindPlayerCharacter();
        return;
    }

    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    FVector ManagerLocation = GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ManagerLocation);

    // Check for narrative events within range
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Distance <= Event.TriggerRadius)
        {
            if (!Event.bHasBeenTriggered || !Event.bIsOneShot)
            {
                TriggerNarrativeEvent(Event.EventID);
            }
        }
    }
}

void ANarrativeManager::UpdateDialoguePlayback(float DeltaTime)
{
    if (!IsDialoguePlaying())
    {
        return;
    }

    CurrentDialogueTimer += DeltaTime;

    const FNarr_DialogueEntry& CurrentEntry = ActiveDialogue[CurrentDialogueIndex];
    
    if (CurrentDialogueTimer >= CurrentEntry.DisplayDuration)
    {
        CurrentDialogueIndex++;
        CurrentDialogueTimer = 0.0f;

        if (CurrentDialogueIndex < ActiveDialogue.Num())
        {
            // Start next dialogue
            const FNarr_DialogueEntry& NextEntry = ActiveDialogue[CurrentDialogueIndex];
            OnDialogueStarted(NextEntry.Speaker, NextEntry.Text);

            // Play audio if available
            if (NextEntry.VoiceClip.IsValid())
            {
                USoundCue* SoundCue = NextEntry.VoiceClip.LoadSynchronous();
                if (SoundCue && NarratorAudioComponent)
                {
                    NarratorAudioComponent->SetSound(SoundCue);
                    NarratorAudioComponent->Play();
                }
            }
        }
        else
        {
            // End dialogue sequence
            StopCurrentDialogue();
        }
    }
}

void ANarrativeManager::InitializeNarrativeEvents()
{
    // Clear existing events
    NarrativeEvents.Empty();

    // River danger warning
    FNarr_NarrativeEvent RiverEvent;
    RiverEvent.EventID = TEXT("river_danger");
    RiverEvent.DialogueText = TEXT("The ancient hunting grounds stretch before you, survivor. The river runs red with danger - massive predator tracks mark the muddy banks. Stay low, move silent.");
    RiverEvent.CharacterName = TEXT("Narrator");
    RiverEvent.TriggerRadius = 800.0f;
    RiverEvent.bIsOneShot = true;
    NarrativeEvents.Add(RiverEvent);

    // Thunderfoot warning
    FNarr_NarrativeEvent ThunderfootEvent;
    ThunderfootEvent.EventID = TEXT("thunderfoot_warning");
    ThunderfootEvent.DialogueText = TEXT("Warning! Thunderfoot approaches from the eastern ridge. Seek shelter in the caves or climb the tall rocks. Do not attempt to outrun a creature that size.");
    ThunderfootEvent.CharacterName = TEXT("Tribal Scout");
    ThunderfootEvent.TriggerRadius = 1200.0f;
    ThunderfootEvent.bIsOneShot = false;
    NarrativeEvents.Add(ThunderfootEvent);

    // First survival tip
    FNarr_NarrativeEvent SurvivalEvent;
    SurvivalEvent.EventID = TEXT("survival_basics");
    SurvivalEvent.DialogueText = TEXT("Remember the old ways, young hunter. Water flows downhill, predators hunt at dawn and dusk, and the safest path is not always the shortest.");
    SurvivalEvent.CharacterName = TEXT("Elder Voice");
    SurvivalEvent.TriggerRadius = 600.0f;
    SurvivalEvent.bIsOneShot = true;
    NarrativeEvents.Add(SurvivalEvent);
}

void ANarrativeManager::FindPlayerCharacter()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                PlayerCharacter = Cast<ATranspersonalCharacter>(PlayerPawn);
            }
        }
    }
}