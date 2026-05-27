#include "Quest_CrowdMissionSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

AQuest_CrowdMissionSystem::AQuest_CrowdMissionSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxConcurrentMissions = 3;
    MissionCheckInterval = 5.0f;
    bAutoGenerateMissions = true;
    LastMissionCheckTime = 0.0f;
    PlayerController = nullptr;
}

void AQuest_CrowdMissionSystem::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    
    if (bAutoGenerateMissions)
    {
        // Generate initial mission after 10 seconds
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, this, &AQuest_CrowdMissionSystem::GenerateRandomMission, 10.0f, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionSystem initialized"));
}

void AQuest_CrowdMissionSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMissionTimers(DeltaTime);
    
    LastMissionCheckTime += DeltaTime;
    if (LastMissionCheckTime >= MissionCheckInterval)
    {
        CheckMissionCompletion();
        UpdateCrowdCount();
        ValidateCrowdActors();
        LastMissionCheckTime = 0.0f;

        if (bAutoGenerateMissions && ActiveMissions.Num() < MaxConcurrentMissions)
        {
            GenerateRandomMission();
        }
    }
}

void AQuest_CrowdMissionSystem::StartCrowdMission(const FQuest_CrowdMissionData& MissionData)
{
    if (ActiveMissions.Num() >= MaxConcurrentMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission: Maximum concurrent missions reached"));
        return;
    }

    FQuest_CrowdMissionData NewMission = MissionData;
    NewMission.Status = EQuest_CrowdMissionStatus::Active;
    NewMission.ElapsedTime = 0.0f;
    NewMission.CurrentCrowdCount = 0;

    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Started crowd mission: %s"), *NewMission.MissionName);
}

void AQuest_CrowdMissionSystem::CompleteMission(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return;
    }

    FQuest_CrowdMissionData CompletedMission = ActiveMissions[MissionIndex];
    CompletedMission.Status = EQuest_CrowdMissionStatus::Completed;
    
    CompletedMissions.Add(CompletedMission);
    ActiveMissions.RemoveAt(MissionIndex);

    UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s"), *CompletedMission.MissionName);
}

void AQuest_CrowdMissionSystem::FailMission(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return;
    }

    FQuest_CrowdMissionData FailedMission = ActiveMissions[MissionIndex];
    FailedMission.Status = EQuest_CrowdMissionStatus::Failed;
    
    CompletedMissions.Add(FailedMission);
    ActiveMissions.RemoveAt(MissionIndex);

    UE_LOG(LogTemp, Warning, TEXT("Mission failed: %s"), *FailedMission.MissionName);
}

bool AQuest_CrowdMissionSystem::CheckMissionProgress(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return false;
    }

    FQuest_CrowdMissionData& Mission = ActiveMissions[MissionIndex];
    
    // Update crowd count in mission area
    TArray<AActor*> NearbyActors = GetNearbyActors(Mission.TargetLocation, Mission.MissionRadius, AActor::StaticClass());
    Mission.CurrentCrowdCount = NearbyActors.Num();

    // Check mission type specific conditions
    switch (Mission.MissionType)
    {
        case EQuest_CrowdMissionType::HerdDinosaurs:
            return Mission.CurrentCrowdCount >= Mission.RequiredCrowdSize;
            
        case EQuest_CrowdMissionType::DefendTerritory:
            return IsPlayerInMissionArea(Mission) && Mission.CurrentCrowdCount >= Mission.RequiredCrowdSize;
            
        case EQuest_CrowdMissionType::EscortGroup:
            return Mission.CurrentCrowdCount >= Mission.RequiredCrowdSize && IsPlayerInMissionArea(Mission);
            
        case EQuest_CrowdMissionType::HuntPack:
            return Mission.CurrentCrowdCount >= Mission.RequiredCrowdSize;
            
        case EQuest_CrowdMissionType::ScavengeResources:
            return IsPlayerInMissionArea(Mission);
    }

    return false;
}

void AQuest_CrowdMissionSystem::UpdateCrowdCount()
{
    // Get all actors in the world that could be part of crowd missions
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), AllActors);
    
    CrowdActors.Empty();
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Crowd")) || Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            CrowdActors.Add(Actor);
        }
    }
}

TArray<AActor*> AQuest_CrowdMissionSystem::GetNearbyActors(FVector Location, float Radius, TSubclassOf<AActor> ActorClass)
{
    TArray<AActor*> NearbyActors;
    TArray<AActor*> AllActors;
    
    UGameplayStatics::GetAllActorsOfClass(this, ActorClass, AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Location) <= Radius)
        {
            NearbyActors.Add(Actor);
        }
    }
    
    return NearbyActors;
}

