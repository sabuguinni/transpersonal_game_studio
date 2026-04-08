#include "ProceduralWorldGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "WorldPartition/WorldPartition.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UProceduralWorldGenerator::UProceduralWorldGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Default world parameters for Jurassic survival game
    WorldSize = FVector2D(8192.0f, 8192.0f); // 8km x 8km region
    LandscapeResolution = 4033; // Recommended size for UE5
    MaxElevation = 2000.0f; // 20 meters max height variation
    SeaLevel = 0.0f;
    
    BiomeTransitionDistance = 500.0f;
    MaxRiverBranches = 3;
    
    // Initialize default biome definitions
    InitializeDefaultBiomes();
}

void UProceduralWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Initialized for Jurassic world generation"));
}

void UProceduralWorldGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UProceduralWorldGenerator::GenerateWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Starting world generation..."));
    
    // Step 1: Generate base terrain
    GenerateTerrain();
    
    // Step 2: Generate river systems (affects terrain)
    GenerateRiverSystems();
    
    // Step 3: Apply geological processes
    ApplyGeologicalProcesses();
    
    // Step 4: Generate biome distribution
    GenerateBiomes();
    
    // Step 5: Place geological landmarks
    GenerateGeologicalFeatures();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: World generation complete"));
}

void UProceduralWorldGenerator::GenerateTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Generating base terrain..."));
    
    // Initialize heightmap data
    InitializeHeightmapData();
    
    // Create landscape if it doesn't exist
    if (!GeneratedLandscape)
    {
        // Find existing landscape or create new one
        GeneratedLandscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(GetWorld(), ALandscape::StaticClass()));
        
        if (!GeneratedLandscape)
        {
            UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Creating new landscape"));
            // Landscape creation would be handled by PCG or manual setup
        }
    }
    
    // Setup PCG components for terrain generation
    SetupPCGComponents();
}

void UProceduralWorldGenerator::GenerateBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Generating biome distribution..."));
    
    // Calculate biome maps based on elevation, humidity, temperature, and drainage
    int32 MapSize = LandscapeResolution;
    
    for (int32 Y = 0; Y < MapSize; Y++)
    {
        for (int32 X = 0; X < MapSize; X++)
        {
            int32 Index = Y * MapSize + X;
            
            float Elevation = HeightmapData[Index];
            float Humidity = HumidityMap[Index];
            float Temperature = TemperatureMap[Index];
            float Drainage = DrainageMap[Index];
            
            // Determine biome based on environmental factors
            EBiomeType BiomeType = DetermineBiomeType(Elevation, Humidity, Temperature, Drainage);
            
            // Store biome data for later use by Environment Artist
        }
    }
}

void UProceduralWorldGenerator::GenerateRiverSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Generating river systems..."));
    
    // Generate main river system
    FRiverSystemData MainRiver;
    MainRiver.SourceLocation = FVector(WorldSize.X * 0.8f, WorldSize.Y * 0.2f, MaxElevation * 0.8f);
    MainRiver.MouthLocation = FVector(WorldSize.X * 0.1f, WorldSize.Y * 0.9f, SeaLevel);
    MainRiver.Width = 800.0f;
    MainRiver.Depth = 300.0f;
    MainRiver.FlowRate = 1.5f;
    
    // Generate tributaries
    GenerateTributaries(MainRiver);
    
    RiverSystems.Add(MainRiver);
    
    // Apply river influence to heightmap and drainage
    ApplyRiverErosion();
}

void UProceduralWorldGenerator::GenerateGeologicalFeatures()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldGenerator: Placing geological landmarks..."));
    
    // Place distinctive landmarks for navigation
    PlaceLandmarks();
    
    // Generate cave entrances (for future cave systems)
    GenerateCaveEntrances();
    
    // Create natural clearings for potential base locations
    GenerateNaturalClearings();
}

