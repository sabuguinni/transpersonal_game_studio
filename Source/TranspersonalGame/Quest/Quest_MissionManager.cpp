#include "Quest_MissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

AQuest_MissionManager::AQuest_MissionManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create manager mesh component
    ManagerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ManagerMesh"));
    ManagerMesh->SetupAttachment(RootComponent);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);

    // Initialize quest system properties
    MaxActiveMissions = 5;
    ObjectiveCheckInterval = 2.0f;

    // Set default mesh to a simple cube if available
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded() && ManagerMesh)
    {
        ManagerMesh->SetStaticMesh(CubeMeshAsset.Object);
        ManagerMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }
}

void AQuest_MissionManager::BeginPlay()
{
    Super::BeginPlay();

    // Start objective checking timer
    GetWorldTimerManager().SetTimer(ObjectiveCheckTimer, this, &AQuest_MissionManager::CheckAllObjectives, ObjectiveCheckInterval, true);

    // Generate some initial missions
    GenerateRandomMissions();

    UE_LOG(LogTemp, Warning, TEXT("Quest Mission Manager initialized with %d active missions"), ActiveMissions.Num());
}

void AQuest_MissionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQuest_MissionManager::StartMission(const FQuest_Mission& Mission)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission %s - maximum active missions reached"), *Mission.MissionName);
        return;
    }

    FQuest_Mission NewMission = Mission;
    NewMission.bActive = true;
    NewMission.bCompleted = false;

    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Started mission: %s"), *Mission.MissionName);
}

void AQuest_MissionManager::CompleteMission(const FString& MissionName)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionName == MissionName)
        {
            ActiveMissions[i].bCompleted = true;
            CompletedMissions.Add(ActiveMissions[i]);
            ActiveMissions.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Completed mission: %s"), *MissionName);
            break;
        }
    }
}

void AQuest_MissionManager::UpdateObjective(const FString& MissionName, int32 ObjectiveIndex, bool bComplete)
{
    for (FQuest_Mission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && Mission.Objectives.IsValidIndex(ObjectiveIndex))
        {
            Mission.Objectives[ObjectiveIndex].bCompleted = bComplete;
            UE_LOG(LogTemp, Warning, TEXT("Updated objective %d for mission %s"), ObjectiveIndex, *MissionName);

            // Check if all objectives are complete
            bool bAllComplete = true;
            for (const FQuest_MissionObjective& Obj : Mission.Objectives)
            {
                if (!Obj.bCompleted)
                {
                    bAllComplete = false;
                    break;
                }
            }

            if (bAllComplete)
            {
                CompleteMission(MissionName);
            }
            break;
        }
    }
}

bool AQuest_MissionManager::CheckObjectiveCompletion(const FQuest_MissionObjective& Objective, const FVector& PlayerLocation)
{
    if (Objective.bCompleted)
    {
        return true;
    }

    float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
    return Distance <= Objective.CompletionRadius;
}

TArray<FQuest_Mission> AQuest_MissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

FQuest_Mission AQuest_MissionManager::GetMissionByName(const FString& MissionName)
{
    for (const FQuest_Mission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName)
        {
            return Mission;
        }
    }
    return FQuest_Mission(); // Return default mission if not found
}

void AQuest_MissionManager::CreateHuntMission(const FString& DinosaurType, const FVector& HuntLocation)
{
    FQuest_Mission HuntMission;
    HuntMission.MissionName = FString::Printf(TEXT("Hunt %s"), *DinosaurType);
    HuntMission.Description = FString::Printf(TEXT("Track and hunt a %s in the designated area"), *DinosaurType);
    HuntMission.ExperienceReward = 250;

    FQuest_MissionObjective TrackObjective;
    TrackObjective.ObjectiveText = FString::Printf(TEXT("Travel to %s hunting grounds"), *DinosaurType);
    TrackObjective.TargetLocation = HuntLocation;
    TrackObjective.CompletionRadius = 1000.0f;

    FQuest_MissionObjective HuntObjective;
    HuntObjective.ObjectiveText = FString::Printf(TEXT("Defeat the %s"), *DinosaurType);
    HuntObjective.TargetLocation = HuntLocation;
    HuntObjective.CompletionRadius = 500.0f;

    HuntMission.Objectives.Add(TrackObjective);
    HuntMission.Objectives.Add(HuntObjective);

    StartMission(HuntMission);
}

void AQuest_MissionManager::CreateGatherMission(const FString& ResourceType, int32 Quantity)
{
    FQuest_Mission GatherMission;
    GatherMission.MissionName = FString::Printf(TEXT("Gather %s"), *ResourceType);
    GatherMission.Description = FString::Printf(TEXT("Collect %d units of %s for the tribe"), Quantity, *ResourceType);
    GatherMission.ExperienceReward = 150;

    FQuest_MissionObjective GatherObjective;
    GatherObjective.ObjectiveText = FString::Printf(TEXT("Collect %d %s"), Quantity, *ResourceType);
    GatherObjective.TargetLocation = FVector::ZeroVector; // Will be set dynamically
    GatherObjective.CompletionRadius = 100.0f;

    GatherMission.Objectives.Add(GatherObjective);
    StartMission(GatherMission);
}

void AQuest_MissionManager::CreateExplorationMission(const FVector& ExploreLocation, float ExploreRadius)
{
    FQuest_Mission ExploreMission;
    ExploreMission.MissionName = TEXT("Explore Unknown Territory");
    ExploreMission.Description = TEXT("Scout new areas and report back with findings");
    ExploreMission.ExperienceReward = 200;

    FQuest_MissionObjective ExploreObjective;
    ExploreObjective.ObjectiveText = TEXT("Explore the marked area");
    ExploreObjective.TargetLocation = ExploreLocation;
    ExploreObjective.CompletionRadius = ExploreRadius;

    ExploreMission.Objectives.Add(ExploreObjective);
    StartMission(ExploreMission);
}

void AQuest_MissionManager::CheckAllObjectives()
{
    // This would typically check player location against objectives
    // For now, we'll just log that we're checking
    if (ActiveMissions.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Checking objectives for %d active missions"), ActiveMissions.Num());
    }
}

void AQuest_MissionManager::GenerateRandomMissions()
{
    // Create some default missions for testing
    CreateHuntMission(TEXT("Velociraptor"), FVector(2000.0f, 1500.0f, 100.0f));
    CreateGatherMission(TEXT("Stone"), 5);
    CreateExplorationMission(FVector(-1000.0f, -2000.0f, 200.0f), 800.0f);
}