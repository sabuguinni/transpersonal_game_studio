// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "NarrativeSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"
#include "../TranspersonalPlayerController.h"
#include "../TranspersonalGameState.h"

UNarrativeSystemManager::UNarrativeSystemManager()
{
    bIsDialoguePlaying = false;
    CurrentDialogueID = TEXT("");
    DialogueStartTime = 0.0f;
    LastLocationCheckTime = 0.0f;
    LastPlayerLocation = FVector::ZeroVector;
    
    // Initialize dialogue voices
    PlayerVoice = nullptr;
    NarratorVoice = nullptr;
}

void UNarrativeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Initializing narrative system..."));
    
    // Set up periodic checks
    if (UWorld* World = GetWorld())
    {
        // Check for memory unlocks every 2 seconds
        World->GetTimerManager().SetTimer(
            MemoryCheckTimerHandle,
            [this]()
            {
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
                {
                    if (APawn* PlayerPawn = PC->GetPawn())
                    {
                        FVector PlayerLocation = PlayerPawn->GetActorLocation();
                        
                        // Get player tags from game state or player controller
                        FGameplayTagContainer PlayerTags;
                        if (ATranspersonalPlayerController* TPC = Cast<ATranspersonalPlayerController>(PC))
                        {
                            // Assume player controller has a method to get current tags
                            // PlayerTags = TPC->GetPlayerTags();
                        }
                        
                        CheckMemoryUnlocks(PlayerLocation, PlayerTags, PC);
                    }
                }
            },
            MEMORY_CHECK_INTERVAL,
            true
        );
        
        // Check for story beats every 0.5 seconds
        World->GetTimerManager().SetTimer(
            StoryBeatCheckTimerHandle,
            [this]()
            {
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
                {
                    if (APawn* PlayerPawn = PC->GetPawn())
                    {
                        FVector PlayerLocation = PlayerPawn->GetActorLocation();
                        CheckLocationBasedStoryBeats(PlayerLocation, PC);
                    }
                }
            },
            STORY_BEAT_CHECK_INTERVAL,
            true
        );
    }
    
    // Load default narrative content
    LoadDefaultStoryBeats();
    LoadDefaultMemories();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Initialization complete"));
}

void UNarrativeSystemManager::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MemoryCheckTimerHandle);
        World->GetTimerManager().ClearTimer(StoryBeatCheckTimerHandle);
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    // Save narrative state before shutdown
    SaveNarrativeState();
    
    Super::Deinitialize();
}

void UNarrativeSystemManager::RegisterStoryBeat(const FStoryBeat& StoryBeat)
{
    if (StoryBeat.StoryBeatID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Cannot register story beat with empty ID"));
        return;
    }
    
    StoryBeats.Add(StoryBeat.StoryBeatID, StoryBeat);
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Registered story beat '%s'"), *StoryBeat.StoryBeatID);
}

bool UNarrativeSystemManager::TriggerStoryBeat(const FString& StoryBeatID, APlayerController* PlayerController)
{
    if (!StoryBeats.Contains(StoryBeatID))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Story beat '%s' not found"), *StoryBeatID);
        return false;
    }
    
    FStoryBeat& StoryBeat = StoryBeats[StoryBeatID];
    
    // Check if already completed and can't repeat
    if (StoryBeat.bHasTriggered && !StoryBeat.bCanRepeat)
    {
        return false;
    }
    
    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (StoryBeat.bHasTriggered && (CurrentTime - StoryBeat.LastTriggerTime) < StoryBeat.CooldownTime)
    {
        return false;
    }
    
    // Check conditions
    if (!CheckStoryBeatConditions(StoryBeat, PlayerController))
    {
        return false;
    }
    
    // Trigger the story beat
    StoryBeat.bHasTriggered = true;
    StoryBeat.LastTriggerTime = CurrentTime;
    StoryBeat.TriggerCount++;
    
    // Add to completed list if not repeatable
    if (!StoryBeat.bCanRepeat)
    {
        CompletedStoryBeats.AddUnique(StoryBeatID);
    }
    
    // Apply effects
    ApplyStoryBeatEffects(StoryBeat, PlayerController);
    
    // Play dialogue if available
    if (StoryBeat.DialogueLines.Num() > 0)
    {
        FDialogueLine DialogueLine;
        DialogueLine.DialogueText = FText::FromString(StoryBeat.DialogueLines[0]);
        DialogueLine.Context = EDialogueContext::Discovery; // Default context
        DialogueLine.bIsThought = true;
        
        PlayDialogueLine(DialogueLine, PlayerController);
    }
    
    // Broadcast event
    OnStoryBeatTriggered.Broadcast(StoryBeatID, StoryBeat.StoryTags);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Triggered story beat '%s'"), *StoryBeatID);
    return true;
}

