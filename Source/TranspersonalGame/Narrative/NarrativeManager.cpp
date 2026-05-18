#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/DataTable.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative subsystem"));
    
    CurrentStoryPhase = ENarr_StoryPhase::Discovery;
    CurrentBiome = ENarr_BiomeType::Savana;
    CurrentTimeOfDay = ENarr_TimeOfDay::Dawn;
    CurrentWeather = ENarr_WeatherType::Clear;
    
    InitializeStoryBeats();
    LoadDialogueData();
    
    // Register initial dialogue conditions
    RegisterDialogueCondition(TEXT("GameStarted"), true);
    RegisterDialogueCondition(TEXT("FirstDinosaurSeen"), false);
    RegisterDialogueCondition(TEXT("FirstNightSurvived"), false);
    RegisterDialogueCondition(TEXT("WaterFound"), false);
    RegisterDialogueCondition(TEXT("ShelterBuilt"), false);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialization complete"));
}

void UNarrativeManager::Deinitialize()
{
    DialogueConditions.Empty();
    StoryBeats.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!IsDialogueAvailable(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue %s not available"), *DialogueID);
        return;
    }
    
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: No dialogue data table set"));
        return;
    }
    
    FNarr_DialogueEntry* DialogueEntry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT(""));
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue entry %s not found in data table"), *DialogueID);
        return;
    }
    
    BroadcastDialogue(*DialogueEntry, Speaker);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggered dialogue %s"), *DialogueID);
}

bool UNarrativeManager::IsDialogueAvailable(const FString& DialogueID) const
{
    if (!DialogueDataTable)
    {
        return false;
    }
    
    FNarr_DialogueEntry* DialogueEntry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT(""));
    if (!DialogueEntry)
    {
        return false;
    }
    
    return EvaluateConditions(DialogueEntry->TriggerConditions);
}

void UNarrativeManager::RegisterDialogueCondition(const FString& ConditionID, bool bValue)
{
    DialogueConditions.Add(ConditionID, bValue);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered condition %s = %s"), 
           *ConditionID, bValue ? TEXT("true") : TEXT("false"));
    
    // Check if this condition unlocks new story beats
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (!Beat.bIsCompleted && Beat.RequiredConditions.Contains(ConditionID))
        {
            if (EvaluateConditions(Beat.RequiredConditions))
            {
                AdvanceStoryBeat(Beat.BeatID);
            }
        }
    }
}

void UNarrativeManager::AdvanceStoryBeat(const FString& BeatID)
{
    FNarr_StoryBeat* Beat = StoryBeats.FindByPredicate([&BeatID](const FNarr_StoryBeat& B) {
        return B.BeatID == BeatID;
    });
    
    if (!Beat)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story beat %s not found"), *BeatID);
        return;
    }
    
    if (Beat->bIsCompleted)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Story beat %s already completed"), *BeatID);
        return;
    }
    
    Beat->bIsCompleted = true;
    CurrentStoryPhase = Beat->Phase;
    
    // Unlock associated dialogues
    for (const FString& DialogueID : Beat->UnlockedDialogues)
    {
        RegisterDialogueCondition(DialogueID + TEXT("_Unlocked"), true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Advanced story beat %s, phase now %d"), 
           *BeatID, static_cast<int32>(CurrentStoryPhase));
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    const FNarr_StoryBeat* Beat = StoryBeats.FindByPredicate([&BeatID](const FNarr_StoryBeat& B) {
        return B.BeatID == BeatID;
    });
    
    return Beat ? Beat->bIsCompleted : false;
}

ENarr_StoryPhase UNarrativeManager::GetCurrentStoryPhase() const
{
    return CurrentStoryPhase;
}

