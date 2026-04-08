#include "JurassicBiomeManager.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Components/SceneComponent.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeInfo.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AJurassicBiomeManager::AJurassicBiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create PCG component for biome generation
    BiomePCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("BiomePCGComponent"));
    BiomePCGComponent->SetupAttachment(RootComponent);

    // Initialize default world bounds (20.16km x 20.16km world)
    WorldOrigin = FVector::ZeroVector;
    WorldSize = FVector(2016000.0f, 2016000.0f, 25600.0f); // 20.16km x 20.16km x 256m
}

void AJurassicBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup PCG component for biome generation
    SetupPCGBiomeGeneration();
    
    // Generate initial biome distribution if data asset is available
    if (BiomeDataAsset)
    {
        GenerateBiomeDistribution();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicBiomeManager: No BiomeDataAsset assigned"));
    }
}

void AJurassicBiomeManager::SetupPCGBiomeGeneration()
{
    if (!BiomePCGComponent)
    {
        return;
    }

    // Configure PCG component for hierarchical generation
    BiomePCGComponent->bIsPartitioned = true;
    BiomePCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::Generate;
    
    // Set up for runtime generation
    BiomePCGComponent->bGenerated = false;
    
    UE_LOG(LogTemp, Log, TEXT("JurassicBiomeManager: PCG biome generation setup complete"));
}

EJurassicBiomeType AJurassicBiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (!BiomeDataAsset)
    {
        return EJurassicBiomeType::DenseForest; // Default fallback
    }

    // Check biome override map first
    if (BiomeOverrideMap)
    {
        FVector2D UV = WorldToUV(WorldLocation);
        float OverrideValue = SampleTexture(BiomeOverrideMap, UV);
        
        if (OverrideValue > 0.1f) // If there's a manual override
        {
            // Convert texture value to biome type
            int32 BiomeIndex = FMath::FloorToInt(OverrideValue * 8.0f);
            BiomeIndex = FMath::Clamp(BiomeIndex, 0, 7);
            return static_cast<EJurassicBiomeType>(BiomeIndex);
        }
    }

    // Determine biome based on environmental conditions
    return DetermineBiomeType(WorldLocation);
}

FJurassicBiomeData AJurassicBiomeManager::GetBiomeDataAtLocation(FVector WorldLocation) const
{
    if (!BiomeDataAsset)
    {
        return FJurassicBiomeData(); // Return default biome data
    }

    EJurassicBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    
    // Find matching biome configuration
    for (const FJurassicBiomeData& BiomeData : BiomeDataAsset->BiomeConfigurations)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            return BiomeData;
        }
    }

    // Return first biome as fallback
    if (BiomeDataAsset->BiomeConfigurations.Num() > 0)
    {
        return BiomeDataAsset->BiomeConfigurations[0];
    }

    return FJurassicBiomeData();
}

TArray<EJurassicBiomeType> AJurassicBiomeManager::GetBiomesInRadius(FVector WorldLocation, float Radius) const
{
    TArray<EJurassicBiomeType> BiomesFound;
    
    // Sample biomes in a grid pattern within the radius
    int32 SampleCount = 16; // 4x4 grid
    float SampleSpacing = Radius * 2.0f / SampleCount;
    
    for (int32 X = 0; X < SampleCount; X++)
    {
        for (int32 Y = 0; Y < SampleCount; Y++)
        {
            FVector SampleLocation = WorldLocation + FVector(
                (X - SampleCount * 0.5f) * SampleSpacing,
                (Y - SampleCount * 0.5f) * SampleSpacing,
                0.0f
            );
            
            // Check if sample point is within radius
            float Distance = FVector::Dist2D(WorldLocation, SampleLocation);
            if (Distance <= Radius)
            {
                EJurassicBiomeType BiomeType = GetBiomeAtLocation(SampleLocation);
                BiomesFound.AddUnique(BiomeType);
            }
        }
    }
    
    return BiomesFound;
}

float AJurassicBiomeManager::GetMoistureAtLocation(FVector WorldLocation) const
{
    if (!BiomeDataAsset)
    {
        return 0.65f; // Default Jurassic moisture level
    }

    if (MoistureMap)
    {
        FVector2D UV = WorldToUV(WorldLocation);
        return SampleTexture(MoistureMap, UV);
    }

    // Use global moisture with some variation based on elevation and water proximity
    float BaseMoisture = BiomeDataAsset->GlobalMoisture;
    
    // Reduce moisture at higher elevations
    float Elevation = GetElevationAtLocation(WorldLocation);
    float ElevationFactor = FMath::Clamp(1.0f - (Elevation / 200000.0f), 0.3f, 1.0f); // 2km max elevation
    
    return BaseMoisture * ElevationFactor;
}

float AJurassicBiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    if (!BiomeDataAsset)
    {
        return 0.75f; // Default warm Jurassic temperature
    }

    if (TemperatureMap)
    {
        FVector2D UV = WorldToUV(WorldLocation);
        return SampleTexture(TemperatureMap, UV);
    }

    // Use global temperature with elevation-based cooling
    float BaseTemperature = BiomeDataAsset->GlobalTemperature;
    
    // Temperature decreases with elevation (lapse rate)
    float Elevation = GetElevationAtLocation(WorldLocation);
    float TemperatureReduction = (Elevation / 100000.0f) * 0.1f; // 0.1 reduction per 1km elevation
    
    return FMath::Clamp(BaseTemperature - TemperatureReduction, 0.0f, 1.0f);
}

float AJurassicBiomeManager::GetElevationAtLocation(FVector WorldLocation) const
{
    if (ElevationMap)
    {
        FVector2D UV = WorldToUV(WorldLocation);
        float ElevationValue = SampleTexture(ElevationMap, UV);
        
        // Convert from [0,1] to actual elevation range
        return ElevationValue * 200000.0f; // 0-2km elevation range
    }

    // Fallback: sample from landscape if available
    UWorld* World = GetWorld();
    if (World)
    {
        ALandscape* Landscape = nullptr;
        for (TActorIterator<ALandscape> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            Landscape = *ActorIterator;
            break;
        }

        if (Landscape)
        {
            FVector LandscapeLocation = WorldLocation;
            LandscapeLocation.Z = 0.0f; // Sample at ground level
            
            float Height = 0.0f;
            if (Landscape->GetHeightAtLocation(LandscapeLocation, Height))
            {
                return Height;
            }
        }
    }

    return 0.0f; // Sea level fallback
}

float AJurassicBiomeManager::GetSlopeAtLocation(FVector WorldLocation) const
{
    // Calculate slope by sampling elevation at nearby points
    float SampleDistance = 100.0f; // 1m sample distance
    
    float CenterHeight = GetElevationAtLocation(WorldLocation);
    float EastHeight = GetElevationAtLocation(WorldLocation + FVector(SampleDistance, 0.0f, 0.0f));
    float NorthHeight = GetElevationAtLocation(WorldLocation + FVector(0.0f, SampleDistance, 0.0f));
    
    // Calculate gradient
    float DX = (EastHeight - CenterHeight) / SampleDistance;
    float DY = (NorthHeight - CenterHeight) / SampleDistance;
    
    // Calculate slope angle in degrees
    float SlopeRadians = FMath::Atan(FMath::Sqrt(DX * DX + DY * DY));
    return FMath::RadiansToDegrees(SlopeRadians);
}

void AJurassicBiomeManager::GenerateBiomeDistribution()
{
    if (!BiomeDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicBiomeManager: Cannot generate biomes without BiomeDataAsset"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicBiomeManager: Generating biome distribution..."));

    // Clear any existing generation
    ClearBiomeGeneration();

    // Trigger PCG generation for biomes
    if (BiomePCGComponent)
    {
        BiomePCGComponent->GenerateLocal(true);
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicBiomeManager: Biome distribution generation completed"));
}

void AJurassicBiomeManager::RefreshBiomeGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicBiomeManager: Refreshing biome generation..."));
    
    if (BiomePCGComponent)
    {
        BiomePCGComponent->CleanupLocal(true);
        BiomePCGComponent->GenerateLocal(true);
    }
}

void AJurassicBiomeManager::ClearBiomeGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicBiomeManager: Clearing biome generation..."));
    
    if (BiomePCGComponent)
    {
        BiomePCGComponent->CleanupLocal(true);
    }
}

FVector2D AJurassicBiomeManager::WorldToUV(FVector WorldLocation) const
{
    // Convert world location to UV coordinates [0,1]
    FVector RelativeLocation = WorldLocation - WorldOrigin;
    
    float U = FMath::Clamp(RelativeLocation.X / WorldSize.X + 0.5f, 0.0f, 1.0f);
    float V = FMath::Clamp(RelativeLocation.Y / WorldSize.Y + 0.5f, 0.0f, 1.0f);
    
    return FVector2D(U, V);
}

