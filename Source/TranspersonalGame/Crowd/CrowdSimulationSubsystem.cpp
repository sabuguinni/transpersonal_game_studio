#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogCrowdSimulation);

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogCrowdSimulation, Log, TEXT("Initializing Crowd Simulation Subsystem"));

    // Get Mass Entity subsystem references
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();

    if (!MassEntitySubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassEntitySubsystem"));
        return;
    }

    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassSpawnerSubsystem"));
        return;
    }

    // Initialize default migration routes
    InitializeDefaultMigrationRoutes();

    // Start crowd update timer
    GetWorld()->GetTimerManager().SetTimer(
        CrowdUpdateTimer,
        [this]() { UpdateCrowdBehaviors(CrowdUpdateFrequency); },
        CrowdUpdateFrequency,
        true
    );

    UE_LOG(LogCrowdSimulation, Log, TEXT("Crowd Simulation Subsystem initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogCrowdSimulation, Log, TEXT("Deinitializing Crowd Simulation Subsystem"));

    // Clear timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CrowdUpdateTimer);
    }

    // Clean up all active crowds
    ActiveCrowds.Empty();
    CrowdParameters.Empty();
    ActiveMigrations.Empty();

    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowdSimulationSubsystem::SpawnDinosaurCrowd(const FVector& Location, const FDinosaurCrowdParams& Params, int32 GroupSize)
{
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Mass subsystems not available for crowd spawning"));
        return;
    }

    if (ActiveCrowds.Num() >= MaxSimultaneousCrowds)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Maximum crowd limit reached (%d), cannot spawn new crowd"), MaxSimultaneousCrowds);
        return;
    }

    // Determine actual group size
    int32 ActualGroupSize = GroupSize > 0 ? GroupSize : FMath::RandRange(Params.MinGroupSize, Params.MaxGroupSize);
    ActualGroupSize = FMath::Clamp(ActualGroupSize, 1, 500);

    UE_LOG(LogCrowdSimulation, Log, TEXT("Spawning dinosaur crowd of %d individuals at location %s"), 
           ActualGroupSize, *Location.ToString());

    // Create crowd formation around the spawn location
    TArray<FVector> SpawnPositions;
    GenerateCrowdFormation(Location, ActualGroupSize, Params, SpawnPositions);

    // Spawn entities using Mass Entity system
    for (int32 i = 0; i < SpawnPositions.Num(); i++)
    {
        FMassEntityHandle NewEntity = SpawnDinosaurEntity(SpawnPositions[i], Params);
        if (NewEntity.IsValid())
        {
            ActiveCrowds.Add(NewEntity);
            CrowdParameters.Add(NewEntity, Params);
        }
    }

    UE_LOG(LogCrowdSimulation, Log, TEXT("Successfully spawned crowd with %d entities"), SpawnPositions.Num());
}

void UCrowdSimulationSubsystem::TriggerPanicBehavior(const FVector& ThreatLocation, float PanicRadius)
{
    UE_LOG(LogCrowdSimulation, Log, TEXT("Triggering panic behavior at %s with radius %f"), 
           *ThreatLocation.ToString(), PanicRadius);

    int32 AffectedEntities = 0;

    for (const FMassEntityHandle& Entity : ActiveCrowds)
    {
        if (!Entity.IsValid()) continue;

        FVector EntityLocation = GetEntityLocation(Entity);
        float Distance = FVector::Dist(EntityLocation, ThreatLocation);

        if (Distance <= PanicRadius)
        {
            ApplyPanicBehavior(Entity, ThreatLocation);
            AffectedEntities++;
        }
    }

    UE_LOG(LogCrowdSimulation, Log, TEXT("Panic behavior applied to %d entities"), AffectedEntities);
}

void UCrowdSimulationSubsystem::StartMigrationEvent(int32 RouteIndex)
{
    if (!MigrationRoutes.IsValidIndex(RouteIndex))
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Invalid migration route index: %d"), RouteIndex);
        return;
    }

    if (ActiveMigrations.Contains(RouteIndex))
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Migration route %d is already active"), RouteIndex);
        return;
    }

    MigrationRoutes[RouteIndex].bIsActive = true;
    ActiveMigrations.Add(RouteIndex);

    UE_LOG(LogCrowdSimulation, Log, TEXT("Started migration event on route %d"), RouteIndex);

    // Spawn migration herds along the route
    SpawnMigrationHerds(RouteIndex);
}

