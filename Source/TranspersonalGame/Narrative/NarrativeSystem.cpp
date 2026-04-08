#include "NarrativeSystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "Components/AudioComponent.h"

UNarrativeSystem::UNarrativeSystem()
{
    CurrentEmotionalState = EEmotionalState::Calm;
}

void UNarrativeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadNarrativeData();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative System initialized - The story begins..."));
}

void UNarrativeSystem::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("Narrative System shutdown - Story preserved in memory"));
}

void UNarrativeSystem::LoadNarrativeData()
{
    // Initialize with core story tags
    CurrentStoryTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Story.Act1.Beginning")));
    CurrentStoryTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Protagonist.Lost")));
    CurrentStoryTags.AddTag(FGameplayTag::RequestGameplayTag(FName("World.Jurassic.Unknown")));
    
    // Add initial narrative beat
    ActiveNarrativeBeats.Add(FName("Awakening"));
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative data loaded - %d story tags active"), CurrentStoryTags.Num());
}

void UNarrativeSystem::TriggerDialogue(FName DialogueID, AActor* Speaker, EEmotionalState EmotionalContext)
{
    if (!Speaker)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger dialogue %s - no speaker provided"), *DialogueID.ToString());
        return;
    }

    FDialogueContext* DialogueContext = FindDialogueContext(DialogueID, EmotionalContext);
    if (!DialogueContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue context not found: %s"), *DialogueID.ToString());
        return;
    }

    // Check if dialogue can be triggered based on current story state
    if (!CanTriggerDialogue(DialogueID, CurrentStoryTags))
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue %s cannot be triggered - requirements not met"), *DialogueID.ToString());
        return;
    }

    // Update emotional state if this is the protagonist
    if (DialogueContext->SpeakerType == EDialogueVoiceType::Protagonist)
    {
        CurrentEmotionalState = EmotionalContext;
    }

    // Broadcast dialogue event
    OnDialogueTriggered.Broadcast(DialogueID, Speaker);
    
    // Play audio if available
    if (DialogueContext->AudioClip.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::PlaySoundAtLocation(World, DialogueContext->AudioClip.Get(), Speaker->GetActorLocation());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue triggered: %s - %s"), *DialogueID.ToString(), *DialogueContext->DialogueText.ToString());
}

bool UNarrativeSystem::CanTriggerDialogue(FName DialogueID, const FGameplayTagContainer& CurrentTags) const
{
    FDialogueContext* DialogueContext = FindDialogueContext(DialogueID, CurrentEmotionalState);
    if (!DialogueContext)
    {
        return false;
    }

    // Check if all required tags are present
    return CurrentTags.HasAllExact(DialogueContext->RequiredTags);
}

void UNarrativeSystem::PlayInternalMonologue(FName DialogueID, EEmotionalState EmotionalContext)
{
    FDialogueContext* DialogueContext = FindDialogueContext(DialogueID, EmotionalContext);
    if (!DialogueContext || !DialogueContext->bIsInternalMonologue)
    {
        UE_LOG(LogTemp, Warning, TEXT("Internal monologue not found or not configured: %s"), *DialogueID.ToString());
        return;
    }

    CurrentEmotionalState = EmotionalContext;
    
    // Trigger without speaker for internal thoughts
    OnDialogueTriggered.Broadcast(DialogueID, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("Internal monologue: %s - %s"), *DialogueID.ToString(), *DialogueContext->DialogueText.ToString());
}

void UNarrativeSystem::AdvanceNarrativeBeat(FName BeatID)
{
    if (CompletedNarrativeBeats.Contains(BeatID))
    {
        UE_LOG(LogTemp, Log, TEXT("Narrative beat %s already completed"), *BeatID.ToString());
        return;
    }

    // Move from active to completed
    ActiveNarrativeBeats.Remove(BeatID);
    CompletedNarrativeBeats.Add(BeatID);

    // Add completion tags to story state
    FGameplayTag CompletionTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("Story.Beat.%s.Completed"), *BeatID.ToString())));
    CurrentStoryTags.AddTag(CompletionTag);

    // Broadcast narrative event
    OnNarrativeEvent.Broadcast(CompletionTag);

    UE_LOG(LogTemp, Warning, TEXT("Narrative beat completed: %s"), *BeatID.ToString());

    // Check for new narrative beats that might be unlocked
    CheckNarrativeProgression(CurrentStoryTags);
}

bool UNarrativeSystem::IsNarrativeBeatActive(FName BeatID) const
{
    return ActiveNarrativeBeats.Contains(BeatID);
}

