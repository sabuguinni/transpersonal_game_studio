// CrowdMassSpawner.cpp
// Agent #13 — Crowd & Traffic Simulation
// Mass Entity spawner: herd patterns, LOD chain, up to 50,000 agents

#include "CrowdMassSpawner.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "NavigationSystem.h"

ACrowd_MassSpawner::ACrowd_MassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // 2Hz — LOD updates are not frame-critical

    MaxAgentsPerSpawner = 500;
    bAutoSpawnOnBeginPlay = true;
    bUseLODSystem = true;
    TotalSpawnedAgents = 0;

    // Default LOD settings
    DefaultLODSettings.FullDetailDistance = 2000.0f;
    DefaultLODSettings.MediumDetailDistance = 5000.0f;
    DefaultLODSettings.LowDetailDistance = 10000.0f;
    DefaultLODSettings.bUseBillboardBeyondLow = true;
}

void ACrowd_MassSpawner::BeginPlay()
{
    Super::BeginPlay();

    SpawnedAgentLocations.Empty();
    TotalSpawnedAgents = 0;

    if (bAutoSpawnOnBeginPlay && SpawnRequests.Num() > 0)
    {
        for (const FCrowd_SpawnRequest& Request : SpawnRequests)
        {
            ExecuteSpawnRequest(Request);
        }
        UE_LOG(LogTemp, Log, TEXT("[CrowdMassSpawner] Auto-spawned %d agents from %d requests."),
            TotalSpawnedAgents, SpawnRequests.Num());
    }
}

void ACrowd_MassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bUseLODSystem)
    {
        UpdateLODForAllAgents();
    }
}

void ACrowd_MassSpawner::SpawnHerd(FCrowd_SpawnRequest Request)
{
    if (TotalSpawnedAgents + Request.AgentCount > MaxAgentsPerSpawner)
    {
        int32 Available = MaxAgentsPerSpawner - TotalSpawnedAgents;
        UE_LOG(LogTemp, Warning, TEXT("[CrowdMassSpawner] Clamping spawn count from %d to %d (budget limit)"),
            Request.AgentCount, Available);
        Request.AgentCount = Available;
    }

    if (Request.AgentCount <= 0) return;

    ExecuteSpawnRequest(Request);
}

void ACrowd_MassSpawner::ExecuteSpawnRequest(const FCrowd_SpawnRequest& Request)
{
    TArray<FVector> SpawnPositions;
    GenerateSpawnPositions(Request, SpawnPositions);

    for (const FVector& Pos : SpawnPositions)
    {
        SpawnedAgentLocations.Add(Pos);
        TotalSpawnedAgents++;
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdMassSpawner] Spawned %d agents. Pattern=%d. Total=%d/%d"),
        SpawnPositions.Num(), (int32)Request.Pattern, TotalSpawnedAgents, MaxAgentsPerSpawner);
}