void UNarrativeSystemManager::CheckLocationBasedStoryBeats(const FVector& PlayerLocation, APlayerController* PlayerController)
{
    for (auto& StoryBeatPair : StoryBeats)
    {
        FStoryBeat& StoryBeat = StoryBeatPair.Value;
        
        // Skip if already triggered and can't repeat
        if (StoryBeat.bHasTriggered && !StoryBeat.bCanRepeat)
        {
            continue;
        }
        
        // Check if player is within trigger radius
        float Distance = FVector::Dist(PlayerLocation, StoryBeat.TriggerLocation);
        if (Distance <= StoryBeat.TriggerRadius)
        {
            TriggerStoryBeat(StoryBeat.StoryBeatID, PlayerController);
        }
    }
}

TArray<FStoryBeat> UNarrativeSystemManager::GetActiveStoryBeats() const
{
    TArray<FStoryBeat> ActiveBeats;
    
    for (const auto& StoryBeatPair : StoryBeats)
    {
        const FStoryBeat& StoryBeat = StoryBeatPair.Value;
        
        // Include if not triggered or can repeat
        if (!StoryBeat.bHasTriggered || StoryBeat.bCanRepeat)
        {
            ActiveBeats.Add(StoryBeat);
        }
    }
    
    return ActiveBeats;
}

FStoryBeat UNarrativeSystemManager::GetStoryBeat(const FString& StoryBeatID) const
{
    if (StoryBeats.Contains(StoryBeatID))
    {
        return StoryBeats[StoryBeatID];
    }
    
    return FStoryBeat(); // Return empty story beat if not found
}

void UNarrativeSystemManager::RegisterMemory(const FMemoryDefinition& Memory)
{
    if (Memory.MemoryID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Cannot register memory with empty ID"));
        return;
    }
    
    Memories.Add(Memory.MemoryID, Memory);
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Registered memory '%s'"), *Memory.MemoryID);
}

bool UNarrativeSystemManager::UnlockMemory(const FString& MemoryID, APlayerController* PlayerController)
{
    if (!Memories.Contains(MemoryID))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Memory '%s' not found"), *MemoryID);
        return false;
    }
    
    FMemoryDefinition& Memory = Memories[MemoryID];
    
    // Check if already unlocked
    if (Memory.bIsUnlocked)
    {
        return false;
    }
    
    // Check unlock conditions
    FGameplayTagContainer PlayerTags; // Would get from player controller
    if (!CheckMemoryUnlockConditions(Memory, PlayerTags))
    {
        return false;
    }
    
    // Unlock the memory
    Memory.bIsUnlocked = true;
    Memory.UnlockTime = GetWorld()->GetTimeSeconds();
    UnlockedMemories.AddUnique(MemoryID);
    
    // Apply effects
    ApplyMemoryEffects(Memory, PlayerController);
    
    // Play memory narration if available
    if (!Memory.MemoryNarration.IsEmpty())
    {
        FDialogueLine MemoryLine;
        MemoryLine.DialogueText = Memory.MemoryNarration;
        MemoryLine.Context = EDialogueContext::Memory;
        MemoryLine.bIsThought = true;
        MemoryLine.LineDuration = 5.0f; // Longer for memories
        
        PlayDialogueLine(MemoryLine, PlayerController);
    }
    
    // Broadcast event
    OnMemoryUnlocked.Broadcast(MemoryID, Memory.MemoryDescription);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Unlocked memory '%s'"), *MemoryID);
    return true;
}

void UNarrativeSystemManager::CheckMemoryUnlocks(const FVector& PlayerLocation, const FGameplayTagContainer& PlayerTags, APlayerController* PlayerController)
{
    for (auto& MemoryPair : Memories)
    {
        FMemoryDefinition& Memory = MemoryPair.Value;
        
        // Skip if already unlocked
        if (Memory.bIsUnlocked)
        {
            continue;
        }
        
        // Check location-based unlock
        if (Memory.UnlockLocation != FVector::ZeroVector)
        {
            float Distance = FVector::Dist(PlayerLocation, Memory.UnlockLocation);
            if (Distance <= Memory.UnlockRadius)
            {
                UnlockMemory(Memory.MemoryID, PlayerController);
            }
        }
    }
}

