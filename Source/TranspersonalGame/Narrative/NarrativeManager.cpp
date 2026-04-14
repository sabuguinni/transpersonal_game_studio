#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    DialogueDataTable = nullptr;
    CurrentDialogueID = TEXT("");
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative system"));
    
    InitializeStoryState();
    LoadDialogueData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialization complete"));
}

void UNarrativeManager::Deinitialize()
{
    // Clear any active dialogue
    EndDialogue();
    
    // Clear delegates
    OnDialogueStarted.Clear();
    OnStoryBeatCompleted.Clear();
    OnConsciousnessLevelChanged.Clear();
    
    Super::Deinitialize();
}

void UNarrativeManager::InitializeStoryState()
{
    CurrentStoryState = FNarr_StoryState();
    CurrentStoryState.CurrentBeat = ENarr_StoryBeat::Awakening;
    CurrentStoryState.ConsciousnessLevel = 0.0f;
    
    // Initialize basic character relationships
    CurrentStoryState.CharacterRelationships.Add(TEXT("Elder_Shaman"), false);
    CurrentStoryState.CharacterRelationships.Add(TEXT("Beast_Speaker"), false);
    CurrentStoryState.CharacterRelationships.Add(TEXT("Vision_Walker"), false);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Story state initialized"));
}

void UNarrativeManager::LoadDialogueData()
{
    // TODO: Load dialogue data from data table
    // For now, create some default dialogue entries
    
    FNarr_DialogueEntry WelcomeDialogue;
    WelcomeDialogue.SpeakerName = TEXT("Elder_Shaman");
    WelcomeDialogue.DialogueText = FText::FromString(TEXT("Welcome, young seeker. The spirits have been expecting you."));
    WelcomeDialogue.RequiredStoryBeat = ENarr_StoryBeat::Awakening;
    WelcomeDialogue.EmotionalWeight = 2.0f;
    WelcomeDialogue.bIsTranscendentalMoment = true;
    WelcomeDialogue.ResponseOptions.Add(TEXT("I feel drawn to this place..."));
    WelcomeDialogue.ResponseOptions.Add(TEXT("What do the spirits want from me?"));
    
    ActiveDialogues.Add(TEXT("shaman_welcome"), WelcomeDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dialogue data loaded"));
}

void UNarrativeManager::AdvanceStoryBeat(ENarr_StoryBeat NewBeat)
{
    if (!ValidateStoryBeatProgression(NewBeat))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Invalid story beat progression to %d"), (int32)NewBeat);
        return;
    }
    
    // Mark current beat as completed
    if (!CurrentStoryState.CompletedBeats.Contains(CurrentStoryState.CurrentBeat))
    {
        CurrentStoryState.CompletedBeats.Add(CurrentStoryState.CurrentBeat);
    }
    
    // Set new current beat
    ENarr_StoryBeat PreviousBeat = CurrentStoryState.CurrentBeat;
    CurrentStoryState.CurrentBeat = NewBeat;
    
    // Increase consciousness level based on story progression
    float ConsciousnessIncrease = 10.0f;
    if (NewBeat == ENarr_StoryBeat::PlantCeremony || NewBeat == ENarr_StoryBeat::SpiritWalk)
    {
        ConsciousnessIncrease = 25.0f; // Major consciousness expansions
    }
    
    ModifyConsciousnessLevel(ConsciousnessIncrease);
    
    // Broadcast completion event
    OnStoryBeatCompleted.Broadcast(PreviousBeat);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Advanced story beat to %d"), (int32)NewBeat);
}

bool UNarrativeManager::IsStoryBeatCompleted(ENarr_StoryBeat Beat) const
{
    return CurrentStoryState.CompletedBeats.Contains(Beat);
}

ENarr_StoryBeat UNarrativeManager::GetCurrentStoryBeat() const
{
    return CurrentStoryState.CurrentBeat;
}

void UNarrativeManager::ModifyConsciousnessLevel(float Delta)
{
    float PreviousLevel = CurrentStoryState.ConsciousnessLevel;
    CurrentStoryState.ConsciousnessLevel = FMath::Clamp(CurrentStoryState.ConsciousnessLevel + Delta, 0.0f, 100.0f);
    
    if (FMath::Abs(CurrentStoryState.ConsciousnessLevel - PreviousLevel) > 0.1f)
    {
        OnConsciousnessLevelChanged.Broadcast(CurrentStoryState.ConsciousnessLevel);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Consciousness level changed to %.2f"), CurrentStoryState.ConsciousnessLevel);
    }
}

