#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    MaxCrowdEntities = 5000;
    UpdateFrequency = 0.1f;
    HighLODDistance = 2000.0f;
    MediumLODDistance = 5000.0f;
    LowLODDistance = 10000.0f;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem Initialized"));
    
    // Initialize biome configurations
    BiomeConfigurations.Empty();
    
    // Setup default biomes
    FCrowd_BiomeConfig Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0, 0, 0);
    Savana.MaxEntities = 1000;
    BiomeConfigurations.Add(Savana);
    
    FCrowd_BiomeConfig Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000, -45000, 0);
    Pantano.MaxEntities = 800;
    BiomeConfigurations.Add(Pantano);
    
    FCrowd_BiomeConfig Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000, 40000, 0);
    Floresta.MaxEntities = 1200;
    BiomeConfigurations.Add(Floresta);
    
    FCrowd_BiomeConfig Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000, 0, 0);
    Deserto.MaxEntities = 600;
    BiomeConfigurations.Add(Deserto);
    
    FCrowd_BiomeConfig Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000, 50000, 0);
    Montanha.MaxEntities = 400;
    BiomeConfigurations.Add(Montanha);
    
    InitializeCrowdSystem();
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CrowdUpdateTimer);
    }
    
    ClearAllCrowdEntities();
    
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Crowd System"));
    
    CrowdEntities.Empty();
    
    // Start update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CrowdUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UCrowd_MassEntitySubsystem::UpdateCrowdMovement, UpdateFrequency),
            UpdateFrequency,
            true
        );
    }
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(int32 Count, const FVector& Location, float Radius)
{
    if (CrowdEntities.Num() + Count > MaxCrowdEntities)
    {
        Count = FMath::Max(0, MaxCrowdEntities - CrowdEntities.Num());
        UE_LOG(LogTemp, Warning, TEXT("Limiting crowd spawn to %d entities (max capacity)"), Count);
    }
    
    int32 BiomeID = GetBiomeForLocation(Location);
    
    for (int32 i = 0; i < Count; i++)
    {
        FCrowd_EntityData NewEntity;
        
        // Random position within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0
        );
        
        NewEntity.Position = Location + RandomOffset;
        NewEntity.Velocity = FVector::ZeroVector;
        NewEntity.Speed = FMath::RandRange(80.0f, 120.0f);
        NewEntity.BiomeID = BiomeID;
        NewEntity.bIsActive = true;
        
        CrowdEntities.Add(NewEntity);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities at location %s"), Count, *Location.ToString());
}

void UCrowd_MassEntitySubsystem::UpdateCrowdMovement(float DeltaTime)
{
    if (CrowdEntities.Num() == 0)
    {
        return;
    }
    
    // Update each entity
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (!Entity.bIsActive)
        {
            continue;
        }
        
        ProcessEntityMovement(Entity, DeltaTime);
    }
}

void UCrowd_MassEntitySubsystem::ProcessEntityMovement(FCrowd_EntityData& Entity, float DeltaTime)
{
    // Get nearby entities for flocking
    TArray<FCrowd_EntityData> NearbyEntities = GetNearbyEntities(Entity.Position, 500.0f);
    
    // Apply flocking behavior
    ApplyFlockingBehavior(Entity, NearbyEntities);
    
    // Update position
    Entity.Position += Entity.Velocity * DeltaTime;
    
    // Constrain to biome
    ConstrainToBiome(Entity);
}

void UCrowd_MassEntitySubsystem::ApplyFlockingBehavior(FCrowd_EntityData& Entity, const TArray<FCrowd_EntityData>& NearbyEntities)
{
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    
    int32 NeighborCount = 0;
    
    for (const FCrowd_EntityData& Other : NearbyEntities)
    {
        if (&Other == &Entity || !Other.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Entity.Position, Other.Position);
        
        if (Distance < 200.0f) // Separation distance
        {
            FVector Diff = Entity.Position - Other.Position;
            Diff.Normalize();
            Separation += Diff / Distance;
        }
        
        if (Distance < 500.0f) // Alignment and cohesion distance
        {
            Alignment += Other.Velocity;
            Cohesion += Other.Position;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        Alignment /= NeighborCount;
        Cohesion /= NeighborCount;
        Cohesion = (Cohesion - Entity.Position).GetSafeNormal();
    }
    
    // Combine behaviors
    FVector DesiredVelocity = (Separation * 2.0f + Alignment * 1.0f + Cohesion * 1.0f).GetSafeNormal();
    
    // Add some random wandering
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0
    ).GetSafeNormal();
    
    DesiredVelocity += RandomDirection * 0.3f;
    DesiredVelocity.Normalize();
    
    Entity.Velocity = DesiredVelocity * Entity.Speed;
}

void UCrowd_MassEntitySubsystem::ConstrainToBiome(FCrowd_EntityData& Entity)
{
    if (Entity.BiomeID >= 0 && Entity.BiomeID < BiomeConfigurations.Num())
    {
        const FCrowd_BiomeConfig& Biome = BiomeConfigurations[Entity.BiomeID];
        
        float DistanceFromCenter = FVector::Dist2D(Entity.Position, Biome.CenterLocation);
        
        if (DistanceFromCenter > Biome.Radius)
        {
            // Push back towards biome center
            FVector Direction = (Biome.CenterLocation - Entity.Position).GetSafeNormal();
            Entity.Position = Biome.CenterLocation + Direction * (Biome.Radius - 100.0f);
            Entity.Velocity = Direction * Entity.Speed * 0.5f;
        }
    }
}

int32 UCrowd_MassEntitySubsystem::GetBiomeForLocation(const FVector& Location)
{
    float MinDistance = FLT_MAX;
    int32 ClosestBiome = 0;
    
    for (int32 i = 0; i < BiomeConfigurations.Num(); i++)
    {
        float Distance = FVector::Dist2D(Location, BiomeConfigurations[i].CenterLocation);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = i;
        }
    }
    
    return ClosestBiome;
}

TArray<FCrowd_EntityData> UCrowd_MassEntitySubsystem::GetNearbyEntities(const FVector& Location, float Radius)
{
    TArray<FCrowd_EntityData> NearbyEntities;
    
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (!Entity.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Location, Entity.Position);
        if (Distance <= Radius)
        {
            NearbyEntities.Add(Entity);
        }
    }
    
    return NearbyEntities;
}

void UCrowd_MassEntitySubsystem::SetBiomeConfiguration(const TArray<FCrowd_BiomeConfig>& BiomeConfigs)
{
    BiomeConfigurations = BiomeConfigs;
    UE_LOG(LogTemp, Warning, TEXT("Updated biome configurations: %d biomes"), BiomeConfigs.Num());
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdEntityCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void UCrowd_MassEntitySubsystem::ClearAllCrowdEntities()
{
    CrowdEntities.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all crowd entities"));
}

void UCrowd_MassEntitySubsystem::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float Distance = FVector::Dist(PlayerLocation, Entity.Position);
        
        // Deactivate entities beyond low LOD distance
        if (Distance > LowLODDistance)
        {
            Entity.bIsActive = false;
        }
        else
        {
            Entity.bIsActive = true;
        }
    }
}

void UCrowd_MassEntitySubsystem::SetLODDistances(float HighLOD, float MediumLOD, float LowLOD)
{
    HighLODDistance = HighLOD;
    MediumLODDistance = MediumLOD;
    LowLODDistance = LowLOD;
    
    UE_LOG(LogTemp, Warning, TEXT("Updated LOD distances: High=%f, Medium=%f, Low=%f"), HighLOD, MediumLOD, LowLOD);
}