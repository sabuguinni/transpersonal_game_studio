#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default values
    CurrentConsciousnessLevel = EConsciousnessLevel::Unaware;
    ConsciousnessProgress = 0.0f;
    CurrentStoryBeat = EStoryBeat::Introduction;
    bDialogueActive = false;
    CurrentSpeaker = nullptr;
    
    // Load narrative data
    LoadNarrativeData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized"));
}

void UNarrativeManager::Deinitialize()
{
    // Clean up any active dialogues
    if (bDialogueActive)
    {
        bDialogueActive = false;
        CurrentSpeaker = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::LoadNarrativeData()
{
    // Load dialogue data table
    DialogueDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/TranspersonalGame/Data/DT_Dialogues"));
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load DialogueDataTable"));
    }
    
    // Load narrative events data table
    NarrativeEventsDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/TranspersonalGame/Data/DT_NarrativeEvents"));
    if (!NarrativeEventsDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load NarrativeEventsDataTable"));
    }
    
    // Initialize with basic unlocked dialogues
    UnlockedDialogues.Add(TEXT("Narrator_Opening"));
    UnlockedDialogues.Add(TEXT("Aria_Introduction"));
}

void UNarrativeManager::TriggerNarrativeEvent(const FString& EventID)
{
    if (!NarrativeEventsDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger narrative event - no data table loaded"));
        return;
    }
    
    // Check if event already completed
    if (CompletedEvents.Contains(EventID))
    {
        UE_LOG(LogTemp, Log, TEXT("Narrative event %s already completed"), *EventID);
        return;
    }
    
    // Find the event in the data table
    FNarrativeEvent* EventData = NarrativeEventsDataTable->FindRow<FNarrativeEvent>(FName(*EventID), TEXT(""));
    if (!EventData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative event %s not found in data table"), *EventID);
        return;
    }
    
    // Process the event
    ProcessNarrativeEvent(*EventData);
    
    // Mark as completed
    CompletedEvents.Add(EventID);
    
    // Broadcast the event
    OnNarrativeEvent.Broadcast(*EventData);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered narrative event: %s"), *EventID);
}

void UNarrativeManager::ProcessNarrativeEvent(const FNarrativeEvent& Event)
{
    // Update consciousness
    UpdateConsciousnessLevel(Event.ConsciousnessImpact);
    
    // Update story beat if necessary
    if (Event.StoryBeat != CurrentStoryBeat)
    {
        CurrentStoryBeat = Event.StoryBeat;
        UE_LOG(LogTemp, Log, TEXT("Story beat advanced to: %d"), (int32)CurrentStoryBeat);
    }
    
    // Unlock new dialogues
    for (const FString& DialogueID : Event.UnlockedDialogues)
    {
        if (!UnlockedDialogues.Contains(DialogueID))
        {
            UnlockedDialogues.Add(DialogueID);
            UE_LOG(LogTemp, Log, TEXT("Unlocked dialogue: %s"), *DialogueID);
        }
    }
}

void UNarrativeManager::UpdateConsciousnessLevel(float DeltaConsciousness)
{
    EConsciousnessLevel OldLevel = CurrentConsciousnessLevel;
    
    // Update progress
    ConsciousnessProgress = FMath::Clamp(ConsciousnessProgress + DeltaConsciousness, 0.0f, 100.0f);
    
    // Calculate new level
    EConsciousnessLevel NewLevel = CalculateConsciousnessLevel(ConsciousnessProgress);
    
    // Check for level change
    if (NewLevel != OldLevel)
    {
        CurrentConsciousnessLevel = NewLevel;
        UpdateStoryBeatBasedOnConsciousness();
        
        // Broadcast level change
        OnConsciousnessChanged.Broadcast(OldLevel, NewLevel);
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness level changed from %d to %d (Progress: %.1f)"), 
               (int32)OldLevel, (int32)NewLevel, ConsciousnessProgress);
    }
}

EConsciousnessLevel UNarrativeManager::CalculateConsciousnessLevel(float Progress) const
{
    if (Progress >= 80.0f)
        return EConsciousnessLevel::Transcendent;
    else if (Progress >= 60.0f)
        return EConsciousnessLevel::Enlightened;
    else if (Progress >= 40.0f)
        return EConsciousnessLevel::Aware;
    else if (Progress >= 20.0f)
        return EConsciousnessLevel::Awakening;
    else
        return EConsciousnessLevel::Unaware;
}