void UCrowdSimulationSubsystem::StopMigrationEvent(int32 RouteIndex)
{
    if (!MigrationRoutes.IsValidIndex(RouteIndex))
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Invalid migration route index: %d"), RouteIndex);
        return;
    }

    MigrationRoutes[RouteIndex].bIsActive = false;
    ActiveMigrations.Remove(RouteIndex);

    UE_LOG(LogCrowdSimulation, Log, TEXT("Stopped migration event on route %d"), RouteIndex);
}

void UCrowdSimulationSubsystem::SetGlobalCrowdDensity(float DensityMultiplier)
{
    GlobalDensityMultiplier = FMath::Clamp(DensityMultiplier, 0.1f, 5.0f);
    UE_LOG(LogCrowdSimulation, Log, TEXT("Global crowd density set to %f"), GlobalDensityMultiplier);
}

void UCrowdSimulationSubsystem::RegisterMigrationRoute(const FMigrationRoute& Route)
{
    MigrationRoutes.Add(Route);
    UE_LOG(LogCrowdSimulation, Log, TEXT("Registered new migration route with %d waypoints"), Route.Waypoints.Num());
}

int32 UCrowdSimulationSubsystem::GetActiveCrowdCount() const
{
    return ActiveCrowds.Num();
}

TArray<FVector> UCrowdSimulationSubsystem::GetNearbyGroupCenters(const FVector& Location, float Radius) const
{
    TArray<FVector> GroupCenters;
    
    // Calculate group centers from active crowds
    TMap<int32, TArray<FVector>> GroupPositions;
    
    for (const FMassEntityHandle& Entity : ActiveCrowds)
    {
        if (!Entity.IsValid()) continue;
        
        FVector EntityLocation = GetEntityLocation(Entity);
        if (FVector::Dist(EntityLocation, Location) <= Radius)
        {
            // Group entities by proximity to find group centers
            bool bFoundGroup = false;
            for (auto& Group : GroupPositions)
            {
                if (Group.Value.Num() > 0)
                {
                    FVector GroupCenter = CalculateGroupCenter(Group.Value);
                    if (FVector::Dist(EntityLocation, GroupCenter) <= 1000.0f)
                    {
                        Group.Value.Add(EntityLocation);
                        bFoundGroup = true;
                        break;
                    }
                }
            }
            
            if (!bFoundGroup)
            {
                int32 NewGroupIndex = GroupPositions.Num();
                GroupPositions.Add(NewGroupIndex, {EntityLocation});
            }
        }
    }
    
    // Calculate final group centers
    for (const auto& Group : GroupPositions)
    {
        if (Group.Value.Num() >= 3) // Only consider groups with 3+ members
        {
            GroupCenters.Add(CalculateGroupCenter(Group.Value));
        }
    }
    
    return GroupCenters;
}

bool UCrowdSimulationSubsystem::IsLocationInMigrationPath(const FVector& Location) const
{
    for (int32 RouteIndex : ActiveMigrations)
    {
        if (!MigrationRoutes.IsValidIndex(RouteIndex)) continue;
        
        const FMigrationRoute& Route = MigrationRoutes[RouteIndex];
        
        for (int32 i = 0; i < Route.Waypoints.Num() - 1; i++)
        {
            FVector SegmentStart = Route.Waypoints[i];
            FVector SegmentEnd = Route.Waypoints[i + 1];
            
            float DistanceToSegment = FMath::PointDistToSegment(Location, SegmentStart, SegmentEnd);
            
            if (DistanceToSegment <= Route.RouteWidth)
            {
                return true;
            }
        }
    }
    
    return false;
}

void UCrowdSimulationSubsystem::UpdateCrowdBehaviors(float DeltaTime)
{
    if (!MassEntitySubsystem) return;

    LastUpdateTime += DeltaTime;

    // Clean up invalid entities
    CleanupInactiveCrowds();

    // Update flocking behaviors for all active crowds
    for (const FMassEntityHandle& Entity : ActiveCrowds)
    {
        if (!Entity.IsValid()) continue;

        const FDinosaurCrowdParams* Params = CrowdParameters.Find(Entity);
        if (!Params) continue;

        // Calculate and apply flocking forces
        FVector FlockingForce = CalculateFlockingForce(Entity, *Params);
        ApplyMovementForce(Entity, FlockingForce);
    }

    // Process active migrations
    ProcessMigrations(DeltaTime);
}

