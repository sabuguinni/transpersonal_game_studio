#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    bInDialogue = false;
    CurrentDialogueTreeID = TEXT("");
    CurrentNodeID = TEXT("");
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Story Manager initialized"));
    
    // Initialize story progress
    StoryProgress = FNarr_StoryProgress();
    
    // Load dialogue database
    LoadDialogueDatabase();
    
    // Load saved story progress
    LoadStoryProgress();
}

void UNarr_StoryManager::Deinitialize()
{
    SaveStoryProgress();
    Super::Deinitialize();
}

void UNarr_StoryManager::AdvanceChapter(ENarr_StoryChapter NewChapter)
{
    if (CanAccessChapter(NewChapter))
    {
        StoryProgress.CurrentChapter = NewChapter;
        StoryProgress.ChapterProgress = 0;
        
        UE_LOG(LogTemp, Log, TEXT("Story chapter advanced to: %d"), (int32)NewChapter);
        
        // Trigger chapter-specific events
        switch (NewChapter)
        {
        case ENarr_StoryChapter::FirstHunt:
            TriggerStoryEvent(TEXT("FirstHuntUnlocked"));
            break;
        case ENarr_StoryChapter::TribalContact:
            TriggerStoryEvent(TEXT("TribalContactAvailable"));
            break;
        case ENarr_StoryChapter::TerritoryWars:
            TriggerStoryEvent(TEXT("TerritoryWarsBegin"));
            break;
        case ENarr_StoryChapter::AlphaChallenge:
            TriggerStoryEvent(TEXT("AlphaChallengeUnlocked"));
            break;
        case ENarr_StoryChapter::Exodus:
            TriggerStoryEvent(TEXT("ExodusPathOpened"));
            break;
        }
        
        SaveStoryProgress();
    }
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    if (!HasCompletedEvent(EventID))
    {
        StoryProgress.CompletedEvents.Add(EventID);
        StoryProgress.ChapterProgress++;
        
        UE_LOG(LogTemp, Log, TEXT("Story event triggered: %s"), *EventID);
        
        // Handle special events
        if (EventID == TEXT("TribalFirstContact"))
        {
            StoryProgress.bHasMetTribe = true;
            ModifyRelationship(TEXT("TribalElder"), 10);
        }
        else if (EventID == TEXT("AlphaKilled"))
        {
            StoryProgress.bHasKilledAlpha = true;
            ModifyRelationship(TEXT("TribalElder"), 25);
            ModifyRelationship(TEXT("TribalWarrior"), 15);
        }
        
        SaveStoryProgress();
    }
}

bool UNarr_StoryManager::HasCompletedEvent(const FString& EventID) const
{
    return StoryProgress.CompletedEvents.Contains(EventID);
}

ENarr_StoryChapter UNarr_StoryManager::GetCurrentChapter() const
{
    return StoryProgress.CurrentChapter;
}

void UNarr_StoryManager::ModifyRelationship(const FString& CharacterName, int32 Delta)
{
    int32* CurrentLevel = StoryProgress.CharacterRelationships.Find(CharacterName);
    if (CurrentLevel)
    {
        *CurrentLevel = FMath::Clamp(*CurrentLevel + Delta, -100, 100);
    }
    else
    {
        StoryProgress.CharacterRelationships.Add(CharacterName, FMath::Clamp(Delta, -100, 100));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Relationship with %s modified by %d, new level: %d"), 
           *CharacterName, Delta, GetRelationshipLevel(CharacterName));
}

int32 UNarr_StoryManager::GetRelationshipLevel(const FString& CharacterName) const
{
    const int32* Level = StoryProgress.CharacterRelationships.Find(CharacterName);
    return Level ? *Level : 0;
}

ENarr_DialogueState UNarr_StoryManager::GetDialogueState(const FString& CharacterName) const
{
    return CalculateDialogueState(CharacterName);
}

void UNarr_StoryManager::StartDialogue(const FString& CharacterName, const FString& DialogueTreeID)
{
    CurrentDialogueTreeID = DialogueTreeID;
    CurrentNodeID = DialogueTreeID + TEXT("_Start");
    bInDialogue = true;
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue with %s, tree: %s"), *CharacterName, *DialogueTreeID);
}

FNarr_DialogueNode UNarr_StoryManager::GetCurrentDialogueNode() const
{
    if (bInDialogue && !CurrentNodeID.IsEmpty())
    {
        const FNarr_DialogueNode* Node = DialogueDatabase.Find(CurrentNodeID);
        if (Node)
        {
            return *Node;
        }
    }
    
    return FNarr_DialogueNode();
}

