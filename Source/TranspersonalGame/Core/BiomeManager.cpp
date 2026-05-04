#include "BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize runtime state
    LastValidationTime = 0.0f;
    
    // Create root component for editor visibility
    USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootComp;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome system
    InitializeBiomeBoundaries();
    InitializeBiomeConditions();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with 5 biomes on 157km x 153km map"));
    
    // Validate current actor placement
    ValidateActorBiomePlacement();
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic validation every 30 seconds
    if (GetWorld()->GetTimeSeconds() - LastValidationTime > 30.0f)
    {
        ValidateActorBiomePlacement();
        LastValidationTime = GetWorld()->GetTimeSeconds();
    }
}

void ABiomeManager::InitializeBiomeBoundaries()
{
    // Based on brain memory coordinates: 157,000 x 153,000 UU map
    // X range: -77,500 to +79,500 | Y range: -76,500 to +76,500
    
    FEng_BiomeBounds SwampBounds;
    SwampBounds.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampBounds.MinBounds = FVector(-77500.0f, -76500.0f, -1000.0f);
    SwampBounds.MaxBounds = FVector(-25000.0f, -15000.0f, 1000.0f);
    BiomeBoundaries.Add(EEng_BiomeType::Swamp, SwampBounds);
    
    FEng_BiomeBounds ForestBounds;
    ForestBounds.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestBounds.MinBounds = FVector(-77500.0f, 15000.0f, -500.0f);
    ForestBounds.MaxBounds = FVector(-15000.0f, 76500.0f, 1500.0f);
    BiomeBoundaries.Add(EEng_BiomeType::Forest, ForestBounds);
    
    FEng_BiomeBounds SavannaBounds;
    SavannaBounds.Center = FVector(0.0f, 0.0f, 0.0f);
    SavannaBounds.MinBounds = FVector(-20000.0f, -20000.0f, -200.0f);
    SavannaBounds.MaxBounds = FVector(20000.0f, 20000.0f, 800.0f);
    BiomeBoundaries.Add(EEng_BiomeType::Savanna, SavannaBounds);
    
    FEng_BiomeBounds DesertBounds;
    DesertBounds.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertBounds.MinBounds = FVector(25000.0f, -30000.0f, -100.0f);
    DesertBounds.MaxBounds = FVector(79500.0f, 30000.0f, 600.0f);
    BiomeBoundaries.Add(EEng_BiomeType::Desert, DesertBounds);
    
    FEng_BiomeBounds MountainBounds;
    MountainBounds.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MountainBounds.MinBounds = FVector(15000.0f, 20000.0f, 0.0f);
    MountainBounds.MaxBounds = FVector(79500.0f, 76500.0f, 2000.0f);
    BiomeBoundaries.Add(EEng_BiomeType::Mountain, MountainBounds);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized 5 biome boundaries"));
}

void ABiomeManager::InitializeBiomeConditions()
{
    // Swamp conditions
    FEng_BiomeConditions SwampConditions;
    SwampConditions.Temperature = 28.0f;
    SwampConditions.Humidity = 95.0f;
    SwampConditions.WindSpeed = 5.0f;
    SwampConditions.Visibility = 0.3f;
    SwampConditions.DangerLevel = 0.8f;
    BiomeConditions.Add(EEng_BiomeType::Swamp, SwampConditions);
    
    // Forest conditions
    FEng_BiomeConditions ForestConditions;
    ForestConditions.Temperature = 22.0f;
    ForestConditions.Humidity = 75.0f;
    ForestConditions.WindSpeed = 8.0f;
    ForestConditions.Visibility = 0.5f;
    ForestConditions.DangerLevel = 0.6f;
    BiomeConditions.Add(EEng_BiomeType::Forest, ForestConditions);
    
    // Savanna conditions
    FEng_BiomeConditions SavannaConditions;
    SavannaConditions.Temperature = 32.0f;
    SavannaConditions.Humidity = 45.0f;
    SavannaConditions.WindSpeed = 12.0f;
    SavannaConditions.Visibility = 0.9f;
    SavannaConditions.DangerLevel = 0.7f;
    BiomeConditions.Add(EEng_BiomeType::Savanna, SavannaConditions);
    
    // Desert conditions
    FEng_BiomeConditions DesertConditions;
    DesertConditions.Temperature = 45.0f;
    DesertConditions.Humidity = 15.0f;
    DesertConditions.WindSpeed = 15.0f;
    DesertConditions.Visibility = 0.7f;
    DesertConditions.DangerLevel = 0.9f;
    BiomeConditions.Add(EEng_BiomeType::Desert, DesertConditions);
    
    // Mountain conditions
    FEng_BiomeConditions MountainConditions;
    MountainConditions.Temperature = 5.0f;
    MountainConditions.Humidity = 60.0f;
    MountainConditions.WindSpeed = 25.0f;
    MountainConditions.Visibility = 0.8f;
    MountainConditions.DangerLevel = 0.5f;
    BiomeConditions.Add(EEng_BiomeType::Mountain, MountainConditions);
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    for (const auto& BiomePair : BiomeBoundaries)
    {
        const FEng_BiomeBounds& Bounds = BiomePair.Value;
        
        if (WorldLocation.X >= Bounds.MinBounds.X && WorldLocation.X <= Bounds.MaxBounds.X &&
            WorldLocation.Y >= Bounds.MinBounds.Y && WorldLocation.Y <= Bounds.MaxBounds.Y)
        {
            return BiomePair.Key;
        }
    }
    
    // Default to Savanna if outside all bounds
    return EEng_BiomeType::Savanna;
}

