#include "Eng_TerrainSystemManager.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UEng_TerrainSystemManager::UEng_TerrainSystemManager()
{
    bIsInitialized = false;
    CurrentLandscape = nullptr;
    LODUpdateDistance = 5000.0f;
    MaxLODLevel = 4;
    
    // Initialize default configuration
    DefaultConfiguration.HeightScale = 512.0f;
    DefaultConfiguration.NoiseScale = 0.001f;
    DefaultConfiguration.TerrainSize = 2048;
    DefaultConfiguration.bEnableErosion = true;
    DefaultConfiguration.ErosionStrength = 0.5f;
    DefaultConfiguration.ErosionIterations = 10;
}

void UEng_TerrainSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Initializing terrain system"));
    
    CreateDefaultLayers();
    InitializeTerrainSystem();
}

void UEng_TerrainSystemManager::Deinitialize()
{
    bIsInitialized = false;
    CurrentLandscape = nullptr;
    TerrainLayers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Deinitializing terrain system"));
    
    Super::Deinitialize();
}

void UEng_TerrainSystemManager::InitializeTerrainSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainSystemManager: No valid world found"));
        return;
    }
    
    // Find existing landscape in the world
    for (TActorIterator<ALandscapeProxy> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscapeProxy* Landscape = *ActorItr;
        if (Landscape)
        {
            CurrentLandscape = Landscape;
            break;
        }
    }
    
    if (CurrentLandscape)
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Found existing landscape: %s"), *CurrentLandscape->GetName());
        CacheTerrainData();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainSystemManager: No landscape found in world"));
    }
    
    bIsInitialized = true;
}

void UEng_TerrainSystemManager::GenerateTerrain(const FEng_TerrainConfiguration& Config)
{
    if (!bIsInitialized)
    {
        InitializeTerrainSystem();
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Generating terrain with size %d"), Config.TerrainSize);
    
    // Store configuration for runtime use
    DefaultConfiguration = Config;
    
    // If we have a landscape, modify its heightmap
    if (CurrentLandscape)
    {
        ApplyHeightmapModifications(TArray<float>());
        
        if (Config.bEnableErosion)
        {
            ApplyErosion(Config.ErosionStrength, Config.ErosionIterations);
        }
    }
}

void UEng_TerrainSystemManager::ApplyErosion(float Strength, int32 Iterations)
{
    if (!CurrentLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainSystemManager: Cannot apply erosion - no landscape found"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Applying erosion with strength %f for %d iterations"), Strength, Iterations);
    
    // Simplified erosion simulation
    for (int32 i = 0; i < Iterations; ++i)
    {
        // This would typically involve complex heightmap manipulation
        // For now, we log the process
        UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainSystemManager: Erosion iteration %d/%d"), i + 1, Iterations);
    }
}

void UEng_TerrainSystemManager::AddTerrainLayer(const FEng_TerrainLayer& Layer)
{
    // Check if layer already exists
    for (int32 i = 0; i < TerrainLayers.Num(); ++i)
    {
        if (TerrainLayers[i].LayerName == Layer.LayerName)
        {
            TerrainLayers[i] = Layer;
            UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Updated existing layer: %s"), *Layer.LayerName);
            return;
        }
    }
    
    TerrainLayers.Add(Layer);
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Added new terrain layer: %s"), *Layer.LayerName);
}

void UEng_TerrainSystemManager::RemoveTerrainLayer(const FString& LayerName)
{
    for (int32 i = TerrainLayers.Num() - 1; i >= 0; --i)
    {
        if (TerrainLayers[i].LayerName == LayerName)
        {
            TerrainLayers.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Removed terrain layer: %s"), *LayerName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TerrainSystemManager: Layer not found for removal: %s"), *LayerName);
}

float UEng_TerrainSystemManager::GetHeightAtLocation(const FVector& WorldLocation) const
{
    if (!CurrentLandscape)
    {
        return 0.0f;
    }
    
    // Use landscape's built-in height sampling
    FVector LandscapeLocation = CurrentLandscape->GetTransform().InverseTransformPosition(WorldLocation);
    
    // Sample height from landscape
    float Height = 0.0f;
    if (ULandscapeInfo* LandscapeInfo = CurrentLandscape->GetLandscapeInfo())
    {
        Height = LandscapeInfo->GetHeightAtLocation(FVector2D(LandscapeLocation.X, LandscapeLocation.Y));
    }
    
    return Height;
}

FVector UEng_TerrainSystemManager::GetSurfaceNormalAtLocation(const FVector& WorldLocation) const
{
    if (!CurrentLandscape)
    {
        return FVector::UpVector;
    }
    
    // Calculate surface normal by sampling nearby heights
    const float SampleDistance = 100.0f;
    
    float HeightCenter = GetHeightAtLocation(WorldLocation);
    float HeightX = GetHeightAtLocation(WorldLocation + FVector(SampleDistance, 0, 0));
    float HeightY = GetHeightAtLocation(WorldLocation + FVector(0, SampleDistance, 0));
    
    FVector Normal = FVector(HeightCenter - HeightX, HeightCenter - HeightY, SampleDistance).GetSafeNormal();
    return Normal;
}

EBiomeType UEng_TerrainSystemManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float Height = GetHeightAtLocation(WorldLocation);
    
    // Simple height-based biome determination
    if (Height < 100.0f)
    {
        return EBiomeType::Wetlands;
    }
    else if (Height < 300.0f)
    {
        return EBiomeType::Plains;
    }
    else if (Height < 600.0f)
    {
        return EBiomeType::Forest;
    }
    else
    {
        return EBiomeType::Mountains;
    }
}

