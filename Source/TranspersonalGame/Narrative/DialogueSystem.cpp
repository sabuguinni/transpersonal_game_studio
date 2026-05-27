#include "DialogueSystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsPlaying = false;
    CurrentLineIndex = 0;
    DialogueVolume = 1.0f;
    DialogueAudioComponent = nullptr;
    
    InitializeDefaultSequences();
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueSystem initialized with %d sequences"), RegisteredSequences.Num());
}

void UDialogueSystem::Deinitialize()
{
    StopCurrentDialogue();
    RegisteredSequences.Empty();
    
    Super::Deinitialize();
}

void UDialogueSystem::PlayDialogueSequence(const FString& SequenceID)
{
    if (bIsPlaying)
    {
        StopCurrentDialogue();
    }

    if (FNarr_DialogueSequence* Sequence = RegisteredSequences.Find(SequenceID))
    {
        CurrentSequence = *Sequence;
        CurrentLineIndex = 0;
        bIsPlaying = true;
        
        PlayNextLine();
        
        UE_LOG(LogTemp, Warning, TEXT("Playing dialogue sequence: %s"), *SequenceID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    }
}

void UDialogueSystem::StopCurrentDialogue()
{
    if (bIsPlaying)
    {
        bIsPlaying = false;
        CurrentLineIndex = 0;
        
        if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
        {
            DialogueAudioComponent->Stop();
        }
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Dialogue stopped"));
    }
}

bool UDialogueSystem::IsDialoguePlaying() const
{
    return bIsPlaying;
}

void UDialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    RegisteredSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Warning, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

void UDialogueSystem::TriggerContextualDialogue(ENarr_TriggerType TriggerType, const FVector& Location)
{
    if (bIsPlaying)
    {
        return; // Don't interrupt current dialogue
    }

    FString SequenceToPlay;
    
    switch (TriggerType)
    {
        case ENarr_TriggerType::DinosaurEncounter:
            SequenceToPlay = TEXT("DangerWarning");
            break;
        case ENarr_TriggerType::SurvivalStatus:
            SequenceToPlay = TEXT("SurvivalTips");
            break;
        case ENarr_TriggerType::ResourceDiscovery:
            SequenceToPlay = TEXT("Discovery");
            break;
        default:
            return;
    }
    
    PlayDialogueSequence(SequenceToPlay);
}

void UDialogueSystem::SetDialogueVolume(float Volume)
{
    DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
    }
}

void UDialogueSystem::PlayNextLine()
{
    if (!bIsPlaying || CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        OnLineFinished();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    
    // Display text (in a real game, this would go to UI)
    FString DisplayText = FString::Printf(TEXT("%s: %s"), 
        *CurrentLine.SpeakerName, 
        *CurrentLine.DialogueText.ToString());
    
    UE_LOG(LogTemp, Warning, TEXT("DIALOGUE: %s"), *DisplayText);
    
    // Play audio if available
    if (CurrentLine.VoiceAudio.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            if (!DialogueAudioComponent)
            {
                DialogueAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
            }
            
            if (DialogueAudioComponent)
            {
                DialogueAudioComponent->SetSound(CurrentLine.VoiceAudio.LoadSynchronous());
                DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
                DialogueAudioComponent->Play();
            }
        }
    }
    
    // Set timer for next line
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &UDialogueSystem::OnLineFinished,
            CurrentLine.Duration,
            false
        );
    }
}

void UDialogueSystem::OnLineFinished()
{
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // Sequence finished
        bIsPlaying = false;
        CurrentLineIndex = 0;
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence completed"));
    }
    else
    {
        PlayNextLine();
    }
}

void UDialogueSystem::InitializeDefaultSequences()
{
    RegisterDialogueSequence(CreateSurvivalSequence());
    RegisterDialogueSequence(CreateDangerSequence());
    RegisterDialogueSequence(CreateDiscoverySequence());
}

FNarr_DialogueSequence UDialogueSystem::CreateSurvivalSequence()
{
    FNarr_DialogueSequence Sequence;
    Sequence.SequenceID = TEXT("SurvivalTips");
    Sequence.TriggerCondition = ENarr_TriggerType::SurvivalStatus;
    Sequence.bIsRepeatable = true;
    Sequence.Priority = 2;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.DialogueText = FText::FromString(TEXT("Fire keeps the darkness away. Gather wood before night falls."));
    Line1.Duration = 4.0f;
    Line1.EmotionType = ENarr_EmotionType::Wise;
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.DialogueText = FText::FromString(TEXT("Water flows in the valley. Follow the sound when thirst comes."));
    Line2.Duration = 4.0f;
    Line2.EmotionType = ENarr_EmotionType::Wise;
    
    Sequence.DialogueLines.Add(Line1);
    Sequence.DialogueLines.Add(Line2);
    
    return Sequence;
}

FNarr_DialogueSequence UDialogueSystem::CreateDangerSequence()
{
    FNarr_DialogueSequence Sequence;
    Sequence.SequenceID = TEXT("DangerWarning");
    Sequence.TriggerCondition = ENarr_TriggerType::DinosaurEncounter;
    Sequence.bIsRepeatable = true;
    Sequence.Priority = 5;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Instinct");
    Line1.DialogueText = FText::FromString(TEXT("Something massive moves through the grass. Your blood runs cold."));
    Line1.Duration = 3.5f;
    Line1.EmotionType = ENarr_EmotionType::Fear;
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Instinct");
    Line2.DialogueText = FText::FromString(TEXT("Predator territory. Move slowly. Make no sudden movements."));
    Line2.Duration = 3.5f;
    Line2.EmotionType = ENarr_EmotionType::Fear;
    
    Sequence.DialogueLines.Add(Line1);
    Sequence.DialogueLines.Add(Line2);
    
    return Sequence;
}

FNarr_DialogueSequence UDialogueSystem::CreateDiscoverySequence()
{
    FNarr_DialogueSequence Sequence;
    Sequence.SequenceID = TEXT("Discovery");
    Sequence.TriggerCondition = ENarr_TriggerType::ResourceDiscovery;
    Sequence.bIsRepeatable = false;
    Sequence.Priority = 3;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Explorer");
    Line1.DialogueText = FText::FromString(TEXT("These tracks... something enormous passed this way."));
    Line1.Duration = 3.0f;
    Line1.EmotionType = ENarr_EmotionType::Curious;
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Explorer");
    Line2.DialogueText = FText::FromString(TEXT("The bones tell stories of ancient battles. Learn from the dead."));
    Line2.Duration = 4.0f;
    Line2.EmotionType = ENarr_EmotionType::Wise;
    
    Sequence.DialogueLines.Add(Line1);
    Sequence.DialogueLines.Add(Line2);
    
    return Sequence;
}