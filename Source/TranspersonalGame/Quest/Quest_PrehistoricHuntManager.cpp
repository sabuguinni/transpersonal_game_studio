#include "Quest_PrehistoricHuntManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_PrehistoricHuntManager::UQuest_PrehistoricHuntManager()
{
    CompletedHunts = 0;
    TotalHuntTime = 0.0f;
}

void UQuest_PrehistoricHuntManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Initialized"));
    
    ActiveHunts.Empty();
    CompletedHunts = 0;
    TotalHuntTime = 0.0f;
}

void UQuest_PrehistoricHuntManager::Deinitialize()
{
    ActiveHunts.Empty();
    Super::Deinitialize();
}

void UQuest_PrehistoricHuntManager::StartHuntQuest(const FString& QuestID, const FQuest_HuntTarget& HuntTarget)
{
    if (QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Cannot start hunt with empty QuestID"));
        return;
    }

    // Check if hunt already active
    for (const FQuest_ActiveHunt& ExistingHunt : ActiveHunts)
    {
        if (ExistingHunt.QuestID == QuestID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Hunt %s already active"), *QuestID);
            return;
        }
    }

    FQuest_ActiveHunt NewHunt;
    NewHunt.QuestID = QuestID;
    NewHunt.HuntTarget = HuntTarget;
    NewHunt.CurrentKills = 0;
    NewHunt.bIsCompleted = false;
    NewHunt.TimeRemaining = 600.0f; // 10 minutes default

    ActiveHunts.Add(NewHunt);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Started hunt quest %s for %d %s"), 
           *QuestID, 
           HuntTarget.RequiredCount,
           *UEnum::GetValueAsString(HuntTarget.TargetSpecies));
    
    BroadcastHuntEvent(TEXT("HuntStarted"), QuestID);
}

void UQuest_PrehistoricHuntManager::RegisterDinosaurKill(EDinosaurSpecies Species, const FVector& KillLocation)
{
    bool bFoundMatchingHunt = false;
    
    for (FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (Hunt.bIsCompleted || Hunt.HuntTarget.TargetSpecies != Species)
        {
            continue;
        }
        
        Hunt.CurrentKills++;
        bFoundMatchingHunt = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Registered kill for quest %s (%d/%d)"), 
               *Hunt.QuestID, 
               Hunt.CurrentKills, 
               Hunt.HuntTarget.RequiredCount);
        
        if (Hunt.CurrentKills >= Hunt.HuntTarget.RequiredCount)
        {
            ProcessHuntCompletion(Hunt);
        }
        
        BroadcastHuntEvent(TEXT("DinosaurKilled"), Hunt.QuestID);
    }
    
    if (!bFoundMatchingHunt)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_PrehistoricHuntManager: No active hunt found for species %s"), 
               *UEnum::GetValueAsString(Species));
    }
}

bool UQuest_PrehistoricHuntManager::IsHuntActive(const FString& QuestID) const
{
    for (const FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (Hunt.QuestID == QuestID && !Hunt.bIsCompleted)
        {
            return true;
        }
    }
    return false;
}

FQuest_ActiveHunt UQuest_PrehistoricHuntManager::GetActiveHunt(const FString& QuestID) const
{
    for (const FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (Hunt.QuestID == QuestID)
        {
            return Hunt;
        }
    }
    return FQuest_ActiveHunt();
}

TArray<FQuest_ActiveHunt> UQuest_PrehistoricHuntManager::GetAllActiveHunts() const
{
    TArray<FQuest_ActiveHunt> ActiveOnly;
    for (const FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (!Hunt.bIsCompleted)
        {
            ActiveOnly.Add(Hunt);
        }
    }
    return ActiveOnly;
}

void UQuest_PrehistoricHuntManager::CompleteHunt(const FString& QuestID)
{
    for (FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (Hunt.QuestID == QuestID && !Hunt.bIsCompleted)
        {
            ProcessHuntCompletion(Hunt);
            break;
        }
    }
}