bool UEng_TerrainSystemManager::IsLocationWalkable(const FVector& WorldLocation, float MaxSlope) const
{
    FVector SurfaceNormal = GetSurfaceNormalAtLocation(WorldLocation);
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
    
    return SlopeAngle <= MaxSlope;
}

TArray<FVector> UEng_TerrainSystemManager::GenerateSpawnPoints(int32 NumPoints, float MinDistance) const
{
    TArray<FVector> SpawnPoints;
    
    if (!CurrentLandscape)
    {
        return SpawnPoints;
    }
    
    // Get landscape bounds
    FBox LandscapeBounds = CurrentLandscape->GetComponentsBoundingBox();
    
    for (int32 i = 0; i < NumPoints; ++i)
    {
        bool bValidPoint = false;
        int32 Attempts = 0;
        const int32 MaxAttempts = 100;
        
        while (!bValidPoint && Attempts < MaxAttempts)
        {
            // Generate random point within landscape bounds
            FVector TestPoint = FVector(
                FMath::RandRange(LandscapeBounds.Min.X, LandscapeBounds.Max.X),
                FMath::RandRange(LandscapeBounds.Min.Y, LandscapeBounds.Max.Y),
                0.0f
            );
            
            // Adjust Z to terrain height
            TestPoint.Z = GetHeightAtLocation(TestPoint) + 100.0f; // Add offset above ground
            
            // Check if point is walkable and far enough from other points
            if (IsLocationWalkable(TestPoint))
            {
                bool bTooClose = false;
                for (const FVector& ExistingPoint : SpawnPoints)
                {
                    if (FVector::Dist(TestPoint, ExistingPoint) < MinDistance)
                    {
                        bTooClose = true;
                        break;
                    }
                }
                
                if (!bTooClose)
                {
                    SpawnPoints.Add(TestPoint);
                    bValidPoint = true;
                }
            }
            
            ++Attempts;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Generated %d spawn points out of %d requested"), SpawnPoints.Num(), NumPoints);
    return SpawnPoints;
}

void UEng_TerrainSystemManager::UpdateTerrainLOD(const FVector& ViewerLocation)
{
    if (!CurrentLandscape)
    {
        return;
    }
    
    // Calculate distance-based LOD
    float DistanceToViewer = FVector::Dist(ViewerLocation, CurrentLandscape->GetActorLocation());
    int32 DesiredLOD = FMath::Clamp(FMath::FloorToInt(DistanceToViewer / LODUpdateDistance), 0, MaxLODLevel);
    
    // Apply LOD to landscape components (simplified)
    UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainSystemManager: Updating terrain LOD to level %d"), DesiredLOD);
}

void UEng_TerrainSystemManager::ValidateTerrainSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN SYSTEM VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bIsInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Current Landscape: %s"), CurrentLandscape ? *CurrentLandscape->GetName() : TEXT("NONE"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Layers: %d"), TerrainLayers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Height Scale: %f"), DefaultConfiguration.HeightScale);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Size: %d"), DefaultConfiguration.TerrainSize);
    
    for (const FEng_TerrainLayer& Layer : TerrainLayers)
    {
        UE_LOG(LogTemp, Warning, TEXT("Layer: %s (Active: %s, Height: %f-%f)"), 
               *Layer.LayerName, 
               Layer.bIsActive ? TEXT("YES") : TEXT("NO"),
               Layer.MinHeight, 
               Layer.MaxHeight);
    }
}

void UEng_TerrainSystemManager::CreateDefaultLayers()
{
    TerrainLayers.Empty();
    
    // Create default terrain layers
    FEng_TerrainLayer GrassLayer;
    GrassLayer.LayerName = TEXT("Grass");
    GrassLayer.MinHeight = 0.0f;
    GrassLayer.MaxHeight = 200.0f;
    GrassLayer.BlendRange = 50.0f;
    GrassLayer.bIsActive = true;
    TerrainLayers.Add(GrassLayer);
    
    FEng_TerrainLayer RockLayer;
    RockLayer.LayerName = TEXT("Rock");
    RockLayer.MinHeight = 150.0f;
    RockLayer.MaxHeight = 500.0f;
    RockLayer.BlendRange = 100.0f;
    RockLayer.bIsActive = true;
    TerrainLayers.Add(RockLayer);
    
    FEng_TerrainLayer SnowLayer;
    SnowLayer.LayerName = TEXT("Snow");
    SnowLayer.MinHeight = 450.0f;
    SnowLayer.MaxHeight = 1000.0f;
    SnowLayer.BlendRange = 150.0f;
    SnowLayer.bIsActive = true;
    TerrainLayers.Add(SnowLayer);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Created %d default terrain layers"), TerrainLayers.Num());
}

void UEng_TerrainSystemManager::CacheTerrainData()
{
    if (!CurrentLandscape)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Caching terrain data for landscape: %s"), *CurrentLandscape->GetName());
    
    // Cache important landscape properties for faster access
    FBox Bounds = CurrentLandscape->GetComponentsBoundingBox();
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Landscape bounds: %s"), *Bounds.ToString());
}

float UEng_TerrainSystemManager::CalculateNoiseValue(float X, float Y, float Scale) const
{
    // Simple Perlin-like noise calculation
    float Noise = FMath::PerlinNoise2D(FVector2D(X * Scale, Y * Scale));
    return FMath::Clamp(Noise, -1.0f, 1.0f);
}

void UEng_TerrainSystemManager::ApplyHeightmapModifications(const TArray<float>& HeightData)
{
    if (!CurrentLandscape)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainSystemManager: Applying heightmap modifications"));
    
    // This would typically involve direct heightmap manipulation
    // For now, we simulate the process
}