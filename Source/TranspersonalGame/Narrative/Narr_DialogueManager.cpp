#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    DialogueAudioComponent = nullptr;
    bIsPlayingDialogue = false;
    CurrentLineIndex = 0;
    CurrentSequenceID = TEXT("");
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initializing narrative dialogue system"));
    
    // Create audio component for dialogue playback
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UWorld* World = GameInstance->GetWorld())
        {
            AActor* DummyActor = World->SpawnActor<AActor>();
            if (DummyActor)
            {
                DialogueAudioComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
                if (DialogueAudioComponent)
                {
                    DialogueAudioComponent->bAutoActivate = false;
                    DialogueAudioComponent->SetVolumeMultiplier(0.8f);
                }
            }
        }
    }
    
    // Create predefined dialogue sequences
    CreateSurvivalWarningSequences();
    CreateDinosaurEncounterSequences();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialized with %d dialogue sequences"), RegisteredSequences.Num());
}

void UNarr_DialogueManager::Deinitialize()
{
    StopCurrentDialogue();
    
    if (DialogueAudioComponent && IsValid(DialogueAudioComponent))
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    RegisteredSequences.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::PlayDialogueSequence(const FString& SequenceID, AActor* Speaker)
{
    if (!RegisteredSequences.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Sequence '%s' not found"), *SequenceID);
        return;
    }
    
    FNarr_DialogueSequence& Sequence = RegisteredSequences[SequenceID];
    
    // Check if sequence can be played
    if (Sequence.bHasBeenPlayed && !Sequence.bIsRepeatable)
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Sequence '%s' already played and not repeatable"), *SequenceID);
        return;
    }
    
    // Stop current dialogue if playing
    StopCurrentDialogue();
    
    // Start new sequence
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsPlayingDialogue = true;
    Sequence.bHasBeenPlayed = true;
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Starting sequence '%s' with %d lines"), *SequenceID, Sequence.DialogueLines.Num());
    
    PlayNextDialogueLine();
}

void UNarr_DialogueManager::PlayNarration(const FText& NarrationText, float Duration)
{
    StopCurrentDialogue();
    
    // Create temporary narration sequence
    FNarr_DialogueLine NarrationLine;
    NarrationLine.SpeakerName = TEXT("Narrator");
    NarrationLine.DialogueText = NarrationText;
    NarrationLine.Duration = Duration;
    NarrationLine.bIsNarration = true;
    
    CurrentDialogueLine = NarrationLine;
    bIsPlayingDialogue = true;
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Playing narration: %s"), *NarrationText.ToString());
    
    // Set timer for narration duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UNarr_DialogueManager::OnDialogueLineFinished, Duration, false);
    }
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        CurrentLineIndex = 0;
        CurrentSequenceID = TEXT("");
        
        if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
        {
            DialogueAudioComponent->Stop();
        }
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }
        
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Stopped current dialogue"));
    }
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return bIsPlayingDialogue;
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    RegisteredSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Registered sequence '%s'"), *Sequence.SequenceID);
}

void UNarr_DialogueManager::PlaySurvivalWarning(ESurvivalThreatLevel ThreatLevel)
{
    FString SequenceID;
    
    switch (ThreatLevel)
    {
        case ESurvivalThreatLevel::Low:
            SequenceID = TEXT("SurvivalWarning_Low");
            break;
        case ESurvivalThreatLevel::Medium:
            SequenceID = TEXT("SurvivalWarning_Medium");
            break;
        case ESurvivalThreatLevel::High:
            SequenceID = TEXT("SurvivalWarning_High");
            break;
        case ESurvivalThreatLevel::Extreme:
            SequenceID = TEXT("SurvivalWarning_Extreme");
            break;
        default:
            SequenceID = TEXT("SurvivalWarning_Medium");
            break;
    }
    
    PlayDialogueSequence(SequenceID);
}

void UNarr_DialogueManager::PlayDinosaurEncounterDialogue(EDinosaurSpecies Species, bool bIsHostile)
{
    FString SequenceID = TEXT("Encounter_");
    
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            SequenceID += bIsHostile ? TEXT("TRex_Hostile") : TEXT("TRex_Neutral");
            break;
        case EDinosaurSpecies::Velociraptor:
            SequenceID += bIsHostile ? TEXT("Raptor_Hostile") : TEXT("Raptor_Neutral");
            break;
        case EDinosaurSpecies::Triceratops:
            SequenceID += bIsHostile ? TEXT("Triceratops_Hostile") : TEXT("Triceratops_Neutral");
            break;
        case EDinosaurSpecies::Brachiosaurus:
            SequenceID += bIsHostile ? TEXT("Brachio_Hostile") : TEXT("Brachio_Neutral");
            break;
        default:
            SequenceID += bIsHostile ? TEXT("Generic_Hostile") : TEXT("Generic_Neutral");
            break;
    }
    
    PlayDialogueSequence(SequenceID);
}

