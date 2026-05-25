#include "Crowd_MassSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "DrawDebugHelpers.h"

UCrowd_MassSimulationManager::UCrowd_MassSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default configuration
    EntityConfig.MaxEntities = 1000;
    EntityConfig.SpawnRadius = 5000.0f;
    EntityConfig.MovementSpeed = 200.0f;
    EntityConfig.WanderRadius = 1000.0f;
    
    UpdateFrequency = 0.1f;
    CullDistance = 10000.0f;
    TotalActiveEntities = 0;
    bSimulationActive = false;
    LastUpdateTime = 0.0f;
    MassEntitySubsystem = nullptr;
}

void UCrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get Mass Entity subsystem reference
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass Entity Subsystem initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Failed to get Mass Entity Subsystem"));
        }
    }
    
    // Initialize biome populations
    InitializeBiomePopulations();
    
    // Start simulation
    InitializeMassSimulation();
}

void UCrowd_MassSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bSimulationActive)
    {
        LastUpdateTime += DeltaTime;
        
        if (LastUpdateTime >= UpdateFrequency)
        {
            UpdateCrowdBehavior(DeltaTime);
            UpdateLODLevels();
            CullDistantEntities();
            LastUpdateTime = 0.0f;
        }
    }
}

void UCrowd_MassSimulationManager::InitializeMassSimulation()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Cannot initialize - Mass Entity Subsystem not available"));
        return;
    }
    
    bSimulationActive = true;
    TotalActiveEntities = 0;
    
    // Populate default biomes
    PopulateBiome(EBiomeType::Savanna, 200);
    PopulateBiome(EBiomeType::Forest, 150);
    PopulateBiome(EBiomeType::Desert, 100);
    PopulateBiome(EBiomeType::Swamp, 120);
    PopulateBiome(EBiomeType::Mountain, 80);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass simulation initialized with %d total entities"), TotalActiveEntities);
}

void UCrowd_MassSimulationManager::SpawnCrowdEntities(const FVector& Location, int32 Count)
{
    if (!MassEntitySubsystem || Count <= 0)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn entities in a radius around the location
    for (int32 i = 0; i < Count; ++i)
    {
        // Generate random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, EntityConfig.SpawnRadius);
        
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Adjust Z to ground level
        SpawnLocation.Z = Location.Z;
        
        // For now, spawn basic pawns as crowd entities
        // TODO: Replace with Mass Entity spawning when Mass framework is fully integrated
        if (APawn* CrowdEntity = World->SpawnActor<APawn>(SpawnLocation, FRotator::ZeroRotator))
        {
            CrowdEntity->Tags.Add(FName("CrowdEntity"));
            TotalActiveEntities++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Spawned %d crowd entities at %s"), Count, *Location.ToString());
}

void UCrowd_MassSimulationManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (!bSimulationActive)
    {
        return;
    }
    
    // Update entity movement
    UpdateEntityMovement(DeltaTime);
    
    // Update biome populations
    UpdateBiomePopulations();
}

void UCrowd_MassSimulationManager::SetCrowdDestination(const FVector& Destination)
{
    // TODO: Implement crowd pathfinding to destination
    // For now, log the destination change
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Setting crowd destination to %s"), *Destination.ToString());
}

void UCrowd_MassSimulationManager::PopulateBiome(EBiomeType BiomeType, int32 EntityCount)
{
    FVector BiomeCenter = GetRandomPositionInBiome(BiomeType);
    
    // Find or create biome population entry
    FCrowd_BiomePopulation* BiomePopulation = nullptr;
    for (FCrowd_BiomePopulation& Population : BiomePopulations)
    {
        if (Population.BiomeType == BiomeType)
        {
            BiomePopulation = &Population;
            break;
        }
    }
    
    if (!BiomePopulation)
    {
        FCrowd_BiomePopulation NewPopulation;
        NewPopulation.BiomeType = BiomeType;
        NewPopulation.BiomeCenter = BiomeCenter;
        NewPopulation.PopulationDensity = EntityCount;
        BiomePopulations.Add(NewPopulation);
        BiomePopulation = &BiomePopulations.Last();
    }
    
    // Spawn entities in this biome
    SpawnCrowdEntities(BiomeCenter, EntityCount);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Populated biome %d with %d entities"), 
           (int32)BiomeType, EntityCount);
}

void UCrowd_MassSimulationManager::UpdateBiomePopulations()
{
    for (FCrowd_BiomePopulation& Population : BiomePopulations)
    {
        // TODO: Implement biome-specific behavior updates
        // For now, just maintain population counts
    }
}

