#include "Quest_SurvivalMissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"

AQuest_SurvivalMissionManager::AQuest_SurvivalMissionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    MaxActiveMissions = 3;
    MissionGenerationInterval = 120.0f; // 2 minutes
    MissionGenerationRadius = 5000.0f; // 5km radius
    bAutoGenerateMissions = true;
    LastMissionGenerationTime = 0.0f;
    PlayerCharacter = nullptr;
    
    // Set allowed mission types
    AllowedMissionTypes.Add(EQuest_SurvivalMissionType::Hunt_DinosaurPack);
    AllowedMissionTypes.Add(EQuest_SurvivalMissionType::Gather_Resources);
    AllowedMissionTypes.Add(EQuest_SurvivalMissionType::Explore_Territory);
    AllowedMissionTypes.Add(EQuest_SurvivalMissionType::Defend_Camp);
    AllowedMissionTypes.Add(EQuest_SurvivalMissionType::Find_Water);
}

void AQuest_SurvivalMissionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character
    PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    // Generate initial missions
    if (bAutoGenerateMissions)
    {
        GenerateRandomMissions();
    }
}

void AQuest_SurvivalMissionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update active missions
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        
        if (Mission.Status == EQuest_MissionStatus::Active)
        {
            Mission.ElapsedTime += DeltaTime;
            
            // Check for timeout
            if (Mission.TimeLimit > 0.0f && Mission.ElapsedTime >= Mission.TimeLimit)
            {
                FailMission(i, TEXT("Time limit exceeded"));
                continue;
            }
            
            // Check objective completion for location-based objectives
            if (PlayerCharacter)
            {
                FVector PlayerLocation = PlayerCharacter->GetActorLocation();
                
                for (int32 j = 0; j < Mission.Objectives.Num(); j++)
                {
                    if (!Mission.Objectives[j].bIsCompleted)
                    {
                        CheckObjectiveCompletion(i, j, PlayerLocation);
                    }
                }
            }
        }
    }
    
    // Generate new missions periodically
    if (bAutoGenerateMissions && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastMissionGenerationTime >= MissionGenerationInterval)
        {
            GenerateRandomMissions();
            LastMissionGenerationTime = CurrentTime;
        }
    }
    
    // Cleanup expired missions
    CleanupExpiredMissions();
}

void AQuest_SurvivalMissionManager::StartMission(const FQuest_SurvivalMission& Mission)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission: Maximum active missions reached"));
        return;
    }
    
    FQuest_SurvivalMission NewMission = Mission;
    NewMission.Status = EQuest_MissionStatus::Active;
    NewMission.ElapsedTime = 0.0f;
    
    ActiveMissions.Add(NewMission);
    
    NotifyPlayerOfMissionUpdate(FString::Printf(TEXT("New mission started: %s"), *NewMission.MissionName));
}

void AQuest_SurvivalMissionManager::CompleteMission(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return;
    }
    
    FQuest_SurvivalMission& Mission = ActiveMissions[MissionIndex];
    Mission.Status = EQuest_MissionStatus::Completed;
    
    // Move to completed missions
    CompletedMissions.Add(Mission);
    ActiveMissions.RemoveAt(MissionIndex);
    
    NotifyPlayerOfMissionUpdate(FString::Printf(TEXT("Mission completed: %s"), *Mission.MissionName));
}

void AQuest_SurvivalMissionManager::FailMission(int32 MissionIndex, const FString& Reason)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return;
    }
    
    FQuest_SurvivalMission& Mission = ActiveMissions[MissionIndex];
    Mission.Status = EQuest_MissionStatus::Failed;
    
    ActiveMissions.RemoveAt(MissionIndex);
    
    NotifyPlayerOfMissionUpdate(FString::Printf(TEXT("Mission failed: %s - %s"), *Mission.MissionName, *Reason));
}

void AQuest_SurvivalMissionManager::AbandonMission(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return;
    }
    
    FQuest_SurvivalMission& Mission = ActiveMissions[MissionIndex];
    Mission.Status = EQuest_MissionStatus::Abandoned;
    
    ActiveMissions.RemoveAt(MissionIndex);
    
    NotifyPlayerOfMissionUpdate(FString::Printf(TEXT("Mission abandoned: %s"), *Mission.MissionName));
}

