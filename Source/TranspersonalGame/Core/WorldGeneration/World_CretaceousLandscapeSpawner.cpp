#include "World_CretaceousLandscapeSpawner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeEditorObject.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "World_BiomeManager.h"
#include "World_BiomeClimateSystem.h"

AWorld_CretaceousLandscapeSpawner::AWorld_CretaceousLandscapeSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default landscape parameters
    LandscapeSizeKm = 10.0f;
    ComponentCountPerAxis = 8;
    QuadsPerComponent = 127;
    TerrainHeightScale = 2000.0f;
    TerrainRoughness = 1.5f;
    bEnableWorldPartition = true;
    bEnableLandscapeLOD = true;
    
    // Initialize state
    MainLandscape = nullptr;
    BiomeManager = nullptr;
    ClimateSystem = nullptr;
    bLandscapeCreated = false;
    LastErrorMessage = TEXT("");
    LandscapeEditorObject = nullptr;
    LastCreationTime = 0.0f;
    CreationAttempts = 0;
    
    InitializeLandscapeParameters();
}

void AWorld_CretaceousLandscapeSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    // Find biome manager in world
    BiomeManager = GetWorld()->GetSubsystem<UWorld_BiomeManager>();
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeSpawner: BiomeManager not found"));
    }
    
    // Find climate system
    ClimateSystem = GetWorld()->GetSubsystem<UWorld_BiomeClimateSystem>();
    if (!ClimateSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeSpawner: ClimateSystem not found"));
    }
}

void AWorld_CretaceousLandscapeSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor landscape health if created
    if (MainLandscape && bLandscapeCreated)
    {
        // Check if landscape is still valid
        if (!IsValid(MainLandscape))
        {
            UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeSpawner: MainLandscape became invalid"));
            bLandscapeCreated = false;
            MainLandscape = nullptr;
        }
    }
}

ALandscape* AWorld_CretaceousLandscapeSpawner::CreateMainLandscape(float SizeKm, EWorld_BiomeType BiomeLayout)
{
    UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeSpawner: Starting landscape creation - %f km"), SizeKm);
    
    if (!ValidateLandscapeParameters())
    {
        LastErrorMessage = TEXT("Invalid landscape parameters");
        UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeSpawner: %s"), *LastErrorMessage);
        return nullptr;
    }
    
    CreationAttempts++;
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Calculate landscape dimensions
    int32 TotalQuads = ComponentCountPerAxis * QuadsPerComponent;
    int32 HeightmapSize = TotalQuads + 1;
    float WorldSize = SizeKm * 100000.0f; // Convert km to cm
    float QuadSize = WorldSize / TotalQuads;
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Dimensions - %d components, %d quads, %f cm quad size"), 
           ComponentCountPerAxis, TotalQuads, QuadSize);
    
    // Check if landscape already exists
    TArray<AActor*> ExistingLandscapes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), ExistingLandscapes);
    
    if (ExistingLandscapes.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeSpawner: Found %d existing landscapes"), ExistingLandscapes.Num());
        
        // Use existing landscape if suitable
        for (AActor* Actor : ExistingLandscapes)
        {
            ALandscape* ExistingLandscape = Cast<ALandscape>(Actor);
            if (ExistingLandscape)
            {
                FBox Bounds = ExistingLandscape->GetComponentsBoundingBox();
                float ExistingSize = FMath::Max(Bounds.GetSize().X, Bounds.GetSize().Y);
                
                if (ExistingSize >= WorldSize * 0.8f) // Within 80% of target size
                {
                    MainLandscape = ExistingLandscape;
                    bLandscapeCreated = true;
                    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Using existing landscape %s"), *ExistingLandscape->GetName());
                    return MainLandscape;
                }
            }
        }
    }
    
    // Generate heightmap for new landscape
    TArray<uint16> HeightmapData = GenerateCretaceousHeightmap(HeightmapSize, HeightmapSize, EWorld_TerrainType::CretaceousPlains);
    
    if (HeightmapData.Num() == 0)
    {
        LastErrorMessage = TEXT("Failed to generate heightmap data");
        UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeSpawner: %s"), *LastErrorMessage);
        return nullptr;
    }
    
    // Create landscape using spawn actor
    FVector LandscapeLocation = GetActorLocation();
    FRotator LandscapeRotation = GetActorRotation();
    
    MainLandscape = GetWorld()->SpawnActor<ALandscape>(ALandscape::StaticClass(), LandscapeLocation, LandscapeRotation);
    
    if (!MainLandscape)
    {
        LastErrorMessage = TEXT("Failed to spawn landscape actor");
        UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeSpawner: %s"), *LastErrorMessage);
        return nullptr;
    }
    
    // Configure landscape properties
    MainLandscape->SetActorLabel(FString::Printf(TEXT("MainLandscape_Cretaceous_%dkm"), FMath::RoundToInt(SizeKm)));
    
    // Setup landscape materials and collision
    SetupLandscapeMaterials(MainLandscape);
    ConfigureLandscapeCollision(MainLandscape);
    
    // Apply biome-specific modifications
    if (BiomeManager)
    {
        FBox LandscapeBounds = MainLandscape->GetComponentsBoundingBox();
        ApplyBiomeTerrainModifications(MainLandscape, BiomeLayout, LandscapeBounds);
    }
    
    // Create streaming proxies if World Partition is enabled
    if (bEnableWorldPartition && SizeKm >= 5.0f)
    {
        CreateLandscapeStreamingProxies(MainLandscape, 200000.0f);
    }
    
    bLandscapeCreated = true;
    LastCreationTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Successfully created landscape in %f seconds"), LastCreationTime);
    
    return MainLandscape;
}