void UNarrativeSystem::CheckNarrativeProgression(const FGameplayTagContainer& CurrentGameState)
{
    // Example progression logic - in real implementation this would be data-driven
    
    // If awakening is complete and we haven't started exploration
    if (CompletedNarrativeBeats.Contains(FName("Awakening")) && 
        !ActiveNarrativeBeats.Contains(FName("FirstExploration")))
    {
        ActiveNarrativeBeats.Add(FName("FirstExploration"));
        CurrentStoryTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Story.Act1.Exploration")));
        
        OnNarrativeEvent.Broadcast(FGameplayTag::RequestGameplayTag(FName("Story.Beat.FirstExploration.Started")));
    }

    // If first dinosaur encounter happened
    if (CurrentGameState.HasTag(FGameplayTag::RequestGameplayTag(FName("Encounter.Dinosaur.First"))) &&
        !ActiveNarrativeBeats.Contains(FName("FirstContact")))
    {
        ActiveNarrativeBeats.Add(FName("FirstContact"));
        CurrentStoryTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Story.Act1.FirstContact")));
    }
}

void UNarrativeSystem::UpdateProtagonistState(EEmotionalState NewState, const FString& Reason)
{
    EEmotionalState PreviousState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;

    // Log the emotional transition
    FString StateTransition = FString::Printf(TEXT("Emotional state: %s -> %s (%s)"), 
        *UEnum::GetValueAsString(PreviousState),
        *UEnum::GetValueAsString(NewState),
        *Reason);
    
    DiscoveryLog.Add(StateTransition);

    // Broadcast the change
    FGameplayTag EmotionalTag = FGameplayTag::RequestGameplayTag(
        FName(*FString::Printf(TEXT("Character.Emotion.%s"), *UEnum::GetValueAsString(NewState))));
    OnNarrativeEvent.Broadcast(EmotionalTag);

    UE_LOG(LogTemp, Log, TEXT("Protagonist emotional state updated: %s"), *StateTransition);
}

void UNarrativeSystem::RegisterDiscovery(FGameplayTag DiscoveryTag, const FString& DiscoveryDescription)
{
    FString Discovery = FString::Printf(TEXT("[Discovery] %s: %s"), 
        *DiscoveryTag.ToString(), *DiscoveryDescription);
    
    DiscoveryLog.Add(Discovery);
    CurrentStoryTags.AddTag(DiscoveryTag);

    // Check if this discovery triggers new narrative content
    CheckNarrativeProgression(CurrentStoryTags);

    UE_LOG(LogTemp, Warning, TEXT("Discovery registered: %s"), *Discovery);
}

FDialogueContext* UNarrativeSystem::FindDialogueContext(FName DialogueID, EEmotionalState EmotionalContext) const
{
    // In a real implementation, this would search the DataTable
    // For now, return nullptr as placeholder
    return nullptr;
}

void UNarrativeSystem::ProcessNarrativeBeatCompletion(const FNarrativeBeat& Beat)
{
    // Add completion tags
    for (const FGameplayTag& Tag : Beat.CompletionTags.GetGameplayTagArray())
    {
        CurrentStoryTags.AddTag(Tag);
    }

    // Trigger associated dialogues
    for (const FName& DialogueID : Beat.DialogueSequence)
    {
        // Queue dialogue for later playback
        UE_LOG(LogTemp, Log, TEXT("Queuing dialogue for narrative beat: %s"), *DialogueID.ToString());
    }
}

// Dialogue Component Implementation
UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    VoiceType = EDialogueVoiceType::Environment;
    CurrentEmotionalState = EEmotionalState::Calm;
}

void UDialogueComponent::InitiateDialogue(FName DialogueID, AActor* Target)
{
    if (UNarrativeSystem* NarrativeSystem = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeSystem>())
    {
        NarrativeSystem->TriggerDialogue(DialogueID, GetOwner(), CurrentEmotionalState);
    }
}

void UDialogueComponent::SetEmotionalState(EEmotionalState NewState)
{
    CurrentEmotionalState = NewState;
    UE_LOG(LogTemp, Log, TEXT("Actor %s emotional state set to %s"), 
        *GetOwner()->GetName(), *UEnum::GetValueAsString(NewState));
}

void UDialogueComponent::AddDialogueTag(FGameplayTag Tag)
{
    DialogueTags.AddTag(Tag);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue tag %s to %s"), *Tag.ToString(), *GetOwner()->GetName());
}

void UDialogueComponent::RemoveDialogueTag(FGameplayTag Tag)
{
    DialogueTags.RemoveTag(Tag);
    UE_LOG(LogTemp, Log, TEXT("Removed dialogue tag %s from %s"), *Tag.ToString(), *GetOwner()->GetName());
}