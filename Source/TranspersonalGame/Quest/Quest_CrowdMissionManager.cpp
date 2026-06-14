#include "Quest_CrowdMissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "../Crowd/CrowdSimulationManager.h"
#include "../TranspersonalCharacter.h"

AQuest_CrowdMissionManager::AQuest_CrowdMissionManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mission marker
    MissionMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissionMarker"));
    MissionMarker->SetupAttachment(RootComponent);
    
    // Create mission trigger
    MissionTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MissionTrigger"));
    MissionTrigger->SetupAttachment(RootComponent);
    MissionTrigger->SetSphereRadius(500.0f);

    // Initialize default values
    MaxActiveMissions = 3;
    CrowdDetectionRadius = 2000.0f;
    MinCrowdForMission = 5;
    MissionCounter = 0;
    LastMissionCheck = 0.0f;
    bSystemInitialized = false;
    CrowdManager = nullptr;
}

void AQuest_CrowdMissionManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeMissionSystem();
}

void AQuest_CrowdMissionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSystemInitialized)
        return;

    // Update active missions
    for (FQuest_CrowdMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted)
        {
            UpdateMissionProgress(Mission.MissionID, DeltaTime);
            
            // Check for completion
            if (CheckMissionCompletion(Mission.MissionID))
            {
                CompleteMission(Mission.MissionID);
            }
        }
    }

    // Cleanup completed missions periodically
    LastMissionCheck += DeltaTime;
    if (LastMissionCheck >= 30.0f)
    {
        CleanupCompletedMissions();
        LastMissionCheck = 0.0f;
    }
}

void AQuest_CrowdMissionManager::InitializeMissionSystem()
{
    // Find crowd simulation manager
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrowdSimulationManager::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        CrowdManager = Cast<ACrowdSimulationManager>(FoundActors[0]);
        if (CrowdManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Connected to CrowdSimulationManager"));
            bSystemInitialized = true;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: No CrowdSimulationManager found"));
    }

    // Create initial test missions
    CreateEscortMission(FVector(0, 0, 100), FVector(5000, 0, 100), 8);
    CreateCrowdControlMission(FVector(2500, 2500, 100), 15);
    CreateGatheringMission(FVector(-2000, 1000, 100), 6);
}

void AQuest_CrowdMissionManager::CreateEscortMission(FVector StartLocation, FVector EndLocation, int32 CrowdSize)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
        return;

    FQuest_CrowdMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Escort Crowd");
    NewMission.Description = FString::Printf(TEXT("Guide %d people safely from start to destination"), CrowdSize);
    NewMission.QuestType = EQuestType::Escort;
    NewMission.TargetLocation = EndLocation;
    NewMission.RequiredCrowdSize = CrowdSize;
    NewMission.MissionRadius = FVector::Dist(StartLocation, EndLocation) + 500.0f;
    NewMission.TimeLimit = 600.0f;
    NewMission.bIsActive = false;

    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Created escort mission %s"), *NewMission.MissionID);
}

void AQuest_CrowdMissionManager::CreateCrowdControlMission(FVector CongestionPoint, int32 MaxCrowdDensity)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
        return;

    FQuest_CrowdMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Crowd Control");
    NewMission.Description = FString::Printf(TEXT("Manage crowd density at congestion point - keep under %d people"), MaxCrowdDensity);
    NewMission.QuestType = EQuestType::Defense;
    NewMission.TargetLocation = CongestionPoint;
    NewMission.RequiredCrowdSize = MaxCrowdDensity;
    NewMission.MissionRadius = 800.0f;
    NewMission.TimeLimit = 300.0f;
    NewMission.bIsActive = false;

    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Created crowd control mission %s"), *NewMission.MissionID);
}

void AQuest_CrowdMissionManager::CreateGatheringMission(FVector ResourceLocation, int32 RequiredGatherers)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
        return;

    FQuest_CrowdMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Gathering Operation");
    NewMission.Description = FString::Printf(TEXT("Organize %d people to gather resources at location"), RequiredGatherers);
    NewMission.QuestType = EQuestType::Gather;
    NewMission.TargetLocation = ResourceLocation;
    NewMission.RequiredCrowdSize = RequiredGatherers;
    NewMission.MissionRadius = 600.0f;
    NewMission.TimeLimit = 400.0f;
    NewMission.bIsActive = false;

    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Created gathering mission %s"), *NewMission.MissionID);
}

void AQuest_CrowdMissionManager::CreateDefenseMission(FVector DefensePoint, int32 DefenderCount)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
        return;

    FQuest_CrowdMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Defense Formation");
    NewMission.Description = FString::Printf(TEXT("Position %d defenders at strategic point"), DefenderCount);
    NewMission.QuestType = EQuestType::Defense;
    NewMission.TargetLocation = DefensePoint;
    NewMission.RequiredCrowdSize = DefenderCount;
    NewMission.MissionRadius = 400.0f;
    NewMission.TimeLimit = 180.0f;
    NewMission.bIsActive = false;

    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Created defense mission %s"), *NewMission.MissionID);
}

bool AQuest_CrowdMissionManager::StartMission(const FString& MissionID)
{
    for (FQuest_CrowdMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && !Mission.bIsActive)
        {
            if (ValidateMissionRequirements(Mission))
            {
                Mission.bIsActive = true;
                Mission.ElapsedTime = 0.0f;
                UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Started mission %s"), *MissionID);
                return true;
            }
        }
    }
    return false;
}

