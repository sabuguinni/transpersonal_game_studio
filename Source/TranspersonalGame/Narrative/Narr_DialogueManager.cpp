#include "Narr_DialogueManager.h"
#include "../TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bDialogueActive = false;
    CurrentPlayer = nullptr;
    CurrentSpeaker = nullptr;
    DialogueDataTable = nullptr;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initializing narrative system"));
    
    // Initialize default dialogues for survival game
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Survival dialogue system ready"));
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    CompletedStoryEvents.Empty();
    
    Super::Deinitialize();
}

bool UNarr_DialogueManager::StartDialogue(const FString& DialogueID, ATranspersonalCharacter* Player, AActor* Speaker)
{
    if (!Player || !Speaker)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Invalid player or speaker for dialogue %s"), *DialogueID);
        return false;
    }

    // End any existing dialogue
    if (bDialogueActive)
    {
        EndDialogue();
    }

    // Get dialogue data
    FNarr_DialogueData DialogueData = GetDialogueData(DialogueID);
    if (DialogueData.DialogueID.IsEmpty() || DialogueData.DialogueID == "default_dialogue")
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Dialogue %s not found, using default"), *DialogueID);
    }

    // Set current dialogue state
    bDialogueActive = true;
    CurrentDialogueID = DialogueID;
    CurrentPlayer = Player;
    CurrentSpeaker = Speaker;
    CurrentDialogueData = DialogueData;
    CurrentSpeakerName = DialogueData.SpeakerName;

    // Play opening audio if available
    if (!DialogueData.AudioAssetPath.IsEmpty())
    {
        PlayDialogueAudio(DialogueData.AudioAssetPath);
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Started dialogue %s with %s"), *DialogueID, *DialogueData.SpeakerName);
    return true;
}

void UNarr_DialogueManager::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Ending dialogue %s"), *CurrentDialogueID);

    bDialogueActive = false;
    CurrentDialogueID.Empty();
    CurrentSpeakerName.Empty();
    CurrentPlayer = nullptr;
    CurrentSpeaker = nullptr;
    CurrentDialogueData = FNarr_DialogueData();
}

