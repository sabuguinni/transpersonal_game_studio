#include "JurassicWorldGenerator.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/World.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeInfo.h"
#include "Landscape/Classes/LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AJurassicWorldGenerator::AJurassicWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create PCG component for procedural generation
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    PCGComponent->SetupAttachment(RootComponent);

    // Initialize default terrain settings
    TerrainSettings.LandscapeSize = 2017; // Valid UE5 heightmap size
    TerrainSettings.WorldScale = 100.0f;
    TerrainSettings.HeightScale = 256.0f;
    TerrainSettings.BaseFrequency = 0.001f;
    TerrainSettings.BaseAmplitude = 1.0f;
    TerrainSettings.NoiseOctaves = 6;
    TerrainSettings.NoisePersistence = 0.5f;
    TerrainSettings.MainRiverCount = 2;
    TerrainSettings.TributaryCount = 8;
    TerrainSettings.RiverWidth = 500.0f;
    TerrainSettings.RiverDepth = 200.0f;

    // Initialize default biome settings
    BiomeSettings.Empty();
    
    // Dense Forest - Primary biome for stealth and fear
    FJurassicBiomeSettings DenseForest;
    DenseForest.BiomeType = EJurassicBiomeType::DenseForest;
    DenseForest.DensityWeight = 0.4f;
    DenseForest.ElevationMin = 50.0f;
    DenseForest.ElevationMax = 300.0f;
    DenseForest.SlopeMin = 0.0f;
    DenseForest.SlopeMax = 25.0f;
    DenseForest.WaterDistance = 200.0f;
    DenseForest.bNearWater = false;
    BiomeSettings.Add(DenseForest);

    // River Valley - Water sources and open areas
    FJurassicBiomeSettings RiverValley;
    RiverValley.BiomeType = EJurassicBiomeType::RiverValley;
    RiverValley.DensityWeight = 0.2f;
    RiverValley.ElevationMin = 0.0f;
    RiverValley.ElevationMax = 100.0f;
    RiverValley.SlopeMin = 0.0f;
    RiverValley.SlopeMax = 15.0f;
    RiverValley.WaterDistance = 100.0f;
    RiverValley.bNearWater = true;
    BiomeSettings.Add(RiverValley);

    // Open Plains - Dangerous exposed areas
    FJurassicBiomeSettings OpenPlains;
    OpenPlains.BiomeType = EJurassicBiomeType::OpenPlains;
    OpenPlains.DensityWeight = 0.15f;
    OpenPlains.ElevationMin = 20.0f;
    OpenPlains.ElevationMax = 150.0f;
    OpenPlains.SlopeMin = 0.0f;
    OpenPlains.SlopeMax = 10.0f;
    OpenPlains.WaterDistance = 500.0f;
    OpenPlains.bNearWater = false;
    BiomeSettings.Add(OpenPlains);

    // Rocky Outcrops - Defensive positions and resources
    FJurassicBiomeSettings RockyOutcrops;
    RockyOutcrops.BiomeType = EJurassicBiomeType::RockyOutcrops;
    RockyOutcrops.DensityWeight = 0.15f;
    RockyOutcrops.ElevationMin = 200.0f;
    RockyOutcrops.ElevationMax = 500.0f;
    RockyOutcrops.SlopeMin = 20.0f;
    RockyOutcrops.SlopeMax = 60.0f;
    RockyOutcrops.WaterDistance = 300.0f;
    RockyOutcrops.bNearWater = false;
    BiomeSettings.Add(RockyOutcrops);

    // Swamp Lands - Hazardous wetlands
    FJurassicBiomeSettings SwampLands;
    SwampLands.BiomeType = EJurassicBiomeType::SwampLands;
    SwampLands.DensityWeight = 0.1f;
    SwampLands.ElevationMin = 0.0f;
    SwampLands.ElevationMax = 50.0f;
    SwampLands.SlopeMin = 0.0f;
    SwampLands.SlopeMax = 5.0f;
    SwampLands.WaterDistance = 50.0f;
    SwampLands.bNearWater = true;
    BiomeSettings.Add(SwampLands);

    GeneratedLandscape = nullptr;
}

void AJurassicWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    SetupPCGComponent();
}

void AJurassicWorldGenerator::SetupPCGComponent()
{
    if (PCGComponent && WorldGenerationGraph)
    {
        PCGComponent->SetGraph(WorldGenerationGraph);
        
        // Configure for hierarchical generation
        PCGComponent->bIsPartitioned = true;
        PCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::Generate;
        
        // Set up runtime generation for large worlds
        PCGComponent->bGenerated = false;
    }
}

