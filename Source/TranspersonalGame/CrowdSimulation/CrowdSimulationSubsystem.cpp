#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassSpawner.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing..."));
    
    InitializeMassFramework();
    
    // Initialize default migration routes if none are set
    if (MigrationRoutes.IsEmpty())
    {
        // Create a basic circular migration route for herbivore herds
        FMigrationRoute HerbivoreRoute;
        HerbivoreRoute.RouteName = TEXT("HerbivoreCircuit");
        HerbivoreRoute.Waypoints = {
            FVector(0, 0, 0),
            FVector(5000, 0, 0),
            FVector(5000, 5000, 0),
            FVector(0, 5000, 0)
        };
        HerbivoreRoute.SeasonalTrigger = 0.2f;
        HerbivoreRoute.ParticipatingGroups = {TEXT("Triceratops"), TEXT("Parasaurolophus")};
        HerbivoreRoute.MigrationSpeed = 150.0f;
        
        MigrationRoutes.Add(HerbivoreRoute);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Deinitializing..."));
    
    // Clean up all active groups
    for (auto& GroupPair : ActiveGroups)
    {
        for (const FMassEntityHandle& Entity : GroupPair.Value)
        {
            if (MassEntitySubsystem && Entity.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
    }
    ActiveGroups.Empty();
    
    Super::Deinitialize();
}

void UCrowdSimulationSubsystem::Tick(float DeltaTime)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UpdateMigrations(DeltaTime);
    UpdatePopulationLOD(DeltaTime);
    ProcessEmergencyResponses(DeltaTime);
}

void UCrowdSimulationSubsystem::InitializeMassFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: No valid world found"));
        return;
    }
    
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get MassEntitySubsystem"));
    }
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get MassSpawnerSubsystem"));
    }
}

void UCrowdSimulationSubsystem::SpawnCrowdGroup(const FName& GroupType, const FVector& Location, int32 GroupSize)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Cannot spawn group - MassEntitySubsystem not available"));
        return;
    }
    
    FCrowdGroupConfig* GroupConfig = GetGroupConfig(GroupType);
    if (!GroupConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: No configuration found for group type: %s"), *GroupType.ToString());
        return;
    }
    
    // Determine actual group size
    int32 ActualGroupSize = GroupSize > 0 ? GroupSize : FMath::RandRange(GroupConfig->MinGroupSize, GroupConfig->MaxGroupSize);
    
    // Find optimal spawn location
    FVector SpawnLocation = FindOptimalSpawnLocation(*GroupConfig, Location);
    
    // Create group entities
    TArray<FMassEntityHandle> GroupEntities;
    
    for (int32 i = 0; i < ActualGroupSize; ++i)
    {
        // Calculate spawn position within group formation
        float Angle = (2.0f * PI * i) / ActualGroupSize;
        float Radius = FMath::Sqrt(i) * 50.0f; // Spiral formation
        
        FVector EntityLocation = SpawnLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        // Create entity (this would normally use the Mass spawning system)
        FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
        if (NewEntity.IsValid())
        {
            GroupEntities.Add(NewEntity);
            
            // Here we would add components like Transform, Movement, GroupBehavior, etc.
            // This is simplified for the example
        }
    }
    
    // Register the group
    if (!ActiveGroups.Contains(GroupType))
    {
        ActiveGroups.Add(GroupType, TArray<FMassEntityHandle>());
    }
    ActiveGroups[GroupType].Append(GroupEntities);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Spawned group %s with %d entities at %s"), 
           *GroupType.ToString(), ActualGroupSize, *SpawnLocation.ToString());
}