FVector ABiomeManager::GetRandomSpawnLocationInBiome(EEng_BiomeType BiomeType) const
{
    if (!BiomeBoundaries.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid biome type for spawn"));
        return FVector::ZeroVector;
    }
    
    const FEng_BiomeBounds& Bounds = BiomeBoundaries[BiomeType];
    return GetRandomLocationInBounds(Bounds);
}

bool ABiomeManager::IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

FVector ABiomeManager::GetBiomeCenterLocation(EEng_BiomeType BiomeType) const
{
    if (BiomeBoundaries.Contains(BiomeType))
    {
        return BiomeBoundaries[BiomeType].Center;
    }
    
    return FVector::ZeroVector;
}

FEng_BiomeConditions ABiomeManager::GetBiomeConditions(EEng_BiomeType BiomeType) const
{
    if (BiomeConditions.Contains(BiomeType))
    {
        return BiomeConditions[BiomeType];
    }
    
    // Return default conditions
    FEng_BiomeConditions DefaultConditions;
    DefaultConditions.Temperature = 25.0f;
    DefaultConditions.Humidity = 50.0f;
    DefaultConditions.WindSpeed = 10.0f;
    DefaultConditions.Visibility = 1.0f;
    DefaultConditions.DangerLevel = 0.5f;
    
    return DefaultConditions;
}

void ABiomeManager::DistributeActorsAcrossBiomes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: No world for actor distribution"));
        return;
    }
    
    // Get all actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 RedistributedCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
            continue;
            
        FVector CurrentLocation = Actor->GetActorLocation();
        
        // Skip actors that are already properly placed
        if (FMath::Abs(CurrentLocation.X) > 1000 || FMath::Abs(CurrentLocation.Y) > 1000)
            continue;
            
        // Determine appropriate biome for this actor type
        EEng_BiomeType TargetBiome = EEng_BiomeType::Savanna; // Default
        
        FString ActorName = Actor->GetClass()->GetName();
        if (ActorName.Contains("Tree") || ActorName.Contains("Foliage"))
        {
            TargetBiome = EEng_BiomeType::Forest;
        }
        else if (ActorName.Contains("Rock") || ActorName.Contains("Stone"))
        {
            TargetBiome = EEng_BiomeType::Mountain;
        }
        else if (ActorName.Contains("Dinosaur") || ActorName.Contains("TRex") || ActorName.Contains("Raptor"))
        {
            TargetBiome = EEng_BiomeType::Savanna;
        }
        
        // Get new spawn location
        FVector NewLocation = GetRandomSpawnLocationInBiome(TargetBiome);
        
        if (NewLocation != FVector::ZeroVector)
        {
            Actor->SetActorLocation(NewLocation);
            RedistributedCount++;
            
            UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Moved %s to %s biome"), 
                   *ActorName, *UEnum::GetValueAsString(TargetBiome));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Redistributed %d actors across biomes"), RedistributedCount);
}

void ABiomeManager::FixOriginSpawnsAndRedistribute()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    MisplacedActors.Empty();
    
    // Find actors at or near origin
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
            continue;
            
        FVector Location = Actor->GetActorLocation();
        if (FMath::Abs(Location.X) < 100 && FMath::Abs(Location.Y) < 100)
        {
            MisplacedActors.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Found %d actors at origin to redistribute"), MisplacedActors.Num());
    
    // Redistribute them
    DistributeActorsAcrossBiomes();
}