void UNarrativeManager::UpdateStoryBeatBasedOnConsciousness()
{
    EStoryBeat NewBeat = CurrentStoryBeat;
    
    switch (CurrentConsciousnessLevel)
    {
        case EConsciousnessLevel::Unaware:
            NewBeat = EStoryBeat::Introduction;
            break;
        case EConsciousnessLevel::Awakening:
            NewBeat = EStoryBeat::FirstAwakening;
            break;
        case EConsciousnessLevel::Aware:
            NewBeat = EStoryBeat::SpiritualCrisis;
            break;
        case EConsciousnessLevel::Enlightened:
            NewBeat = EStoryBeat::Deepening;
            break;
        case EConsciousnessLevel::Transcendent:
            NewBeat = EStoryBeat::Transcendence;
            break;
    }
    
    if (NewBeat != CurrentStoryBeat)
    {
        CurrentStoryBeat = NewBeat;
        UE_LOG(LogTemp, Log, TEXT("Story beat automatically advanced to: %d"), (int32)CurrentStoryBeat);
    }
}

bool UNarrativeManager::StartDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!DialogueDataTable || !Speaker)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - missing data table or speaker"));
        return false;
    }
    
    // Check if dialogue is unlocked
    if (!IsDialogueUnlocked(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s is not unlocked"), *DialogueID);
        return false;
    }
    
    // Find dialogue data
    FDialogueEntry* DialogueData = DialogueDataTable->FindRow<FDialogueEntry>(FName(*DialogueID), TEXT(""));
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s not found in data table"), *DialogueID);
        return false;
    }
    
    // Check consciousness requirement
    if (DialogueData->RequiredConsciousnessLevel > CurrentConsciousnessLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s requires consciousness level %d, current is %d"), 
               *DialogueID, (int32)DialogueData->RequiredConsciousnessLevel, (int32)CurrentConsciousnessLevel);
        return false;
    }
    
    // Start the dialogue
    bDialogueActive = true;
    CurrentDialogue = *DialogueData;
    CurrentSpeaker = Speaker;
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with %s"), *DialogueID, *Speaker->GetName());
    return true;
}

void UNarrativeManager::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (!bDialogueActive || ChoiceIndex < 0 || ChoiceIndex >= CurrentDialogue.DialogueChoices.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid dialogue choice: %d"), ChoiceIndex);
        return;
    }
    
    // Process the choice consequence
    if (ChoiceIndex < CurrentDialogue.ChoiceConsequences.Num())
    {
        const FString& Consequence = CurrentDialogue.ChoiceConsequences[ChoiceIndex];
        
        // Parse consequence (simple format: "consciousness:+10" or "event:EventID")
        if (Consequence.StartsWith(TEXT("consciousness:")))
        {
            FString ValueStr = Consequence.RightChop(13); // Remove "consciousness:"
            float Value = FCString::Atof(*ValueStr);
            UpdateConsciousnessLevel(Value);
        }
        else if (Consequence.StartsWith(TEXT("event:")))
        {
            FString EventID = Consequence.RightChop(6); // Remove "event:"
            TriggerNarrativeEvent(EventID);
        }
    }
    
    // End dialogue for now (could be extended to chain dialogues)
    bDialogueActive = false;
    CurrentSpeaker = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Selected dialogue choice: %d"), ChoiceIndex);
}

TArray<FString> UNarrativeManager::GetAvailableDialogueChoices() const
{
    if (!bDialogueActive)
    {
        return TArray<FString>();
    }
    
    return CurrentDialogue.DialogueChoices;
}

void UNarrativeManager::AdvanceStoryBeat()
{
    int32 CurrentBeatIndex = (int32)CurrentStoryBeat;
    int32 NextBeatIndex = FMath::Min(CurrentBeatIndex + 1, (int32)EStoryBeat::Transcendence);
    
    CurrentStoryBeat = (EStoryBeat)NextBeatIndex;
    
    UE_LOG(LogTemp, Log, TEXT("Manually advanced story beat to: %d"), NextBeatIndex);
}

bool UNarrativeManager::IsDialogueUnlocked(const FString& DialogueID) const
{
    return UnlockedDialogues.Contains(DialogueID);
}