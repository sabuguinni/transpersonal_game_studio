#include "ProceduralWorldManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGSubsystem.h"
#include "WorldPartition/WorldPartition.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AProceduralWorldManager::AProceduralWorldManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    // Create main PCG component
    MainPCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("MainPCGComponent"));
    RootComponent = MainPCGComponent;

    // Set default world configuration
    WorldConfig = nullptr;
    GeneratedLandscape = nullptr;
}

void AProceduralWorldManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache biome definitions for performance
    if (WorldConfig.LoadSynchronous())
    {
        UWorldGenerationConfig* Config = WorldConfig.Get();
        if (Config)
        {
            for (const TSoftObjectPtr<UBiomeDefinition>& BiomePtr : Config->BiomeDefinitions)
            {
                if (UBiomeDefinition* BiomeDef = BiomePtr.LoadSynchronous())
                {
                    CachedBiomeDefinitions.Add(BiomeDef->BiomeType, BiomeDef);
                }
            }
        }
    }
}

void AProceduralWorldManager::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Setup World Partition if we're in a partitioned world
    if (GetWorld() && GetWorld()->GetWorldPartition())
    {
        SetupWorldPartition();
    }
}

void AProceduralWorldManager::GenerateWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Starting complete world generation..."));

    if (!WorldConfig.LoadSynchronous())
    {
        UE_LOG(LogTemp, Error, TEXT("ProceduralWorldManager: No world configuration assigned!"));
        return;
    }

    // Clear any existing content first
    ClearGeneratedContent();

    // Generate in sequence for dependencies
    GenerateTerrain();
    GenerateBiomes();
    GenerateWaterSystems();
    GenerateVegetation();

    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: World generation completed!"));
}

void AProceduralWorldManager::GenerateTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generating base terrain..."));

    UWorldGenerationConfig* Config = WorldConfig.LoadSynchronous();
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("ProceduralWorldManager: Cannot load world configuration"));
        return;
    }

    // Calculate landscape parameters
    const float WorldSizeCm = Config->WorldSizeKm * 100000.0f; // Convert km to cm
    const int32 ComponentCountPerSide = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Config->LandscapeResolution - 1) / (Config->ComponentSize - 1)));
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Landscape will be %dx%d components, each %dx%d quads"), 
           ComponentCountPerSide, ComponentCountPerSide, Config->ComponentSize - 1, Config->ComponentSize - 1);

    // TODO: Create landscape programmatically
    // For now, we'll use PCG to generate heightmap data that can be imported
    
    // Generate heightmap using Perlin noise
    TArray<uint16> HeightData;
    const int32 HeightmapSize = Config->LandscapeResolution;
    HeightData.SetNum(HeightmapSize * HeightmapSize);

    for (int32 Y = 0; Y < HeightmapSize; Y++)
    {
        for (int32 X = 0; X < HeightmapSize; X++)
        {
            const float NormalizedX = static_cast<float>(X) / HeightmapSize;
            const float NormalizedY = static_cast<float>(Y) / HeightmapSize;
            
            // Multi-octave noise for realistic terrain
            float Height = 0.0f;
            float Amplitude = 1.0f;
            float Frequency = 0.01f;
            
            // Base terrain shape
            for (int32 Octave = 0; Octave < 6; Octave++)
            {
                Height += FMath::PerlinNoise2D(FVector2D(NormalizedX * Frequency, NormalizedY * Frequency)) * Amplitude;
                Amplitude *= 0.5f;
                Frequency *= 2.0f;
            }
            
            // Add volcanic features
            const FVector2D Center1(0.3f, 0.7f);
            const FVector2D Center2(0.8f, 0.2f);
            const float Dist1 = FVector2D::Distance(FVector2D(NormalizedX, NormalizedY), Center1);
            const float Dist2 = FVector2D::Distance(FVector2D(NormalizedX, NormalizedY), Center2);
            
            if (Dist1 < 0.15f)
            {
                Height += (0.15f - Dist1) * 3.0f; // Volcanic peak
            }
            if (Dist2 < 0.1f)
            {
                Height += (0.1f - Dist2) * 2.0f; // Smaller volcanic feature
            }
            
            // River valleys (lower terrain along specific paths)
            const float RiverNoise = FMath::PerlinNoise2D(FVector2D(NormalizedX * 0.02f, NormalizedY * 0.02f));
            if (FMath::Abs(RiverNoise) < 0.1f)
            {
                Height -= 0.3f; // Create river valleys
            }
            
            // Normalize and convert to heightmap range
            Height = FMath::Clamp(Height, -1.0f, 1.0f);
            const uint16 HeightValue = static_cast<uint16>((Height + 1.0f) * 0.5f * 65535.0f);
            HeightData[Y * HeightmapSize + X] = HeightValue;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generated heightmap data with %d samples"), HeightData.Num());
}

void AProceduralWorldManager::GenerateBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generating biome distribution..."));

    // Biome generation will be handled by PCG graphs
    // This sets up the biome noise patterns and transition zones
    
    if (MainPCGComponent)
    {
        // Trigger PCG generation for biomes
        if (UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(GetWorld()))
        {
            PCGSubsystem->ScheduleComponent(MainPCGComponent, /*bSave=*/true);
        }
    }
}