EBiomeType UProceduralWorldGenerator::GetBiomeAtLocation(FVector Location)
{
    // Convert world location to heightmap coordinates
    float X = (Location.X / WorldSize.X) * LandscapeResolution;
    float Y = (Location.Y / WorldSize.Y) * LandscapeResolution;
    
    // Clamp to valid range
    X = FMath::Clamp(X, 0.0f, LandscapeResolution - 1.0f);
    Y = FMath::Clamp(Y, 0.0f, LandscapeResolution - 1.0f);
    
    int32 Index = FMath::FloorToInt(Y) * LandscapeResolution + FMath::FloorToInt(X);
    
    if (HeightmapData.IsValidIndex(Index))
    {
        float Elevation = HeightmapData[Index];
        float Humidity = HumidityMap.IsValidIndex(Index) ? HumidityMap[Index] : 0.7f;
        float Temperature = TemperatureMap.IsValidIndex(Index) ? TemperatureMap[Index] : 0.6f;
        float Drainage = DrainageMap.IsValidIndex(Index) ? DrainageMap[Index] : 0.5f;
        
        return DetermineBiomeType(Elevation, Humidity, Temperature, Drainage);
    }
    
    return EBiomeType::DenseForest; // Default fallback
}

float UProceduralWorldGenerator::GetElevationAtLocation(FVector Location)
{
    // Convert world location to heightmap coordinates
    float X = (Location.X / WorldSize.X) * LandscapeResolution;
    float Y = (Location.Y / WorldSize.Y) * LandscapeResolution;
    
    // Clamp to valid range
    X = FMath::Clamp(X, 0.0f, LandscapeResolution - 1.0f);
    Y = FMath::Clamp(Y, 0.0f, LandscapeResolution - 1.0f);
    
    int32 Index = FMath::FloorToInt(Y) * LandscapeResolution + FMath::FloorToInt(X);
    
    if (HeightmapData.IsValidIndex(Index))
    {
        return HeightmapData[Index];
    }
    
    return SeaLevel;
}

bool UProceduralWorldGenerator::IsLocationNearRiver(FVector Location, float Threshold)
{
    for (const FRiverSystemData& River : RiverSystems)
    {
        // Check distance to main river path
        float DistanceToRiver = CalculateDistanceToRiverPath(Location, River);
        if (DistanceToRiver <= Threshold)
        {
            return true;
        }
        
        // Check tributaries
        for (const FVector& TributaryPoint : River.TributaryPoints)
        {
            float DistanceToTributary = FVector::Dist2D(Location, TributaryPoint);
            if (DistanceToTributary <= Threshold)
            {
                return true;
            }
        }
    }
    
    return false;
}

FVector UProceduralWorldGenerator::FindNearestRiverPoint(FVector Location)
{
    FVector NearestPoint = FVector::ZeroVector;
    float MinDistance = FLT_MAX;
    
    for (const FRiverSystemData& River : RiverSystems)
    {
        // Find nearest point on main river
        FVector RiverPoint = FindNearestPointOnRiver(Location, River);
        float Distance = FVector::Dist(Location, RiverPoint);
        
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestPoint = RiverPoint;
        }
    }
    
    return NearestPoint;
}

// Private implementation methods

void UProceduralWorldGenerator::InitializeHeightmapData()
{
    int32 MapSize = LandscapeResolution;
    int32 TotalSize = MapSize * MapSize;
    
    HeightmapData.SetNum(TotalSize);
    HumidityMap.SetNum(TotalSize);
    TemperatureMap.SetNum(TotalSize);
    DrainageMap.SetNum(TotalSize);
    
    // Generate base terrain using Perlin noise and geological principles
    for (int32 Y = 0; Y < MapSize; Y++)
    {
        for (int32 X = 0; X < MapSize; X++)
        {
            int32 Index = Y * MapSize + X;
            
            // Normalized coordinates
            float NormX = (float)X / MapSize;
            float NormY = (float)Y / MapSize;
            
            // Generate base elevation using multiple octaves of noise
            float Elevation = GenerateTerrainHeight(NormX, NormY);
            HeightmapData[Index] = Elevation;
            
            // Generate environmental maps
            HumidityMap[Index] = GenerateHumidity(NormX, NormY, Elevation);
            TemperatureMap[Index] = GenerateTemperature(NormX, NormY, Elevation);
            DrainageMap[Index] = GenerateDrainage(NormX, NormY, Elevation);
        }
    }
}

