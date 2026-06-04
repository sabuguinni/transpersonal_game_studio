#include "Quest_SurvivalMissionController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UQuest_SurvivalMissionController::UQuest_SurvivalMissionController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MissionCheckInterval = 5.0f;
    MaxActiveMissions = 3;
    bAutoGenerateMissions = true;
    PlayerHealthThreshold = 30.0f;
    PlayerHungerThreshold = 20.0f;
    PlayerThirstThreshold = 15.0f;
}

void UQuest_SurvivalMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateMissions)
    {
        GetWorld()->GetTimerManager().SetTimer(
            MissionCheckTimer,
            this,
            &UQuest_SurvivalMissionController::CheckPlayerSurvivalStatus,
            MissionCheckInterval,
            true
        );
    }
}

void UQuest_SurvivalMissionController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupExpiredMissions();
}

void UQuest_SurvivalMissionController::CreateSurvivalMission(EQuest_SurvivalMissionType MissionType, EQuest_SurvivalUrgency Urgency, FVector Location)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMissionData NewMission;
    NewMission.MissionType = MissionType;
    NewMission.UrgencyLevel = Urgency;
    NewMission.TargetLocation = Location;
    NewMission.MissionDescription = GetMissionDescription(MissionType);
    
    switch (Urgency)
    {
        case EQuest_SurvivalUrgency::Low:
            NewMission.TimeLimit = 600.0f;
            NewMission.CompletionReward = 5.0f;
            break;
        case EQuest_SurvivalUrgency::Medium:
            NewMission.TimeLimit = 300.0f;
            NewMission.CompletionReward = 10.0f;
            break;
        case EQuest_SurvivalUrgency::High:
            NewMission.TimeLimit = 180.0f;
            NewMission.CompletionReward = 20.0f;
            break;
        case EQuest_SurvivalUrgency::Critical:
            NewMission.TimeLimit = 120.0f;
            NewMission.CompletionReward = 35.0f;
            break;
        case EQuest_SurvivalUrgency::LifeThreatening:
            NewMission.TimeLimit = 60.0f;
            NewMission.CompletionReward = 50.0f;
            break;
    }
    
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::GatherWater:
            NewMission.RequiredQuantity = 3;
            break;
        case EQuest_SurvivalMissionType::HuntPrey:
            NewMission.RequiredQuantity = 1;
            break;
        case EQuest_SurvivalMissionType::FindFood:
            NewMission.RequiredQuantity = 5;
            break;
        case EQuest_SurvivalMissionType::CraftTool:
            NewMission.RequiredQuantity = 1;
            break;
        default:
            NewMission.RequiredQuantity = 1;
            break;
    }
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Survival Mission Created: %s (Urgency: %d)"), 
           *NewMission.MissionDescription, (int32)NewMission.UrgencyLevel);
}

void UQuest_SurvivalMissionController::CompleteMission(int32 MissionIndex)
{
    if (ActiveMissions.IsValidIndex(MissionIndex))
    {
        FQuest_SurvivalMissionData CompletedMission = ActiveMissions[MissionIndex];
        CompletedMission.bIsCompleted = true;
        
        CompletedMissions.Add(CompletedMission);
        ActiveMissions.RemoveAt(MissionIndex);
        
        UE_LOG(LogTemp, Warning, TEXT("Survival Mission Completed: %s (Reward: %.1f)"), 
               *CompletedMission.MissionDescription, CompletedMission.CompletionReward);
    }
}

bool UQuest_SurvivalMissionController::CheckMissionCompletion(const FQuest_SurvivalMissionData& Mission)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }
    
    float DistanceToTarget = FVector::Dist(PlayerPawn->GetActorLocation(), Mission.TargetLocation);
    
    switch (Mission.MissionType)
    {
        case EQuest_SurvivalMissionType::GatherWater:
        case EQuest_SurvivalMissionType::FindFood:
            return DistanceToTarget < 200.0f;
            
        case EQuest_SurvivalMissionType::BuildShelter:
        case EQuest_SurvivalMissionType::DefendCamp:
            return DistanceToTarget < 500.0f;
            
        case EQuest_SurvivalMissionType::ExploreTerritory:
            return DistanceToTarget < 100.0f;
            
        case EQuest_SurvivalMissionType::EscapePredator:
            return DistanceToTarget > 1000.0f;
            
        default:
            return DistanceToTarget < 300.0f;
    }
}