bool UNarr_DialogueManager::SelectDialogueOption(int32 OptionIndex)
{
    if (!bDialogueActive || !CurrentDialogueData.DialogueOptions.IsValidIndex(OptionIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Invalid dialogue option %d"), OptionIndex);
        return false;
    }

    const FNarr_DialogueOption& SelectedOption = CurrentDialogueData.DialogueOptions[OptionIndex];

    // Check if player meets condition for this option
    if (!CheckDialogueCondition(SelectedOption.RequiredCondition, SelectedOption.RequiredValue, CurrentPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Player doesn't meet condition for option %d"), OptionIndex);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Player selected option: %s"), *SelectedOption.OptionText.ToString());

    // End dialogue if this option terminates it
    if (SelectedOption.bEndsDialogue)
    {
        EndDialogue();
    }

    return true;
}

FNarr_DialogueData UNarr_DialogueManager::GetDialogueData(const FString& DialogueID)
{
    // For now, return hardcoded survival dialogues
    // In production, this would query the DataTable
    
    if (DialogueID == "tribal_elder_greeting")
    {
        FNarr_DialogueData Data;
        Data.DialogueID = DialogueID;
        Data.SpeakerName = "Tribal Elder";
        Data.DialogueType = ENarr_DialogueType::Story;
        Data.OpeningText = FText::FromString("The ancient stories speak of survival through unity. When the earth shook and the great beasts ruled, our ancestors learned that knowledge shared is survival earned.");
        
        FNarr_DialogueOption Option1;
        Option1.OptionText = FText::FromString("Tell me about the ancient ways.");
        Option1.ResponseText = FText::FromString("Long ago, our people learned to read the signs - the way raptors hunt, how herbivores protect their young, where to find shelter when the storms come.");
        Option1.bEndsDialogue = false;
        
        FNarr_DialogueOption Option2;
        Option2.OptionText = FText::FromString("What dangers should I know about?");
        Option2.ResponseText = FText::FromString("The pack hunters are clever - they test your resolve before striking. Never show fear, but never show aggression without cause. Respect earns you passage.");
        Option2.bEndsDialogue = true;
        
        Data.DialogueOptions.Add(Option1);
        Data.DialogueOptions.Add(Option2);
        Data.bRepeatable = true;
        
        return Data;
    }
    else if (DialogueID == "scout_guide_warning")
    {
        FNarr_DialogueData Data;
        Data.DialogueID = DialogueID;
        Data.SpeakerName = "Scout Guide";
        Data.DialogueType = ENarr_DialogueType::Warning;
        Data.OpeningText = FText::FromString("Stay close and move quietly. The forest has eyes - Triceratops ahead, grazing peacefully. See how they position themselves? Always one watching while others feed.");
        
        FNarr_DialogueOption Option1;
        Option1.OptionText = FText::FromString("How do I approach them safely?");
        Option1.ResponseText = FText::FromString("Slow movements, no sudden gestures. They're peaceful unless threatened. But hear that distant roar? That's a predator marking territory. We go around, not through.");
        Option1.bEndsDialogue = true;
        
        Data.DialogueOptions.Add(Option1);
        Data.bRepeatable = false;
        
        return Data;
    }
    else if (DialogueID == "veteran_hunter_advice")
    {
        FNarr_DialogueData Data;
        Data.DialogueID = DialogueID;
        Data.SpeakerName = "Veteran Hunter";
        Data.DialogueType = ENarr_DialogueType::Information;
        Data.OpeningText = FText::FromString("The wounded raptor limps toward the river - injured prey becomes desperate, unpredictable. This is when they're most dangerous.");
        
        FNarr_DialogueOption Option1;
        Option1.OptionText = FText::FromString("Should we pursue it?");
        Option1.ResponseText = FText::FromString("Keep your spear ready, but remember - even a dying predator can take you with it. Sometimes the wisest hunt is the one you abandon.");
        Option1.RequiredCondition = ENarr_DialogueCondition::SurvivalLevel;
        Option1.RequiredValue = 3;
        Option1.bEndsDialogue = true;
        
        Data.DialogueOptions.Add(Option1);
        Data.bRepeatable = true;
        
        return Data;
    }

    // Default dialogue for unknown IDs
    FNarr_DialogueData DefaultData;
    DefaultData.DialogueID = "default_dialogue";
    DefaultData.SpeakerName = "Survivor";
    DefaultData.DialogueType = ENarr_DialogueType::Greeting;
    DefaultData.OpeningText = FText::FromString("Stay alert out there. The wilderness shows no mercy to the unprepared.");
    
    FNarr_DialogueOption DefaultOption;
    DefaultOption.OptionText = FText::FromString("I understand.");
    DefaultOption.ResponseText = FText::FromString("Good. Survival depends on it.");
    DefaultOption.bEndsDialogue = true;
    
    DefaultData.DialogueOptions.Add(DefaultOption);
    DefaultData.bRepeatable = true;
    
    return DefaultData;
}

TArray<FNarr_DialogueOption> UNarr_DialogueManager::GetCurrentDialogueOptions()
{
    if (!bDialogueActive)
    {
        return TArray<FNarr_DialogueOption>();
    }

    TArray<FNarr_DialogueOption> AvailableOptions;
    
    for (const FNarr_DialogueOption& Option : CurrentDialogueData.DialogueOptions)
    {
        if (CheckDialogueCondition(Option.RequiredCondition, Option.RequiredValue, CurrentPlayer))
        {
            AvailableOptions.Add(Option);
        }
    }

    return AvailableOptions;
}

bool UNarr_DialogueManager::CheckDialogueCondition(ENarr_DialogueCondition Condition, int32 RequiredValue, ATranspersonalCharacter* Player)
{
    if (!Player)
    {
        return false;
    }

    switch (Condition)
    {
        case ENarr_DialogueCondition::None:
            return true;

        case ENarr_DialogueCondition::SurvivalLevel:
            return Player->GetSurvivalLevel() >= RequiredValue;

        case ENarr_DialogueCondition::PlayerHealth:
            return Player->GetCurrentHealth() >= RequiredValue;

        case ENarr_DialogueCondition::TimeOfDay:
        {
            // Get current time of day (0-24 hours)
            UWorld* World = Player->GetWorld();
            if (World)
            {
                float TimeOfDay = World->GetTimeSeconds() / 3600.0f; // Convert to hours
                int32 CurrentHour = FMath::FloorToInt(FMath::Fmod(TimeOfDay, 24.0f));
                return CurrentHour >= RequiredValue;
            }
            return true;
        }

        case ENarr_DialogueCondition::QuestCompleted:
            // This would check quest completion status
            // For now, always return true
            return true;

        case ENarr_DialogueCondition::ItemInInventory:
            // This would check player inventory
            // For now, always return true
            return true;

        default:
            return true;
    }
}

void UNarr_DialogueManager::MarkStoryEvent(const FString& EventID)
{
    CompletedStoryEvents.Add(EventID);
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Story event marked: %s"), *EventID);
}

bool UNarr_DialogueManager::HasStoryEventOccurred(const FString& EventID)
{
    return CompletedStoryEvents.Contains(EventID);
}

void UNarr_DialogueManager::PlayDialogueAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty() || !CurrentSpeaker)
    {
        return;
    }

    // Try to load and play the audio asset
    USoundBase* AudioAsset = LoadObject<USoundBase>(nullptr, *AudioPath);
    if (AudioAsset)
    {
        UGameplayStatics::PlaySoundAtLocation(CurrentSpeaker->GetWorld(), AudioAsset, CurrentSpeaker->GetActorLocation());
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Playing audio %s"), *AudioPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Failed to load audio %s"), *AudioPath);
    }
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    CreateSurvivalDialogues();
    CreateQuestDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Default survival dialogues initialized"));
}

void UNarr_DialogueManager::CreateSurvivalDialogues()
{
    // Mark initial story events
    MarkStoryEvent("game_started");
    MarkStoryEvent("first_dialogue_system_ready");
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Survival dialogue templates created"));
}

void UNarr_DialogueManager::CreateQuestDialogues()
{
    // Initialize quest-related dialogue hooks
    MarkStoryEvent("quest_system_integrated");
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Quest dialogue integration ready"));
}