TArray<uint16> AWorld_CretaceousLandscapeSpawner::GenerateCretaceousHeightmap(int32 Width, int32 Height, EWorld_TerrainType TerrainType)
{
    TArray<uint16> HeightmapData;
    HeightmapData.SetNum(Width * Height);
    
    // Generate base Perlin noise heightmap
    TArray<uint16> BaseHeightmap = GeneratePerlinHeightmap(Width, Height, 0.01f, 6);
    
    if (BaseHeightmap.Num() != HeightmapData.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeSpawner: Heightmap size mismatch"));
        return TArray<uint16>();
    }
    
    // Copy base heightmap
    for (int32 i = 0; i < HeightmapData.Num(); i++)
    {
        HeightmapData[i] = BaseHeightmap[i];
    }
    
    // Apply geological features based on terrain type
    ApplyGeologicalFeatures(HeightmapData, Width, Height);
    
    // Apply Cretaceous-specific terrain characteristics
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            float NormalizedX = (float)X / (float)Width;
            float NormalizedY = (float)Y / (float)Height;
            
            // Create river valleys (lower elevation in center areas)
            float DistanceFromCenter = FVector2D(NormalizedX - 0.5f, NormalizedY - 0.5f).Size();
            if (DistanceFromCenter < 0.3f)
            {
                float ValleyDepth = (0.3f - DistanceFromCenter) / 0.3f;
                HeightmapData[Index] = FMath::Lerp(HeightmapData[Index], HeightmapData[Index] * 0.7f, ValleyDepth * 0.5f);
            }
            
            // Create coastal lowlands (lower elevation near edges)
            float DistanceFromEdge = FMath::Min({NormalizedX, 1.0f - NormalizedX, NormalizedY, 1.0f - NormalizedY});
            if (DistanceFromEdge < 0.1f)
            {
                float CoastalFactor = DistanceFromEdge / 0.1f;
                HeightmapData[Index] = FMath::Lerp(HeightmapData[Index] * 0.3f, HeightmapData[Index], CoastalFactor);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Generated heightmap %dx%d with %d points"), Width, Height, HeightmapData.Num());
    
    return HeightmapData;
}

void AWorld_CretaceousLandscapeSpawner::ApplyBiomeTerrainModifications(ALandscape* Landscape, EWorld_BiomeType BiomeType, FBox BiomeBounds)
{
    if (!Landscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeSpawner: Cannot apply biome modifications to null landscape"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Applying biome modifications for type %d"), (int32)BiomeType);
    
    // Apply biome-specific terrain characteristics
    switch (BiomeType)
    {
        case EWorld_BiomeType::CretaceousForest:
            // Gentle rolling hills for forest
            break;
            
        case EWorld_BiomeType::CretaceousSwampland:
            // Low, flat terrain with water features
            break;
            
        case EWorld_BiomeType::CretaceousDesert:
            // Sand dunes and rocky outcrops
            break;
            
        case EWorld_BiomeType::CretaceousMountains:
            // High elevation with steep slopes
            break;
            
        default:
            // Default plains terrain
            break;
    }
}

