#include "EngArch_TerrainSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UEngArch_TerrainSystem::UEngArch_TerrainSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize terrain parameters
    TerrainSizeKm = 16.0f;
    HeightVariation = 2000.0f;
    BiomeBlendDistance = 5000.0f;
    
    // Initialize biome elevation ranges
    BiomeElevationRanges.Add(EEng_BiomeType::Savanna, FEng_ElevationRange(0.0f, 500.0f));
    BiomeElevationRanges.Add(EEng_BiomeType::Forest, FEng_ElevationRange(200.0f, 1200.0f));
    BiomeElevationRanges.Add(EEng_BiomeType::Desert, FEng_ElevationRange(0.0f, 800.0f));
    BiomeElevationRanges.Add(EEng_BiomeType::Mountain, FEng_ElevationRange(800.0f, 2000.0f));
    BiomeElevationRanges.Add(EEng_BiomeType::Swamp, FEng_ElevationRange(-100.0f, 200.0f));
    
    // Initialize terrain materials
    InitializeTerrainMaterials();
}

void UEngArch_TerrainSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_TerrainSystem: System initialized for world"));
        ValidateTerrainConfiguration();
    }
}

void UEngArch_TerrainSystem::InitializeTerrainMaterials()
{
    // Load default landscape materials
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SavannaMat(TEXT("/Game/LandscapePackOne/Materials/M_Landscape_Grass"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ForestMat(TEXT("/Game/Tropical_Jungle_Pack/Materials/M_Forest_Ground"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DesertMat(TEXT("/Game/Desert_Oasis/Materials/M_Desert_Sand"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MountainMat(TEXT("/Game/ANGRY_MESH/Materials/M_Mountain_Rock"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SwampMat(TEXT("/Game/Landscape_AutoMaterial_5_RainForest/Materials/M_Swamp"));
    
    if (SavannaMat.Succeeded()) BiomeMaterials.Add(EEng_BiomeType::Savanna, SavannaMat.Object);
    if (ForestMat.Succeeded()) BiomeMaterials.Add(EEng_BiomeType::Forest, ForestMat.Object);
    if (DesertMat.Succeeded()) BiomeMaterials.Add(EEng_BiomeType::Desert, DesertMat.Object);
    if (MountainMat.Succeeded()) BiomeMaterials.Add(EEng_BiomeType::Mountain, MountainMat.Object);
    if (SwampMat.Succeeded()) BiomeMaterials.Add(EEng_BiomeType::Swamp, SwampMat.Object);
}

bool UEngArch_TerrainSystem::ValidateTerrainConfiguration()
{
    bool bIsValid = true;
    
    // Check terrain size limits
    if (TerrainSizeKm > 64.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_TerrainSystem: Terrain size exceeds 64km limit"));
        bIsValid = false;
    }
    
    // Check height variation limits
    if (HeightVariation > 5000.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_TerrainSystem: Height variation exceeds 5km limit"));
        bIsValid = false;
    }
    
    // Validate biome materials
    for (const auto& BiomePair : BiomeMaterials)
    {
        if (!BiomePair.Value)
        {
            UE_LOG(LogTemp, Warning, TEXT("EngArch_TerrainSystem: Missing material for biome %d"), (int32)BiomePair.Key);
        }
    }
    
    return bIsValid;
}

FEng_TerrainData UEngArch_TerrainSystem::GenerateTerrainData(const FVector& WorldLocation)
{
    FEng_TerrainData TerrainData;
    
    // Calculate normalized coordinates (0-1 range)
    float NormalizedX = (WorldLocation.X + (TerrainSizeKm * 50000.0f)) / (TerrainSizeKm * 100000.0f);
    float NormalizedY = (WorldLocation.Y + (TerrainSizeKm * 50000.0f)) / (TerrainSizeKm * 100000.0f);
    
    // Generate height using multiple octaves of noise
    float Height = GenerateHeightNoise(NormalizedX, NormalizedY);
    TerrainData.Elevation = Height * HeightVariation;
    
    // Determine biome based on location and elevation
    TerrainData.BiomeType = DetermineBiomeType(WorldLocation, TerrainData.Elevation);
    
    // Calculate slope
    float DeltaX = GenerateHeightNoise(NormalizedX + 0.001f, NormalizedY) - Height;
    float DeltaY = GenerateHeightNoise(NormalizedX, NormalizedY + 0.001f) - Height;
    TerrainData.Slope = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY) * 100.0f;
    
    // Determine if location is near water
    TerrainData.bIsNearWater = (TerrainData.Elevation < 50.0f) || (TerrainData.BiomeType == EEng_BiomeType::Swamp);
    
    return TerrainData;
}

float UEngArch_TerrainSystem::GenerateHeightNoise(float X, float Y)
{
    // Simple multi-octave noise function
    float Height = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    
    for (int32 i = 0; i < 4; i++)
    {
        Height += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return FMath::Clamp(Height * 0.5f + 0.5f, 0.0f, 1.0f);
}

EEng_BiomeType UEngArch_TerrainSystem::DetermineBiomeType(const FVector& WorldLocation, float Elevation)
{
    // Biome determination based on world coordinates and elevation
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    
    // Desert biome (positive X, negative Y)
    if (X > 25000.0f && Y < -25000.0f)
    {
        return EEng_BiomeType::Desert;
    }
    
    // Forest biome (negative X, positive Y)
    if (X < -25000.0f && Y > 25000.0f)
    {
        return EEng_BiomeType::Forest;
    }
    
    // Mountain biome (high elevation)
    if (Elevation > 800.0f)
    {
        return EEng_BiomeType::Mountain;
    }
    
    // Swamp biome (low elevation near water)
    if (Elevation < 100.0f && FMath::Abs(X) < 10000.0f && FMath::Abs(Y) < 10000.0f)
    {
        return EEng_BiomeType::Swamp;
    }
    
    // Default to Savanna
    return EEng_BiomeType::Savanna;
}

UMaterialInterface* UEngArch_TerrainSystem::GetBiomeMaterial(EEng_BiomeType BiomeType)
{
    if (UMaterialInterface** FoundMaterial = BiomeMaterials.Find(BiomeType))
    {
        return *FoundMaterial;
    }
    
    return nullptr;
}

bool UEngArch_TerrainSystem::IsLocationSuitableForStructure(const FVector& WorldLocation)
{
    FEng_TerrainData TerrainData = GenerateTerrainData(WorldLocation);
    
    // Check slope (structures need relatively flat ground)
    if (TerrainData.Slope > 30.0f)
    {
        return false;
    }
    
    // Check elevation (avoid very low or very high areas)
    if (TerrainData.Elevation < -50.0f || TerrainData.Elevation > 1500.0f)
    {
        return false;
    }
    
    // Avoid swamp areas for most structures
    if (TerrainData.BiomeType == EEng_BiomeType::Swamp)
    {
        return false;
    }
    
    return true;
}

TArray<FVector> UEngArch_TerrainSystem::GetSuitableSpawnLocations(EEng_BiomeType BiomeType, int32 NumLocations)
{
    TArray<FVector> SpawnLocations;
    
    // Define search area based on biome type
    FVector SearchCenter = GetBiomeCenter(BiomeType);
    float SearchRadius = 20000.0f;
    
    int32 Attempts = 0;
    int32 MaxAttempts = NumLocations * 10;
    
    while (SpawnLocations.Num() < NumLocations && Attempts < MaxAttempts)
    {
        // Generate random location within search area
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, SearchRadius);
        
        FVector TestLocation = SearchCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Generate terrain data for this location
        FEng_TerrainData TerrainData = GenerateTerrainData(TestLocation);
        
        // Check if location matches desired biome and is suitable
        if (TerrainData.BiomeType == BiomeType && IsLocationSuitableForStructure(TestLocation))
        {
            TestLocation.Z = TerrainData.Elevation;
            SpawnLocations.Add(TestLocation);
        }
        
        Attempts++;
    }
    
    return SpawnLocations;
}

FVector UEngArch_TerrainSystem::GetBiomeCenter(EEng_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EEng_BiomeType::Savanna:
            return FVector(0.0f, 0.0f, 0.0f);
        case EEng_BiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 0.0f);
        case EEng_BiomeType::Desert:
            return FVector(50000.0f, -40000.0f, 0.0f);
        case EEng_BiomeType::Mountain:
            return FVector(30000.0f, 30000.0f, 0.0f);
        case EEng_BiomeType::Swamp:
            return FVector(-20000.0f, -20000.0f, 0.0f);
        default:
            return FVector::ZeroVector;
    }
}