float UProceduralWorldGenerator::GenerateTerrainHeight(float X, float Y)
{
    // Multi-octave Perlin noise for realistic terrain
    float Height = 0.0f;
    float Amplitude = MaxElevation;
    float Frequency = 0.01f;
    
    // Large scale features
    Height += UKismetMathLibrary::PerlinNoise1D(X * Frequency + Y * Frequency) * Amplitude;
    
    // Medium scale features
    Amplitude *= 0.5f;
    Frequency *= 2.0f;
    Height += UKismetMathLibrary::PerlinNoise1D(X * Frequency + Y * Frequency) * Amplitude;
    
    // Small scale features
    Amplitude *= 0.5f;
    Frequency *= 2.0f;
    Height += UKismetMathLibrary::PerlinNoise1D(X * Frequency + Y * Frequency) * Amplitude;
    
    // Ensure minimum elevation above sea level
    Height = FMath::Max(Height, SeaLevel + 50.0f);
    
    return Height;
}

void UProceduralWorldGenerator::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Dense Forest - Primary biome
    FBiomeDefinition DenseForest;
    DenseForest.BiomeType = EBiomeType::DenseForest;
    DenseForest.Humidity = 0.8f;
    DenseForest.Temperature = 0.7f;
    DenseForest.Elevation = 0.5f;
    DenseForest.DrainageLevel = 0.6f;
    DenseForest.SupportedFeatures = {ETerrainFeature::Hill, ETerrainFeature::Valley};
    BiomeDefinitions.Add(DenseForest);
    
    // River Valley
    FBiomeDefinition RiverValley;
    RiverValley.BiomeType = EBiomeType::RiverValley;
    RiverValley.Humidity = 0.9f;
    RiverValley.Temperature = 0.6f;
    RiverValley.Elevation = 0.2f;
    RiverValley.DrainageLevel = 0.9f;
    RiverValley.SupportedFeatures = {ETerrainFeature::River, ETerrainFeature::Valley};
    BiomeDefinitions.Add(RiverValley);
    
    // Rocky Outcrop
    FBiomeDefinition RockyOutcrop;
    RockyOutcrop.BiomeType = EBiomeType::RockyOutcrop;
    RockyOutcrop.Humidity = 0.4f;
    RockyOutcrop.Temperature = 0.5f;
    RockyOutcrop.Elevation = 0.8f;
    RockyOutcrop.DrainageLevel = 0.3f;
    RockyOutcrop.SupportedFeatures = {ETerrainFeature::Plateau, ETerrainFeature::Ravine};
    BiomeDefinitions.Add(RockyOutcrop);
    
    // Wetlands
    FBiomeDefinition Wetlands;
    Wetlands.BiomeType = EBiomeType::Wetlands;
    Wetlands.Humidity = 0.95f;
    Wetlands.Temperature = 0.8f;
    Wetlands.Elevation = 0.1f;
    Wetlands.DrainageLevel = 0.1f;
    Wetlands.SupportedFeatures = {ETerrainFeature::River};
    BiomeDefinitions.Add(Wetlands);
    
    // Clearing Meadow
    FBiomeDefinition ClearingMeadow;
    ClearingMeadow.BiomeType = EBiomeType::ClearingMeadow;
    ClearingMeadow.Humidity = 0.6f;
    ClearingMeadow.Temperature = 0.7f;
    ClearingMeadow.Elevation = 0.4f;
    ClearingMeadow.DrainageLevel = 0.5f;
    ClearingMeadow.SupportedFeatures = {ETerrainFeature::Hill};
    BiomeDefinitions.Add(ClearingMeadow);
}

EBiomeType UProceduralWorldGenerator::DetermineBiomeType(float Elevation, float Humidity, float Temperature, float Drainage)
{
    float BestScore = -1.0f;
    EBiomeType BestBiome = EBiomeType::DenseForest;
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        // Calculate similarity score
        float ElevationScore = 1.0f - FMath::Abs(Elevation - Biome.Elevation);
        float HumidityScore = 1.0f - FMath::Abs(Humidity - Biome.Humidity);
        float TemperatureScore = 1.0f - FMath::Abs(Temperature - Biome.Temperature);
        float DrainageScore = 1.0f - FMath::Abs(Drainage - Biome.DrainageLevel);
        
        float TotalScore = (ElevationScore + HumidityScore + TemperatureScore + DrainageScore) / 4.0f;
        
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestBiome = Biome.BiomeType;
        }
    }
    
    return BestBiome;
}