void UQuest_PrehistoricHuntManager::UpdateHuntTimer(float DeltaTime)
{
    TotalHuntTime += DeltaTime;
    
    for (FQuest_ActiveHunt& Hunt : ActiveHunts)
    {
        if (!Hunt.bIsCompleted && Hunt.TimeRemaining > 0.0f)
        {
            Hunt.TimeRemaining -= DeltaTime;
            
            if (Hunt.TimeRemaining <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Hunt %s timed out"), *Hunt.QuestID);
                Hunt.bIsCompleted = true;
                BroadcastHuntEvent(TEXT("HuntTimedOut"), Hunt.QuestID);
            }
        }
    }
}

FQuest_HuntTarget UQuest_PrehistoricHuntManager::CreateTRexHunt()
{
    FQuest_HuntTarget TRexHunt;
    TRexHunt.TargetSpecies = EDinosaurSpecies::TRex;
    TRexHunt.RequiredCount = 1;
    TRexHunt.RewardMeat = 100.0f;
    TRexHunt.RewardHide = 50.0f;
    TRexHunt.HuntDescription = TEXT("Hunt the apex predator - the mighty T-Rex. Extreme danger, massive rewards.");
    return TRexHunt;
}

FQuest_HuntTarget UQuest_PrehistoricHuntManager::CreateRaptorPackHunt()
{
    FQuest_HuntTarget RaptorHunt;
    RaptorHunt.TargetSpecies = EDinosaurSpecies::Raptor;
    RaptorHunt.RequiredCount = 3;
    RaptorHunt.RewardMeat = 75.0f;
    RaptorHunt.RewardHide = 30.0f;
    RaptorHunt.HuntDescription = TEXT("Hunt a pack of clever raptors. They hunt in groups - be prepared for coordinated attacks.");
    return RaptorHunt;
}

FQuest_HuntTarget UQuest_PrehistoricHuntManager::CreateTriceratopsHunt()
{
    FQuest_HuntTarget TriceratopsHunt;
    TriceratopsHunt.TargetSpecies = EDinosaurSpecies::Triceratops;
    TriceratopsHunt.RequiredCount = 1;
    TriceratopsHunt.RewardMeat = 80.0f;
    TriceratopsHunt.RewardHide = 40.0f;
    TriceratopsHunt.HuntDescription = TEXT("Hunt the armored herbivore. Thick hide and dangerous horns make this a challenging target.");
    return TriceratopsHunt;
}

void UQuest_PrehistoricHuntManager::ProcessHuntCompletion(FQuest_ActiveHunt& Hunt)
{
    Hunt.bIsCompleted = true;
    CompletedHunts++;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Hunt %s completed! Giving rewards..."), *Hunt.QuestID);
    
    GiveHuntRewards(Hunt.HuntTarget);
    BroadcastHuntEvent(TEXT("HuntCompleted"), Hunt.QuestID);
}

void UQuest_PrehistoricHuntManager::GiveHuntRewards(const FQuest_HuntTarget& Target)
{
    // In a full implementation, this would interface with inventory/resource system
    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricHuntManager: Rewarding player with %.1f meat and %.1f hide"), 
           Target.RewardMeat, Target.RewardHide);
    
    // TODO: Interface with TranspersonalCharacter inventory system
    // TODO: Add experience points based on hunt difficulty
    // TODO: Unlock new hunt types based on completed hunts
}

void UQuest_PrehistoricHuntManager::BroadcastHuntEvent(const FString& EventType, const FString& QuestID)
{
    // In a full implementation, this would broadcast to UI and other systems
    UE_LOG(LogTemp, Log, TEXT("Quest_PrehistoricHuntManager: Broadcasting event %s for quest %s"), 
           *EventType, *QuestID);
    
    // TODO: Broadcast to UI system for quest updates
    // TODO: Trigger audio cues for hunt events
    // TODO: Update quest journal/log system
}