void AProceduralWorldManager::GenerateWaterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generating water systems..."));

    UWorldGenerationConfig* Config = WorldConfig.LoadSynchronous();
    if (!Config || !Config->bGenerateRivers)
    {
        return;
    }

    // Generate river splines based on terrain flow
    // This will create AWaterBody actors for rivers and lakes
    
    const float WorldSize = Config->WorldSizeKm * 100000.0f;
    
    for (int32 RiverIndex = 0; RiverIndex < Config->MajorRiverCount; RiverIndex++)
    {
        // Find high elevation starting points
        FVector StartLocation;
        StartLocation.X = FMath::RandRange(-WorldSize * 0.4f, WorldSize * 0.4f);
        StartLocation.Y = FMath::RandRange(-WorldSize * 0.4f, WorldSize * 0.4f);
        StartLocation.Z = 5000.0f; // Start at elevated position
        
        // TODO: Trace down to find actual terrain height
        // TODO: Create spline following terrain gradient
        // TODO: Spawn AWaterBody actor with the spline
        
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generated river %d starting at %s"), 
               RiverIndex, *StartLocation.ToString());
    }
}

void AProceduralWorldManager::GenerateVegetation()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generating vegetation for all biomes..."));

    // Vegetation generation will use PCG with biome-specific rules
    // Each biome will have its own vegetation density and species distribution
    
    for (const auto& BiomePair : CachedBiomeDefinitions)
    {
        const EBiomeType BiomeType = BiomePair.Key;
        const UBiomeDefinition* BiomeDef = BiomePair.Value;
        
        if (BiomeDef && BiomeDef->VegetationGraph.LoadSynchronous())
        {
            UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Generating vegetation for biome %d"), 
                   static_cast<int32>(BiomeType));
            
            // TODO: Create PCG component for this biome's vegetation
            // TODO: Set biome-specific parameters
            // TODO: Schedule generation
        }
    }
}

void AProceduralWorldManager::ClearGeneratedContent()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Clearing all generated content..."));

    // Clear water bodies
    for (AWaterBody* WaterBody : GeneratedWaterBodies)
    {
        if (IsValid(WaterBody))
        {
            WaterBody->Destroy();
        }
    }
    GeneratedWaterBodies.Empty();

    // Clear landscape reference
    GeneratedLandscape = nullptr;

    // Clear PCG generated content
    if (MainPCGComponent)
    {
        MainPCGComponent->CleanupLocalImmediate(/*bRemoveComponents=*/true);
    }
}

EBiomeType AProceduralWorldManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Use noise-based biome distribution
    const float NoiseValue = GenerateBiomeNoise(FVector2D(WorldLocation.X, WorldLocation.Y));
    
    // Map noise value to biome types based on elevation and moisture
    const float Elevation = WorldLocation.Z;
    
    if (Elevation < 1000.0f) // Low elevation
    {
        if (NoiseValue < -0.3f) return EBiomeType::Swampland;
        if (NoiseValue < 0.0f) return EBiomeType::RiverDelta;
        if (NoiseValue < 0.3f) return EBiomeType::CoastalPlains;
        return EBiomeType::DenseForest;
    }
    else if (Elevation < 3000.0f) // Medium elevation
    {
        if (NoiseValue < -0.2f) return EBiomeType::OpenWoodland;
        if (NoiseValue < 0.2f) return EBiomeType::DenseForest;
        return EBiomeType::Highlands;
    }
    else // High elevation
    {
        if (NoiseValue < 0.0f) return EBiomeType::VolcanicRegion;
        return EBiomeType::CanyonLands;
    }
}

UBiomeDefinition* AProceduralWorldManager::GetBiomeDefinition(EBiomeType BiomeType) const
{
    if (const TObjectPtr<UBiomeDefinition>* BiomeDef = CachedBiomeDefinitions.Find(BiomeType))
    {
        return BiomeDef->Get();
    }
    return nullptr;
}