float UNarrativeManager::GetConsciousnessLevel() const
{
    return CurrentStoryState.ConsciousnessLevel;
}

bool UNarrativeManager::StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (!Speaker || !Listener)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot start dialogue - invalid Speaker or Listener"));
        return false;
    }
    
    FNarr_DialogueEntry* DialogueEntry = GetDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue entry not found: %s"), *DialogueID);
        return false;
    }
    
    // Check if story beat requirements are met
    if (!IsStoryBeatCompleted(DialogueEntry->RequiredStoryBeat) && 
        DialogueEntry->RequiredStoryBeat != CurrentStoryState.CurrentBeat)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story beat requirement not met for dialogue: %s"), *DialogueID);
        return false;
    }
    
    // Set current dialogue
    CurrentDialogueID = DialogueID;
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    
    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(DialogueEntry->SpeakerName, DialogueEntry->DialogueText);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue %s with speaker %s"), 
           *DialogueID, *DialogueEntry->SpeakerName);
    
    return true;
}

void UNarrativeManager::EndDialogue()
{
    if (!CurrentDialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ending dialogue %s"), *CurrentDialogueID);
        
        CurrentDialogueID = TEXT("");
        CurrentSpeaker = nullptr;
        CurrentListener = nullptr;
    }
}

TArray<FString> UNarrativeManager::GetAvailableDialogueOptions() const
{
    if (CurrentDialogueID.IsEmpty())
    {
        return TArray<FString>();
    }
    
    FNarr_DialogueEntry* DialogueEntry = const_cast<UNarrativeManager*>(this)->GetDialogueEntry(CurrentDialogueID);
    if (DialogueEntry)
    {
        return DialogueEntry->ResponseOptions;
    }
    
    return TArray<FString>();
}

bool UNarrativeManager::SelectDialogueOption(int32 OptionIndex)
{
    TArray<FString> Options = GetAvailableDialogueOptions();
    
    if (!Options.IsValidIndex(OptionIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Invalid dialogue option index: %d"), OptionIndex);
        return false;
    }
    
    FString SelectedOption = Options[OptionIndex];
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Selected dialogue option: %s"), *SelectedOption);
    
    // TODO: Process dialogue option selection and potentially advance story
    
    return true;
}

void UNarrativeManager::SetCharacterRelationship(const FString& CharacterName, bool bIsFriendly)
{
    CurrentStoryState.CharacterRelationships.Add(CharacterName, bIsFriendly);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Set relationship with %s to %s"), 
           *CharacterName, bIsFriendly ? TEXT("Friendly") : TEXT("Unfriendly"));
}

bool UNarrativeManager::GetCharacterRelationship(const FString& CharacterName) const
{
    const bool* Relationship = CurrentStoryState.CharacterRelationships.Find(CharacterName);
    return Relationship ? *Relationship : false;
}

void UNarrativeManager::UnlockLoreEntry(const FString& LoreID)
{
    if (!CurrentStoryState.UnlockedLoreEntries.Contains(LoreID))
    {
        CurrentStoryState.UnlockedLoreEntries.Add(LoreID);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Unlocked lore entry: %s"), *LoreID);
    }
}

bool UNarrativeManager::IsLoreEntryUnlocked(const FString& LoreID) const
{
    return CurrentStoryState.UnlockedLoreEntries.Contains(LoreID);
}

TArray<FString> UNarrativeManager::GetUnlockedLoreEntries() const
{
    return CurrentStoryState.UnlockedLoreEntries;
}

FNarr_DialogueEntry* UNarrativeManager::GetDialogueEntry(const FString& DialogueID)
{
    return ActiveDialogues.Find(DialogueID);
}

bool UNarrativeManager::ValidateStoryBeatProgression(ENarr_StoryBeat NewBeat) const
{
    // Basic validation - ensure we're not skipping too far ahead
    int32 CurrentBeatIndex = (int32)CurrentStoryState.CurrentBeat;
    int32 NewBeatIndex = (int32)NewBeat;
    
    // Allow progression to next beat or replay of current/previous beats
    return (NewBeatIndex <= CurrentBeatIndex + 1);
}