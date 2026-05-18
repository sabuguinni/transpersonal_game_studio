#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
    : MassEntitySubsystem(nullptr)
    , SimulationTickRate(DefaultTickRate)
    , MaxEntitiesPerFrame(DefaultMaxEntitiesPerFrame)
    , bIsSimulationActive(false)
    , CurrentBehaviorMode(0)
    , LastUpdateTime(0.0f)
    , EntitiesProcessedThisFrame(0)
{
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Initializing"));
    
    // Get Mass Entity subsystem reference
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Mass Entity subsystem found"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntitySubsystem: Mass Entity subsystem not found"));
        }
    }
    
    // Initialize default biomes
    FCrowd_BiomeSettings SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    SavanaBiome.BiomeRadius = 10000.0f;
    SavanaBiome.MaxCrowdSize = 500;
    SavanaBiome.SpawnDensity = 0.05f;
    RegisteredBiomes.Add(SavanaBiome);
    
    FCrowd_BiomeSettings PantanoBiome;
    PantanoBiome.BiomeName = TEXT("Pantano");
    PantanoBiome.BiomeCenter = FVector(-50000.0f, -45000.0f, 100.0f);
    PantanoBiome.BiomeRadius = 8000.0f;
    PantanoBiome.MaxCrowdSize = 300;
    PantanoBiome.SpawnDensity = 0.03f;
    RegisteredBiomes.Add(PantanoBiome);
    
    FCrowd_BiomeSettings FlorestaBiome;
    FlorestaBiome.BiomeName = TEXT("Floresta");
    FlorestaBiome.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    FlorestaBiome.BiomeRadius = 12000.0f;
    FlorestaBiome.MaxCrowdSize = 800;
    FlorestaBiome.SpawnDensity = 0.08f;
    RegisteredBiomes.Add(FlorestaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Initialized with %d biomes"), RegisteredBiomes.Num());
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Deinitializing"));
    
    ClearAllCrowds();
    RegisteredBiomes.Empty();
    MassEntitySubsystem = nullptr;
    
    Super::Deinitialize();
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Initializing crowd simulation"));
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntitySubsystem: Cannot initialize - Mass Entity subsystem not available"));
        return;
    }
    
    // Clear existing crowds
    ClearAllCrowds();
    
    // Spawn crowds in all registered biomes
    for (const FCrowd_BiomeSettings& Biome : RegisteredBiomes)
    {
        SpawnCrowdInBiome(Biome);
    }
    
    bIsSimulationActive = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Crowd simulation initialized with %d entities"), CrowdEntities.Num());
}

void UCrowd_MassEntitySubsystem::SpawnCrowdInBiome(const FCrowd_BiomeSettings& BiomeSettings)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Spawning crowd in biome %s"), *BiomeSettings.BiomeName);
    
    int32 EntitiesToSpawn = FMath::RoundToInt(BiomeSettings.MaxCrowdSize * BiomeSettings.SpawnDensity);
    
    for (int32 i = 0; i < EntitiesToSpawn; i++)
    {
        FCrowd_EntityData NewEntity;
        
        // Random position within biome radius
        FVector2D RandomCircle = FMath::RandPointInCircle(BiomeSettings.BiomeRadius);
        NewEntity.Position = BiomeSettings.BiomeCenter + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
        
        // Random initial velocity
        FVector RandomDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
        NewEntity.Velocity = RandomDirection * FMath::RandRange(50.0f, 200.0f);
        
        // Random target within biome
        FVector2D RandomTarget = FMath::RandPointInCircle(BiomeSettings.BiomeRadius * 0.8f);
        NewEntity.TargetLocation = BiomeSettings.BiomeCenter + FVector(RandomTarget.X, RandomTarget.Y, 0.0f);
        
        NewEntity.Speed = FMath::RandRange(100.0f, 250.0f);
        NewEntity.WanderRadius = FMath::RandRange(500.0f, 2000.0f);
        NewEntity.EntityID = CrowdEntities.Num();
        
        CrowdEntities.Add(NewEntity);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Spawned %d entities in biome %s"), EntitiesToSpawn, *BiomeSettings.BiomeName);
}

void UCrowd_MassEntitySubsystem::UpdateCrowdSimulation(float DeltaTime)
{
    if (!bIsSimulationActive || CrowdEntities.Num() == 0)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ActualDeltaTime = CurrentTime - LastUpdateTime;
    
    if (ActualDeltaTime < SimulationTickRate)
    {
        return; // Skip update if not enough time has passed
    }
    
    EntitiesProcessedThisFrame = 0;
    
    ProcessCrowdMovement(ActualDeltaTime);
    ProcessCrowdBehavior(ActualDeltaTime);
    UpdateEntityPositions(ActualDeltaTime);
    
    LastUpdateTime = CurrentTime;
}

void UCrowd_MassEntitySubsystem::ProcessCrowdMovement(float DeltaTime)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (EntitiesProcessedThisFrame >= MaxEntitiesPerFrame)
        {
            break; // Limit processing per frame for performance
        }
        
        // Calculate steering forces
        FVector FlockingForce = CalculateFlockingForce(Entity);
        FVector SeparationForce = CalculateSeparationForce(Entity);
        FVector WanderForce = CalculateWanderForce(Entity);
        
        // Combine forces
        FVector TotalForce = FlockingForce + SeparationForce + WanderForce;
        TotalForce = TotalForce.GetClampedToMaxSize(Entity.Speed);
        
        // Update velocity
        Entity.Velocity = (Entity.Velocity + TotalForce * DeltaTime).GetClampedToMaxSize(Entity.Speed);
        
        EntitiesProcessedThisFrame++;
    }
}