void UCrowdSimulationSubsystem::DespawnCrowdGroup(const FMassEntityHandle& GroupLeader)
{
    if (!MassEntitySubsystem || !GroupLeader.IsValid())
    {
        return;
    }
    
    // Find and remove the group
    for (auto& GroupPair : ActiveGroups)
    {
        TArray<FMassEntityHandle>& Entities = GroupPair.Value;
        
        int32 LeaderIndex = Entities.Find(GroupLeader);
        if (LeaderIndex != INDEX_NONE)
        {
            // Remove all entities in this group
            for (const FMassEntityHandle& Entity : Entities)
            {
                if (Entity.IsValid())
                {
                    MassEntitySubsystem->DestroyEntity(Entity);
                }
            }
            
            Entities.Empty();
            break;
        }
    }
}

void UCrowdSimulationSubsystem::UpdateGroupDensity(const FName& BiomeName, float DensityMultiplier)
{
    // This would adjust spawning rates for specific biomes
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Updated density for biome %s to %f"), 
           *BiomeName.ToString(), DensityMultiplier);
}

void UCrowdSimulationSubsystem::StartMigration(const FName& RouteName)
{
    for (FMigrationRoute& Route : MigrationRoutes)
    {
        if (Route.RouteName == RouteName)
        {
            Route.bIsActive = true;
            UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Started migration route: %s"), *RouteName.ToString());
            break;
        }
    }
}

void UCrowdSimulationSubsystem::StopMigration(const FName& RouteName)
{
    for (FMigrationRoute& Route : MigrationRoutes)
    {
        if (Route.RouteName == RouteName)
        {
            Route.bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Stopped migration route: %s"), *RouteName.ToString());
            break;
        }
    }
}

void UCrowdSimulationSubsystem::UpdateSeasonalCycle(float SeasonProgress)
{
    CurrentSeasonProgress = FMath::Clamp(SeasonProgress, 0.0f, 1.0f);
    
    // Check if any migrations should be triggered
    for (FMigrationRoute& Route : MigrationRoutes)
    {
        bool ShouldBeActive = CurrentSeasonProgress >= Route.SeasonalTrigger && 
                             CurrentSeasonProgress <= (Route.SeasonalTrigger + 0.3f); // Migration lasts 30% of season cycle
        
        if (ShouldBeActive && !Route.bIsActive)
        {
            StartMigration(Route.RouteName);
        }
        else if (!ShouldBeActive && Route.bIsActive)
        {
            StopMigration(Route.RouteName);
        }
    }
}

void UCrowdSimulationSubsystem::TriggerPanicResponse(const FVector& ThreatLocation, float ThreatRadius, float ThreatIntensity)
{
    FEmergencyResponse NewResponse;
    NewResponse.Location = ThreatLocation;
    NewResponse.Radius = ThreatRadius;
    NewResponse.Intensity = FMath::Clamp(ThreatIntensity, 0.0f, 1.0f);
    NewResponse.TimeRemaining = 10.0f * ThreatIntensity; // Panic lasts longer for more intense threats
    NewResponse.bIsStampede = false;
    
    ActiveEmergencyResponses.Add(NewResponse);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Panic response triggered at %s with radius %f"), 
           *ThreatLocation.ToString(), ThreatRadius);
}

void UCrowdSimulationSubsystem::TriggerStampedeResponse(const FVector& StampedeOrigin, const FVector& StampedeDirection)
{
    FEmergencyResponse NewResponse;
    NewResponse.Location = StampedeOrigin;
    NewResponse.Radius = 2000.0f; // Large radius for stampede
    NewResponse.Intensity = 1.0f; // Maximum intensity
    NewResponse.TimeRemaining = 30.0f; // Stampedes last longer
    NewResponse.bIsStampede = true;
    NewResponse.StampedeDirection = StampedeDirection.GetSafeNormal();
    
    ActiveEmergencyResponses.Add(NewResponse);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Stampede triggered at %s in direction %s"), 
           *StampedeOrigin.ToString(), *StampedeDirection.ToString());
}

void UCrowdSimulationSubsystem::SetGlobalPopulationScale(float Scale)
{
    GlobalPopulationScale = FMath::Max(0.0f, Scale);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Global population scale set to %f"), GlobalPopulationScale);
}