TArray<FMemoryDefinition> UNarrativeSystemManager::GetUnlockedMemories() const
{
    TArray<FMemoryDefinition> UnlockedMemoryList;
    
    for (const auto& MemoryPair : Memories)
    {
        const FMemoryDefinition& Memory = MemoryPair.Value;
        if (Memory.bIsUnlocked)
        {
            UnlockedMemoryList.Add(Memory);
        }
    }
    
    return UnlockedMemoryList;
}

FMemoryDefinition UNarrativeSystemManager::GetMemory(const FString& MemoryID) const
{
    if (Memories.Contains(MemoryID))
    {
        return Memories[MemoryID];
    }
    
    return FMemoryDefinition(); // Return empty memory if not found
}

void UNarrativeSystemManager::PlayDialogueLine(const FDialogueLine& DialogueLine, APlayerController* PlayerController)
{
    if (bIsDialoguePlaying)
    {
        StopCurrentDialogue();
    }
    
    bIsDialoguePlaying = true;
    DialogueStartTime = GetWorld()->GetTimeSeconds();
    CurrentDialogueID = FString::Printf(TEXT("Line_%f"), DialogueStartTime);
    
    // Broadcast dialogue start event
    OnDialogueStarted.Broadcast(CurrentDialogueID, DialogueLine.Speaker, DialogueLine.DialogueText);
    
    // Play voice line if available
    if (DialogueLine.VoiceLine && PlayerController)
    {
        if (APawn* PlayerPawn = PlayerController->GetPawn())
        {
            UGameplayStatics::PlayDialogueAtLocation(
                GetWorld(),
                DialogueLine.VoiceLine,
                DialogueLine.Speaker,
                PlayerVoice, // Target voice (player listening)
                PlayerPawn->GetActorLocation()
            );
        }
    }
    
    // Set timer to end dialogue
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            [this]()
            {
                StopCurrentDialogue();
            },
            DialogueLine.LineDuration,
            false
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Playing dialogue line: %s"), *DialogueLine.DialogueText.ToString());
}

void UNarrativeSystemManager::PlayInternalMonologue(const FText& MonologueText, EDialogueContext Context, APlayerController* PlayerController)
{
    FDialogueLine MonologueLine;
    MonologueLine.DialogueText = MonologueText;
    MonologueLine.Context = Context;
    MonologueLine.bIsThought = true;
    MonologueLine.Speaker = PlayerVoice;
    MonologueLine.LineDuration = FMath::Max(3.0f, MonologueText.ToString().Len() * 0.1f); // Estimate based on text length
    
    PlayDialogueLine(MonologueLine, PlayerController);
}

void UNarrativeSystemManager::StopCurrentDialogue()
{
    if (!bIsDialoguePlaying)
    {
        return;
    }
    
    bIsDialoguePlaying = false;
    
    // Stop any playing audio
    UGameplayStatics::StopDialogueAtLocation(GetWorld(), FVector::ZeroVector);
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    // Broadcast end event
    OnDialogueEnded.Broadcast(CurrentDialogueID);
    
    CurrentDialogueID = TEXT("");
    DialogueStartTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Stopped current dialogue"));
}

bool UNarrativeSystemManager::IsDialoguePlaying() const
{
    return bIsDialoguePlaying;
}

void UNarrativeSystemManager::SetNarrativeTag(const FGameplayTag& Tag, bool bAdd)
{
    if (bAdd)
    {
        CurrentNarrativeTags.AddTag(Tag);
    }
    else
    {
        CurrentNarrativeTags.RemoveTag(Tag);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: %s narrative tag '%s'"), 
           bAdd ? TEXT("Added") : TEXT("Removed"), *Tag.ToString());
}

bool UNarrativeSystemManager::HasNarrativeTag(const FGameplayTag& Tag) const
{
    return CurrentNarrativeTags.HasTag(Tag);
}

FGameplayTagContainer UNarrativeSystemManager::GetNarrativeTags() const
{
    return CurrentNarrativeTags;
}

void UNarrativeSystemManager::ClearNarrativeTags()
{
    CurrentNarrativeTags.Reset();
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Cleared all narrative tags"));
}

void UNarrativeSystemManager::TriggerEnvironmentalNarrative(const FVector& Location, const FString& NarrativeID)
{
    if (EnvironmentalStoryPoints.Contains(Location))
    {
        FString StoryText = EnvironmentalStoryPoints[Location];
        
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PlayInternalMonologue(FText::FromString(StoryText), EDialogueContext::Environmental, PC);
        }
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Triggered environmental narrative at location"));
    }
}

