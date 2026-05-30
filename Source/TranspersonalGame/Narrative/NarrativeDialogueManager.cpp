#include "NarrativeDialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    DefaultVolume = 0.8f;
    FadeInDuration = 0.5f;
    FadeOutDuration = 0.3f;
    CurrentAudioComponent = nullptr;
    
    LoadEnvironmentalNarration();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager initialized"));
}

void UNarrativeDialogueManager::Deinitialize()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent = nullptr;
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    Super::Deinitialize();
}

void UNarrativeDialogueManager::PlayDialogue(const FString& DialogueID)
{
    if (IsDialoguePlaying())
    {
        StopCurrentDialogue();
    }
    
    // Find dialogue in data table
    if (DialogueDataTable.IsValid())
    {
        if (UDataTable* Table = DialogueDataTable.LoadSynchronous())
        {
            if (FNarr_DialogueEntry* Entry = Table->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT("PlayDialogue")))
            {
                PlayDialogueEntry(*Entry);
            }
        }
    }
    
    // Check quest dialogues
    if (QuestDialogues.Contains(DialogueID))
    {
        const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[DialogueID];
        if (QuestDialogue.DialogueEntries.Num() > 0)
        {
            PlayDialogueEntry(QuestDialogue.DialogueEntries[0]);
        }
    }
}

void UNarrativeDialogueManager::TriggerEnvironmentalNarration(ENarr_BiomeType BiomeType, ENarr_ThreatLevel ThreatLevel)
{
    if (IsDialoguePlaying())
    {
        return; // Don't interrupt current dialogue
    }
    
    FNarr_DialogueEntry Entry = GetEnvironmentalDialogue(BiomeType, ThreatLevel);
    if (!Entry.DialogueText.IsEmpty())
    {
        PlayDialogueEntry(Entry);
    }
}

void UNarrativeDialogueManager::RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& Entries)
{
    FNarr_QuestDialogue NewQuestDialogue;
    NewQuestDialogue.QuestID = QuestID;
    NewQuestDialogue.DialogueEntries = Entries;
    NewQuestDialogue.bIsCompleted = false;
    
    QuestDialogues.Add(QuestID, NewQuestDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("Registered quest dialogue: %s with %d entries"), *QuestID, Entries.Num());
}

bool UNarrativeDialogueManager::IsDialoguePlaying() const
{
    return CurrentAudioComponent && IsValid(CurrentAudioComponent) && CurrentAudioComponent->IsPlaying();
}

void UNarrativeDialogueManager::StopCurrentDialogue()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->FadeOut(FadeOutDuration, 0.0f);
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
}

void UNarrativeDialogueManager::PlayDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Load and play voice clip if available
    if (Entry.VoiceClip.IsValid())
    {
        if (USoundCue* SoundCue = Entry.VoiceClip.LoadSynchronous())
        {
            CurrentAudioComponent = UGameplayStatics::SpawnSound2D(World, SoundCue, DefaultVolume);
            if (CurrentAudioComponent)
            {
                CurrentAudioComponent->FadeIn(FadeInDuration, DefaultVolume);
            }
        }
    }
    
    // Display text (this would integrate with UI system)
    UE_LOG(LogTemp, Warning, TEXT("[%s]: %s"), *Entry.SpeakerName, *Entry.DialogueText.ToString());
    
    // Set timer for dialogue duration
    World->GetTimerManager().SetTimer(
        DialogueTimerHandle,
        this,
        &UNarrativeDialogueManager::OnDialogueFinished,
        Entry.Duration,
        false
    );
}

void UNarrativeDialogueManager::OnDialogueFinished()
{
    if (CurrentAudioComponent && IsValid(CurrentAudioComponent))
    {
        CurrentAudioComponent->FadeOut(FadeOutDuration, 0.0f);
        CurrentAudioComponent = nullptr;
    }
}

void UNarrativeDialogueManager::LoadEnvironmentalNarration()
{
    // Populate environmental narration entries
    EnvironmentalNarration.Empty();
    
    // Savana narration
    FNarr_DialogueEntry SavanaEntry;
    SavanaEntry.SpeakerName = TEXT("Narrator");
    SavanaEntry.DialogueText = FText::FromString(TEXT("The vast savana stretches endlessly. Herds of Triceratops graze in the distance."));
    SavanaEntry.Duration = 4.0f;
    SavanaEntry.TriggerType = ENarr_DialogueTrigger::BiomeEnter;
    SavanaEntry.TriggerCondition = TEXT("Savana");
    EnvironmentalNarration.Add(SavanaEntry);
    
    // Forest narration
    FNarr_DialogueEntry ForestEntry;
    ForestEntry.SpeakerName = TEXT("Guide");
    ForestEntry.DialogueText = FText::FromString(TEXT("Ancient trees tower above. The Brachiosaurus herds migrate through these paths."));
    ForestEntry.Duration = 4.5f;
    ForestEntry.TriggerType = ENarr_DialogueTrigger::BiomeEnter;
    ForestEntry.TriggerCondition = TEXT("Forest");
    EnvironmentalNarration.Add(ForestEntry);
    
    // Danger narration
    FNarr_DialogueEntry DangerEntry;
    DangerEntry.SpeakerName = TEXT("Survival Guide");
    DangerEntry.DialogueText = FText::FromString(TEXT("Danger detected. Large predator nearby. Move carefully and avoid sudden movements."));
    DangerEntry.Duration = 3.5f;
    DangerEntry.TriggerType = ENarr_DialogueTrigger::ThreatDetected;
    DangerEntry.TriggerCondition = TEXT("High");
    EnvironmentalNarration.Add(DangerEntry);
}

FNarr_DialogueEntry UNarrativeDialogueManager::GetEnvironmentalDialogue(ENarr_BiomeType BiomeType, ENarr_ThreatLevel ThreatLevel)
{
    for (const FNarr_DialogueEntry& Entry : EnvironmentalNarration)
    {
        if (Entry.TriggerType == ENarr_DialogueTrigger::BiomeEnter)
        {
            FString BiomeName;
            switch (BiomeType)
            {
                case ENarr_BiomeType::Savana: BiomeName = TEXT("Savana"); break;
                case ENarr_BiomeType::Forest: BiomeName = TEXT("Forest"); break;
                case ENarr_BiomeType::Desert: BiomeName = TEXT("Desert"); break;
                case ENarr_BiomeType::Swamp: BiomeName = TEXT("Swamp"); break;
                case ENarr_BiomeType::Mountain: BiomeName = TEXT("Mountain"); break;
                default: BiomeName = TEXT("Unknown"); break;
            }
            
            if (Entry.TriggerCondition == BiomeName)
            {
                return Entry;
            }
        }
        else if (Entry.TriggerType == ENarr_DialogueTrigger::ThreatDetected)
        {
            FString ThreatName;
            switch (ThreatLevel)
            {
                case ENarr_ThreatLevel::Low: ThreatName = TEXT("Low"); break;
                case ENarr_ThreatLevel::Medium: ThreatName = TEXT("Medium"); break;
                case ENarr_ThreatLevel::High: ThreatName = TEXT("High"); break;
                case ENarr_ThreatLevel::Extreme: ThreatName = TEXT("Extreme"); break;
                default: ThreatName = TEXT("Unknown"); break;
            }
            
            if (Entry.TriggerCondition == ThreatName)
            {
                return Entry;
            }
        }
    }
    
    return FNarr_DialogueEntry(); // Return empty entry if no match
}