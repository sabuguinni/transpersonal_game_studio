#include "Narr_DialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    CurrentLineIndex = 0;
    bIsPlaying = false;
    DialogueVolume = 1.0f;
    CurrentSpeakerName = TEXT("");
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CreateDefaultTribalSequences();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized with tribal sequences"));
}

void UNarr_DialogueManager::PlayDialogueSequence(const FString& SequenceName)
{
    if (RegisteredSequences.Contains(SequenceName))
    {
        CurrentSequence = RegisteredSequences[SequenceName];
        CurrentLineIndex = 0;
        bIsPlaying = true;
        
        if (CurrentSequence.DialogueLines.Num() > 0)
        {
            PlaySingleDialogue(CurrentSequence.DialogueLines[0]);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceName);
    }
}

void UNarr_DialogueManager::PlaySingleDialogue(const FNarr_DialogueLine& DialogueLine)
{
    CurrentSpeakerName = DialogueLine.SpeakerName;
    
    // Display dialogue text (in a real implementation, this would trigger UI)
    FString DisplayText = FString::Printf(TEXT("%s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.Duration, 
            DialogueLine.bIsImportant ? FColor::Red : FColor::White, DisplayText);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s"), *DisplayText);
    
    // Set timer for dialogue duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, this, 
            &UNarr_DialogueManager::OnDialogueLineComplete, DialogueLine.Duration, false);
    }
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    bIsPlaying = false;
    CurrentLineIndex = 0;
    CurrentSpeakerName = TEXT("");
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Stopped current dialogue"));
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FString& SequenceName, const FNarr_DialogueSequence& Sequence)
{
    RegisteredSequences.Add(SequenceName, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s with %d lines"), *SequenceName, Sequence.DialogueLines.Num());
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return bIsPlaying;
}

FString UNarr_DialogueManager::GetCurrentSpeaker() const
{
    return CurrentSpeakerName;
}

void UNarr_DialogueManager::SetDialogueVolume(float Volume)
{
    DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UNarr_DialogueManager::CreateTribalDialogues()
{
    CreateDefaultTribalSequences();
}

void UNarr_DialogueManager::AdvanceDialogue()
{
    if (bIsPlaying && CurrentSequence.DialogueLines.IsValidIndex(CurrentLineIndex + 1))
    {
        CurrentLineIndex++;
        PlaySingleDialogue(CurrentSequence.DialogueLines[CurrentLineIndex]);
    }
    else
    {
        // End of sequence
        bIsPlaying = false;
        CurrentLineIndex = 0;
        CurrentSpeakerName = TEXT("");
        UE_LOG(LogTemp, Log, TEXT("Dialogue sequence completed"));
    }
}

void UNarr_DialogueManager::OnDialogueLineComplete()
{
    if (CurrentSequence.bAutoAdvance)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(DialogueTimerHandle, this, 
                &UNarr_DialogueManager::AdvanceDialogue, CurrentSequence.AutoAdvanceDelay, false);
        }
    }
}

void UNarr_DialogueManager::CreateDefaultTribalSequences()
{
    // Create "Tribal Welcome" sequence
    FNarr_DialogueSequence WelcomeSequence;
    WelcomeSequence.SequenceName = TEXT("TribalWelcome");
    WelcomeSequence.bAutoAdvance = true;
    WelcomeSequence.AutoAdvanceDelay = 1.5f;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.DialogueText = TEXT("Welcome, young hunter. The ancient valley holds many secrets.");
    Line1.SpeakerType = ENarr_DialogueType::TribalElder;
    Line1.Duration = 4.0f;
    Line1.bIsImportant = true;
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Scout");
    Line2.DialogueText = TEXT("The Thunder Lizards roam these lands. Stay alert.");
    Line2.SpeakerType = ENarr_DialogueType::TribalScout;
    Line2.Duration = 3.5f;
    Line2.bIsImportant = true;
    
    WelcomeSequence.DialogueLines.Add(Line1);
    WelcomeSequence.DialogueLines.Add(Line2);
    
    RegisterDialogueSequence(TEXT("TribalWelcome"), WelcomeSequence);
    
    // Create "Hunt Warning" sequence
    FNarr_DialogueSequence HuntSequence;
    HuntSequence.SequenceName = TEXT("HuntWarning");
    HuntSequence.bAutoAdvance = true;
    HuntSequence.AutoAdvanceDelay = 2.0f;
    
    FNarr_DialogueLine HuntLine1;
    HuntLine1.SpeakerName = TEXT("Tribal Warrior");
    HuntLine1.DialogueText = TEXT("Pack hunters approach from the north! Prepare for battle!");
    HuntLine1.SpeakerType = ENarr_DialogueType::TribalWarrior;
    HuntLine1.Duration = 4.0f;
    HuntLine1.bIsImportant = true;
    
    FNarr_DialogueLine HuntLine2;
    HuntLine2.SpeakerName = TEXT("Tribal Hunter");
    HuntLine2.DialogueText = TEXT("Use the terrain to your advantage. Strike fast, retreat faster.");
    HuntLine2.SpeakerType = ENarr_DialogueType::TribalHunter;
    HuntLine2.Duration = 4.5f;
    HuntLine2.bIsImportant = false;
    
    HuntSequence.DialogueLines.Add(HuntLine1);
    HuntSequence.DialogueLines.Add(HuntLine2);
    
    RegisterDialogueSequence(TEXT("HuntWarning"), HuntSequence);
    
    // Create "Ancient Wisdom" sequence
    FNarr_DialogueSequence WisdomSequence;
    WisdomSequence.SequenceName = TEXT("AncientWisdom");
    WisdomSequence.bAutoAdvance = true;
    WisdomSequence.AutoAdvanceDelay = 2.5f;
    
    FNarr_DialogueLine WisdomLine1;
    WisdomLine1.SpeakerName = TEXT("Tribal Shaman");
    WisdomLine1.DialogueText = TEXT("The great herds follow the seasons. Learn their patterns.");
    WisdomLine1.SpeakerType = ENarr_DialogueType::TribalShaman;
    WisdomLine1.Duration = 4.0f;
    WisdomLine1.bIsImportant = false;
    
    FNarr_DialogueLine WisdomLine2;
    WisdomLine2.SpeakerName = TEXT("Tribal Elder");
    WisdomLine2.DialogueText = TEXT("Survival is not about strength alone, but wisdom and patience.");
    WisdomLine2.SpeakerType = ENarr_DialogueType::TribalElder;
    WisdomLine2.Duration = 5.0f;
    WisdomLine2.bIsImportant = true;
    
    WisdomSequence.DialogueLines.Add(WisdomLine1);
    WisdomSequence.DialogueLines.Add(WisdomLine2);
    
    RegisterDialogueSequence(TEXT("AncientWisdom"), WisdomSequence);
    
    UE_LOG(LogTemp, Log, TEXT("Created default tribal dialogue sequences: TribalWelcome, HuntWarning, AncientWisdom"));
}