void ABiomeManager::ValidateActorBiomePlacement()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Reset counts
    CurrentActorCounts.Empty();
    for (int32 i = 0; i < (int32)EEng_BiomeType::Count; i++)
    {
        CurrentActorCounts.Add((EEng_BiomeType)i, 0);
    }
    
    // Count actors per biome
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
            continue;
            
        FVector Location = Actor->GetActorLocation();
        EEng_BiomeType Biome = GetBiomeAtLocation(Location);
        
        if (CurrentActorCounts.Contains(Biome))
        {
            CurrentActorCounts[Biome]++;
        }
    }
    
    // Log results
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME ACTOR DISTRIBUTION ==="));
    for (const auto& CountPair : CurrentActorCounts)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %d actors"), 
               *UEnum::GetValueAsString(CountPair.Key), CountPair.Value);
    }
}

void ABiomeManager::GenerateBiomeReport()
{
    ValidateActorBiomePlacement();
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME SYSTEM REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Map Size: 157,000 x 153,000 UU"));
    UE_LOG(LogTemp, Warning, TEXT("Total Biomes: 5"));
    UE_LOG(LogTemp, Warning, TEXT("Misplaced Actors: %d"), MisplacedActors.Num());
    
    for (const auto& BiomePair : BiomeBoundaries)
    {
        const FEng_BiomeBounds& Bounds = BiomePair.Value;
        const FEng_BiomeConditions& Conditions = BiomeConditions[BiomePair.Key];
        int32 ActorCount = CurrentActorCounts.Contains(BiomePair.Key) ? CurrentActorCounts[BiomePair.Key] : 0;
        
        UE_LOG(LogTemp, Warning, TEXT("--- %s ---"), *UEnum::GetValueAsString(BiomePair.Key));
        UE_LOG(LogTemp, Warning, TEXT("  Center: (%.0f, %.0f, %.0f)"), 
               Bounds.Center.X, Bounds.Center.Y, Bounds.Center.Z);
        UE_LOG(LogTemp, Warning, TEXT("  Actors: %d"), ActorCount);
        UE_LOG(LogTemp, Warning, TEXT("  Temp: %.1f°C, Humidity: %.1f%%"), 
               Conditions.Temperature, Conditions.Humidity);
    }
}

FVector ABiomeManager::GetRandomLocationInBounds(const FEng_BiomeBounds& Bounds) const
{
    float RandomX = FMath::RandRange(Bounds.MinBounds.X, Bounds.MaxBounds.X);
    float RandomY = FMath::RandRange(Bounds.MinBounds.Y, Bounds.MaxBounds.Y);
    float TerrainZ = SampleTerrainHeight(FVector(RandomX, RandomY, 0.0f));
    
    return FVector(RandomX, RandomY, TerrainZ + 100.0f); // 100cm above terrain
}

float ABiomeManager::SampleTerrainHeight(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
        return 0.0f;
    
    // Simple terrain height sampling
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 5000.0f);
    FVector EndLocation = Location - FVector(0, 0, 5000.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location.Z;
    }
    
    return 0.0f; // Default ground level
}

bool ABiomeManager::IsActorTypeValidForBiome(AActor* Actor, EEng_BiomeType BiomeType) const
{
    if (!Actor)
        return false;
    
    FString ActorName = Actor->GetClass()->GetName();
    
    switch (BiomeType)
    {
        case EEng_BiomeType::Swamp:
            return ActorName.Contains("Water") || ActorName.Contains("Mud") || ActorName.Contains("Swamp");
            
        case EEng_BiomeType::Forest:
            return ActorName.Contains("Tree") || ActorName.Contains("Foliage") || ActorName.Contains("Forest");
            
        case EEng_BiomeType::Savanna:
            return ActorName.Contains("Grass") || ActorName.Contains("Dinosaur") || ActorName.Contains("Rock");
            
        case EEng_BiomeType::Desert:
            return ActorName.Contains("Sand") || ActorName.Contains("Dune") || ActorName.Contains("Cactus");
            
        case EEng_BiomeType::Mountain:
            return ActorName.Contains("Rock") || ActorName.Contains("Stone") || ActorName.Contains("Mountain");
            
        default:
            return true;
    }
}