void UNarrativeSystemManager::RegisterEnvironmentalStoryPoint(const FVector& Location, const FString& StoryText, float TriggerRadius)
{
    EnvironmentalStoryPoints.Add(Location, StoryText);
    EnvironmentalTriggerRadii.Add(Location, TriggerRadius);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Registered environmental story point"));
}

void UNarrativeSystemManager::SaveNarrativeState()
{
    // Implementation would save to a save game object
    // For now, just log the action
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Saving narrative state - %d story beats completed, %d memories unlocked"),
           CompletedStoryBeats.Num(), UnlockedMemories.Num());
}

void UNarrativeSystemManager::LoadNarrativeState()
{
    // Implementation would load from a save game object
    // For now, just log the action
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Loading narrative state"));
}

void UNarrativeSystemManager::DebugPrintNarrativeState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== NARRATIVE SYSTEM DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Story Beats: %d registered, %d completed"), StoryBeats.Num(), CompletedStoryBeats.Num());
    UE_LOG(LogTemp, Warning, TEXT("Memories: %d registered, %d unlocked"), Memories.Num(), UnlockedMemories.Num());
    UE_LOG(LogTemp, Warning, TEXT("Narrative Tags: %d active"), CurrentNarrativeTags.Num());
    UE_LOG(LogTemp, Warning, TEXT("Dialogue Playing: %s"), bIsDialoguePlaying ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Environmental Points: %d"), EnvironmentalStoryPoints.Num());
}

void UNarrativeSystemManager::ForceUnlockAllMemories()
{
    for (auto& MemoryPair : Memories)
    {
        FMemoryDefinition& Memory = MemoryPair.Value;
        if (!Memory.bIsUnlocked)
        {
            Memory.bIsUnlocked = true;
            Memory.UnlockTime = GetWorld()->GetTimeSeconds();
            UnlockedMemories.AddUnique(Memory.MemoryID);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Force unlocked all memories for debug"));
}

void UNarrativeSystemManager::ResetNarrativeProgress()
{
    // Reset story beats
    for (auto& StoryBeatPair : StoryBeats)
    {
        FStoryBeat& StoryBeat = StoryBeatPair.Value;
        StoryBeat.bHasTriggered = false;
        StoryBeat.LastTriggerTime = 0.0f;
        StoryBeat.TriggerCount = 0;
    }
    
    // Reset memories
    for (auto& MemoryPair : Memories)
    {
        FMemoryDefinition& Memory = MemoryPair.Value;
        Memory.bIsUnlocked = false;
        Memory.UnlockTime = 0.0f;
    }
    
    // Clear progress arrays
    CompletedStoryBeats.Empty();
    UnlockedMemories.Empty();
    CurrentNarrativeTags.Reset();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeSystemManager: Reset all narrative progress"));
}

bool UNarrativeSystemManager::CheckStoryBeatConditions(const FStoryBeat& StoryBeat, APlayerController* PlayerController) const
{
    // Check required player tags
    if (StoryBeat.RequiredPlayerTags.Num() > 0)
    {
        // Would check against player's current tags
        // For now, assume conditions are met
    }
    
    // Check required world state
    if (StoryBeat.RequiredWorldState.Num() > 0)
    {
        // Would check against world state tags
        // For now, assume conditions are met
    }
    
    // Check required completed quests
    if (StoryBeat.RequiredCompletedQuests.Num() > 0)
    {
        // Would check against quest system
        // For now, assume conditions are met
    }
    
    return true; // All conditions met
}

bool UNarrativeSystemManager::CheckMemoryUnlockConditions(const FMemoryDefinition& Memory, const FGameplayTagContainer& PlayerTags) const
{
    // Check required tags
    if (Memory.RequiredTags.Num() > 0)
    {
        if (!PlayerTags.HasAll(Memory.RequiredTags))
        {
            return false;
        }
    }
    
    // Check required story beats
    if (Memory.RequiredStoryBeats.Num() > 0)
    {
        for (const FString& RequiredBeat : Memory.RequiredStoryBeats)
        {
            if (!CompletedStoryBeats.Contains(RequiredBeat))
            {
                return false;
            }
        }
    }
    
    return true; // All conditions met
}

void UNarrativeSystemManager::ApplyStoryBeatEffects(const FStoryBeat& StoryBeat, APlayerController* PlayerController)
{
    // Add tags
    for (const FGameplayTag& Tag : StoryBeat.TagsToAdd)
    {
        SetNarrativeTag(Tag, true);
    }
    
    // Remove tags
    for (const FGameplayTag& Tag : StoryBeat.TagsToRemove)
    {
        SetNarrativeTag(Tag, false);
    }
    
    // Activate quests (would interface with quest system)
    for (const FString& QuestID : StoryBeat.QuestsToActivate)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Would activate quest '%s'"), *QuestID);
    }
    
    // Unlock memories
    for (const FString& MemoryID : StoryBeat.MemoriesToUnlock)
    {
        UnlockMemory(MemoryID, PlayerController);
    }
}

void UNarrativeSystemManager::ApplyMemoryEffects(const FMemoryDefinition& Memory, APlayerController* PlayerController)
{
    // Grant tags
    for (const FGameplayTag& Tag : Memory.TagsGranted)
    {
        SetNarrativeTag(Tag, true);
    }
    
    // Unlock quests (would interface with quest system)
    for (const FString& QuestID : Memory.QuestsUnlocked)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Would unlock quest '%s'"), *QuestID);
    }
    
    // Grant experience (would interface with progression system)
    if (Memory.ExperienceGranted > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Would grant %d experience"), Memory.ExperienceGranted);
    }
}