float AJurassicBiomeManager::SampleTexture(UTexture2D* Texture, FVector2D UV) const
{
    if (!Texture)
    {
        return 0.0f;
    }

    // Clamp UV coordinates
    UV.X = FMath::Clamp(UV.X, 0.0f, 1.0f);
    UV.Y = FMath::Clamp(UV.Y, 0.0f, 1.0f);

    // Convert UV to pixel coordinates
    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();
    
    int32 X = FMath::Clamp(FMath::FloorToInt(UV.X * Width), 0, Width - 1);
    int32 Y = FMath::Clamp(FMath::FloorToInt(UV.Y * Height), 0, Height - 1);

    // For now, return a procedural value based on UV
    // In a real implementation, you would sample the actual texture data
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(UV.X * 10.0f, UV.Y * 10.0f));
    return FMath::Clamp((NoiseValue + 1.0f) * 0.5f, 0.0f, 1.0f);
}

float AJurassicBiomeManager::CalculateBiomeSuitability(const FJurassicBiomeData& BiomeData, FVector WorldLocation) const
{
    float Suitability = 1.0f;

    // Check elevation constraints
    float Elevation = GetElevationAtLocation(WorldLocation);
    if (Elevation < BiomeData.ElevationMin || Elevation > BiomeData.ElevationMax)
    {
        float ElevationDistance = FMath::Min(
            FMath::Abs(Elevation - BiomeData.ElevationMin),
            FMath::Abs(Elevation - BiomeData.ElevationMax)
        );
        Suitability *= FMath::Exp(-ElevationDistance / 10000.0f); // 100m falloff
    }

    // Check slope constraints
    float Slope = GetSlopeAtLocation(WorldLocation);
    if (Slope < BiomeData.SlopeMin || Slope > BiomeData.SlopeMax)
    {
        float SlopeDistance = FMath::Min(
            FMath::Abs(Slope - BiomeData.SlopeMin),
            FMath::Abs(Slope - BiomeData.SlopeMax)
        );
        Suitability *= FMath::Exp(-SlopeDistance / 5.0f); // 5 degree falloff
    }

    // Check moisture constraints
    float Moisture = GetMoistureAtLocation(WorldLocation);
    if (Moisture < BiomeData.MoistureMin || Moisture > BiomeData.MoistureMax)
    {
        float MoistureDistance = FMath::Min(
            FMath::Abs(Moisture - BiomeData.MoistureMin),
            FMath::Abs(Moisture - BiomeData.MoistureMax)
        );
        Suitability *= FMath::Exp(-MoistureDistance / 0.1f); // 0.1 moisture falloff
    }

    // Check temperature constraints
    float Temperature = GetTemperatureAtLocation(WorldLocation);
    if (Temperature < BiomeData.TemperatureMin || Temperature > BiomeData.TemperatureMax)
    {
        float TemperatureDistance = FMath::Min(
            FMath::Abs(Temperature - BiomeData.TemperatureMin),
            FMath::Abs(Temperature - BiomeData.TemperatureMax)
        );
        Suitability *= FMath::Exp(-TemperatureDistance / 0.1f); // 0.1 temperature falloff
    }

    // Check water proximity requirements
    if (BiomeData.bRequiresWaterProximity)
    {
        // For now, use a simple distance calculation
        // In a real implementation, this would check actual water body distances
        float WaterDistance = FMath::Abs(FMath::Sin(WorldLocation.X * 0.001f) * FMath::Sin(WorldLocation.Y * 0.001f)) * 1000.0f;
        
        if (WaterDistance > BiomeData.MaxWaterDistance)
        {
            float DistanceExcess = WaterDistance - BiomeData.MaxWaterDistance;
            Suitability *= FMath::Exp(-DistanceExcess / 500.0f); // 500m falloff
        }
    }

    return FMath::Clamp(Suitability, 0.0f, 1.0f);
}

EJurassicBiomeType AJurassicBiomeManager::DetermineBiomeType(FVector WorldLocation) const
{
    if (!BiomeDataAsset || BiomeDataAsset->BiomeConfigurations.Num() == 0)
    {
        return EJurassicBiomeType::DenseForest;
    }

    float BestSuitability = 0.0f;
    EJurassicBiomeType BestBiome = EJurassicBiomeType::DenseForest;

    // Evaluate suitability for each biome type
    for (const FJurassicBiomeData& BiomeData : BiomeDataAsset->BiomeConfigurations)
    {
        float Suitability = CalculateBiomeSuitability(BiomeData, WorldLocation);
        
        if (Suitability > BestSuitability)
        {
            BestSuitability = Suitability;
            BestBiome = BiomeData.BiomeType;
        }
    }

    return BestBiome;
}