void AWorld_CretaceousLandscapeSpawner::CreateLandscapeStreamingProxies(ALandscape* MainLandscape, float ProxySize)
{
    if (!MainLandscape || !bEnableWorldPartition)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Creating streaming proxies with size %f"), ProxySize);
    
    // Clear existing proxies
    StreamingProxies.Empty();
    
    // Calculate number of proxies needed
    FBox LandscapeBounds = MainLandscape->GetComponentsBoundingBox();
    float LandscapeSize = FMath::Max(LandscapeBounds.GetSize().X, LandscapeBounds.GetSize().Y);
    int32 ProxiesPerAxis = FMath::CeilToInt(LandscapeSize / ProxySize);
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Creating %dx%d streaming proxies"), ProxiesPerAxis, ProxiesPerAxis);
    
    // Create streaming proxies (placeholder implementation)
    // Note: Full implementation would require UE5 World Partition API
    for (int32 X = 0; X < ProxiesPerAxis; X++)
    {
        for (int32 Y = 0; Y < ProxiesPerAxis; Y++)
        {
            FVector ProxyLocation = LandscapeBounds.Min + FVector(X * ProxySize, Y * ProxySize, 0.0f);
            
            // Placeholder: In full implementation, create ALandscapeStreamingProxy here
            UE_LOG(LogTemp, VeryVerbose, TEXT("CretaceousLandscapeSpawner: Would create proxy at %s"), *ProxyLocation.ToString());
        }
    }
}

bool AWorld_CretaceousLandscapeSpawner::ValidateLandscapeParameters()
{
    if (LandscapeSizeKm < 1.0f || LandscapeSizeKm > 50.0f)
    {
        LastErrorMessage = FString::Printf(TEXT("Invalid landscape size: %f km (must be 1-50 km)"), LandscapeSizeKm);
        return false;
    }
    
    if (ComponentCountPerAxis < 1 || ComponentCountPerAxis > 32)
    {
        LastErrorMessage = FString::Printf(TEXT("Invalid component count: %d (must be 1-32)"), ComponentCountPerAxis);
        return false;
    }
    
    if (QuadsPerComponent < 7 || QuadsPerComponent > 255)
    {
        LastErrorMessage = FString::Printf(TEXT("Invalid quads per component: %d (must be 7-255)"), QuadsPerComponent);
        return false;
    }
    
    return true;
}

float AWorld_CretaceousLandscapeSpawner::CalculateLandscapeMemoryUsage()
{
    int32 TotalQuads = ComponentCountPerAxis * QuadsPerComponent;
    int32 HeightmapSize = TotalQuads + 1;
    
    // Estimate memory usage (heightmap + weightmaps + collision)
    float HeightmapMemory = (HeightmapSize * HeightmapSize * 2) / (1024.0f * 1024.0f); // 2 bytes per pixel
    float WeightmapMemory = HeightmapMemory * 4; // Assume 4 layers
    float CollisionMemory = HeightmapMemory * 0.5f; // Collision data
    
    return HeightmapMemory + WeightmapMemory + CollisionMemory;
}