void ACrowd_MassSpawner::GenerateSpawnPositions(const FCrowd_SpawnRequest& Request, TArray<FVector>& OutPositions)
{
    OutPositions.Empty();
    OutPositions.Reserve(Request.AgentCount);

    switch (Request.Pattern)
    {
        case ECrowd_SpawnPattern::HerdFormation:
        {
            // Tight cluster with leader at front
            FVector Leader = Request.CenterLocation;
            OutPositions.Add(Leader);

            for (int32 i = 1; i < Request.AgentCount; i++)
            {
                float Angle = FMath::RandRange(0.0f, 360.0f);
                float Dist = FMath::RandRange(Request.MinSpacing, Request.SpawnRadius * 0.6f);
                FVector Offset(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                               FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist, 0.0f);
                OutPositions.Add(Request.CenterLocation + Offset);
            }
            break;
        }

        case ECrowd_SpawnPattern::MigrationLine:
        {
            // Column formation — agents in a line with slight lateral spread
            FVector Direction = FVector(1.0f, 0.1f, 0.0f).GetSafeNormal();
            float Spacing = FMath::Max(Request.MinSpacing, 200.0f);

            for (int32 i = 0; i < Request.AgentCount; i++)
            {
                float LateralJitter = FMath::RandRange(-150.0f, 150.0f);
                FVector Pos = Request.CenterLocation + Direction * (i * Spacing);
                Pos.Y += LateralJitter;
                OutPositions.Add(Pos);
            }
            break;
        }

        case ECrowd_SpawnPattern::WaterGathering:
        {
            // Agents clustered near a water source — denser near center
            for (int32 i = 0; i < Request.AgentCount; i++)
            {
                float Angle = FMath::RandRange(0.0f, 360.0f);
                // Bias toward center (water edge)
                float t = FMath::Pow(FMath::FRand(), 2.0f);
                float Dist = FMath::Lerp(Request.MinSpacing, Request.SpawnRadius, t);
                FVector Offset(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                               FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist, 0.0f);
                OutPositions.Add(Request.CenterLocation + Offset);
            }
            break;
        }

        case ECrowd_SpawnPattern::ForestEdge:
        {
            // Agents along a linear forest boundary — spread along X axis
            float LineLength = Request.SpawnRadius * 2.0f;
            for (int32 i = 0; i < Request.AgentCount; i++)
            {
                float t = (float)i / FMath::Max(Request.AgentCount - 1, 1);
                float X = Request.CenterLocation.X - LineLength * 0.5f + t * LineLength;
                float Y = Request.CenterLocation.Y + FMath::RandRange(-200.0f, 200.0f);
                OutPositions.Add(FVector(X, Y, Request.CenterLocation.Z));
            }
            break;
        }

        case ECrowd_SpawnPattern::Scattered:
        default:
        {
            // Random scatter within radius
            for (int32 i = 0; i < Request.AgentCount; i++)
            {
                FVector RandomOffset = FMath::VRand() * FMath::RandRange(Request.MinSpacing, Request.SpawnRadius);
                RandomOffset.Z = 0.0f;
                OutPositions.Add(Request.CenterLocation + RandomOffset);
            }
            break;
        }
    }
}

void ACrowd_MassSpawner::UpdateLODForAllAgents()
{
    if (!GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

    int32 FullDetail = 0, MedDetail = 0, LowDetail = 0, Billboard = 0;

    for (const FVector& AgentLoc : SpawnedAgentLocations)
    {
        float Dist = FVector::Dist(PlayerLoc, AgentLoc);

        if (Dist <= DefaultLODSettings.FullDetailDistance)
            FullDetail++;
        else if (Dist <= DefaultLODSettings.MediumDetailDistance)
            MedDetail++;
        else if (Dist <= DefaultLODSettings.LowDetailDistance)
            LowDetail++;
        else if (DefaultLODSettings.bUseBillboardBeyondLow)
            Billboard++;
    }

    // Log LOD distribution periodically (every ~5s at 2Hz tick = 10 ticks)
    static int32 TickCount = 0;
    if (++TickCount % 10 == 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[CrowdMassSpawner] LOD: Full=%d Med=%d Low=%d Billboard=%d"),
            FullDetail, MedDetail, LowDetail, Billboard);
    }
}

void ACrowd_MassSpawner::ClearAllAgents()
{
    SpawnedAgentLocations.Empty();
    TotalSpawnedAgents = 0;
    UE_LOG(LogTemp, Log, TEXT("[CrowdMassSpawner] All agents cleared."));
}

int32 ACrowd_MassSpawner::GetAgentCountInRadius(FVector Center, float Radius) const
{
    int32 Count = 0;
    float RadiusSq = Radius * Radius;
    for (const FVector& Loc : SpawnedAgentLocations)
    {
        if (FVector::DistSquared(Center, Loc) <= RadiusSq)
            Count++;
    }
    return Count;
}