void AJurassicWorldGenerator::ConfigureWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicWorldGenerator: No valid world found"));
        return;
    }

    // Configure World Partition settings for optimal streaming
    // This would typically be done through World Settings in the editor
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: World Partition configuration should be set in World Settings"));
    UE_LOG(LogTemp, Log, TEXT("Recommended settings:"));
    UE_LOG(LogTemp, Log, TEXT("- Cell Size: 51200 (512m)"));
    UE_LOG(LogTemp, Log, TEXT("- Loading Range: 153600 (1536m)"));
    UE_LOG(LogTemp, Log, TEXT("- Enable Streaming: True"));
}

void AJurassicWorldGenerator::GenerateWorld()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Starting world generation..."));

    // Clear any existing generated content
    ClearGeneratedWorld();

    // Configure World Partition
    ConfigureWorldPartition();

    // Generate base landscape
    GeneratedLandscape = CreateBaseLandscape();
    if (!GeneratedLandscape)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicWorldGenerator: Failed to create base landscape"));
        return;
    }

    // Generate river system
    GenerateRiverSystem();

    // Generate biomes using PCG
    GenerateBiomes();

    // Trigger PCG generation
    if (PCGComponent)
    {
        PCGComponent->GenerateLocal(true);
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: World generation completed"));
}

void AJurassicWorldGenerator::ClearGeneratedWorld()
{
    if (GeneratedLandscape)
    {
        GeneratedLandscape->Destroy();
        GeneratedLandscape = nullptr;
    }

    if (PCGComponent)
    {
        PCGComponent->CleanupLocal(true);
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Cleared existing generated world"));
}

ALandscape* AJurassicWorldGenerator::CreateBaseLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Calculate landscape parameters
    int32 ComponentCountX = 16; // 16x16 components for 2017x2017 heightmap
    int32 ComponentCountY = 16;
    int32 ComponentSizeQuads = 126; // 126 quads per component (127 vertices)
    int32 SubsectionSizeQuads = 63; // 63 quads per subsection
    int32 ComponentSections = 4; // 2x2 subsections per component

    // Create heightmap data
    int32 HeightmapSize = TerrainSettings.LandscapeSize;
    TArray<uint16> HeightData;
    HeightData.SetNumZeroed(HeightmapSize * HeightmapSize);

    // Generate heightmap using Perlin noise
    for (int32 Y = 0; Y < HeightmapSize; Y++)
    {
        for (int32 X = 0; X < HeightmapSize; X++)
        {
            float WorldX = (float)X * TerrainSettings.WorldScale;
            float WorldY = (float)Y * TerrainSettings.WorldScale;

            // Generate base terrain height using Perlin noise
            float Height = CalculatePerlinNoise(WorldX, WorldY, 
                TerrainSettings.NoiseOctaves, 
                TerrainSettings.NoisePersistence, 
                TerrainSettings.BaseFrequency);

            // Add some variation for more interesting terrain
            float DetailNoise = CalculatePerlinNoise(WorldX * 4.0f, WorldY * 4.0f, 3, 0.3f, 0.01f);
            Height += DetailNoise * 0.3f;

            // Normalize and scale height
            Height = FMath::Clamp(Height, -1.0f, 1.0f);
            Height = (Height + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
            
            // Apply height scale and convert to uint16
            uint16 HeightValue = (uint16)(Height * TerrainSettings.HeightScale * 256.0f);
            HeightData[Y * HeightmapSize + X] = HeightValue;
        }
    }

    // Create landscape using the heightmap data
    // Note: In a real implementation, you would use the Landscape Editor API
    // This is a simplified version for demonstration
    
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Base landscape heightmap generated (%dx%d)"), 
        HeightmapSize, HeightmapSize);

    // For now, return nullptr as actual landscape creation requires editor tools
    // In a production environment, this would create the actual landscape
    return nullptr;
}

void AJurassicWorldGenerator::GenerateRiverSystem()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Generating river system..."));

    // Generate main rivers
    for (int32 i = 0; i < TerrainSettings.MainRiverCount; i++)
    {
        // Start from high elevation, flow to low elevation
        FVector StartPoint = FVector(
            FMath::RandRange(-50000.0f, 50000.0f),
            FMath::RandRange(-50000.0f, 50000.0f),
            300.0f
        );

        FVector EndPoint = FVector(
            FMath::RandRange(-50000.0f, 50000.0f),
            FMath::RandRange(-50000.0f, 50000.0f),
            0.0f
        );

        TArray<FVector> SplinePoints = GenerateRiverSplinePoints(StartPoint, EndPoint, 20);
        
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Generated main river %d with %d points"), 
            i + 1, SplinePoints.Num());
    }

    // Generate tributaries
    for (int32 i = 0; i < TerrainSettings.TributaryCount; i++)
    {
        FVector StartPoint = FVector(
            FMath::RandRange(-30000.0f, 30000.0f),
            FMath::RandRange(-30000.0f, 30000.0f),
            200.0f
        );

        FVector EndPoint = FVector(
            FMath::RandRange(-30000.0f, 30000.0f),
            FMath::RandRange(-30000.0f, 30000.0f),
            50.0f
        );

        TArray<FVector> SplinePoints = GenerateRiverSplinePoints(StartPoint, EndPoint, 10);
        
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Generated tributary %d with %d points"), 
            i + 1, SplinePoints.Num());
    }
}

