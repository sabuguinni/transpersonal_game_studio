#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;
    DialogueVolume = 1.0f;
    VoiceAudioComponent = nullptr;
    bDialogueActive = false;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized"));
    
    // Initialize default prehistoric survival dialogues
    InitializeDefaultDialogues();
}

void UNarr_DialogueSystem::Deinitialize()
{
    if (bDialogueActive)
    {
        StopCurrentDialogue();
    }
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::TriggerDialogue(FName SequenceID, AActor* Speaker)
{
    if (bDialogueActive)
    {
        StopCurrentDialogue();
    }

    FNarr_DialogueSequence* Sequence = DialogueDatabase.Find(SequenceID);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence %s not found or empty"), *SequenceID.ToString());
        return;
    }

    CurrentSequence = Sequence;
    CurrentLineIndex = 0;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Starting dialogue sequence: %s"), *SequenceID.ToString());
    
    // Start the first line
    AdvanceDialogue();
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    bDialogueActive = false;
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;

    // Stop any playing audio
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueDatabase.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID.ToString());
}

void UNarr_DialogueSystem::PlayContextualDialogue(ESurvivalContext Context, const FVector& Location)
{
    if (bDialogueActive)
    {
        return; // Don't interrupt current dialogue
    }

    FNarr_DialogueSequence* BestSequence = FindBestContextualDialogue(Context);
    if (BestSequence)
    {
        TriggerDialogue(BestSequence->SequenceID);
    }
}

void UNarr_DialogueSystem::PlayVoiceLine(USoundBase* VoiceAudio, const FVector& Location)
{
    if (!VoiceAudio)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(World, VoiceAudio, Location, DialogueVolume);
    }
}

void UNarr_DialogueSystem::SetDialogueVolume(float Volume)
{
    DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Tribal Elder Warning Sequence
    FNarr_DialogueSequence ElderWarning;
    ElderWarning.SequenceID = FName("ElderWarning");
    ElderWarning.TriggerContext = ESurvivalContext::Danger;
    ElderWarning.bIsRepeatable = true;
    ElderWarning.Priority = 3;

    FNarr_DialogueLine ElderLine1;
    ElderLine1.SpeakerName = FText::FromString("Tribal Elder");
    ElderLine1.DialogueText = FText::FromString("The ancient river speaks of danger. Massive footprints lead toward the red canyon where the earth-shakers roam.");
    ElderLine1.DisplayDuration = 6.0f;
    ElderWarning.DialogueLines.Add(ElderLine1);

    FNarr_DialogueLine ElderLine2;
    ElderLine2.SpeakerName = FText::FromString("Tribal Elder");
    ElderLine2.DialogueText = FText::FromString("Only the swift and clever survive when the ground trembles beneath their feet.");
    ElderLine2.DisplayDuration = 5.0f;
    ElderWarning.DialogueLines.Add(ElderLine2);

    RegisterDialogueSequence(ElderWarning);

    // Scout Alert Sequence
    FNarr_DialogueSequence ScoutAlert;
    ScoutAlert.SequenceID = FName("ScoutAlert");
    ScoutAlert.TriggerContext = ESurvivalContext::Combat;
    ScoutAlert.bIsRepeatable = true;
    ScoutAlert.Priority = 4;

    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = FText::FromString("Scout");
    ScoutLine1.DialogueText = FText::FromString("Warning! The pack hunters move through the tall grass. Their eyes gleam in the shadows.");
    ScoutLine1.DisplayDuration = 6.0f;
    ScoutAlert.DialogueLines.Add(ScoutLine1);

    FNarr_DialogueLine ScoutLine2;
    ScoutLine2.SpeakerName = FText::FromString("Scout");
    ScoutLine2.DialogueText = FText::FromString("Stay close to the fire, keep your spear ready, and never turn your back on the darkness.");
    ScoutLine2.DisplayDuration = 5.0f;
    ScoutAlert.DialogueLines.Add(ScoutLine2);

    RegisterDialogueSequence(ScoutAlert);

    // Exploration Discovery Sequence
    FNarr_DialogueSequence ExplorationFind;
    ExplorationFind.SequenceID = FName("ExplorationFind");
    ExplorationFind.TriggerContext = ESurvivalContext::Exploration;
    ExplorationFind.bIsRepeatable = false;
    ExplorationFind.Priority = 2;

    FNarr_DialogueLine ExploreeLine1;
    ExploreeLine1.SpeakerName = FText::FromString("Inner Voice");
    ExploreeLine1.DialogueText = FText::FromString("These tracks are fresh. Something large passed through here recently.");
    ExploreeLine1.DisplayDuration = 4.0f;
    ExplorationFind.DialogueLines.Add(ExploreeLine1);

    RegisterDialogueSequence(ExplorationFind);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogue sequences"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!CurrentSequence || CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        StopCurrentDialogue();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
    
    // Display the dialogue text (would integrate with UI system)
    UE_LOG(LogTemp, Log, TEXT("%s: %s"), 
           *CurrentLine.SpeakerName.ToString(), 
           *CurrentLine.DialogueText.ToString());

    // Play voice audio if available
    if (CurrentLine.VoiceAudio.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            FVector PlayerLocation = FVector::ZeroVector;
            if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
            {
                PlayerLocation = PlayerPawn->GetActorLocation();
            }
            PlayVoiceLine(CurrentLine.VoiceAudio.LoadSynchronous(), PlayerLocation);
        }
    }

    // Set timer for next line
    CurrentLineIndex++;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &UNarr_DialogueSystem::AdvanceDialogue,
            CurrentLine.DisplayDuration,
            false
        );
    }
}

FNarr_DialogueSequence* UNarr_DialogueSystem::FindBestContextualDialogue(ESurvivalContext Context)
{
    FNarr_DialogueSequence* BestSequence = nullptr;
    int32 HighestPriority = 0;

    for (auto& DialoguePair : DialogueDatabase)
    {
        FNarr_DialogueSequence& Sequence = DialoguePair.Value;
        if (Sequence.TriggerContext == Context && Sequence.Priority > HighestPriority)
        {
            BestSequence = &Sequence;
            HighestPriority = Sequence.Priority;
        }
    }

    return BestSequence;
}