void AQuest_CrowdMissionSystem::GenerateRandomMission()
{
    if (ActiveMissions.Num() >= MaxConcurrentMissions)
    {
        return;
    }

    // Generate random location within world bounds
    FVector RandomLocation = GetRandomLocationInRadius(FVector::ZeroVector, 10000.0f);
    
    // Randomly select mission type
    int32 RandomType = FMath::RandRange(0, 4);
    
    FQuest_CrowdMissionData NewMission;
    
    switch (RandomType)
    {
        case 0:
            NewMission = CreateHerdMission(RandomLocation);
            break;
        case 1:
            NewMission = CreateDefenseMission(RandomLocation);
            break;
        case 2:
            {
                FVector EndLocation = GetRandomLocationInRadius(RandomLocation, 5000.0f);
                NewMission = CreateEscortMission(RandomLocation, EndLocation);
            }
            break;
        case 3:
            NewMission = CreateHerdMission(RandomLocation); // Hunt pack similar to herd
            NewMission.MissionType = EQuest_CrowdMissionType::HuntPack;
            NewMission.MissionName = TEXT("Hunt Pack Mission");
            NewMission.Description = TEXT("Coordinate with pack to hunt large prey");
            break;
        case 4:
            NewMission = CreateDefenseMission(RandomLocation);
            NewMission.MissionType = EQuest_CrowdMissionType::ScavengeResources;
            NewMission.MissionName = TEXT("Scavenge Resources");
            NewMission.Description = TEXT("Gather resources in the area");
            break;
    }
    
    StartCrowdMission(NewMission);
}

FQuest_CrowdMissionData AQuest_CrowdMissionSystem::CreateHerdMission(FVector Location)
{
    FQuest_CrowdMissionData Mission;
    Mission.MissionName = TEXT("Herd Dinosaurs");
    Mission.Description = TEXT("Guide dinosaurs to the target location");
    Mission.MissionType = EQuest_CrowdMissionType::HerdDinosaurs;
    Mission.RequiredCrowdSize = FMath::RandRange(3, 8);
    Mission.MissionRadius = 2000.0f;
    Mission.TargetLocation = Location;
    Mission.TimeLimit = 300.0f;
    Mission.bRequiresPlayerPresence = true;
    
    return Mission;
}

FQuest_CrowdMissionData AQuest_CrowdMissionSystem::CreateDefenseMission(FVector Location)
{
    FQuest_CrowdMissionData Mission;
    Mission.MissionName = TEXT("Defend Territory");
    Mission.Description = TEXT("Protect the area from threats");
    Mission.MissionType = EQuest_CrowdMissionType::DefendTerritory;
    Mission.RequiredCrowdSize = FMath::RandRange(5, 10);
    Mission.MissionRadius = 1500.0f;
    Mission.TargetLocation = Location;
    Mission.TimeLimit = 240.0f;
    Mission.bRequiresPlayerPresence = true;
    
    return Mission;
}

FQuest_CrowdMissionData AQuest_CrowdMissionSystem::CreateEscortMission(FVector StartLocation, FVector EndLocation)
{
    FQuest_CrowdMissionData Mission;
    Mission.MissionName = TEXT("Escort Group");
    Mission.Description = TEXT("Safely escort the group to destination");
    Mission.MissionType = EQuest_CrowdMissionType::EscortGroup;
    Mission.RequiredCrowdSize = FMath::RandRange(4, 7);
    Mission.MissionRadius = 1000.0f;
    Mission.TargetLocation = EndLocation;
    Mission.TimeLimit = 360.0f;
    Mission.bRequiresPlayerPresence = true;
    
    return Mission;
}

bool AQuest_CrowdMissionSystem::IsPlayerInMissionArea(const FQuest_CrowdMissionData& Mission)
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return false;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Mission.TargetLocation);
    
    return Distance <= Mission.MissionRadius;
}

void AQuest_CrowdMissionSystem::CleanupCompletedMissions()
{
    // Keep only last 10 completed missions to prevent memory bloat
    if (CompletedMissions.Num() > 10)
    {
        int32 ExcessCount = CompletedMissions.Num() - 10;
        CompletedMissions.RemoveAt(0, ExcessCount);
    }
}

void AQuest_CrowdMissionSystem::UpdateMissionTimers(float DeltaTime)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_CrowdMissionData& Mission = ActiveMissions[i];
        Mission.ElapsedTime += DeltaTime;
        
        if (Mission.ElapsedTime >= Mission.TimeLimit)
        {
            FailMission(i);
        }
    }
}

void AQuest_CrowdMissionSystem::CheckMissionCompletion()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (CheckMissionProgress(i))
        {
            CompleteMission(i);
        }
    }
    
    CleanupCompletedMissions();
}

void AQuest_CrowdMissionSystem::ValidateCrowdActors()
{
    // Remove null or destroyed actors from crowd list
    CrowdActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
}

FVector AQuest_CrowdMissionSystem::GetRandomLocationInRadius(FVector Center, float Radius)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset;
    RandomOffset.X = RandomDistance * FMath::Cos(RandomAngle);
    RandomOffset.Y = RandomDistance * FMath::Sin(RandomAngle);
    RandomOffset.Z = 0.0f;
    
    return Center + RandomOffset;
}