void UQuest_SurvivalMissionController::GenerateUrgentMissions()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Generate water mission if player is thirsty
    CreateSurvivalMission(
        EQuest_SurvivalMissionType::GatherWater,
        EQuest_SurvivalUrgency::Critical,
        FindNearestResourceLocation(EQuest_SurvivalMissionType::GatherWater)
    );
    
    // Generate food mission if player is hungry
    CreateSurvivalMission(
        EQuest_SurvivalMissionType::FindFood,
        EQuest_SurvivalUrgency::High,
        FindNearestResourceLocation(EQuest_SurvivalMissionType::FindFood)
    );
    
    // Generate shelter mission for night survival
    CreateSurvivalMission(
        EQuest_SurvivalMissionType::BuildShelter,
        EQuest_SurvivalUrgency::Medium,
        PlayerLocation + FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), 0)
    );
}

FString UQuest_SurvivalMissionController::GetMissionDescription(EQuest_SurvivalMissionType MissionType)
{
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::GatherWater:
            return TEXT("Find clean water source to refill containers");
        case EQuest_SurvivalMissionType::HuntPrey:
            return TEXT("Hunt small game for meat and materials");
        case EQuest_SurvivalMissionType::BuildShelter:
            return TEXT("Construct shelter for protection from elements");
        case EQuest_SurvivalMissionType::EscapePredator:
            return TEXT("Escape from dangerous predator territory");
        case EQuest_SurvivalMissionType::FindFood:
            return TEXT("Gather edible plants and fruits");
        case EQuest_SurvivalMissionType::CraftTool:
            return TEXT("Craft essential survival tool");
        case EQuest_SurvivalMissionType::ExploreTerritory:
            return TEXT("Scout new area for resources and dangers");
        case EQuest_SurvivalMissionType::DefendCamp:
            return TEXT("Protect camp from hostile creatures");
        default:
            return TEXT("Unknown survival mission");
    }
}

EQuest_SurvivalUrgency UQuest_SurvivalMissionController::CalculateUrgencyLevel(float PlayerHealth, float PlayerHunger, float PlayerThirst)
{
    if (PlayerHealth < 20.0f || PlayerThirst < 10.0f)
    {
        return EQuest_SurvivalUrgency::LifeThreatening;
    }
    else if (PlayerHealth < 40.0f || PlayerThirst < 20.0f || PlayerHunger < 15.0f)
    {
        return EQuest_SurvivalUrgency::Critical;
    }
    else if (PlayerHealth < 60.0f || PlayerThirst < 40.0f || PlayerHunger < 30.0f)
    {
        return EQuest_SurvivalUrgency::High;
    }
    else if (PlayerHealth < 80.0f || PlayerThirst < 60.0f || PlayerHunger < 50.0f)
    {
        return EQuest_SurvivalUrgency::Medium;
    }
    else
    {
        return EQuest_SurvivalUrgency::Low;
    }
}

void UQuest_SurvivalMissionController::CheckPlayerSurvivalStatus()
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    // Simulate player stats check (would integrate with actual player stats system)
    float SimulatedHealth = FMath::RandRange(20.0f, 100.0f);
    float SimulatedHunger = FMath::RandRange(10.0f, 100.0f);
    float SimulatedThirst = FMath::RandRange(5.0f, 100.0f);
    
    EQuest_SurvivalUrgency CurrentUrgency = CalculateUrgencyLevel(SimulatedHealth, SimulatedHunger, SimulatedThirst);
    
    if (CurrentUrgency >= EQuest_SurvivalUrgency::High)
    {
        GenerateUrgentMissions();
    }
}

void UQuest_SurvivalMissionController::CleanupExpiredMissions()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (ActiveMissions[i].TimeLimit <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Survival Mission Expired: %s"), *ActiveMissions[i].MissionDescription);
            ActiveMissions.RemoveAt(i);
        }
        else
        {
            ActiveMissions[i].TimeLimit -= GetWorld()->GetDeltaSeconds();
        }
    }
}

FVector UQuest_SurvivalMissionController::FindNearestResourceLocation(EQuest_SurvivalMissionType ResourceType)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return FVector::ZeroVector;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    switch (ResourceType)
    {
        case EQuest_SurvivalMissionType::GatherWater:
            // Find nearest water source (simulate)
            return PlayerLocation + FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), -50);
            
        case EQuest_SurvivalMissionType::FindFood:
            // Find nearest food source (simulate)
            return PlayerLocation + FVector(FMath::RandRange(-800, 800), FMath::RandRange(-800, 800), 0);
            
        case EQuest_SurvivalMissionType::HuntPrey:
            // Find nearest hunting ground (simulate)
            return PlayerLocation + FVector(FMath::RandRange(-1500, 1500), FMath::RandRange(-1500, 1500), 0);
            
        default:
            return PlayerLocation + FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), 0);
    }
}