#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxCrowdSize = 50000;
    CrowdDensity = 1.0f;
    LODDistance1 = 5000.0f;
    LODDistance2 = 15000.0f;
    CullingDistance = 25000.0f;
    
    MassEntitySubsystem = nullptr;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassEntity();
    
    // Create initial herds in different biomes
    SpawnHerd(EDinosaurSpecies::Triceratops, FVector(0, 0, 100), 50);
    SpawnHerd(EDinosaurSpecies::Parasaurolophus, FVector(-45000, 40000, 100), 30);
    SpawnHerd(EDinosaurSpecies::Brachiosaurus, FVector(-45000, 40000, 100), 15);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager initialized with %d herds"), ActiveHerds.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update all active herds
    for (int32 i = 0; i < ActiveHerds.Num(); i++)
    {
        UpdateHerdBehavior(i, DeltaTime);
    }
    
    // Update LOD system
    UpdateLOD();
}

void ACrowdSimulationManager::InitializeMassEntity()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity subsystem initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Mass Entity subsystem"));
        }
    }
}

void ACrowdSimulationManager::SpawnHerd(EDinosaurSpecies Species, FVector Location, int32 HerdSize)
{
    FCrowd_HerdData NewHerd;
    NewHerd.Species = Species;
    NewHerd.HerdCenter = Location;
    NewHerd.HerdSize = HerdSize;
    NewHerd.HerdRadius = FMath::Sqrt(HerdSize) * 100.0f; // Scale radius with herd size
    
    // Create Mass Entity representation
    CreateHerdEntity(NewHerd);
    
    ActiveHerds.Add(NewHerd);
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned herd of %d %s at location %s"), 
           HerdSize, 
           *UEnum::GetValueAsString(Species),
           *Location.ToString());
}

void ACrowdSimulationManager::CreateHerdEntity(const FCrowd_HerdData& HerdData)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create entities for each herd member
    for (int32 i = 0; i < HerdData.HerdSize; i++)
    {
        // Generate random position within herd radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-HerdData.HerdRadius, HerdData.HerdRadius),
            FMath::RandRange(-HerdData.HerdRadius, HerdData.HerdRadius),
            0
        );
        
        FVector SpawnLocation = HerdData.HerdCenter + RandomOffset;
        
        // For now, we'll use a simplified approach without full Mass Entity setup
        // In a full implementation, this would create Mass Entity archetypes
        UE_LOG(LogTemp, Log, TEXT("Created herd member %d at %s"), i, *SpawnLocation.ToString());
    }
}

void ACrowdSimulationManager::StartMigration(int32 HerdIndex, const FCrowd_MigrationRoute& Route)
{
    if (ActiveHerds.IsValidIndex(HerdIndex))
    {
        MigrationRoutes.Add(Route);
        UE_LOG(LogTemp, Warning, TEXT("Started migration for herd %d with %d waypoints"), 
               HerdIndex, Route.Waypoints.Num());
    }
}

void ACrowdSimulationManager::UpdateHerdBehavior(int32 HerdIndex, float DeltaTime)
{
    if (!ActiveHerds.IsValidIndex(HerdIndex))
    {
        return;
    }
    
    FCrowd_HerdData& Herd = ActiveHerds[HerdIndex];
    
    // Update herd movement
    UpdateHerdMovement(Herd, DeltaTime);
    
    // Apply flocking behavior
    ApplyFlockingBehavior(Herd);
    
    // Handle LOD for this herd
    HandleCrowdLOD(Herd);
}

void ACrowdSimulationManager::UpdateHerdMovement(FCrowd_HerdData& HerdData, float DeltaTime)
{
    // Simple wandering behavior - move herd center slightly
    FVector WanderDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0
    ).GetSafeNormal();
    
    float WanderSpeed = 50.0f; // Units per second
    HerdData.HerdCenter += WanderDirection * WanderSpeed * DeltaTime;
}

void ACrowdSimulationManager::ApplyFlockingBehavior(FCrowd_HerdData& HerdData)
{
    // Implement basic flocking rules:
    // 1. Separation - avoid crowding neighbors
    // 2. Alignment - steer towards average heading of neighbors  
    // 3. Cohesion - steer towards average position of neighbors
    
    // For now, just ensure herd stays within radius
    // In full implementation, this would update individual entity positions
}

void ACrowdSimulationManager::HandleCrowdLOD(const FCrowd_HerdData& HerdData)
{
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float DistanceToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), HerdData.HerdCenter);
        
        // Apply different LOD levels based on distance
        if (DistanceToPlayer > CullingDistance)
        {
            // Cull completely - no rendering or simulation
        }
        else if (DistanceToPlayer > LODDistance2)
        {
            // LOD 2 - Very low detail, minimal simulation
        }
        else if (DistanceToPlayer > LODDistance1)
        {
            // LOD 1 - Medium detail
        }
        else
        {
            // LOD 0 - Full detail
        }
    }
}

void ACrowdSimulationManager::UpdateLOD()
{
    // Update LOD for all herds based on player distance
    for (const FCrowd_HerdData& Herd : ActiveHerds)
    {
        HandleCrowdLOD(Herd);
    }
}

int32 ACrowdSimulationManager::GetTotalCrowdCount() const
{
    int32 TotalCount = 0;
    for (const FCrowd_HerdData& Herd : ActiveHerds)
    {
        TotalCount += Herd.HerdSize;
    }
    return TotalCount;
}