void UCrowd_MassEntitySubsystem::ProcessCrowdBehavior(float DeltaTime)
{
    // Behavior processing based on current behavior mode
    switch (CurrentBehaviorMode)
    {
        case 0: // Normal wandering
            // Already handled in movement processing
            break;
            
        case 1: // Flee behavior
            for (FCrowd_EntityData& Entity : CrowdEntities)
            {
                // Find nearest threat (player) and flee
                if (UWorld* World = GetWorld())
                {
                    if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
                    {
                        FVector PlayerLocation = PlayerPawn->GetActorLocation();
                        FVector FleeDirection = (Entity.Position - PlayerLocation).GetSafeNormal();
                        Entity.Velocity += FleeDirection * Entity.Speed * 2.0f * DeltaTime;
                    }
                }
            }
            break;
            
        case 2: // Gather behavior
            // Entities move toward biome center
            for (FCrowd_EntityData& Entity : CrowdEntities)
            {
                for (const FCrowd_BiomeSettings& Biome : RegisteredBiomes)
                {
                    if (IsEntityInBiome(Entity, Biome))
                    {
                        FVector GatherDirection = (Biome.BiomeCenter - Entity.Position).GetSafeNormal();
                        Entity.Velocity += GatherDirection * Entity.Speed * 0.5f * DeltaTime;
                        break;
                    }
                }
            }
            break;
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityPositions(float DeltaTime)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        // Update position
        Entity.Position += Entity.Velocity * DeltaTime;
        
        // Check biome boundaries and wrap/clamp if necessary
        bool bInAnyBiome = false;
        for (const FCrowd_BiomeSettings& Biome : RegisteredBiomes)
        {
            if (IsEntityInBiome(Entity, Biome))
            {
                bInAnyBiome = true;
                break;
            }
        }
        
        // If entity is outside all biomes, move it back to nearest biome
        if (!bInAnyBiome && RegisteredBiomes.Num() > 0)
        {
            const FCrowd_BiomeSettings& NearestBiome = RegisteredBiomes[0]; // Simplified - use first biome
            FVector ToBiome = (NearestBiome.BiomeCenter - Entity.Position).GetSafeNormal();
            Entity.Position = NearestBiome.BiomeCenter + ToBiome * (NearestBiome.BiomeRadius * 0.9f);
        }
    }
}

FVector UCrowd_MassEntitySubsystem::CalculateFlockingForce(const FCrowd_EntityData& Entity) const
{
    FVector FlockingForce = FVector::ZeroVector;
    FVector AveragePosition = FVector::ZeroVector;
    FVector AverageVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    float FlockingRadius = 500.0f;
    
    // Find neighbors and calculate average position and velocity
    for (const FCrowd_EntityData& OtherEntity : CrowdEntities)
    {
        if (OtherEntity.EntityID == Entity.EntityID)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Entity.Position, OtherEntity.Position);
        if (Distance < FlockingRadius)
        {
            AveragePosition += OtherEntity.Position;
            AverageVelocity += OtherEntity.Velocity;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AveragePosition /= NeighborCount;
        AverageVelocity /= NeighborCount;
        
        // Cohesion: move toward average position
        FVector CohesionForce = (AveragePosition - Entity.Position).GetSafeNormal() * 50.0f;
        
        // Alignment: match average velocity
        FVector AlignmentForce = (AverageVelocity - Entity.Velocity).GetSafeNormal() * 30.0f;
        
        FlockingForce = CohesionForce + AlignmentForce;
    }
    
    return FlockingForce;
}

FVector UCrowd_MassEntitySubsystem::CalculateSeparationForce(const FCrowd_EntityData& Entity) const
{
    FVector SeparationForce = FVector::ZeroVector;
    float SeparationRadius = 200.0f;
    
    for (const FCrowd_EntityData& OtherEntity : CrowdEntities)
    {
        if (OtherEntity.EntityID == Entity.EntityID)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Entity.Position, OtherEntity.Position);
        if (Distance < SeparationRadius && Distance > 0.0f)
        {
            FVector AwayDirection = (Entity.Position - OtherEntity.Position).GetSafeNormal();
            float SeparationStrength = (SeparationRadius - Distance) / SeparationRadius;
            SeparationForce += AwayDirection * SeparationStrength * 100.0f;
        }
    }
    
    return SeparationForce;
}

FVector UCrowd_MassEntitySubsystem::CalculateWanderForce(const FCrowd_EntityData& Entity) const
{
    // Simple wander behavior - random direction changes
    FVector WanderDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    return WanderDirection * 25.0f;
}

bool UCrowd_MassEntitySubsystem::IsEntityInBiome(const FCrowd_EntityData& Entity, const FCrowd_BiomeSettings& Biome) const
{
    float Distance = FVector::Dist2D(Entity.Position, Biome.BiomeCenter);
    return Distance <= Biome.BiomeRadius;
}

void UCrowd_MassEntitySubsystem::SetCrowdBehaviorMode(int32 BehaviorMode)
{
    CurrentBehaviorMode = FMath::Clamp(BehaviorMode, 0, 2);
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Behavior mode set to %d"), CurrentBehaviorMode);
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdCount() const
{
    return CrowdEntities.Num();
}

void UCrowd_MassEntitySubsystem::ClearAllCrowds()
{
    CrowdEntities.Empty();
    bIsSimulationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: All crowds cleared"));
}

void UCrowd_MassEntitySubsystem::RegisterBiome(const FCrowd_BiomeSettings& BiomeSettings)
{
    RegisteredBiomes.Add(BiomeSettings);
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem: Registered biome %s"), *BiomeSettings.BiomeName);
}

TArray<FCrowd_BiomeSettings> UCrowd_MassEntitySubsystem::GetRegisteredBiomes() const
{
    return RegisteredBiomes;
}