void UCrowd_MassSimulationManager::UpdateLODLevels()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // TODO: Implement proper Mass Entity LOD system
    // For now, use basic distance-based culling
    ProcessLODUpdates();
}

void UCrowd_MassSimulationManager::CullDistantEntities()
{
    if (!GetWorld())
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Find and cull distant crowd entities
    TArray<AActor*> CrowdEntities;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("CrowdEntity"), CrowdEntities);
    
    int32 CulledCount = 0;
    for (AActor* Entity : CrowdEntities)
    {
        if (Entity && FVector::Dist(Entity->GetActorLocation(), PlayerLocation) > CullDistance)
        {
            Entity->Destroy();
            CulledCount++;
            TotalActiveEntities--;
        }
    }
    
    if (CulledCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Culled %d distant entities"), CulledCount);
    }
}

int32 UCrowd_MassSimulationManager::GetActiveEntityCount() const
{
    return TotalActiveEntities;
}

TArray<FVector> UCrowd_MassSimulationManager::GetEntityPositions() const
{
    TArray<FVector> Positions;
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> CrowdEntities;
        UGameplayStatics::GetAllActorsWithTag(World, FName("CrowdEntity"), CrowdEntities);
        
        for (AActor* Entity : CrowdEntities)
        {
            if (Entity)
            {
                Positions.Add(Entity->GetActorLocation());
            }
        }
    }
    
    return Positions;
}

void UCrowd_MassSimulationManager::DebugSpawnTestCrowd()
{
    if (UWorld* World = GetWorld())
    {
        FVector SpawnLocation = FVector(0.0f, 0.0f, 100.0f);
        SpawnCrowdEntities(SpawnLocation, 50);
        
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: DEBUG - Spawned test crowd of 50 entities"));
    }
}

void UCrowd_MassSimulationManager::InitializeBiomePopulations()
{
    BiomePopulations.Empty();
    
    // Initialize biome population configurations
    FCrowd_BiomePopulation SavannaPopulation;
    SavannaPopulation.BiomeType = EBiomeType::Savanna;
    SavannaPopulation.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    SavannaPopulation.PopulationDensity = 200;
    BiomePopulations.Add(SavannaPopulation);
    
    FCrowd_BiomePopulation ForestPopulation;
    ForestPopulation.BiomeType = EBiomeType::Forest;
    ForestPopulation.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestPopulation.PopulationDensity = 150;
    BiomePopulations.Add(ForestPopulation);
    
    FCrowd_BiomePopulation DesertPopulation;
    DesertPopulation.BiomeType = EBiomeType::Desert;
    DesertPopulation.BiomeCenter = FVector(55000.0f, 0.0f, 100.0f);
    DesertPopulation.PopulationDensity = 100;
    BiomePopulations.Add(DesertPopulation);
    
    FCrowd_BiomePopulation SwampPopulation;
    SwampPopulation.BiomeType = EBiomeType::Swamp;
    SwampPopulation.BiomeCenter = FVector(-50000.0f, -45000.0f, 100.0f);
    SwampPopulation.PopulationDensity = 120;
    BiomePopulations.Add(SwampPopulation);
    
    FCrowd_BiomePopulation MountainPopulation;
    MountainPopulation.BiomeType = EBiomeType::Mountain;
    MountainPopulation.BiomeCenter = FVector(40000.0f, 50000.0f, 100.0f);
    MountainPopulation.PopulationDensity = 80;
    BiomePopulations.Add(MountainPopulation);
}

void UCrowd_MassSimulationManager::UpdateEntityMovement(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> CrowdEntities;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("CrowdEntity"), CrowdEntities);
    
    for (AActor* Entity : CrowdEntities)
    {
        if (Entity)
        {
            // Simple wandering behavior
            FVector CurrentLocation = Entity->GetActorLocation();
            FVector RandomDirection = FMath::VRand();
            RandomDirection.Z = 0.0f; // Keep on ground
            RandomDirection.Normalize();
            
            FVector NewLocation = CurrentLocation + (RandomDirection * EntityConfig.MovementSpeed * DeltaTime);
            Entity->SetActorLocation(NewLocation);
        }
    }
}

void UCrowd_MassSimulationManager::ProcessLODUpdates()
{
    // TODO: Implement proper Mass Entity LOD processing
    // This is a placeholder for the LOD system
}

FVector UCrowd_MassSimulationManager::GetRandomPositionInBiome(EBiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

bool UCrowd_MassSimulationManager::IsEntityInCullRange(const FVector& EntityPosition) const
{
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            float Distance = FVector::Dist(EntityPosition, PlayerPawn->GetActorLocation());
            return Distance <= CullDistance;
        }
    }
    return false;
}