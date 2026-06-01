#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    // Create audio component for dialogue playback
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UWorld* World = GameInstance->GetWorld())
        {
            DialogueAudioComponent = NewObject<UAudioComponent>(this);
            if (DialogueAudioComponent)
            {
                DialogueAudioComponent->SetAutoDestroy(false);
                DialogueAudioComponent->bAutoActivate = false;
            }
        }
    }
    
    LoadDefaultDialogueSequences();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized"));
}

void UNarr_DialogueSystem::Deinitialize()
{
    StopDialogue();
    
    if (DialogueAudioComponent && IsValid(DialogueAudioComponent))
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent->DestroyComponent();
        DialogueAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsDialogueActive)
    {
        StopDialogue();
    }
    
    if (!DialogueSequences.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return;
    }
    
    const FNarr_DialogueSequence& Sequence = DialogueSequences[SequenceID];
    
    if (!CheckSequenceRequirements(Sequence))
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue sequence requirements not met: %s"), *SequenceID);
        return;
    }
    
    if (Sequence.DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence has no lines: %s"), *SequenceID);
        return;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
    
    OnDialogueStarted.Broadcast(SequenceID);
    PlayCurrentDialogueLine();
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bIsDialogueActive || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // End of sequence
        StopDialogue();
        return;
    }
    
    PlayCurrentDialogueLine();
}

void UNarr_DialogueSystem::StopDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    bIsDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    OnDialogueEnded.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence.SequenceID, NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s with %d lines"), 
           *NewSequence.SequenceID, NewSequence.DialogueLines.Num());
}

void UNarr_DialogueSystem::SetDialogueFlag(const FString& FlagName, bool bValue)
{
    DialogueFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("Set dialogue flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarr_DialogueSystem::GetDialogueFlag(const FString& FlagName) const
{
    if (const bool* FlagValue = DialogueFlags.Find(FlagName))
    {
        return *FlagValue;
    }
    return false;
}

void UNarr_DialogueSystem::PlayCurrentDialogueLine()
{
    if (!bIsDialogueActive || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
    
    if (CurrentLineIndex < 0 || CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        return;
    }
    
    CurrentDialogueLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    
    // Play audio if available
    if (CurrentDialogueLine.VoiceClip && DialogueAudioComponent)
    {
        DialogueAudioComponent->SetSound(CurrentDialogueLine.VoiceClip);
        DialogueAudioComponent->Play();
    }
    
    // Set timer for line duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, 
                                         this, 
                                         &UNarr_DialogueSystem::OnDialogueLineFinished, 
                                         CurrentDialogueLine.Duration, 
                                         false);
    }
    
    OnDialogueLineChanged.Broadcast(CurrentDialogueLine);
    
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue line: %s - %s"), 
           *CurrentDialogueLine.SpeakerName, *CurrentDialogueLine.DialogueText);
}

void UNarr_DialogueSystem::OnDialogueLineFinished()
{
    AdvanceDialogue();
}

bool UNarr_DialogueSystem::CheckSequenceRequirements(const FNarr_DialogueSequence& Sequence) const
{
    for (const FString& RequiredFlag : Sequence.RequiredFlags)
    {
        if (!GetDialogueFlag(RequiredFlag))
        {
            return false;
        }
    }
    return true;
}

void UNarr_DialogueSystem::LoadDefaultDialogueSequences()
{
    // T-Rex encounter dialogue
    FNarr_DialogueSequence TRexEncounter;
    TRexEncounter.SequenceID = TEXT("trex_encounter");
    TRexEncounter.bIsRepeatable = true;
    
    FNarr_DialogueLine TRexLine1;
    TRexLine1.SpeakerName = TEXT("Narrator");
    TRexLine1.DialogueText = TEXT("The ground trembles beneath massive footsteps. A Tyrannosaurus Rex emerges from the forest.");
    TRexLine1.Duration = 4.0f;
    TRexLine1.EmotionalTone = ENarr_EmotionalTone::Tense;
    
    FNarr_DialogueLine TRexLine2;
    TRexLine2.SpeakerName = TEXT("Narrator");
    TRexLine2.DialogueText = TEXT("Its massive jaws can crush bone. Move slowly and avoid sudden movements.");
    TRexLine2.Duration = 3.5f;
    TRexLine2.EmotionalTone = ENarr_EmotionalTone::Fearful;
    
    TRexEncounter.DialogueLines.Add(TRexLine1);
    TRexEncounter.DialogueLines.Add(TRexLine2);
    RegisterDialogueSequence(TRexEncounter);
    
    // Raptor pack dialogue
    FNarr_DialogueSequence RaptorPack;
    RaptorPack.SequenceID = TEXT("raptor_pack");
    RaptorPack.bIsRepeatable = true;
    
    FNarr_DialogueLine RaptorLine1;
    RaptorLine1.SpeakerName = TEXT("Narrator");
    RaptorLine1.DialogueText = TEXT("Sharp clicks echo through the undergrowth. Velociraptors hunt in coordinated packs.");
    RaptorLine1.Duration = 4.0f;
    RaptorLine1.EmotionalTone = ENarr_EmotionalTone::Tense;
    
    FNarr_DialogueLine RaptorLine2;
    RaptorLine2.SpeakerName = TEXT("Narrator");
    RaptorLine2.DialogueText = TEXT("They communicate through calls. One distracts while others flank their prey.");
    RaptorLine2.Duration = 3.5f;
    RaptorLine2.EmotionalTone = ENarr_EmotionalTone::Urgent;
    
    RaptorPack.DialogueLines.Add(RaptorLine1);
    RaptorPack.DialogueLines.Add(RaptorLine2);
    RegisterDialogueSequence(RaptorPack);
    
    // Survival tutorial dialogue
    FNarr_DialogueSequence SurvivalTutorial;
    SurvivalTutorial.SequenceID = TEXT("survival_tutorial");
    SurvivalTutorial.bIsRepeatable = false;
    
    FNarr_DialogueLine TutorialLine1;
    TutorialLine1.SpeakerName = TEXT("Narrator");
    TutorialLine1.DialogueText = TEXT("Welcome to the Cretaceous period. Survival depends on your ability to adapt.");
    TutorialLine1.Duration = 4.0f;
    TutorialLine1.EmotionalTone = ENarr_EmotionalTone::Informative;
    
    FNarr_DialogueLine TutorialLine2;
    TutorialLine2.SpeakerName = TEXT("Narrator");
    TutorialLine2.DialogueText = TEXT("Gather resources, craft tools, and avoid the apex predators that rule this world.");
    TutorialLine2.Duration = 4.5f;
    TutorialLine2.EmotionalTone = ENarr_EmotionalTone::Instructional;
    
    SurvivalTutorial.DialogueLines.Add(TutorialLine1);
    SurvivalTutorial.DialogueLines.Add(TutorialLine2);
    RegisterDialogueSequence(SurvivalTutorial);
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d default dialogue sequences"), DialogueSequences.Num());
}