void UCrowdSimulationSubsystem::ProcessMigrations(float DeltaTime)
{
    for (int32 RouteIndex : ActiveMigrations)
    {
        if (!MigrationRoutes.IsValidIndex(RouteIndex)) continue;
        
        FMigrationRoute& Route = MigrationRoutes[RouteIndex];
        
        // Update migration progress and move herds along routes
        UpdateMigrationProgress(RouteIndex, DeltaTime);
    }
}

void UCrowdSimulationSubsystem::CleanupInactiveCrowds()
{
    ActiveCrowds.RemoveAll([this](const FMassEntityHandle& Entity)
    {
        if (!Entity.IsValid() || !MassEntitySubsystem->IsEntityValid(Entity))
        {
            CrowdParameters.Remove(Entity);
            return true;
        }
        return false;
    });
}

FVector UCrowdSimulationSubsystem::CalculateFlockingForce(const FMassEntityHandle& Entity, const FDinosaurCrowdParams& Params)
{
    if (!Entity.IsValid()) return FVector::ZeroVector;

    FVector EntityLocation = GetEntityLocation(Entity);
    TArray<FMassEntityHandle> Neighbors = GetNearbyEntities(Entity, Params.AlignmentRadius);

    FVector CohesionForce = CalculateCohesion(Entity, Neighbors) * Params.CohesionWeight;
    FVector SeparationForce = CalculateSeparation(Entity, Neighbors, Params.SeparationRadius) * Params.SeparationWeight;
    FVector AlignmentForce = CalculateAlignment(Entity, Neighbors) * Params.AlignmentWeight;

    FVector TotalForce = CohesionForce + SeparationForce + AlignmentForce;
    
    // Clamp force magnitude
    if (TotalForce.SizeSquared() > FMath::Square(Params.MovementSpeed))
    {
        TotalForce = TotalForce.GetSafeNormal() * Params.MovementSpeed;
    }

    return TotalForce;
}

FVector UCrowdSimulationSubsystem::CalculateCohesion(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& Neighbors)
{
    if (Neighbors.Num() == 0) return FVector::ZeroVector;

    FVector EntityLocation = GetEntityLocation(Entity);
    FVector CenterOfMass = FVector::ZeroVector;

    for (const FMassEntityHandle& Neighbor : Neighbors)
    {
        if (Neighbor != Entity && Neighbor.IsValid())
        {
            CenterOfMass += GetEntityLocation(Neighbor);
        }
    }

    if (Neighbors.Num() > 1)
    {
        CenterOfMass /= (Neighbors.Num() - 1);
        return (CenterOfMass - EntityLocation).GetSafeNormal();
    }

    return FVector::ZeroVector;
}

FVector UCrowdSimulationSubsystem::CalculateSeparation(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& Neighbors, float SeparationRadius)
{
    FVector EntityLocation = GetEntityLocation(Entity);
    FVector SeparationForce = FVector::ZeroVector;

    for (const FMassEntityHandle& Neighbor : Neighbors)
    {
        if (Neighbor != Entity && Neighbor.IsValid())
        {
            FVector NeighborLocation = GetEntityLocation(Neighbor);
            FVector Difference = EntityLocation - NeighborLocation;
            float Distance = Difference.Size();

            if (Distance < SeparationRadius && Distance > 0.0f)
            {
                SeparationForce += Difference.GetSafeNormal() / Distance;
            }
        }
    }

    return SeparationForce.GetSafeNormal();
}

FVector UCrowdSimulationSubsystem::CalculateAlignment(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& Neighbors)
{
    if (Neighbors.Num() == 0) return FVector::ZeroVector;

    FVector AverageVelocity = FVector::ZeroVector;
    int32 ValidNeighbors = 0;

    for (const FMassEntityHandle& Neighbor : Neighbors)
    {
        if (Neighbor != Entity && Neighbor.IsValid())
        {
            FVector NeighborVelocity = GetEntityVelocity(Neighbor);
            AverageVelocity += NeighborVelocity;
            ValidNeighbors++;
        }
    }

    if (ValidNeighbors > 0)
    {
        AverageVelocity /= ValidNeighbors;
        return AverageVelocity.GetSafeNormal();
    }

    return FVector::ZeroVector;
}