bool AQuest_CrowdMissionManager::CompleteMission(const FString& MissionID)
{
    for (FQuest_CrowdMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.bIsActive)
        {
            Mission.bIsCompleted = true;
            Mission.bIsActive = false;
            CompletedMissions.Add(Mission);
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Completed mission %s"), *MissionID);
            return true;
        }
    }
    return false;
}

void AQuest_CrowdMissionManager::UpdateMissionProgress(const FString& MissionID, float DeltaTime)
{
    for (FQuest_CrowdMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.bIsActive)
        {
            Mission.ElapsedTime += DeltaTime;
            
            // Check time limit
            if (Mission.ElapsedTime >= Mission.TimeLimit)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Mission %s timed out"), *MissionID);
                Mission.bIsActive = false;
            }
            break;
        }
    }
}

bool AQuest_CrowdMissionManager::CheckMissionCompletion(const FString& MissionID)
{
    for (const FQuest_CrowdMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.bIsActive)
        {
            int32 CrowdCount = GetCrowdCountInRadius(Mission.TargetLocation, Mission.MissionRadius);
            
            switch (Mission.QuestType)
            {
                case EQuestType::Escort:
                    return CrowdCount >= Mission.RequiredCrowdSize;
                case EQuestType::Gather:
                    return CrowdCount >= Mission.RequiredCrowdSize;
                case EQuestType::Defense:
                    return CrowdCount >= Mission.RequiredCrowdSize && Mission.ElapsedTime >= 60.0f;
                default:
                    return false;
            }
        }
    }
    return false;
}

int32 AQuest_CrowdMissionManager::GetCrowdCountInRadius(FVector Location, float Radius)
{
    if (!CrowdManager)
        return 0;

    TArray<AActor*> CrowdActors = GetCrowdActorsInRadius(Location, Radius);
    return CrowdActors.Num();
}

TArray<AActor*> AQuest_CrowdMissionManager::GetCrowdActorsInRadius(FVector Location, float Radius)
{
    TArray<AActor*> FoundActors;
    
    if (GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetActorLabel().Contains(TEXT("Crowd")))
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
                if (Distance <= Radius)
                {
                    FoundActors.Add(Actor);
                }
            }
        }
    }
    
    return FoundActors;
}

void AQuest_CrowdMissionManager::DirectCrowdToLocation(FVector TargetLocation, int32 CrowdCount)
{
    if (!CrowdManager)
        return;

    TArray<AActor*> CrowdActors = GetCrowdActorsInRadius(GetActorLocation(), CrowdDetectionRadius);
    
    int32 DirectedCount = 0;
    for (AActor* Actor : CrowdActors)
    {
        if (DirectedCount >= CrowdCount)
            break;
            
        // Move crowd actor towards target
        if (Actor)
        {
            FVector Direction = (TargetLocation - Actor->GetActorLocation()).GetSafeNormal();
            FVector NewLocation = Actor->GetActorLocation() + Direction * 100.0f;
            Actor->SetActorLocation(NewLocation);
            DirectedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Directed %d crowd actors to target location"), DirectedCount);
}

bool AQuest_CrowdMissionManager::ValidateMissionRequirements(const FQuest_CrowdMission& Mission)
{
    // Check if enough crowd is available
    int32 AvailableCrowd = GetCrowdCountInRadius(GetActorLocation(), CrowdDetectionRadius);
    return AvailableCrowd >= Mission.RequiredCrowdSize;
}

FString AQuest_CrowdMissionManager::GenerateMissionID()
{
    MissionCounter++;
    return FString::Printf(TEXT("CROWD_MISSION_%d"), MissionCounter);
}

void AQuest_CrowdMissionManager::CleanupCompletedMissions()
{
    // Remove completed missions older than 5 minutes
    CompletedMissions.RemoveAll([](const FQuest_CrowdMission& Mission)
    {
        return Mission.bIsCompleted;
    });
    
    // Remove inactive missions from active list
    ActiveMissions.RemoveAll([](const FQuest_CrowdMission& Mission)
    {
        return Mission.bIsCompleted;
    });
}

FVector AQuest_CrowdMissionManager::FindOptimalMissionLocation()
{
    // Find location with moderate crowd density for new missions
    TArray<FVector> TestLocations = {
        FVector(1000, 1000, 100),
        FVector(-1000, 1000, 100),
        FVector(1000, -1000, 100),
        FVector(-1000, -1000, 100),
        FVector(0, 2000, 100)
    };
    
    FVector BestLocation = TestLocations[0];
    int32 BestCrowdCount = 0;
    
    for (const FVector& Location : TestLocations)
    {
        int32 CrowdCount = GetCrowdCountInRadius(Location, 1000.0f);
        if (CrowdCount > MinCrowdForMission && CrowdCount > BestCrowdCount)
        {
            BestLocation = Location;
            BestCrowdCount = CrowdCount;
        }
    }
    
    return BestLocation;
}

bool AQuest_CrowdMissionManager::IsCrowdAvailable(int32 RequiredSize)
{
    int32 TotalCrowd = GetCrowdCountInRadius(GetActorLocation(), CrowdDetectionRadius * 2.0f);
    return TotalCrowd >= RequiredSize;
}