void UNarrativeSystemManager::LoadDefaultStoryBeats()
{
    // Load some default story beats for the game
    
    // First discovery story beat
    FStoryBeat FirstDiscovery;
    FirstDiscovery.StoryBeatID = TEXT("first_discovery");
    FirstDiscovery.StoryBeatTitle = FText::FromString(TEXT("First Discovery"));
    FirstDiscovery.StoryBeatDescription = FText::FromString(TEXT("The player's first moment of realization about where they are"));
    FirstDiscovery.BeatType = EStoryBeatType::Discovery;
    FirstDiscovery.TriggerLocation = FVector(0, 0, 100); // Near spawn point
    FirstDiscovery.TriggerRadius = 1000.0f;
    FirstDiscovery.DialogueLines.Add(TEXT("Where... where am I? This place... it's like nothing I've ever seen."));
    FirstDiscovery.bCanRepeat = false;
    
    RegisterStoryBeat(FirstDiscovery);
    
    // First dinosaur sighting
    FStoryBeat FirstDinosaur;
    FirstDinosaur.StoryBeatID = TEXT("first_dinosaur");
    FirstDinosaur.StoryBeatTitle = FText::FromString(TEXT("First Dinosaur Encounter"));
    FirstDinosaur.StoryBeatDescription = FText::FromString(TEXT("The player sees their first living dinosaur"));
    FirstDinosaur.BeatType = EStoryBeatType::Discovery;
    FirstDinosaur.DialogueLines.Add(TEXT("My God... that's... that's actually a dinosaur. A living, breathing dinosaur."));
    FirstDinosaur.bCanRepeat = false;
    
    RegisterStoryBeat(FirstDinosaur);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Loaded default story beats"));
}

void UNarrativeSystemManager::LoadDefaultMemories()
{
    // Load some default memories for the player character
    
    // Memory of the discovery
    FMemoryDefinition DiscoveryMemory;
    DiscoveryMemory.MemoryID = TEXT("the_discovery");
    DiscoveryMemory.MemoryTitle = FText::FromString(TEXT("The Discovery"));
    DiscoveryMemory.MemoryDescription = FText::FromString(TEXT("How I found the mysterious gem"));
    DiscoveryMemory.MemoryNarration = FText::FromString(TEXT("I was just exploring the forest, following what I thought was a fossil formation. Then I saw it - a strange, glowing gem buried in the earth. The moment I touched it..."));
    DiscoveryMemory.MemoryType = EMemoryType::TheDiscovery;
    DiscoveryMemory.UnlockLocation = FVector(0, 0, 100);
    DiscoveryMemory.UnlockRadius = 500.0f;
    
    RegisterMemory(DiscoveryMemory);
    
    // Memory of family
    FMemoryDefinition FamilyMemory;
    FamilyMemory.MemoryID = TEXT("family_home");
    FamilyMemory.MemoryTitle = FText::FromString(TEXT("Home"));
    FamilyMemory.MemoryDescription = FText::FromString(TEXT("Memories of family and the life I left behind"));
    FamilyMemory.MemoryNarration = FText::FromString(TEXT("Sarah must be wondering where I am by now. I was supposed to be home for dinner. She's probably called the university, maybe even the police..."));
    FamilyMemory.MemoryType = EMemoryType::PersonalLife;
    
    RegisterMemory(FamilyMemory);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeSystemManager: Loaded default memories"));
}