void UNarrativeManager::TriggerEnvironmentalNarration(const FVector& Location, const FString& NarrationType)
{
    FString ContextualDialogueID;
    
    // Generate contextual dialogue based on biome and situation
    if (CurrentBiome == ENarr_BiomeType::Savana)
    {
        if (NarrationType == TEXT("DangerDetected"))
        {
            ContextualDialogueID = TEXT("Savana_Danger_Warning");
        }
        else if (NarrationType == TEXT("ResourceFound"))
        {
            ContextualDialogueID = TEXT("Savana_Resource_Found");
        }
    }
    else if (CurrentBiome == ENarr_BiomeType::Forest)
    {
        if (NarrationType == TEXT("DangerDetected"))
        {
            ContextualDialogueID = TEXT("Forest_Danger_Warning");
        }
    }
    
    if (!ContextualDialogueID.IsEmpty())
    {
        TriggerDialogue(ContextualDialogueID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Environmental narration triggered at %s"), 
           *Location.ToString());
}

void UNarrativeManager::SetNarrativeContext(ENarr_BiomeType Biome, ENarr_TimeOfDay TimeOfDay, ENarr_WeatherType Weather)
{
    CurrentBiome = Biome;
    CurrentTimeOfDay = TimeOfDay;
    CurrentWeather = Weather;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Context updated - Biome: %d, Time: %d, Weather: %d"), 
           static_cast<int32>(Biome), static_cast<int32>(TimeOfDay), static_cast<int32>(Weather));
}

void UNarrativeManager::InitializeStoryBeats()
{
    // Discovery Phase
    FNarr_StoryBeat FirstAwakening;
    FirstAwakening.BeatID = TEXT("FirstAwakening");
    FirstAwakening.BeatTitle = FText::FromString(TEXT("First Light"));
    FirstAwakening.BeatDescription = FText::FromString(TEXT("You awaken in an unfamiliar world"));
    FirstAwakening.Phase = ENarr_StoryPhase::Discovery;
    FirstAwakening.RequiredConditions.Add(TEXT("GameStarted"));
    FirstAwakening.UnlockedDialogues.Add(TEXT("Valley_Intro"));
    StoryBeats.Add(FirstAwakening);
    
    // Survival Phase
    FNarr_StoryBeat FirstEncounter;
    FirstEncounter.BeatID = TEXT("FirstDinosaurEncounter");
    FirstEncounter.BeatTitle = FText::FromString(TEXT("Ancient Predators"));
    FirstEncounter.BeatDescription = FText::FromString(TEXT("Your first encounter with the apex predators of this world"));
    FirstEncounter.Phase = ENarr_StoryPhase::Survival;
    FirstEncounter.RequiredConditions.Add(TEXT("FirstDinosaurSeen"));
    FirstEncounter.UnlockedDialogues.Add(TEXT("Predator_Warning"));
    StoryBeats.Add(FirstEncounter);
    
    // Adaptation Phase
    FNarr_StoryBeat BasicSurvival;
    BasicSurvival.BeatID = TEXT("BasicSurvivalMastery");
    BasicSurvival.BeatTitle = FText::FromString(TEXT("Learning to Survive"));
    BasicSurvival.BeatDescription = FText::FromString(TEXT("You begin to understand the rhythms of this dangerous world"));
    BasicSurvival.Phase = ENarr_StoryPhase::Adaptation;
    BasicSurvival.RequiredConditions.Add(TEXT("WaterFound"));
    BasicSurvival.RequiredConditions.Add(TEXT("ShelterBuilt"));
    BasicSurvival.UnlockedDialogues.Add(TEXT("Survival_Mastery"));
    StoryBeats.Add(BasicSurvival);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized %d story beats"), StoryBeats.Num());
}

void UNarrativeManager::LoadDialogueData()
{
    // In a full implementation, this would load from a data table asset
    // For now, we set up the system to be ready for data table integration
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dialogue data system ready"));
}

bool UNarrativeManager::EvaluateConditions(const TArray<FString>& Conditions) const
{
    for (const FString& Condition : Conditions)
    {
        const bool* ConditionValue = DialogueConditions.Find(Condition);
        if (!ConditionValue || !(*ConditionValue))
        {
            return false;
        }
    }
    
    return true;
}

void UNarrativeManager::BroadcastDialogue(const FNarr_DialogueEntry& DialogueEntry, AActor* Speaker)
{
    // In a full implementation, this would trigger UI display, audio playback, etc.
    // For now, we log the dialogue
    
    FString SpeakerName = Speaker ? Speaker->GetName() : DialogueEntry.SpeakerName;
    
    UE_LOG(LogTemp, Log, TEXT("DIALOGUE [%s]: %s"), 
           *SpeakerName, *DialogueEntry.DialogueText.ToString());
    
    // Broadcast to UI system (when implemented)
    // Trigger audio playback (when implemented)
    // Handle dialogue type-specific behavior
}