FBox AWorld_CretaceousLandscapeSpawner::GetLandscapeWorldBounds()
{
    if (MainLandscape)
    {
        return MainLandscape->GetComponentsBoundingBox();
    }
    
    // Return estimated bounds based on parameters
    float WorldSize = LandscapeSizeKm * 100000.0f; // Convert km to cm
    FVector Center = GetActorLocation();
    FVector Extent(WorldSize * 0.5f, WorldSize * 0.5f, TerrainHeightScale);
    
    return FBox(Center - Extent, Center + Extent);
}

void AWorld_CretaceousLandscapeSpawner::InitializeLandscapeParameters()
{
    // Initialize biome configurations
    BiomeConfigurations.Empty();
    
    // Setup default biome configs (placeholder)
    FWorld_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EWorld_BiomeType::CretaceousForest;
    ForestConfig.Temperature = 25.0f;
    ForestConfig.Humidity = 80.0f;
    BiomeConfigurations.Add(EWorld_BiomeType::CretaceousForest, ForestConfig);
    
    FWorld_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EWorld_BiomeType::CretaceousSwampland;
    SwampConfig.Temperature = 28.0f;
    SwampConfig.Humidity = 95.0f;
    BiomeConfigurations.Add(EWorld_BiomeType::CretaceousSwampland, SwampConfig);
}

TArray<uint16> AWorld_CretaceousLandscapeSpawner::GeneratePerlinHeightmap(int32 Width, int32 Height, float Scale, int32 Octaves)
{
    TArray<uint16> Heightmap;
    Heightmap.SetNum(Width * Height);
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            float NoiseValue = 0.0f;
            float Amplitude = 1.0f;
            float Frequency = Scale;
            
            // Generate multiple octaves of Perlin noise
            for (int32 Octave = 0; Octave < Octaves; Octave++)
            {
                float SampleX = X * Frequency;
                float SampleY = Y * Frequency;
                
                // Simple noise function (replace with proper Perlin noise in production)
                float Noise = FMath::Sin(SampleX) * FMath::Cos(SampleY) + 
                             FMath::Sin(SampleX * 2.0f) * FMath::Cos(SampleY * 2.0f) * 0.5f;
                
                NoiseValue += Noise * Amplitude;
                Amplitude *= 0.5f;
                Frequency *= 2.0f;
            }
            
            // Normalize and convert to uint16
            NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1
            NoiseValue = FMath::Clamp(NoiseValue, 0.0f, 1.0f);
            
            int32 Index = Y * Width + X;
            Heightmap[Index] = (uint16)(NoiseValue * 65535.0f * TerrainRoughness);
        }
    }
    
    return Heightmap;
}

void AWorld_CretaceousLandscapeSpawner::ApplyGeologicalFeatures(TArray<uint16>& Heightmap, int32 Width, int32 Height)
{
    // Apply erosion patterns
    for (int32 Y = 1; Y < Height - 1; Y++)
    {
        for (int32 X = 1; X < Width - 1; X++)
        {
            int32 Index = Y * Width + X;
            
            // Simple erosion: average with neighbors
            uint16 CurrentHeight = Heightmap[Index];
            uint16 AvgNeighbor = (Heightmap[(Y-1)*Width + X] + Heightmap[(Y+1)*Width + X] + 
                                 Heightmap[Y*Width + (X-1)] + Heightmap[Y*Width + (X+1)]) / 4;
            
            // Apply slight erosion
            Heightmap[Index] = FMath::Lerp(CurrentHeight, AvgNeighbor, 0.1f);
        }
    }
}

void AWorld_CretaceousLandscapeSpawner::SetupLandscapeMaterials(ALandscape* Landscape)
{
    if (!Landscape)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Setting up landscape materials"));
    
    // Placeholder for material setup
    // In production, load and assign proper landscape materials
}

void AWorld_CretaceousLandscapeSpawner::ConfigureLandscapeCollision(ALandscape* Landscape)
{
    if (!Landscape)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLandscapeSpawner: Configuring landscape collision"));
    
    // Enable collision for landscape
    Landscape->SetActorEnableCollision(true);
}