void UNarr_StoryManager::SelectDialogueOption(int32 OptionIndex)
{
    if (!bInDialogue) return;
    
    const FNarr_DialogueNode* CurrentNode = DialogueDatabase.Find(CurrentNodeID);
    if (CurrentNode && CurrentNode->NextNodeIDs.IsValidIndex(OptionIndex))
    {
        CurrentNodeID = CurrentNode->NextNodeIDs[OptionIndex];
        
        // Check if this is an end node
        if (CurrentNodeID.Contains(TEXT("_End")) || CurrentNodeID.IsEmpty())
        {
            EndDialogue();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Selected dialogue option %d, next node: %s"), OptionIndex, *CurrentNodeID);
    }
}

bool UNarr_StoryManager::IsInDialogue() const
{
    return bInDialogue;
}

void UNarr_StoryManager::EndDialogue()
{
    bInDialogue = false;
    CurrentDialogueTreeID = TEXT("");
    CurrentNodeID = TEXT("");
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

int32 UNarr_StoryManager::GetSurvivalDays() const
{
    return StoryProgress.SurvivalDays;
}

void UNarr_StoryManager::IncrementSurvivalDay()
{
    StoryProgress.SurvivalDays++;
    
    // Trigger survival milestone events
    if (StoryProgress.SurvivalDays == 1)
    {
        TriggerStoryEvent(TEXT("FirstDaySurvived"));
    }
    else if (StoryProgress.SurvivalDays == 7)
    {
        TriggerStoryEvent(TEXT("WeekSurvived"));
        if (GetCurrentChapter() == ENarr_StoryChapter::Awakening)
        {
            AdvanceChapter(ENarr_StoryChapter::FirstHunt);
        }
    }
    else if (StoryProgress.SurvivalDays == 30)
    {
        TriggerStoryEvent(TEXT("MonthSurvived"));
    }
    
    SaveStoryProgress();
}

bool UNarr_StoryManager::CanAccessChapter(ENarr_StoryChapter Chapter) const
{
    switch (Chapter)
    {
    case ENarr_StoryChapter::Awakening:
        return true;
    case ENarr_StoryChapter::FirstHunt:
        return StoryProgress.SurvivalDays >= 7;
    case ENarr_StoryChapter::TribalContact:
        return HasCompletedEvent(TEXT("FirstHuntCompleted"));
    case ENarr_StoryChapter::TerritoryWars:
        return StoryProgress.bHasMetTribe;
    case ENarr_StoryChapter::AlphaChallenge:
        return HasCompletedEvent(TEXT("TerritoryEstablished"));
    case ENarr_StoryChapter::Exodus:
        return StoryProgress.bHasKilledAlpha;
    default:
        return false;
    }
}

void UNarr_StoryManager::LoadDialogueDatabase()
{
    // Initialize basic dialogue nodes
    FNarr_DialogueNode ElderGreeting;
    ElderGreeting.SpeakerName = TEXT("Tribal Elder");
    ElderGreeting.DialogueText = TEXT("You have survived the wilderness longer than most. Perhaps you are worthy of our tribe's knowledge.");
    ElderGreeting.RequiredState = ENarr_DialogueState::Neutral;
    ElderGreeting.PlayerResponses.Add(TEXT("I seek only to survive."));
    ElderGreeting.PlayerResponses.Add(TEXT("Teach me your ways."));
    ElderGreeting.PlayerResponses.Add(TEXT("I need nothing from you."));
    ElderGreeting.NextNodeIDs.Add(TEXT("Elder_Respect"));
    ElderGreeting.NextNodeIDs.Add(TEXT("Elder_Teaching"));
    ElderGreeting.NextNodeIDs.Add(TEXT("Elder_Dismissal"));
    DialogueDatabase.Add(TEXT("Elder_Start"), ElderGreeting);
    
    FNarr_DialogueNode ScoutWarning;
    ScoutWarning.SpeakerName = TEXT("Tribal Scout");
    ScoutWarning.DialogueText = TEXT("The hunting grounds to the north are dangerous. A great predator has claimed that territory.");
    ScoutWarning.RequiredState = ENarr_DialogueState::Friendly;
    ScoutWarning.PlayerResponses.Add(TEXT("I can handle myself."));
    ScoutWarning.PlayerResponses.Add(TEXT("Tell me more about this predator."));
    ScoutWarning.NextNodeIDs.Add(TEXT("Scout_Confident"));
    ScoutWarning.NextNodeIDs.Add(TEXT("Scout_Information"));
    DialogueDatabase.Add(TEXT("Scout_Start"), ScoutWarning);
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue database loaded with %d nodes"), DialogueDatabase.Num());
}

void UNarr_StoryManager::SaveStoryProgress()
{
    // In a real implementation, this would save to a file or game save system
    UE_LOG(LogTemp, Log, TEXT("Story progress saved - Chapter: %d, Days: %d"), 
           (int32)StoryProgress.CurrentChapter, StoryProgress.SurvivalDays);
}

void UNarr_StoryManager::LoadStoryProgress()
{
    // In a real implementation, this would load from a saved file
    UE_LOG(LogTemp, Log, TEXT("Story progress loaded"));
}

ENarr_DialogueState UNarr_StoryManager::CalculateDialogueState(const FString& CharacterName) const
{
    int32 RelationshipLevel = GetRelationshipLevel(CharacterName);
    
    if (RelationshipLevel >= 50)
    {
        return ENarr_DialogueState::Respectful;
    }
    else if (RelationshipLevel >= 20)
    {
        return ENarr_DialogueState::Friendly;
    }
    else if (RelationshipLevel <= -20)
    {
        return ENarr_DialogueState::Hostile;
    }
    else if (RelationshipLevel <= -5)
    {
        return ENarr_DialogueState::Cautious;
    }
    else
    {
        return ENarr_DialogueState::Neutral;
    }
}