#include "Quest_HuntingQuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UQuest_HuntingQuestManager::UQuest_HuntingQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    TrackingRange = 2000.0f;
    bShowTrackingMarkers = true;
    MaxActiveHunts = 3;
    HuntTimeLimit = 600.0f; // 10 minutes
}

void UQuest_HuntingQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultHunts();
}

void UQuest_HuntingQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bShowTrackingMarkers)
    {
        UpdateTrackingMarkers();
    }
    
    CheckHuntTimeouts();
}

void UQuest_HuntingQuestManager::StartHuntingQuest(const FString& TargetSpecies, int32 RequiredKills, bool bStealth)
{
    if (ActiveHuntTargets.Num() >= MaxActiveHunts)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active hunts reached. Cannot start new hunt for %s"), *TargetSpecies);
        return;
    }

    // Check if hunt already exists
    if (FindHuntTarget(TargetSpecies))
    {
        UE_LOG(LogTemp, Warning, TEXT("Hunt for %s already active"), *TargetSpecies);
        return;
    }

    FQuest_HuntTarget NewHunt;
    NewHunt.TargetSpecies = TargetSpecies;
    NewHunt.RequiredKills = RequiredKills;
    NewHunt.CurrentKills = 0;
    NewHunt.MinDistance = bStealth ? 1000.0f : 500.0f;
    NewHunt.bRequiresStealth = bStealth;

    ActiveHuntTargets.Add(NewHunt);
    
    UE_LOG(LogTemp, Log, TEXT("Started hunting quest for %s - Required: %d, Stealth: %s"), 
           *TargetSpecies, RequiredKills, bStealth ? TEXT("Yes") : TEXT("No"));
}

void UQuest_HuntingQuestManager::RegisterKill(const FString& KilledSpecies, float Distance, bool bWasStealth)
{
    FQuest_HuntTarget* HuntTarget = FindHuntTarget(KilledSpecies);
    if (!HuntTarget)
    {
        return;
    }

    // Check stealth requirement
    if (HuntTarget->bRequiresStealth && !bWasStealth)
    {
        UE_LOG(LogTemp, Warning, TEXT("Kill of %s failed stealth requirement"), *KilledSpecies);
        return;
    }

    // Check distance requirement
    if (Distance < HuntTarget->MinDistance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Kill of %s too close (%.1f < %.1f)"), *KilledSpecies, Distance, HuntTarget->MinDistance);
        return;
    }

    HuntTarget->CurrentKills++;
    UE_LOG(LogTemp, Log, TEXT("Registered kill: %s (%d/%d)"), *KilledSpecies, HuntTarget->CurrentKills, HuntTarget->RequiredKills);

    if (CheckQuestCompletion(KilledSpecies))
    {
        CompleteHuntingQuest(KilledSpecies);
    }
}

bool UQuest_HuntingQuestManager::CheckQuestCompletion(const FString& TargetSpecies)
{
    FQuest_HuntTarget* HuntTarget = FindHuntTarget(TargetSpecies);
    if (!HuntTarget)
    {
        return false;
    }

    return HuntTarget->CurrentKills >= HuntTarget->RequiredKills;
}

void UQuest_HuntingQuestManager::CompleteHuntingQuest(const FString& TargetSpecies)
{
    FQuest_HuntTarget* HuntTarget = FindHuntTarget(TargetSpecies);
    if (!HuntTarget)
    {
        return;
    }

    // Award rewards
    for (const FQuest_HuntingReward& Reward : QuestRewards)
    {
        UE_LOG(LogTemp, Log, TEXT("Awarded: %s x%d, XP: %.1f"), *Reward.ItemName, Reward.Quantity, Reward.ExperiencePoints);
    }

    // Remove completed hunt
    ActiveHuntTargets.RemoveAll([TargetSpecies](const FQuest_HuntTarget& Hunt)
    {
        return Hunt.TargetSpecies == TargetSpecies;
    });

    UE_LOG(LogTemp, Log, TEXT("Completed hunting quest for %s"), *TargetSpecies);
}

TArray<FString> UQuest_HuntingQuestManager::GetActiveHuntTargets()
{
    TArray<FString> Targets;
    for (const FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        Targets.Add(Hunt.TargetSpecies);
    }
    return Targets;
}

float UQuest_HuntingQuestManager::GetHuntProgress(const FString& TargetSpecies)
{
    FQuest_HuntTarget* HuntTarget = FindHuntTarget(TargetSpecies);
    if (!HuntTarget || HuntTarget->RequiredKills == 0)
    {
        return 0.0f;
    }

    return static_cast<float>(HuntTarget->CurrentKills) / static_cast<float>(HuntTarget->RequiredKills);
}

void UQuest_HuntingQuestManager::UpdateTrackingMarkers()
{
    // This would update UI markers showing hunt targets on the map
    // Implementation would depend on UI system
}

void UQuest_HuntingQuestManager::CancelHuntingQuest(const FString& TargetSpecies)
{
    ActiveHuntTargets.RemoveAll([TargetSpecies](const FQuest_HuntTarget& Hunt)
    {
        return Hunt.TargetSpecies == TargetSpecies;
    });

    UE_LOG(LogTemp, Log, TEXT("Cancelled hunting quest for %s"), *TargetSpecies);
}

void UQuest_HuntingQuestManager::InitializeDefaultHunts()
{
    // Setup default reward structure
    FQuest_HuntingReward MeatReward;
    MeatReward.ItemName = TEXT("Raw Meat");
    MeatReward.Quantity = 3;
    MeatReward.ExperiencePoints = 50.0f;

    FQuest_HuntingReward HideReward;
    HideReward.ItemName = TEXT("Animal Hide");
    HideReward.Quantity = 1;
    HideReward.ExperiencePoints = 25.0f;

    QuestRewards.Add(MeatReward);
    QuestRewards.Add(HideReward);
}

void UQuest_HuntingQuestManager::CheckHuntTimeouts()
{
    // Implementation for time-limited hunts
    // Would check elapsed time and cancel expired hunts
}

FQuest_HuntTarget* UQuest_HuntingQuestManager::FindHuntTarget(const FString& TargetSpecies)
{
    for (FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        if (Hunt.TargetSpecies == TargetSpecies)
        {
            return &Hunt;
        }
    }
    return nullptr;
}