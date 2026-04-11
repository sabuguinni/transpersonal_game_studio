#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimSequence.h"

UNarrativeManager::UNarrativeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    bDialogueActive = false;
    ConsciousnessGrowthRate = 1.0f;
    
    // Initialize story progress
    StoryProgress.CurrentState = EStoryState::Prologue;
    StoryProgress.ConsciousnessLevel = 0.0f;
}

void UNarrativeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStoryStateDescriptions();
    
    // Load any saved story progress
    LoadStoryProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: System initialized. Current story state: %d"), (int32)StoryProgress.CurrentState);
}

void UNarrativeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update consciousness level gradually based on story progress
    if (StoryProgress.ConsciousnessLevel < 100.0f)
    {
        float GrowthAmount = ConsciousnessGrowthRate * DeltaTime * 0.1f; // Slow growth
        StoryProgress.ConsciousnessLevel = FMath::Clamp(StoryProgress.ConsciousnessLevel + GrowthAmount, 0.0f, 100.0f);
    }
}

void UNarrativeManager::SetStoryState(EStoryState NewState)
{
    if (StoryProgress.CurrentState != NewState)
    {
        EStoryState OldState = StoryProgress.CurrentState;
        StoryProgress.CurrentState = NewState;
        
        // Broadcast the state change
        OnStoryStateChanged.Broadcast(OldState, NewState);
        
        // Update consciousness level based on story progression
        float ConsciousnessBonus = 0.0f;
        switch (NewState)
        {
            case EStoryState::TribalAwakening:
                ConsciousnessBonus = 10.0f;
                AddStoryFlag(TEXT("tribal_awakening_complete"));
                break;
            case EStoryState::FirstVision:
                ConsciousnessBonus = 15.0f;
                AddStoryFlag(TEXT("first_vision_received"));
                break;
            case EStoryState::ShamanTraining:
                ConsciousnessBonus = 20.0f;
                AddStoryFlag(TEXT("shaman_training_started"));
                break;
            case EStoryState::SpiritQuest:
                ConsciousnessBonus = 25.0f;
                AddStoryFlag(TEXT("spirit_quest_active"));
                break;
            case EStoryState::Transformation:
                ConsciousnessBonus = 30.0f;
                AddStoryFlag(TEXT("transformation_begun"));
                break;
            case EStoryState::Ascension:
                ConsciousnessBonus = 40.0f;
                AddStoryFlag(TEXT("ascension_achieved"));
                break;
            default:
                break;
        }
        
        if (ConsciousnessBonus > 0.0f)
        {
            UpdateConsciousnessLevel(StoryProgress.ConsciousnessLevel + ConsciousnessBonus);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story state changed from %d to %d"), (int32)OldState, (int32)NewState);
        
        // Auto-save progress
        SaveStoryProgress();
    }
}

void UNarrativeManager::AddStoryFlag(const FString& FlagName)
{
    if (!StoryProgress.ActiveStoryFlags.Contains(FlagName))
    {
        StoryProgress.ActiveStoryFlags.Add(FlagName);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Added story flag: %s"), *FlagName);
    }
}

void UNarrativeManager::RemoveStoryFlag(const FString& FlagName)
{
    if (StoryProgress.ActiveStoryFlags.Contains(FlagName))
    {
        StoryProgress.ActiveStoryFlags.Remove(FlagName);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Removed story flag: %s"), *FlagName);
    }
}

bool UNarrativeManager::HasStoryFlag(const FString& FlagName) const
{
    return StoryProgress.ActiveStoryFlags.Contains(FlagName);
}

void UNarrativeManager::CompleteQuest(const FString& QuestID)
{
    if (!StoryProgress.CompletedQuests.Contains(QuestID))
    {
        StoryProgress.CompletedQuests.Add(QuestID);
        AddStoryFlag(FString::Printf(TEXT("quest_%s_completed"), *QuestID));
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Quest completed: %s"), *QuestID);
        
        // Auto-save progress
        SaveStoryProgress();
    }
}

bool UNarrativeManager::IsQuestCompleted(const FString& QuestID) const
{
    return StoryProgress.CompletedQuests.Contains(QuestID);
}

void UNarrativeManager::StartDialogue(const FString& DialogueID)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot start dialogue %s - dialogue already active"), *DialogueID);
        return;
    }
    
    FDialogueEntry DialogueEntry = GetDialogueEntry(DialogueID);
    if (DialogueEntry.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Dialogue entry not found: %s"), *DialogueID);
        return;
    }
    
    // Check requirements
    if (!CheckDialogueRequirements(DialogueEntry))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue requirements not met for: %s"), *DialogueID);
        return;
    }
    
    // Start the dialogue
    bDialogueActive = true;
    CurrentDialogue = DialogueEntry;
    
    // Process dialogue effects
    ProcessDialogueEffects(DialogueEntry);
    
    // Broadcast dialogue started
    OnDialogueStarted.Broadcast(DialogueEntry);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Started dialogue: %s by %s"), *DialogueEntry.DialogueID, *DialogueEntry.SpeakerName);
}

void UNarrativeManager::EndDialogue()
{
    if (bDialogueActive)
    {
        bDialogueActive = false;
        CurrentDialogue = FDialogueEntry(); // Reset to default
        
        // Broadcast dialogue ended
        OnDialogueEnded.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dialogue ended"));
    }
}