void UNarr_DialogueManager::OnDialogueLineFinished()
{
    if (!bIsPlayingDialogue)
        return;
    
    if (CurrentSequenceID.IsEmpty())
    {
        // Single narration finished
        StopCurrentDialogue();
        return;
    }
    
    // Move to next line in sequence
    PlayNextDialogueLine();
}

void UNarr_DialogueManager::PlayNextDialogueLine()
{
    if (!RegisteredSequences.Contains(CurrentSequenceID))
    {
        StopCurrentDialogue();
        return;
    }
    
    FNarr_DialogueSequence& Sequence = RegisteredSequences[CurrentSequenceID];
    
    if (CurrentLineIndex >= Sequence.DialogueLines.Num())
    {
        // Sequence finished
        StopCurrentDialogue();
        return;
    }
    
    // Get current line
    CurrentDialogueLine = Sequence.DialogueLines[CurrentLineIndex];
    CurrentLineIndex++;
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Playing line %d: %s"), 
           CurrentLineIndex, *CurrentDialogueLine.DialogueText.ToString());
    
    // Play audio if available
    if (DialogueAudioComponent && CurrentDialogueLine.VoiceClip.IsValid())
    {
        if (USoundCue* SoundCue = CurrentDialogueLine.VoiceClip.LoadSynchronous())
        {
            DialogueAudioComponent->SetSound(SoundCue);
            DialogueAudioComponent->Play();
        }
    }
    
    // Set timer for line duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, this, 
                                         &UNarr_DialogueManager::OnDialogueLineFinished, 
                                         CurrentDialogueLine.Duration, false);
    }
}

void UNarr_DialogueManager::CreateSurvivalWarningSequences()
{
    // Low threat warnings
    FNarr_DialogueSequence LowThreatSeq;
    LowThreatSeq.SequenceID = TEXT("SurvivalWarning_Low");
    LowThreatSeq.bIsRepeatable = true;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Narrator");
    Line1.DialogueText = FText::FromString(TEXT("Stay alert, survivor. Small predators roam these lands."));
    Line1.Duration = 3.0f;
    Line1.bIsNarration = true;
    LowThreatSeq.DialogueLines.Add(Line1);
    
    RegisteredSequences.Add(LowThreatSeq.SequenceID, LowThreatSeq);
    
    // High threat warnings
    FNarr_DialogueSequence HighThreatSeq;
    HighThreatSeq.SequenceID = TEXT("SurvivalWarning_High");
    HighThreatSeq.bIsRepeatable = true;
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Narrator");
    Line2.DialogueText = FText::FromString(TEXT("Danger approaches. The apex predator has caught your scent."));
    Line2.Duration = 4.0f;
    Line2.bIsNarration = true;
    HighThreatSeq.DialogueLines.Add(Line2);
    
    RegisteredSequences.Add(HighThreatSeq.SequenceID, HighThreatSeq);
}

void UNarr_DialogueManager::CreateDinosaurEncounterSequences()
{
    // T-Rex hostile encounter
    FNarr_DialogueSequence TRexHostile;
    TRexHostile.SequenceID = TEXT("Encounter_TRex_Hostile");
    TRexHostile.bIsRepeatable = true;
    
    FNarr_DialogueLine TRexLine;
    TRexLine.SpeakerName = TEXT("Narrator");
    TRexLine.DialogueText = FText::FromString(TEXT("The thunder lizard roars. Its massive jaws can crush bone in a single bite."));
    TRexLine.Duration = 5.0f;
    TRexLine.bIsNarration = true;
    TRexHostile.DialogueLines.Add(TRexLine);
    
    RegisteredSequences.Add(TRexHostile.SequenceID, TRexHostile);
    
    // Raptor pack encounter
    FNarr_DialogueSequence RaptorHostile;
    RaptorHostile.SequenceID = TEXT("Encounter_Raptor_Hostile");
    RaptorHostile.bIsRepeatable = true;
    
    FNarr_DialogueLine RaptorLine;
    RaptorLine.SpeakerName = TEXT("Narrator");
    RaptorLine.DialogueText = FText::FromString(TEXT("Pack hunters circle their prey. Velociraptors work together."));
    RaptorLine.Duration = 4.0f;
    RaptorLine.bIsNarration = true;
    RaptorHostile.DialogueLines.Add(RaptorLine);
    
    RegisteredSequences.Add(RaptorHostile.SequenceID, RaptorHostile);
}