void AJurassicWorldGenerator::GenerateBiomes()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Generating biomes using PCG..."));

    // Biome generation will be handled by the PCG graph
    // The biome settings are available for the PCG nodes to use
    
    for (const FJurassicBiomeSettings& Biome : BiomeSettings)
    {
        FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGenerator: Configured biome %s (Weight: %.2f)"), 
            *BiomeName, Biome.DensityWeight);
    }
}

TArray<FVector> AJurassicWorldGenerator::GenerateRiverSplinePoints(FVector StartPoint, FVector EndPoint, int32 SegmentCount)
{
    TArray<FVector> Points;
    Points.Add(StartPoint);

    FVector Direction = (EndPoint - StartPoint) / (float)SegmentCount;
    FVector CurrentPoint = StartPoint;

    for (int32 i = 1; i < SegmentCount; i++)
    {
        CurrentPoint += Direction;
        
        // Add some randomness to make the river more natural
        FVector Offset = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        
        CurrentPoint += Offset;
        Points.Add(CurrentPoint);
    }

    Points.Add(EndPoint);
    return Points;
}

float AJurassicWorldGenerator::CalculatePerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Frequency) const
{
    float Total = 0.0f;
    float Amplitude = 1.0f;
    float MaxValue = 0.0f;

    for (int32 i = 0; i < Octaves; i++)
    {
        Total += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= Persistence;
        Frequency *= 2.0f;
    }

    return Total / MaxValue;
}

EJurassicBiomeType AJurassicWorldGenerator::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Simple biome determination based on elevation and slope
    float Elevation = GetElevationAtLocation(WorldLocation);
    float Slope = GetSlopeAtLocation(WorldLocation);
    bool bNearWater = IsNearWater(WorldLocation, 500.0f);

    // Determine biome based on terrain characteristics
    if (bNearWater && Elevation < 50.0f)
    {
        return EJurassicBiomeType::SwampLands;
    }
    else if (bNearWater && Elevation < 100.0f)
    {
        return EJurassicBiomeType::RiverValley;
    }
    else if (Slope > 30.0f && Elevation > 200.0f)
    {
        return EJurassicBiomeType::RockyOutcrops;
    }
    else if (Slope < 10.0f && Elevation < 150.0f)
    {
        return EJurassicBiomeType::OpenPlains;
    }
    else
    {
        return EJurassicBiomeType::DenseForest;
    }
}

float AJurassicWorldGenerator::GetElevationAtLocation(FVector WorldLocation) const
{
    // Simplified elevation calculation
    // In a real implementation, this would sample the actual landscape heightmap
    float NoiseValue = CalculatePerlinNoise(WorldLocation.X, WorldLocation.Y, 
        TerrainSettings.NoiseOctaves, TerrainSettings.NoisePersistence, TerrainSettings.BaseFrequency);
    
    return (NoiseValue + 1.0f) * 0.5f * TerrainSettings.HeightScale;
}

float AJurassicWorldGenerator::GetSlopeAtLocation(FVector WorldLocation) const
{
    // Calculate slope by sampling nearby points
    float SampleDistance = 100.0f;
    float CenterHeight = GetElevationAtLocation(WorldLocation);
    float EastHeight = GetElevationAtLocation(WorldLocation + FVector(SampleDistance, 0, 0));
    float NorthHeight = GetElevationAtLocation(WorldLocation + FVector(0, SampleDistance, 0));

    FVector Normal = FVector::CrossProduct(
        FVector(SampleDistance, 0, EastHeight - CenterHeight),
        FVector(0, SampleDistance, NorthHeight - CenterHeight)
    ).GetSafeNormal();

    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
    return SlopeAngle;
}

bool AJurassicWorldGenerator::IsNearWater(FVector WorldLocation, float MaxDistance) const
{
    // Simplified water detection
    // In a real implementation, this would check against actual water bodies
    
    // For now, assume water exists in low-lying areas with specific noise patterns
    float WaterNoise = CalculatePerlinNoise(WorldLocation.X * 0.0005f, WorldLocation.Y * 0.0005f, 3, 0.5f, 0.002f);
    float Elevation = GetElevationAtLocation(WorldLocation);
    
    return (WaterNoise > 0.3f && Elevation < 100.0f);
}