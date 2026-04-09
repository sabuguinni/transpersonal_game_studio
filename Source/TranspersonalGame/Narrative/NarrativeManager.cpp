// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "NarrativeManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentEmotionalState = 0.0f;
    DominantTone = EEmotionalTone::Wonder;
    MaxEmotionalHistorySize = 100;
    EmotionalDecayRate = 0.1f;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDataTables();
    
    // Initialize emotional state with wonder - the paleontologist's first reaction
    UpdateEmotionalState(EEmotionalTone::Wonder, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialized - The story begins with wonder"));
}

void UNarrativeManager::Deinitialize()
{
    OnNarrativeEvent.Clear();
    OnDialogueTriggered.Clear();
    OnStoryBeatCompleted.Clear();
    
    Super::Deinitialize();
}

void UNarrativeManager::InitializeDataTables()
{
    // Data tables will be set up in Blueprint or loaded from content
    // This allows designers to modify narrative content without code changes
    
    if (!NarrativeEventsTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: NarrativeEventsTable not set - narrative events will not function"));
    }
    
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: DialogueTable not set - dialogue system will not function"));
    }
    
    if (!StoryBeatsTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: StoryBeatsTable not set - story progression will not function"));
    }
}

void UNarrativeManager::TriggerNarrativeEvent(const FString& EventID, AActor* Instigator)
{
    if (TriggeredEvents.Contains(EventID))
    {
        return; // Event already triggered
    }
    
    if (!NarrativeEventsTable)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Cannot trigger event %s - NarrativeEventsTable not set"), *EventID);
        return;
    }
    
    FNarrativeEvent* EventData = NarrativeEventsTable->FindRow<FNarrativeEvent>(FName(*EventID), TEXT("TriggerNarrativeEvent"));
    if (!EventData)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Event %s not found in NarrativeEventsTable"), *EventID);
        return;
    }
    
    // Check if conditions are met
    if (!EvaluateNarrativeConditions(EventData->RequiredTags))
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Event %s conditions not met"), *EventID);
        return;
    }
    
    // Process the event
    TriggeredEvents.Add(EventID);
    ProcessEmotionalImpact(EventData->EmotionalImpact, EventData->EmotionalTone);
    
    // Grant tags
    // TODO: Implement tag granting system when GameplayTags are fully integrated
    
    // Broadcast event
    OnNarrativeEvent.Broadcast(*EventData, EventData->EmotionalImpact);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggered event %s with emotional impact %f"), 
           *EventID, EventData->EmotionalImpact);
}

void UNarrativeManager::PlayDialogue(const FString& LineID, AActor* Speaker, AActor* Listener)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Cannot play dialogue %s - DialogueTable not set"), *LineID);
        return;
    }
    
    FDialogueLine* DialogueData = DialogueTable->FindRow<FDialogueLine>(FName(*LineID), TEXT("PlayDialogue"));
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue line %s not found in DialogueTable"), *LineID);
        return;
    }
    
    // Check conditions
    if (!CanTriggerDialogue(LineID, Speaker, Listener))
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dialogue %s conditions not met"), *LineID);
        return;
    }
    
    // Update emotional state based on dialogue tone
    float EmotionalImpact = 0.5f; // Dialogue has moderate emotional impact
    ProcessEmotionalImpact(EmotionalImpact, DialogueData->Tone);
    
    // Broadcast dialogue event
    OnDialogueTriggered.Broadcast(*DialogueData, Speaker, Listener);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Playing dialogue %s from %s"), 
           *LineID, Speaker ? *Speaker->GetName() : TEXT("Unknown"));
}

void UNarrativeManager::CompleteStoryBeat(const FString& BeatID)
{
    if (CompletedStoryBeats.Contains(BeatID))
    {
        return; // Already completed
    }
    
    if (!StoryBeatsTable)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Cannot complete story beat %s - StoryBeatsTable not set"), *BeatID);
        return;
    }
    
    FStoryBeat* BeatData = StoryBeatsTable->FindRow<FStoryBeat>(FName(*BeatID), TEXT("CompleteStoryBeat"));
    if (!BeatData)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story beat %s not found in StoryBeatsTable"), *BeatID);
        return;
    }
    
    // Mark as completed
    CompletedStoryBeats.Add(BeatID);
    
    // Process emotional impact
    ProcessEmotionalImpact(BeatData->StoryWeight, BeatData->EmotionalArc);
    
    // Trigger follow-up events
    for (const FString& TriggeredEvent : BeatData->TriggeredEvents)
    {
        TriggerNarrativeEvent(TriggeredEvent);
    }
    
    // Broadcast completion
    OnStoryBeatCompleted.Broadcast(*BeatData);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Completed story beat %s"), *BeatID);
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    return CompletedStoryBeats.Contains(BeatID);
}