bool AProceduralWorldManager::IsLocationSuitableForDinosaurs(const FVector& WorldLocation, float RequiredClearance) const
{
    // Check biome suitability
    const EBiomeType Biome = GetBiomeAtLocation(WorldLocation);
    
    // Exclude unsuitable biomes
    if (Biome == EBiomeType::VolcanicRegion || Biome == EBiomeType::CanyonLands)
    {
        return false;
    }
    
    // Check slope (dinosaurs need relatively flat ground)
    // TODO: Implement terrain slope calculation
    
    // Check vegetation density (some clearance needed)
    // TODO: Query vegetation density from PCG
    
    return true;
}

TArray<FVector> AProceduralWorldManager::FindBaseBuildingLocations(int32 MaxLocations, float MinDistanceBetween) const
{
    TArray<FVector> SuitableLocations;
    
    UWorldGenerationConfig* Config = WorldConfig.LoadSynchronous();
    if (!Config)
    {
        return SuitableLocations;
    }
    
    const float WorldSize = Config->WorldSizeKm * 100000.0f;
    const int32 MaxAttempts = MaxLocations * 10;
    
    for (int32 Attempt = 0; Attempt < MaxAttempts && SuitableLocations.Num() < MaxLocations; Attempt++)
    {
        FVector CandidateLocation;
        CandidateLocation.X = FMath::RandRange(-WorldSize * 0.3f, WorldSize * 0.3f);
        CandidateLocation.Y = FMath::RandRange(-WorldSize * 0.3f, WorldSize * 0.3f);
        CandidateLocation.Z = 1000.0f; // TODO: Get actual terrain height
        
        // Check biome suitability
        const EBiomeType Biome = GetBiomeAtLocation(CandidateLocation);
        if (Biome == EBiomeType::VolcanicRegion || Biome == EBiomeType::Swampland)
        {
            continue; // Skip unsuitable biomes
        }
        
        // Check distance from existing locations
        bool bTooClose = false;
        for (const FVector& ExistingLocation : SuitableLocations)
        {
            if (FVector::Distance(CandidateLocation, ExistingLocation) < MinDistanceBetween)
            {
                bTooClose = true;
                break;
            }
        }
        
        if (!bTooClose)
        {
            SuitableLocations.Add(CandidateLocation);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Found %d suitable base building locations"), SuitableLocations.Num());
    return SuitableLocations;
}

float AProceduralWorldManager::GenerateBiomeNoise(const FVector2D& Location) const
{
    UWorldGenerationConfig* Config = WorldConfig.LoadSynchronous();
    if (!Config)
    {
        return 0.0f;
    }
    
    // Multi-octave Perlin noise for biome distribution
    float NoiseValue = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Config->BiomeNoiseScale;
    
    for (int32 Octave = 0; Octave < 4; Octave++)
    {
        NoiseValue += FMath::PerlinNoise2D(Location * Frequency) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return FMath::Clamp(NoiseValue, -1.0f, 1.0f);
}

TMap<EBiomeType, float> AProceduralWorldManager::CalculateBiomeWeights(const FVector& WorldLocation) const
{
    TMap<EBiomeType, float> BiomeWeights;
    
    UWorldGenerationConfig* Config = WorldConfig.LoadSynchronous();
    if (!Config)
    {
        return BiomeWeights;
    }
    
    const EBiomeType PrimaryBiome = GetBiomeAtLocation(WorldLocation);
    BiomeWeights.Add(PrimaryBiome, 1.0f);
    
    // Calculate blend weights for neighboring biomes
    const float BlendDistance = Config->BiomeBlendDistance;
    const TArray<FVector2D> SampleOffsets = {
        FVector2D(BlendDistance, 0.0f),
        FVector2D(-BlendDistance, 0.0f),
        FVector2D(0.0f, BlendDistance),
        FVector2D(0.0f, -BlendDistance)
    };
    
    for (const FVector2D& Offset : SampleOffsets)
    {
        const FVector SampleLocation = WorldLocation + FVector(Offset.X, Offset.Y, 0.0f);
        const EBiomeType SampleBiome = GetBiomeAtLocation(SampleLocation);
        
        if (SampleBiome != PrimaryBiome)
        {
            const float BlendWeight = 0.2f; // 20% influence from neighboring biomes
            BiomeWeights.FindOrAdd(SampleBiome) += BlendWeight;
        }
    }
    
    return BiomeWeights;
}

void AProceduralWorldManager::SetupWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World || !World->GetWorldPartition())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Setting up World Partition integration..."));
    
    // Configure this actor for World Partition
    SetIsSpatiallyLoaded(false); // Always loaded manager
    
    // TODO: Configure runtime grid settings
    // TODO: Set up data layers for different biomes
    // TODO: Configure HLOD generation
}