void AQuest_SurvivalMissionManager::UpdateObjectiveProgress(int32 MissionIndex, int32 ObjectiveIndex, int32 Progress)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return;
    }
    
    FQuest_SurvivalMission& Mission = ActiveMissions[MissionIndex];
    if (!Mission.Objectives.IsValidIndex(ObjectiveIndex))
    {
        return;
    }
    
    FQuest_SurvivalObjective& Objective = Mission.Objectives[ObjectiveIndex];
    Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.TargetCount);
    
    if (Objective.CurrentCount >= Objective.TargetCount)
    {
        Objective.bIsCompleted = true;
        
        // Check if all objectives are completed
        bool bAllCompleted = true;
        for (const FQuest_SurvivalObjective& Obj : Mission.Objectives)
        {
            if (!Obj.bIsCompleted)
            {
                bAllCompleted = false;
                break;
            }
        }
        
        if (bAllCompleted)
        {
            CompleteMission(MissionIndex);
        }
    }
}

bool AQuest_SurvivalMissionManager::CheckObjectiveCompletion(int32 MissionIndex, int32 ObjectiveIndex, const FVector& PlayerLocation)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return false;
    }
    
    FQuest_SurvivalMission& Mission = ActiveMissions[MissionIndex];
    if (!Mission.Objectives.IsValidIndex(ObjectiveIndex))
    {
        return false;
    }
    
    FQuest_SurvivalObjective& Objective = Mission.Objectives[ObjectiveIndex];
    
    // Check location-based completion
    if (Objective.TargetLocation != FVector::ZeroVector)
    {
        float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
        if (Distance <= Objective.CompletionRadius)
        {
            Objective.bIsCompleted = true;
            return true;
        }
    }
    
    return false;
}

FQuest_SurvivalMission AQuest_SurvivalMissionManager::GenerateHuntMission(const FVector& Location)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Hunt the Pack");
    Mission.MissionDescription = TEXT("Track and hunt a pack of dangerous dinosaurs threatening the area");
    Mission.MissionType = EQuest_SurvivalMissionType::Hunt_DinosaurPack;
    Mission.MissionLocation = Location;
    Mission.TimeLimit = 900.0f; // 15 minutes
    Mission.Priority = 3;
    Mission.ExperienceReward = 250;
    
    // Add objectives
    FQuest_SurvivalObjective Objective1;
    Objective1.Description = TEXT("Track dinosaur pack to their territory");
    Objective1.TargetLocation = GetRandomLocationNearPlayer(1000.0f, 2000.0f);
    Objective1.CompletionRadius = 300.0f;
    Mission.Objectives.Add(Objective1);
    
    FQuest_SurvivalObjective Objective2;
    Objective2.Description = TEXT("Eliminate 3 pack members");
    Objective2.TargetCount = 3;
    Mission.Objectives.Add(Objective2);
    
    Mission.RewardItems.Add(TEXT("Dinosaur Meat"));
    Mission.RewardItems.Add(TEXT("Dinosaur Hide"));
    Mission.RequiredItems.Add(TEXT("Spear"));
    
    return Mission;
}

FQuest_SurvivalMission AQuest_SurvivalMissionManager::GenerateGatherMission(const FVector& Location)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Resource Gathering");
    Mission.MissionDescription = TEXT("Gather essential resources for survival");
    Mission.MissionType = EQuest_SurvivalMissionType::Gather_Resources;
    Mission.MissionLocation = Location;
    Mission.TimeLimit = 600.0f; // 10 minutes
    Mission.Priority = 1;
    Mission.ExperienceReward = 100;
    
    // Add objectives
    FQuest_SurvivalObjective Objective1;
    Objective1.Description = TEXT("Collect 10 stones");
    Objective1.TargetCount = 10;
    Mission.Objectives.Add(Objective1);
    
    FQuest_SurvivalObjective Objective2;
    Objective2.Description = TEXT("Collect 5 sticks");
    Objective2.TargetCount = 5;
    Mission.Objectives.Add(Objective2);
    
    Mission.RewardItems.Add(TEXT("Crafting Materials"));
    
    return Mission;
}

FQuest_SurvivalMission AQuest_SurvivalMissionManager::GenerateExploreMission(const FVector& Location)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Territory Exploration");
    Mission.MissionDescription = TEXT("Explore unknown territory and map the area");
    Mission.MissionType = EQuest_SurvivalMissionType::Explore_Territory;
    Mission.MissionLocation = Location;
    Mission.TimeLimit = 1200.0f; // 20 minutes
    Mission.Priority = 2;
    Mission.ExperienceReward = 150;
    
    // Add objectives
    FQuest_SurvivalObjective Objective1;
    Objective1.Description = TEXT("Reach the northern hills");
    Objective1.TargetLocation = GetRandomLocationNearPlayer(2000.0f, 4000.0f);
    Objective1.CompletionRadius = 500.0f;
    Mission.Objectives.Add(Objective1);
    
    FQuest_SurvivalObjective Objective2;
    Objective2.Description = TEXT("Survey the area for resources");
    Objective2.TargetLocation = GetRandomLocationNearPlayer(1500.0f, 3000.0f);
    Objective2.CompletionRadius = 200.0f;
    Mission.Objectives.Add(Objective2);
    
    Mission.RewardItems.Add(TEXT("Map Fragment"));
    Mission.RewardItems.Add(TEXT("Territory Knowledge"));
    
    return Mission;
}