FDialogueEntry UNarrativeManager::GetDialogueEntry(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: DialogueDataTable is null"));
        return FDialogueEntry();
    }
    
    FDialogueEntry* FoundEntry = DialogueDataTable->FindRow<FDialogueEntry>(FName(*DialogueID), TEXT("GetDialogueEntry"));
    if (FoundEntry)
    {
        return *FoundEntry;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue entry not found in data table: %s"), *DialogueID);
    return FDialogueEntry();
}

void UNarrativeManager::ModifyCharacterRelationship(const FString& CharacterName, int32 RelationshipChange)
{
    int32* CurrentRelationship = StoryProgress.CharacterRelationships.Find(CharacterName);
    if (CurrentRelationship)
    {
        *CurrentRelationship = FMath::Clamp(*CurrentRelationship + RelationshipChange, -100, 100);
    }
    else
    {
        StoryProgress.CharacterRelationships.Add(CharacterName, FMath::Clamp(RelationshipChange, -100, 100));
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Character relationship modified - %s: %d"), *CharacterName, GetCharacterRelationship(CharacterName));
}

int32 UNarrativeManager::GetCharacterRelationship(const FString& CharacterName) const
{
    const int32* Relationship = StoryProgress.CharacterRelationships.Find(CharacterName);
    return Relationship ? *Relationship : 0;
}

void UNarrativeManager::UpdateConsciousnessLevel(float NewLevel)
{
    float OldLevel = StoryProgress.ConsciousnessLevel;
    StoryProgress.ConsciousnessLevel = FMath::Clamp(NewLevel, 0.0f, 100.0f);
    
    // Check for consciousness milestones
    if (OldLevel < 25.0f && StoryProgress.ConsciousnessLevel >= 25.0f)
    {
        AddStoryFlag(TEXT("consciousness_quarter"));
        UnlockAbility(TEXT("spirit_sight"));
    }
    else if (OldLevel < 50.0f && StoryProgress.ConsciousnessLevel >= 50.0f)
    {
        AddStoryFlag(TEXT("consciousness_half"));
        UnlockAbility(TEXT("astral_projection"));
    }
    else if (OldLevel < 75.0f && StoryProgress.ConsciousnessLevel >= 75.0f)
    {
        AddStoryFlag(TEXT("consciousness_three_quarters"));
        UnlockAbility(TEXT("reality_manipulation"));
    }
    else if (OldLevel < 100.0f && StoryProgress.ConsciousnessLevel >= 100.0f)
    {
        AddStoryFlag(TEXT("consciousness_complete"));
        UnlockAbility(TEXT("dimensional_transcendence"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Consciousness level updated to: %f"), StoryProgress.ConsciousnessLevel);
}

void UNarrativeManager::UnlockAbility(const FString& AbilityName)
{
    if (!StoryProgress.UnlockedAbilities.Contains(AbilityName))
    {
        StoryProgress.UnlockedAbilities.Add(AbilityName);
        AddStoryFlag(FString::Printf(TEXT("ability_%s_unlocked"), *AbilityName));
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Ability unlocked: %s"), *AbilityName);
    }
}

void UNarrativeManager::SaveStoryProgress()
{
    // Implementation would save to UE5's save game system
    // For now, just log the action
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Story progress saved"));
}

void UNarrativeManager::LoadStoryProgress()
{
    // Implementation would load from UE5's save game system
    // For now, just log the action
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Story progress loaded"));
}

void UNarrativeManager::InitializeStoryStateDescriptions()
{
    StoryStateDescriptions.Add(EStoryState::Prologue, TEXT("The journey begins in the ancient world"));
    StoryStateDescriptions.Add(EStoryState::TribalAwakening, TEXT("Awakening to the tribal consciousness"));
    StoryStateDescriptions.Add(EStoryState::FirstVision, TEXT("The first glimpse beyond the veil"));
    StoryStateDescriptions.Add(EStoryState::ShamanTraining, TEXT("Learning the ways of the spirit"));
    StoryStateDescriptions.Add(EStoryState::SpiritQuest, TEXT("Journeying into the spirit realm"));
    StoryStateDescriptions.Add(EStoryState::Transformation, TEXT("The great transformation begins"));
    StoryStateDescriptions.Add(EStoryState::Ascension, TEXT("Rising beyond mortal limitations"));
    StoryStateDescriptions.Add(EStoryState::Epilogue, TEXT("The new consciousness emerges"));
}

bool UNarrativeManager::CheckDialogueRequirements(const FDialogueEntry& DialogueEntry) const
{
    // Check if all required story flags are present
    for (const FString& RequiredFlag : DialogueEntry.RequiredStoryFlags)
    {
        if (!HasStoryFlag(RequiredFlag))
        {
            return false;
        }
    }
    
    return true;
}

void UNarrativeManager::ProcessDialogueEffects(const FDialogueEntry& DialogueEntry)
{
    // Apply story flags set by this dialogue
    for (const FString& FlagToSet : DialogueEntry.SetStoryFlags)
    {
        AddStoryFlag(FlagToSet);
    }
    
    // Special dialogue effects based on speaker
    if (DialogueEntry.SpeakerName == TEXT("Aria"))
    {
        ModifyCharacterRelationship(TEXT("Aria"), 5); // Talking to Aria improves relationship
    }
    else if (DialogueEntry.SpeakerName == TEXT("Kael"))
    {
        UpdateConsciousnessLevel(StoryProgress.ConsciousnessLevel + 2.0f); // Shaman dialogue increases consciousness
    }
    else if (DialogueEntry.SpeakerName == TEXT("Zara"))
    {
        ModifyCharacterRelationship(TEXT("Zara"), 3); // Hunter dialogue improves relationship
    }
}