// Helper functions implementation
void UCrowdSimulationSubsystem::InitializeDefaultMigrationRoutes()
{
    // Create default migration routes across the map
    FMigrationRoute NorthSouthRoute;
    NorthSouthRoute.Waypoints = {
        FVector(0, -50000, 0),
        FVector(0, -25000, 0),
        FVector(0, 0, 0),
        FVector(0, 25000, 0),
        FVector(0, 50000, 0)
    };
    NorthSouthRoute.RouteWidth = 3000.0f;
    NorthSouthRoute.SeasonStartDay = 0.0f;
    NorthSouthRoute.SeasonEndDay = 90.0f;
    MigrationRoutes.Add(NorthSouthRoute);

    FMigrationRoute EastWestRoute;
    EastWestRoute.Waypoints = {
        FVector(-50000, 0, 0),
        FVector(-25000, 0, 0),
        FVector(0, 0, 0),
        FVector(25000, 0, 0),
        FVector(50000, 0, 0)
    };
    EastWestRoute.RouteWidth = 2500.0f;
    EastWestRoute.SeasonStartDay = 45.0f;
    EastWestRoute.SeasonEndDay = 135.0f;
    MigrationRoutes.Add(EastWestRoute);
}

void UCrowdSimulationSubsystem::GenerateCrowdFormation(const FVector& CenterLocation, int32 GroupSize, const FDinosaurCrowdParams& Params, TArray<FVector>& OutPositions)
{
    OutPositions.Empty();
    OutPositions.Reserve(GroupSize);

    // Generate positions in a natural cluster formation
    for (int32 i = 0; i < GroupSize; i++)
    {
        float Angle = (2.0f * PI * i) / GroupSize + FMath::RandRange(-0.5f, 0.5f);
        float Distance = FMath::RandRange(Params.SeparationRadius * 0.5f, Params.CohesionRadius * 0.8f);
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        OutPositions.Add(CenterLocation + Offset);
    }
}

FMassEntityHandle UCrowdSimulationSubsystem::SpawnDinosaurEntity(const FVector& Location, const FDinosaurCrowdParams& Params)
{
    // This would integrate with the Mass Entity spawning system
    // For now, return an invalid handle as placeholder
    return FMassEntityHandle();
}

FVector UCrowdSimulationSubsystem::GetEntityLocation(const FMassEntityHandle& Entity) const
{
    // This would query the Mass Entity system for the entity's transform
    return FVector::ZeroVector;
}

FVector UCrowdSimulationSubsystem::GetEntityVelocity(const FMassEntityHandle& Entity) const
{
    // This would query the Mass Entity system for the entity's velocity
    return FVector::ZeroVector;
}

TArray<FMassEntityHandle> UCrowdSimulationSubsystem::GetNearbyEntities(const FMassEntityHandle& Entity, float Radius) const
{
    // This would query the Mass Entity system for nearby entities
    return TArray<FMassEntityHandle>();
}

void UCrowdSimulationSubsystem::ApplyMovementForce(const FMassEntityHandle& Entity, const FVector& Force)
{
    // This would apply the force to the Mass Entity's movement component
}

void UCrowdSimulationSubsystem::ApplyPanicBehavior(const FMassEntityHandle& Entity, const FVector& ThreatLocation)
{
    // This would modify the entity's behavior to flee from the threat
}

void UCrowdSimulationSubsystem::SpawnMigrationHerds(int32 RouteIndex)
{
    // This would spawn large herds along the migration route
}

void UCrowdSimulationSubsystem::UpdateMigrationProgress(int32 RouteIndex, float DeltaTime)
{
    // This would update the progress of migrating herds along their routes
}

FVector UCrowdSimulationSubsystem::CalculateGroupCenter(const TArray<FVector>& Positions) const
{
    if (Positions.Num() == 0) return FVector::ZeroVector;
    
    FVector Center = FVector::ZeroVector;
    for (const FVector& Position : Positions)
    {
        Center += Position;
    }
    return Center / Positions.Num();
}