FQuest_SurvivalMission AQuest_SurvivalMissionManager::GenerateDefenseMission(const FVector& Location)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Defend the Camp");
    Mission.MissionDescription = TEXT("Protect the settlement from incoming predator attack");
    Mission.MissionType = EQuest_SurvivalMissionType::Defend_Camp;
    Mission.MissionLocation = Location;
    Mission.TimeLimit = 300.0f; // 5 minutes - urgent!
    Mission.Priority = 5;
    Mission.ExperienceReward = 300;
    
    // Add objectives
    FQuest_SurvivalObjective Objective1;
    Objective1.Description = TEXT("Return to camp immediately");
    Objective1.TargetLocation = FVector(0, 0, 100); // Camp location
    Objective1.CompletionRadius = 1000.0f;
    Mission.Objectives.Add(Objective1);
    
    FQuest_SurvivalObjective Objective2;
    Objective2.Description = TEXT("Defeat attacking predators");
    Objective2.TargetCount = 5;
    Mission.Objectives.Add(Objective2);
    
    Mission.RewardItems.Add(TEXT("Camp Reputation"));
    Mission.RewardItems.Add(TEXT("Defense Materials"));
    Mission.RequiredItems.Add(TEXT("Weapon"));
    
    return Mission;
}

FQuest_SurvivalMission AQuest_SurvivalMissionManager::GenerateRescueMission(const FVector& Location)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Rescue Mission");
    Mission.MissionDescription = TEXT("A survivor is trapped and needs immediate rescue");
    Mission.MissionType = EQuest_SurvivalMissionType::Rescue_Survivor;
    Mission.MissionLocation = Location;
    Mission.TimeLimit = 450.0f; // 7.5 minutes
    Mission.Priority = 4;
    Mission.ExperienceReward = 200;
    
    // Add objectives
    FQuest_SurvivalObjective Objective1;
    Objective1.Description = TEXT("Locate the trapped survivor");
    Objective1.TargetLocation = GetRandomLocationNearPlayer(800.0f, 1500.0f);
    Objective1.CompletionRadius = 200.0f;
    Mission.Objectives.Add(Objective1);
    
    FQuest_SurvivalObjective Objective2;
    Objective2.Description = TEXT("Clear the area of threats");
    Objective2.TargetCount = 2;
    Mission.Objectives.Add(Objective2);
    
    FQuest_SurvivalObjective Objective3;
    Objective3.Description = TEXT("Escort survivor to safety");
    Objective3.TargetLocation = FVector(0, 0, 100); // Safe zone
    Objective3.CompletionRadius = 500.0f;
    Mission.Objectives.Add(Objective3);
    
    Mission.RewardItems.Add(TEXT("Survivor Gratitude"));
    Mission.RewardItems.Add(TEXT("Rescue Equipment"));
    
    return Mission;
}

TArray<FQuest_SurvivalMission> AQuest_SurvivalMissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

TArray<FQuest_SurvivalMission> AQuest_SurvivalMissionManager::GetAvailableMissions() const
{
    return AvailableMissions;
}

FQuest_SurvivalMission AQuest_SurvivalMissionManager::GetMissionByIndex(int32 Index) const
{
    if (ActiveMissions.IsValidIndex(Index))
    {
        return ActiveMissions[Index];
    }
    return FQuest_SurvivalMission();
}

int32 AQuest_SurvivalMissionManager::GetActiveMissionCount() const
{
    return ActiveMissions.Num();
}

bool AQuest_SurvivalMissionManager::HasActiveMissionOfType(EQuest_SurvivalMissionType MissionType) const
{
    for (const FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionType == MissionType)
        {
            return true;
        }
    }
    return false;
}

void AQuest_SurvivalMissionManager::OnPlayerEnteredArea(const FVector& PlayerLocation, float Radius)
{
    // Check for mission triggers based on location
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        for (int32 j = 0; j < ActiveMissions[i].Objectives.Num(); j++)
        {
            CheckObjectiveCompletion(i, j, PlayerLocation);
        }
    }
}