int32 UCrowdSimulationSubsystem::GetActiveEntityCount() const
{
    int32 TotalCount = 0;
    for (const auto& GroupPair : ActiveGroups)
    {
        TotalCount += GroupPair.Value.Num();
    }
    return TotalCount;
}

TArray<FMassEntityHandle> UCrowdSimulationSubsystem::GetEntitiesInRadius(const FVector& Center, float Radius) const
{
    TArray<FMassEntityHandle> EntitiesInRadius;
    
    // This is a simplified implementation
    // In practice, this would use spatial partitioning for efficiency
    for (const auto& GroupPair : ActiveGroups)
    {
        for (const FMassEntityHandle& Entity : GroupPair.Value)
        {
            // Would check actual entity position here
            EntitiesInRadius.Add(Entity);
        }
    }
    
    return EntitiesInRadius;
}

void UCrowdSimulationSubsystem::UpdateMigrations(float DeltaTime)
{
    for (const FMigrationRoute& Route : MigrationRoutes)
    {
        if (!Route.bIsActive)
        {
            continue;
        }
        
        // Update entities participating in this migration
        for (const FName& GroupName : Route.ParticipatingGroups)
        {
            if (ActiveGroups.Contains(GroupName))
            {
                // Move entities along migration route
                // This would involve updating their movement targets
            }
        }
    }
}

void UCrowdSimulationSubsystem::UpdatePopulationLOD(float DeltaTime)
{
    LastLODUpdateTime += DeltaTime;
    if (LastLODUpdateTime < LODUpdateInterval)
    {
        return;
    }
    
    LastLODUpdateTime = 0.0f;
    
    // Get player location for distance-based LOD
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for all active groups based on distance from player
    for (auto& GroupPair : ActiveGroups)
    {
        TArray<FMassEntityHandle>& Entities = GroupPair.Value;
        
        for (const FMassEntityHandle& Entity : Entities)
        {
            if (!Entity.IsValid())
            {
                continue;
            }
            
            // Calculate distance and set appropriate LOD
            // This would involve updating Mass LOD components
        }
    }
}

void UCrowdSimulationSubsystem::ProcessEmergencyResponses(float DeltaTime)
{
    for (int32 i = ActiveEmergencyResponses.Num() - 1; i >= 0; --i)
    {
        FEmergencyResponse& Response = ActiveEmergencyResponses[i];
        Response.TimeRemaining -= DeltaTime;
        
        if (Response.TimeRemaining <= 0.0f)
        {
            ActiveEmergencyResponses.RemoveAt(i);
            continue;
        }
        
        // Apply emergency response to entities in range
        TArray<FMassEntityHandle> AffectedEntities = GetEntitiesInRadius(Response.Location, Response.Radius);
        
        for (const FMassEntityHandle& Entity : AffectedEntities)
        {
            if (!Entity.IsValid())
            {
                continue;
            }
            
            // Apply panic or stampede behavior
            // This would modify entity movement and behavior components
        }
    }
}

FCrowdGroupConfig* UCrowdSimulationSubsystem::GetGroupConfig(const FName& GroupType)
{
    if (!CrowdGroupsDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: CrowdGroupsDataTable is not set"));
        return nullptr;
    }
    
    return CrowdGroupsDataTable->FindRow<FCrowdGroupConfig>(GroupType, TEXT("GetGroupConfig"));
}

FVector UCrowdSimulationSubsystem::FindOptimalSpawnLocation(const FCrowdGroupConfig& Config, const FVector& PreferredLocation)
{
    // Simple implementation - in practice this would check terrain, biomes, obstacles, etc.
    FVector OptimalLocation = PreferredLocation;
    
    // Add some random offset to avoid overlapping spawns
    OptimalLocation += FVector(
        FMath::RandRange(-500.0f, 500.0f),
        FMath::RandRange(-500.0f, 500.0f),
        0.0f
    );
    
    return OptimalLocation;
}