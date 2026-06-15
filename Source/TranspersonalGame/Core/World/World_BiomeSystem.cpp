#include "World_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

AWorld_BiomeSystem::AWorld_BiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    BiomeTransitionDistance = 5000.0f;
    MaxBiomeRegions = 8;

    SetupDefaultBiomes();
}

void AWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: BeginPlay - Initializing biome system"));
    InitializeBiomeDatabase();
}

void AWorld_BiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_BiomeSystem::SetupDefaultBiomes()
{
    BiomeDatabase.Empty();

    // Forest Biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.BiomeName = TEXT("Prehistoric Forest");
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 80.0f;
    ForestBiome.Fertility = 90.0f;
    ForestBiome.VegetationTypes.Add(TEXT("Ferns"));
    ForestBiome.VegetationTypes.Add(TEXT("Conifers"));
    ForestBiome.VegetationTypes.Add(TEXT("Cycads"));
    ForestBiome.AnimalTypes.Add(TEXT("Triceratops"));
    ForestBiome.AnimalTypes.Add(TEXT("Parasaurolophus"));
    BiomeDatabase.Add(ForestBiome);

    // Plains Biome
    FWorld_BiomeData PlainsBiome;
    PlainsBiome.BiomeType = EWorld_BiomeType::Plains;
    PlainsBiome.BiomeName = TEXT("Prehistoric Plains");
    PlainsBiome.Temperature = 25.0f;
    PlainsBiome.Humidity = 40.0f;
    PlainsBiome.Fertility = 60.0f;
    PlainsBiome.VegetationTypes.Add(TEXT("Grasses"));
    PlainsBiome.VegetationTypes.Add(TEXT("Small Shrubs"));
    PlainsBiome.AnimalTypes.Add(TEXT("Brachiosaurus"));
    PlainsBiome.AnimalTypes.Add(TEXT("Ankylosaurus"));
    BiomeDatabase.Add(PlainsBiome);

    // Swamp Biome
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.BiomeName = TEXT("Prehistoric Swamp");
    SwampBiome.Temperature = 28.0f;
    SwampBiome.Humidity = 95.0f;
    SwampBiome.Fertility = 85.0f;
    SwampBiome.VegetationTypes.Add(TEXT("Moss"));
    SwampBiome.VegetationTypes.Add(TEXT("Reeds"));
    SwampBiome.VegetationTypes.Add(TEXT("Water Plants"));
    SwampBiome.AnimalTypes.Add(TEXT("Spinosaurus"));
    SwampBiome.AnimalTypes.Add(TEXT("Crocodiles"));
    BiomeDatabase.Add(SwampBiome);

    // Mountain Biome
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.BiomeName = TEXT("Prehistoric Mountains");
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 30.0f;
    MountainBiome.Fertility = 25.0f;
    MountainBiome.VegetationTypes.Add(TEXT("Hardy Shrubs"));
    MountainBiome.VegetationTypes.Add(TEXT("Rock Lichens"));
    MountainBiome.AnimalTypes.Add(TEXT("Pterodactyls"));
    BiomeDatabase.Add(MountainBiome);
}

EWorld_BiomeType AWorld_BiomeSystem::GetBiomeAtLocation(const FVector& Location)
{
    // Simple noise-based biome determination
    float NoiseX = Location.X * 0.0001f;
    float NoiseY = Location.Y * 0.0001f;
    
    float BiomeNoise = FMath::PerlinNoise2D(FVector2D(NoiseX, NoiseY));
    
    if (BiomeNoise > 0.5f)
        return EWorld_BiomeType::Forest;
    else if (BiomeNoise > 0.0f)
        return EWorld_BiomeType::Plains;
    else if (BiomeNoise > -0.3f)
        return EWorld_BiomeType::Swamp;
    else
        return EWorld_BiomeType::Mountain;
}

FWorld_BiomeData AWorld_BiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType)
{
    for (const FWorld_BiomeData& Biome : BiomeDatabase)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default if not found
    return FWorld_BiomeData();
}

void AWorld_BiomeSystem::GenerateBiomeRegions()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Generating biome regions"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_BiomeSystem: No valid world found"));
        return;
    }

    // Create biome regions in a grid pattern
    int32 GridSize = 4;
    float RegionSize = 10000.0f; // 100m x 100m regions
    
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FVector RegionCenter = FVector(
                X * RegionSize - (GridSize * RegionSize * 0.5f),
                Y * RegionSize - (GridSize * RegionSize * 0.5f),
                0.0f
            );
            
            EWorld_BiomeType BiomeType = GetBiomeAtLocation(RegionCenter);
            SpawnBiomeVegetation(BiomeType, RegionCenter, RegionSize * 0.4f);
        }
    }
}

void AWorld_BiomeSystem::SpawnBiomeVegetation(EWorld_BiomeType BiomeType, const FVector& Location, float Radius)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Spawn vegetation based on biome type
    int32 VegetationCount = FMath::RandRange(5, 15);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(Location, Radius);
        
        // Create a simple cube as vegetation placeholder
        UWorld* World = GetWorld();
        if (World)
        {
            AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
            if (VegetationActor)
            {
                FString BiomeName = UEnum::GetValueAsString(BiomeType);
                VegetationActor->SetActorLabel(FString::Printf(TEXT("%s_Vegetation_%d"), *BiomeName, i));
                
                UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Spawned vegetation at %s for biome %s"), 
                    *SpawnLocation.ToString(), *BiomeName);
            }
        }
    }
}

float AWorld_BiomeSystem::CalculateBiomeInfluence(const FVector& Location, EWorld_BiomeType BiomeType)
{
    EWorld_BiomeType LocalBiome = GetBiomeAtLocation(Location);
    
    if (LocalBiome == BiomeType)
    {
        return 1.0f;
    }
    
    // Calculate transition influence based on distance
    float Distance = FVector::Dist(Location, GetActorLocation());
    float Influence = FMath::Clamp(1.0f - (Distance / BiomeTransitionDistance), 0.0f, 1.0f);
    
    return Influence * 0.3f; // Reduced influence for non-matching biomes
}

void AWorld_BiomeSystem::InitializeBiomeDatabase()
{
    if (BiomeDatabase.Num() == 0)
    {
        SetupDefaultBiomes();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Initialized with %d biomes"), BiomeDatabase.Num());
    
    for (const FWorld_BiomeData& Biome : BiomeDatabase)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome: %s, Temp: %.1f, Humidity: %.1f"), 
            *Biome.BiomeName, Biome.Temperature, Biome.Humidity);
    }
}

void AWorld_BiomeSystem::CreateTestBiomeRegions()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Creating test biome regions"));
    GenerateBiomeRegions();
}

void AWorld_BiomeSystem::CreateBiomeTransitions()
{
    // Implementation for smooth biome transitions
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Creating biome transitions"));
}

FVector AWorld_BiomeSystem::GetRandomLocationInRadius(const FVector& Center, float Radius)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return Center + RandomOffset;
}