void AQuest_SurvivalMissionManager::OnPlayerKilledDinosaur(const FString& DinosaurType, const FVector& Location)
{
    // Update hunt mission objectives
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        if (Mission.MissionType == EQuest_SurvivalMissionType::Hunt_DinosaurPack ||
            Mission.MissionType == EQuest_SurvivalMissionType::Defend_Camp)
        {
            for (int32 j = 0; j < Mission.Objectives.Num(); j++)
            {
                FQuest_SurvivalObjective& Objective = Mission.Objectives[j];
                if (!Objective.bIsCompleted && Objective.TargetCount > 0)
                {
                    UpdateObjectiveProgress(i, j, 1);
                    break;
                }
            }
        }
    }
}

void AQuest_SurvivalMissionManager::OnPlayerGatheredResource(const FString& ResourceType, int32 Amount)
{
    // Update gathering mission objectives
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        if (Mission.MissionType == EQuest_SurvivalMissionType::Gather_Resources)
        {
            for (int32 j = 0; j < Mission.Objectives.Num(); j++)
            {
                FQuest_SurvivalObjective& Objective = Mission.Objectives[j];
                if (!Objective.bIsCompleted && Objective.Description.Contains(ResourceType))
                {
                    UpdateObjectiveProgress(i, j, Amount);
                    break;
                }
            }
        }
    }
}

void AQuest_SurvivalMissionManager::OnPlayerCraftedItem(const FString& ItemType)
{
    // Update crafting mission objectives
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        if (Mission.MissionType == EQuest_SurvivalMissionType::Craft_Tools)
        {
            for (int32 j = 0; j < Mission.Objectives.Num(); j++)
            {
                FQuest_SurvivalObjective& Objective = Mission.Objectives[j];
                if (!Objective.bIsCompleted && Objective.Description.Contains(ItemType))
                {
                    UpdateObjectiveProgress(i, j, 1);
                    break;
                }
            }
        }
    }
}

void AQuest_SurvivalMissionManager::GenerateRandomMissions()
{
    if (AvailableMissions.Num() >= 5 || !PlayerCharacter)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Generate 1-2 random missions
    int32 MissionsToGenerate = FMath::RandRange(1, 2);
    
    for (int32 i = 0; i < MissionsToGenerate; i++)
    {
        if (AllowedMissionTypes.Num() == 0)
        {
            break;
        }
        
        EQuest_SurvivalMissionType RandomType = AllowedMissionTypes[FMath::RandRange(0, AllowedMissionTypes.Num() - 1)];
        FVector MissionLocation = GetRandomLocationNearPlayer(500.0f, MissionGenerationRadius);
        
        FQuest_SurvivalMission NewMission;
        
        switch (RandomType)
        {
            case EQuest_SurvivalMissionType::Hunt_DinosaurPack:
                NewMission = GenerateHuntMission(MissionLocation);
                break;
            case EQuest_SurvivalMissionType::Gather_Resources:
                NewMission = GenerateGatherMission(MissionLocation);
                break;
            case EQuest_SurvivalMissionType::Explore_Territory:
                NewMission = GenerateExploreMission(MissionLocation);
                break;
            case EQuest_SurvivalMissionType::Defend_Camp:
                NewMission = GenerateDefenseMission(MissionLocation);
                break;
            case EQuest_SurvivalMissionType::Rescue_Survivor:
                NewMission = GenerateRescueMission(MissionLocation);
                break;
            default:
                NewMission = GenerateGatherMission(MissionLocation);
                break;
        }
        
        AvailableMissions.Add(NewMission);
    }
}

void AQuest_SurvivalMissionManager::CleanupExpiredMissions()
{
    // Remove old available missions
    for (int32 i = AvailableMissions.Num() - 1; i >= 0; i--)
    {
        // Remove missions that have been available for too long
        if (GetWorld() && GetWorld()->GetTimeSeconds() - LastMissionGenerationTime > 600.0f)
        {
            AvailableMissions.RemoveAt(i);
        }
    }
    
    // Limit completed missions history
    if (CompletedMissions.Num() > 20)
    {
        CompletedMissions.RemoveAt(0, CompletedMissions.Num() - 20);
    }
}

FVector AQuest_SurvivalMissionManager::GetRandomLocationNearPlayer(float MinDistance, float MaxDistance) const
{
    if (!PlayerCharacter)
    {
        return FVector::ZeroVector;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float Distance = FMath::RandRange(MinDistance, MaxDistance);
    float Angle = FMath::RandRange(0.0f, 360.0f);
    
    FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
    
    return PlayerLocation + Offset;
}

bool AQuest_SurvivalMissionManager::IsLocationSafe(const FVector& Location) const
{
    // Basic safety check - avoid water, extreme heights, etc.
    // This would need proper terrain/hazard checking in a real implementation
    return Location.Z > -1000.0f && Location.Z < 5000.0f;
}

void AQuest_SurvivalMissionManager::NotifyPlayerOfMissionUpdate(const FString& Message)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest System: %s"), *Message);
}