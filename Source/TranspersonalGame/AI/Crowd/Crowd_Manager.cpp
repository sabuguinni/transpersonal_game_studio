#include "Crowd_Manager.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACrowd_Manager::ACrowd_Manager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxTotalAgents = 1000;
    UpdateFrequency = 0.1f;
    LODDistance1 = 2000.0f;
    LODDistance2 = 5000.0f;
    CurrentAgentCount = 0;
    LastUpdateTime = 0.0f;

    // Initialize default zones
    FCrowd_Zone DefaultZone;
    DefaultZone.Center = FVector(0, 0, 0);
    DefaultZone.Radius = 1000.0f;
    DefaultZone.MaxPopulation = 100;
    DefaultZone.ActivityType = ECrowd_ActivityType::Wandering;
    CrowdZones.Add(DefaultZone);
}

void ACrowd_Manager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCrowdZones();
    
    // Spawn initial agents
    for (int32 i = 0; i < CrowdZones.Num(); i++)
    {
        int32 InitialCount = FMath::Min(CrowdZones[i].MaxPopulation / 2, 25);
        SpawnAgentsInZone(i, InitialCount);
    }
}

void ACrowd_Manager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateCrowdBehavior(DeltaTime);
        OptimizeLOD();
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_Manager::InitializeCrowdZones()
{
    if (CrowdZones.Num() == 0)
    {
        // Create default zones if none exist
        TArray<FVector> DefaultLocations = {
            FVector(2000, 0, 0),
            FVector(-1500, 1500, 0),
            FVector(0, -2000, 0)
        };

        for (const FVector& Location : DefaultLocations)
        {
            FCrowd_Zone NewZone;
            NewZone.Center = Location;
            NewZone.Radius = 800.0f;
            NewZone.MaxPopulation = 50;
            NewZone.ActivityType = ECrowd_ActivityType::Gathering;
            CrowdZones.Add(NewZone);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd Manager: Initialized %d zones"), CrowdZones.Num());
}

void ACrowd_Manager::SpawnAgentsInZone(int32 ZoneIndex, int32 Count)
{
    if (!CrowdZones.IsValidIndex(ZoneIndex))
    {
        return;
    }

    const FCrowd_Zone& Zone = CrowdZones[ZoneIndex];
    
    for (int32 i = 0; i < Count && CurrentAgentCount < MaxTotalAgents; i++)
    {
        FCrowd_Agent NewAgent;
        
        // Random position within zone radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Zone.Radius);
        
        NewAgent.Position = Zone.Center + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.BehaviorState = ECrowd_BehaviorState::Wandering;
        NewAgent.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
        NewAgent.ZoneID = ZoneIndex;
        
        ActiveAgents.Add(NewAgent);
        CurrentAgentCount++;
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d agents in zone %d. Total: %d"), Count, ZoneIndex, CurrentAgentCount);
}

void ACrowd_Manager::UpdateCrowdBehavior(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

    for (int32 i = 0; i < ActiveAgents.Num(); i++)
    {
        FCrowd_Agent& Agent = ActiveAgents[i];
        
        // Get zone info
        if (!CrowdZones.IsValidIndex(Agent.ZoneID))
        {
            continue;
        }
        
        const FCrowd_Zone& Zone = CrowdZones[Agent.ZoneID];
        float DistanceToPlayer = FVector::Dist(Agent.Position, PlayerLocation);
        
        // Update behavior based on distance to player and zone activity
        if (DistanceToPlayer < 500.0f)
        {
            // Player nearby - flee or observe
            FVector FleeDirection = (Agent.Position - PlayerLocation).GetSafeNormal();
            Agent.Velocity = FleeDirection * Agent.MovementSpeed * 1.5f;
            Agent.BehaviorState = ECrowd_BehaviorState::Fleeing;
        }
        else
        {
            // Normal zone behavior
            switch (Zone.ActivityType)
            {
                case ECrowd_ActivityType::Wandering:
                {
                    // Random walk within zone
                    if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
                    {
                        FVector RandomDirection = FMath::VRand();
                        RandomDirection.Z = 0.0f;
                        Agent.Velocity = RandomDirection.GetSafeNormal() * Agent.MovementSpeed;
                    }
                    Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
                    break;
                }
                case ECrowd_ActivityType::Gathering:
                {
                    // Move towards zone center
                    FVector ToCenter = (Zone.Center - Agent.Position).GetSafeNormal();
                    Agent.Velocity = ToCenter * Agent.MovementSpeed * 0.5f;
                    Agent.BehaviorState = ECrowd_BehaviorState::Gathering;
                    break;
                }
                case ECrowd_ActivityType::Working:
                {
                    // Slower, more purposeful movement
                    if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
                    {
                        FVector WorkDirection = FMath::VRand();
                        WorkDirection.Z = 0.0f;
                        Agent.Velocity = WorkDirection.GetSafeNormal() * Agent.MovementSpeed * 0.3f;
                    }
                    Agent.BehaviorState = ECrowd_BehaviorState::Working;
                    break;
                }
            }
        }
        
        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
        
        // Keep agent within zone bounds
        float DistanceFromCenter = FVector::Dist(Agent.Position, Zone.Center);
        if (DistanceFromCenter > Zone.Radius)
        {
            FVector ToCenter = (Zone.Center - Agent.Position).GetSafeNormal();
            Agent.Position = Zone.Center + ToCenter * (Zone.Radius - 50.0f);
        }
    }
}

void ACrowd_Manager::OptimizeLOD()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FCrowd_Agent& Agent : ActiveAgents)
    {
        float DistanceToPlayer = FVector::Dist(Agent.Position, PlayerLocation);
        
        // LOD levels based on distance
        if (DistanceToPlayer < LODDistance1)
        {
            // High detail - full simulation
            continue;
        }
        else if (DistanceToPlayer < LODDistance2)
        {
            // Medium detail - reduced update frequency
            if (FMath::RandRange(0.0f, 1.0f) > 0.5f)
            {
                continue; // Skip this frame
            }
        }
        else
        {
            // Low detail - minimal simulation
            if (FMath::RandRange(0.0f, 1.0f) > 0.1f)
            {
                continue; // Skip most frames
            }
        }
    }
}

FCrowd_Zone ACrowd_Manager::GetZoneAtLocation(FVector Location)
{
    for (const FCrowd_Zone& Zone : CrowdZones)
    {
        float Distance = FVector::Dist(Location, Zone.Center);
        if (Distance <= Zone.Radius)
        {
            return Zone;
        }
    }
    
    // Return default zone if no match
    FCrowd_Zone DefaultZone;
    return DefaultZone;
}

void ACrowd_Manager::SetZoneActivity(int32 ZoneIndex, ECrowd_ActivityType NewActivity)
{
    if (CrowdZones.IsValidIndex(ZoneIndex))
    {
        CrowdZones[ZoneIndex].ActivityType = NewActivity;
        UE_LOG(LogTemp, Warning, TEXT("Zone %d activity changed to %d"), ZoneIndex, (int32)NewActivity);
    }
}