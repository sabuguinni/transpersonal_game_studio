#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentStoryBeat = TEXT("intro");
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing narrative system"));
    
    InitializeDefaultDialogues();
    InitializeStoryBeats();
}

void UNarrativeManager::Deinitialize()
{
    DialogueDatabase.Empty();
    StoryBeats.Empty();
    ActiveDialogues.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        const FNarr_DialogueEntry& Entry = DialogueDatabase[DialogueID];
        
        if (CheckConditions(Entry.Conditions))
        {
            ActiveDialogues.AddUnique(DialogueID);
            
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Triggering dialogue - %s: %s"), 
                   *Entry.SpeakerName, *Entry.DialogueText);
            
            // Play audio if available
            if (!Entry.AudioPath.IsEmpty())
            {
                PlayNarrationAudio(Entry.AudioPath);
            }
            
            // Display text (in a real implementation, this would trigger UI)
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, Entry.Duration, FColor::Yellow,
                    FString::Printf(TEXT("%s: %s"), *Entry.SpeakerName, *Entry.DialogueText));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Dialogue ID not found - %s"), *DialogueID);
    }
}

void UNarrativeManager::TriggerStoryBeat(const FString& BeatID)
{
    if (StoryBeats.Contains(BeatID))
    {
        FNarr_StoryBeat& Beat = StoryBeats[BeatID];
        
        if (!Beat.bIsCompleted && CheckConditions(Beat.TriggerConditions))
        {
            CurrentStoryBeat = BeatID;
            Beat.bIsCompleted = true;
            
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story beat triggered - %s"), *Beat.Title);
            
            // Process dialogue sequence
            ProcessDialogueSequence(Beat.DialogueSequence);
        }
    }
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    if (StoryBeats.Contains(BeatID))
    {
        return StoryBeats[BeatID].bIsCompleted;
    }
    return false;
}

void UNarrativeManager::RegisterDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    DialogueDatabase.Add(Entry.DialogueID, Entry);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered dialogue - %s"), *Entry.DialogueID);
}

void UNarrativeManager::RegisterStoryBeat(const FNarr_StoryBeat& Beat)
{
    StoryBeats.Add(Beat.BeatID, Beat);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered story beat - %s"), *Beat.BeatID);
}

TArray<FString> UNarrativeManager::GetActiveDialogues() const
{
    return ActiveDialogues;
}

void UNarrativeManager::PlayNarrationAudio(const FString& AudioPath, float Volume)
{
    if (AudioPath.IsEmpty()) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // In a real implementation, this would load and play the audio file
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Playing audio - %s at volume %f"), *AudioPath, Volume);
}

void UNarrativeManager::InitializeDefaultDialogues()
{
    // Survival introduction
    FNarr_DialogueEntry IntroDialogue;
    IntroDialogue.DialogueID = TEXT("intro_welcome");
    IntroDialogue.SpeakerName = TEXT("Narrator");
    IntroDialogue.DialogueText = TEXT("Welcome to the prehistoric wilderness. Your survival depends on wit, courage, and understanding the ancient laws of this land.");
    IntroDialogue.Duration = 5.0f;
    RegisterDialogueEntry(IntroDialogue);
    
    // Danger warning
    FNarr_DialogueEntry DangerDialogue;
    DangerDialogue.DialogueID = TEXT("danger_predator");
    DangerDialogue.SpeakerName = TEXT("Survival Instinct");
    DangerDialogue.DialogueText = TEXT("Predator scent detected. Move carefully and stay alert. Your life depends on it.");
    DangerDialogue.Duration = 3.0f;
    RegisterDialogueEntry(DangerDialogue);
    
    // Resource discovery
    FNarr_DialogueEntry ResourceDialogue;
    ResourceDialogue.DialogueID = TEXT("discovery_resource");
    ResourceDialogue.SpeakerName = TEXT("Explorer");
    ResourceDialogue.DialogueText = TEXT("These materials could be useful. Gather what you can - resources are scarce in this harsh world.");
    ResourceDialogue.Duration = 4.0f;
    RegisterDialogueEntry(ResourceDialogue);
    
    // Shelter advice
    FNarr_DialogueEntry ShelterDialogue;
    ShelterDialogue.DialogueID = TEXT("advice_shelter");
    ShelterDialogue.SpeakerName = TEXT("Survivor");
    ShelterDialogue.DialogueText = TEXT("Night falls quickly here. Find or build shelter before darkness brings the apex predators.");
    ShelterDialogue.Duration = 4.0f;
    RegisterDialogueEntry(ShelterDialogue);
}

void UNarrativeManager::InitializeStoryBeats()
{
    // First survival
    FNarr_StoryBeat FirstSurvival;
    FirstSurvival.BeatID = TEXT("first_survival");
    FirstSurvival.Title = TEXT("First Day");
    FirstSurvival.Description = TEXT("Survive your first day in the prehistoric world");
    FirstSurvival.DialogueSequence.Add(TEXT("intro_welcome"));
    FirstSurvival.DialogueSequence.Add(TEXT("advice_shelter"));
    RegisterStoryBeat(FirstSurvival);
    
    // Predator encounter
    FNarr_StoryBeat PredatorEncounter;
    PredatorEncounter.BeatID = TEXT("predator_encounter");
    PredatorEncounter.Title = TEXT("First Hunt");
    PredatorEncounter.Description = TEXT("Encounter your first major predator");
    PredatorEncounter.DialogueSequence.Add(TEXT("danger_predator"));
    RegisterStoryBeat(PredatorEncounter);
    
    // Resource mastery
    FNarr_StoryBeat ResourceMastery;
    ResourceMastery.BeatID = TEXT("resource_mastery");
    ResourceMastery.Title = TEXT("Gatherer's Wisdom");
    ResourceMastery.Description = TEXT("Learn to identify and gather essential resources");
    ResourceMastery.DialogueSequence.Add(TEXT("discovery_resource"));
    RegisterStoryBeat(ResourceMastery);
}

bool UNarrativeManager::CheckConditions(const TArray<FString>& Conditions) const
{
    // Simple condition checking - in a real implementation, this would check game state
    for (const FString& Condition : Conditions)
    {
        if (Condition == TEXT("always_true"))
        {
            continue;
        }
        // Add more condition checks as needed
    }
    return true;
}

void UNarrativeManager::ProcessDialogueSequence(const TArray<FString>& Sequence)
{
    for (const FString& DialogueID : Sequence)
    {
        TriggerDialogue(DialogueID);
    }
}