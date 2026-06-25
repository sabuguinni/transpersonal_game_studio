// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements herd behavior, migration corridors, and LOD-aware crowd management
// for the prehistoric survival game.

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5s for performance

    MaxHerdSize = 20;
    MigrationSpeed = 150.0f;
    FlockingRadius = 500.0f;
    SeparationRadius = 120.0f;
    LODDistanceClose = 2000.0f;
    LODDistanceMid = 5000.0f;
    LODDistanceFar = 10000.0f;
    bHerdMigrationActive = false;
    CurrentWaypointIndex = 0;
    ThreatResponseRadius = 1500.0f;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd();
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bHerdMigrationActive && HerdAgents.Num() > 0)
    {
        UpdateHerdMigration(DeltaTime);
        UpdateLODForAgents();
        CheckThreatResponse();
    }
}

void UCrowdSimulationManager::InitializeHerd()
{
    HerdAgents.Empty();

    // Discover all crowd agents in the world tagged with herd species
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("CrowdAgent"), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && HerdAgents.Num() < MaxHerdSize)
        {
            HerdAgents.Add(Actor);
        }
    }

    if (HerdAgents.Num() > 0)
    {
        bHerdMigrationActive = true;
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized herd with %d agents"), HerdAgents.Num());
    }
}

void UCrowdSimulationManager::UpdateHerdMigration(float DeltaTime)
{
    if (MigrationWaypoints.Num() == 0 || HerdAgents.Num() == 0) return;

    FVector TargetWaypoint = MigrationWaypoints[CurrentWaypointIndex % MigrationWaypoints.Num()];

    for (AActor* Agent : HerdAgents)
    {
        if (!IsValid(Agent)) continue;

        FVector AgentLoc = Agent->GetActorLocation();
        FVector ToTarget = TargetWaypoint - AgentLoc;
        float DistToTarget = ToTarget.Size();

        // Apply flocking: separation + cohesion + alignment
        FVector FlockForce = ComputeFlockingForce(Agent);
        FVector MoveDir = (ToTarget.GetSafeNormal() + FlockForce * 0.3f).GetSafeNormal();

        FVector NewLoc = AgentLoc + MoveDir * MigrationSpeed * DeltaTime;
        Agent->SetActorLocation(NewLoc, true);

        // Face movement direction
        if (!MoveDir.IsNearlyZero())
        {
            FRotator NewRot = MoveDir.Rotation();
            Agent->SetActorRotation(NewRot);
        }
    }

    // Advance waypoint when herd centroid is close enough
    FVector Centroid = ComputeHerdCentroid();
    if (FVector::Dist(Centroid, TargetWaypoint) < 300.0f)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % MigrationWaypoints.Num();
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Herd advancing to waypoint %d"), CurrentWaypointIndex);
    }
}

FVector UCrowdSimulationManager::ComputeFlockingForce(AActor* Agent) const
{
    if (!IsValid(Agent)) return FVector::ZeroVector;

    FVector Separation = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    int32 Neighbors = 0;

    FVector AgentLoc = Agent->GetActorLocation();

    for (AActor* Other : HerdAgents)
    {
        if (!IsValid(Other) || Other == Agent) continue;

        FVector OtherLoc = Other->GetActorLocation();
        float Dist = FVector::Dist(AgentLoc, OtherLoc);

        if (Dist < FlockingRadius)
        {
            Cohesion += OtherLoc;
            Neighbors++;

            // Separation: push away if too close
            if (Dist < SeparationRadius && Dist > 0.0f)
            {
                Separation += (AgentLoc - OtherLoc) / Dist;
            }
        }
    }

    FVector Result = Separation;
    if (Neighbors > 0)
    {
        Cohesion /= Neighbors;
        Result += (Cohesion - AgentLoc).GetSafeNormal() * 0.5f;
    }

    return Result.GetSafeNormal();
}

FVector UCrowdSimulationManager::ComputeHerdCentroid() const
{
    if (HerdAgents.Num() == 0) return FVector::ZeroVector;

    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;

    for (AActor* Agent : HerdAgents)
    {
        if (IsValid(Agent))
        {
            Sum += Agent->GetActorLocation();
            ValidCount++;
        }
    }

    return ValidCount > 0 ? Sum / ValidCount : FVector::ZeroVector;
}

void UCrowdSimulationManager::UpdateLODForAgents()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (AActor* Agent : HerdAgents)
    {
        if (!IsValid(Agent)) continue;

        float Dist = FVector::Dist(Agent->GetActorLocation(), PlayerLoc);
        USkeletalMeshComponent* SkelComp = Agent->FindComponentByClass<USkeletalMeshComponent>();

        if (!SkelComp) continue;

        if (Dist < LODDistanceClose)
        {
            // Full detail: animations, full tick
            SkelComp->SetComponentTickInterval(0.033f); // ~30fps
            SkelComp->bEnableUpdateRateOptimizations = false;
        }
        else if (Dist < LODDistanceMid)
        {
            // Mid LOD: reduced tick rate
            SkelComp->SetComponentTickInterval(0.1f); // 10fps
            SkelComp->bEnableUpdateRateOptimizations = true;
        }
        else if (Dist < LODDistanceFar)
        {
            // Far LOD: very low tick rate
            SkelComp->SetComponentTickInterval(0.5f); // 2fps
            SkelComp->bEnableUpdateRateOptimizations = true;
        }
        else
        {
            // Beyond far: hide
            Agent->SetActorHiddenInGame(true);
        }

        // Unhide if came back into range
        if (Dist < LODDistanceFar && Agent->IsHidden())
        {
            Agent->SetActorHiddenInGame(false);
        }
    }
}

void UCrowdSimulationManager::CheckThreatResponse()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FVector Centroid = ComputeHerdCentroid();
    float DistToPlayer = FVector::Dist(Centroid, PlayerLoc);

    if (DistToPlayer < ThreatResponseRadius)
    {
        // Scatter: move away from threat
        for (AActor* Agent : HerdAgents)
        {
            if (!IsValid(Agent)) continue;

            FVector AgentLoc = Agent->GetActorLocation();
            FVector FleeDir = (AgentLoc - PlayerLoc).GetSafeNormal();
            FVector NewLoc = AgentLoc + FleeDir * MigrationSpeed * 2.0f * 0.5f;
            Agent->SetActorLocation(NewLoc, true);
        }

        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Herd fleeing from player threat! Dist=%.0f"), DistToPlayer);
    }
}

void UCrowdSimulationManager::AddMigrationWaypoint(FVector Waypoint)
{
    MigrationWaypoints.Add(Waypoint);
}

void UCrowdSimulationManager::SetHerdMigrationActive(bool bActive)
{
    bHerdMigrationActive = bActive;
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (AActor* Agent : HerdAgents)
    {
        if (IsValid(Agent) && !Agent->IsHidden()) Count++;
    }
    return Count;
}