TArray<FStoryBeat> UNarrativeManager::GetAvailableStoryBeats() const
{
    TArray<FStoryBeat> AvailableBeats;
    
    if (!StoryBeatsTable)
    {
        return AvailableBeats;
    }
    
    TArray<FStoryBeat*> AllBeats;
    StoryBeatsTable->GetAllRows<FStoryBeat>(TEXT("GetAvailableStoryBeats"), AllBeats);
    
    for (FStoryBeat* Beat : AllBeats)
    {
        if (!Beat || CompletedStoryBeats.Contains(Beat->BeatID))
        {
            continue; // Skip completed beats
        }
        
        // Check if all required events are completed
        bool bCanStart = true;
        for (const FString& RequiredEvent : Beat->RequiredEvents)
        {
            if (!TriggeredEvents.Contains(RequiredEvent))
            {
                bCanStart = false;
                break;
            }
        }
        
        if (bCanStart)
        {
            AvailableBeats.Add(*Beat);
        }
    }
    
    return AvailableBeats;
}

void UNarrativeManager::UpdateEmotionalState(EEmotionalTone NewTone, float Intensity)
{
    // Map emotional tones to numerical values for blending
    float ToneValue = 0.0f;
    switch (NewTone)
    {
        case EEmotionalTone::Fear: ToneValue = -1.0f; break;
        case EEmotionalTone::Desperation: ToneValue = -0.8f; break;
        case EEmotionalTone::Loneliness: ToneValue = -0.5f; break;
        case EEmotionalTone::Determination: ToneValue = 0.0f; break;
        case EEmotionalTone::Hope: ToneValue = 0.3f; break;
        case EEmotionalTone::Wonder: ToneValue = 0.6f; break;
        case EEmotionalTone::Peace: ToneValue = 0.8f; break;
        case EEmotionalTone::Awe: ToneValue = 1.0f; break;
    }
    
    // Blend with current state
    float NewEmotionalValue = ToneValue * Intensity;
    CurrentEmotionalState = FMath::Lerp(CurrentEmotionalState, NewEmotionalValue, 0.3f);
    
    // Update dominant tone
    DominantTone = NewTone;
    
    // Add to history
    UpdateEmotionalHistory(CurrentEmotionalState);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Emotional state updated to %f (Tone: %d)"), 
           CurrentEmotionalState, (int32)NewTone);
}

FDialogueLine UNarrativeManager::GetDialogueForContext(const FString& SpeakerID, EDialogueContext Context) const
{
    FDialogueLine DefaultLine;
    
    if (!DialogueTable)
    {
        return DefaultLine;
    }
    
    // Search for dialogue lines matching speaker and context
    TArray<FDialogueLine*> AllDialogue;
    DialogueTable->GetAllRows<FDialogueLine>(TEXT("GetDialogueForContext"), AllDialogue);
    
    for (FDialogueLine* Line : AllDialogue)
    {
        if (Line && Line->SpeakerName.ToString() == SpeakerID && Line->Context == Context)
        {
            if (EvaluateNarrativeConditions(Line->RequiredConditions))
            {
                return *Line;
            }
        }
    }
    
    return DefaultLine;
}

bool UNarrativeManager::CanTriggerDialogue(const FString& LineID, AActor* Speaker, AActor* Listener) const
{
    if (!DialogueTable)
    {
        return false;
    }
    
    FDialogueLine* DialogueData = DialogueTable->FindRow<FDialogueLine>(FName(*LineID), TEXT("CanTriggerDialogue"));
    if (!DialogueData)
    {
        return false;
    }
    
    return EvaluateNarrativeConditions(DialogueData->RequiredConditions);
}

void UNarrativeManager::RecordPlayerChoice(const FString& ChoiceID, const FString& ChoiceValue)
{
    PlayerChoices.Add(ChoiceID, ChoiceValue);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Recorded player choice %s = %s"), *ChoiceID, *ChoiceValue);
}

FString UNarrativeManager::GetPlayerChoice(const FString& ChoiceID) const
{
    if (const FString* Choice = PlayerChoices.Find(ChoiceID))
    {
        return *Choice;
    }
    return FString();
}

void UNarrativeManager::UpdateEmotionalHistory(float NewValue)
{
    EmotionalHistory.Add(NewValue);
    
    // Maintain maximum history size
    if (EmotionalHistory.Num() > MaxEmotionalHistorySize)
    {
        EmotionalHistory.RemoveAt(0);
    }
}

bool UNarrativeManager::EvaluateNarrativeConditions(const FGameplayTagContainer& RequiredTags) const
{
    // TODO: Implement proper gameplay tag evaluation when system is integrated
    // For now, return true to allow all narrative events
    return true;
}

void UNarrativeManager::ProcessEmotionalImpact(float Impact, EEmotionalTone Tone)
{
    // Apply emotional decay to previous state
    CurrentEmotionalState *= (1.0f - EmotionalDecayRate);
    
    // Update with new emotional input
    UpdateEmotionalState(Tone, Impact);
}