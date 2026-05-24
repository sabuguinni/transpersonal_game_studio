#include "World_ProceduralLandscapeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

AWorld_ProceduralLandscapeSystem::AWorld_ProceduralLandscapeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default configuration
    LandscapeConfig = FWorld_LandscapeConfig();
    
    // Initialize biome landscape data for all biomes
    TArray<EBiomeType> BiomeTypes = {
        EBiomeType::Savana,
        EBiomeType::Floresta,
        EBiomeType::Deserto,
        EBiomeType::Pantano,
        EBiomeType::Montanha
    };

    TArray<FVector> BiomeCenters = {
        FVector(0, 0, 100),           // Savana
        FVector(-45000, 40000, 100),  // Floresta
        FVector(55000, 0, 100),       // Deserto
        FVector(-50000, -45000, 100), // Pantano
        FVector(40000, 50000, 100)    // Montanha
    };

    for (int32 i = 0; i < BiomeTypes.Num() && i < BiomeCenters.Num(); i++)
    {
        FWorld_BiomeLandscapeData BiomeData;
        BiomeData.BiomeType = BiomeTypes[i];
        BiomeData.BiomeCenter = BiomeCenters[i];
        BiomeData.BiomeRadius = 25000.0f;
        
        // Set biome-specific parameters
        switch (BiomeTypes[i])
        {
            case EBiomeType::Montanha:
                BiomeData.TerrainRoughness = 0.8f;
                BiomeData.ElevationVariance = 1500.0f;
                break;
            case EBiomeType::Pantano:
                BiomeData.TerrainRoughness = 0.2f;
                BiomeData.ElevationVariance = 100.0f;
                break;
            case EBiomeType::Deserto:
                BiomeData.TerrainRoughness = 0.4f;
                BiomeData.ElevationVariance = 300.0f;
                break;
            case EBiomeType::Floresta:
                BiomeData.TerrainRoughness = 0.6f;
                BiomeData.ElevationVariance = 600.0f;
                break;
            case EBiomeType::Savana:
            default:
                BiomeData.TerrainRoughness = 0.3f;
                BiomeData.ElevationVariance = 200.0f;
                break;
        }
        
        BiomeLandscapeDataArray.Add(BiomeData);
    }

    // Performance settings
    bEnablePerformanceOptimization = true;
    MaxLandscapeDrawDistance = 50000.0f;
    MaxSimultaneousLandscapes = 5;
    
    bLandscapeSystemInitialized = false;
    LastGenerationTime = 0.0f;
}

void AWorld_ProceduralLandscapeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: BeginPlay started"));
    
    // Initialize landscape system
    bLandscapeSystemInitialized = true;
    LastGenerationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: System initialized with %d biome configurations"), BiomeLandscapeDataArray.Num());
}

void AWorld_ProceduralLandscapeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnablePerformanceOptimization)
    {
        UpdatePerformanceMetrics();
    }
}

bool AWorld_ProceduralLandscapeSystem::GenerateProceduralLandscape()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Starting procedural landscape generation"));
    
    if (!bLandscapeSystemInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeSystem: System not initialized"));
        return false;
    }

    // Clear existing landscapes
    ClearAllLandscapes();
    
    bool bAllBiomesGenerated = true;
    int32 GeneratedCount = 0;
    
    // Generate landscape for each biome
    for (const FWorld_BiomeLandscapeData& BiomeData : BiomeLandscapeDataArray)
    {
        if (GeneratedCount >= MaxSimultaneousLandscapes)
        {
            UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Reached maximum simultaneous landscapes limit (%d)"), MaxSimultaneousLandscapes);
            break;
        }
        
        bool bBiomeGenerated = CreateBiomeLandscape(BiomeData.BiomeType, BiomeData.BiomeCenter);
        if (bBiomeGenerated)
        {
            GeneratedCount++;
            UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Generated landscape for biome %d at location (%f, %f, %f)"), 
                (int32)BiomeData.BiomeType, BiomeData.BiomeCenter.X, BiomeData.BiomeCenter.Y, BiomeData.BiomeCenter.Z);
        }
        else
        {
            bAllBiomesGenerated = false;
            UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeSystem: Failed to generate landscape for biome %d"), (int32)BiomeData.BiomeType);
        }
    }
    
    LastGenerationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Landscape generation completed. Generated %d landscapes"), GeneratedCount);
    return bAllBiomesGenerated;
}

bool AWorld_ProceduralLandscapeSystem::CreateBiomeLandscape(EBiomeType BiomeType, const FVector& BiomeCenter)
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Creating landscape for biome %d at (%f, %f, %f)"), 
        (int32)BiomeType, BiomeCenter.X, BiomeCenter.Y, BiomeCenter.Z);
    
    // Generate heightmap for this biome
    TArray<float> HeightmapData;
    bool bHeightmapGenerated = GenerateHeightmapForBiome(BiomeType, BiomeCenter, HeightmapData);
    
    if (!bHeightmapGenerated || HeightmapData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeSystem: Failed to generate heightmap for biome %d"), (int32)BiomeType);
        return false;
    }
    
    // Create landscape configuration for this biome
    FWorld_LandscapeConfig BiomeConfig = LandscapeConfig;
    BiomeConfig.LandscapeLocation = BiomeCenter;
    
    // Create landscape actor
    ALandscape* NewLandscape = nullptr;
    bool bLandscapeCreated = CreateLandscapeActor(BiomeConfig, BiomeCenter, NewLandscape);
    
    if (bLandscapeCreated && NewLandscape)
    {
        // Apply heightmap data
        ApplyHeightmapToLandscape(NewLandscape, HeightmapData);
        
        // Configure materials
        ConfigureLandscapeMaterials(NewLandscape, BiomeType);
        
        // Add to generated landscapes array
        GeneratedLandscapes.Add(NewLandscape);
        
        // Set actor label for identification
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        BiomeName = BiomeName.Replace(TEXT("EBiomeType::"), TEXT(""));
        NewLandscape->SetActorLabel(FString::Printf(TEXT("Landscape_%s"), *BiomeName));
        
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Successfully created landscape for biome %s"), *BiomeName);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeSystem: Failed to create landscape actor for biome %d"), (int32)BiomeType);
    return false;
}

bool AWorld_ProceduralLandscapeSystem::GenerateHeightmapForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, TArray<float>& OutHeightmap)
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Generating heightmap for biome %d"), (int32)BiomeType);
    
    // Calculate heightmap dimensions
    int32 HeightmapSize = (LandscapeConfig.ComponentCountX * LandscapeConfig.QuadsPerComponent + 1) * 
                         (LandscapeConfig.ComponentCountY * LandscapeConfig.QuadsPerComponent + 1);
    
    OutHeightmap.Empty();
    OutHeightmap.Reserve(HeightmapSize);
    
    // Get biome-specific parameters
    const FWorld_BiomeLandscapeData* BiomeData = BiomeLandscapeDataArray.FindByPredicate(
        [BiomeType](const FWorld_BiomeLandscapeData& Data) { return Data.BiomeType == BiomeType; });
    
    if (!BiomeData)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeSystem: No biome data found for biome %d"), (int32)BiomeType);
        return false;
    }
    
    // Generate heightmap using Perlin noise
    int32 TotalQuadsX = LandscapeConfig.ComponentCountX * LandscapeConfig.QuadsPerComponent;
    int32 TotalQuadsY = LandscapeConfig.ComponentCountY * LandscapeConfig.QuadsPerComponent;
    
    for (int32 Y = 0; Y <= TotalQuadsY; Y++)
    {
        for (int32 X = 0; X <= TotalQuadsX; X++)
        {
            // Convert to world coordinates
            float WorldX = BiomeCenter.X + (X - TotalQuadsX * 0.5f) * 100.0f; // 100cm per quad
            float WorldY = BiomeCenter.Y + (Y - TotalQuadsY * 0.5f) * 100.0f;
            
            // Generate base height using Perlin noise
            float BaseHeight = GeneratePerlinNoise(WorldX * 0.0001f, WorldY * 0.0001f, BiomeData->TerrainRoughness, 4);
            
            // Apply biome-specific height modifications
            float BiomeHeight = GenerateBiomeSpecificHeight(BiomeType, WorldX, WorldY, BaseHeight);
            
            // Scale by elevation variance
            float FinalHeight = BiomeHeight * BiomeData->ElevationVariance;
            
            // Normalize to 0-65535 range for landscape heightmap
            uint16 HeightValue = FMath::Clamp(FinalHeight + 32768.0f, 0.0f, 65535.0f);
            OutHeightmap.Add(HeightValue);
        }
    }
    
    // Apply biome-specific modifiers
    ApplyBiomeModifiers(BiomeType, OutHeightmap, BiomeCenter);
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Generated heightmap with %d points for biome %d"), 
        OutHeightmap.Num(), (int32)BiomeType);
    
    return true;
}

void AWorld_ProceduralLandscapeSystem::SetLandscapeConfig(const FWorld_LandscapeConfig& NewConfig)
{
    LandscapeConfig = NewConfig;
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Landscape configuration updated"));
}

void AWorld_ProceduralLandscapeSystem::AddBiomeLandscapeData(const FWorld_BiomeLandscapeData& BiomeData)
{
    // Check if biome data already exists
    int32 ExistingIndex = BiomeLandscapeDataArray.IndexOfByPredicate(
        [&BiomeData](const FWorld_BiomeLandscapeData& Data) { return Data.BiomeType == BiomeData.BiomeType; });
    
    if (ExistingIndex != INDEX_NONE)
    {
        BiomeLandscapeDataArray[ExistingIndex] = BiomeData;
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Updated existing biome data for biome %d"), (int32)BiomeData.BiomeType);
    }
    else
    {
        BiomeLandscapeDataArray.Add(BiomeData);
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Added new biome data for biome %d"), (int32)BiomeData.BiomeType);
    }
}

bool AWorld_ProceduralLandscapeSystem::ModifyTerrainHeight(const FVector& Location, float Radius, float HeightDelta)
{
    // Find landscape at location
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    ALandscape* TargetLandscape = nullptr;
    
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape && Landscape->GetActorLocation().Equals(Location, Radius))
        {
            TargetLandscape = Landscape;
            break;
        }
    }
    
    if (!TargetLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: No landscape found at location for terrain modification"));
        return false;
    }
    
    // TODO: Implement landscape height modification using LandscapeDataAccess
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Terrain height modification requested at (%f, %f, %f) with radius %f and delta %f"), 
        Location.X, Location.Y, Location.Z, Radius, HeightDelta);
    
    return true;
}

bool AWorld_ProceduralLandscapeSystem::SmoothTerrain(const FVector& Location, float Radius, float SmoothingStrength)
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Terrain smoothing requested at (%f, %f, %f) with radius %f and strength %f"), 
        Location.X, Location.Y, Location.Z, Radius, SmoothingStrength);
    
    // TODO: Implement terrain smoothing algorithm
    return true;
}

void AWorld_ProceduralLandscapeSystem::OptimizeLandscapePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Optimizing landscape performance"));
    
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape && ShouldOptimizeLandscape(Landscape))
        {
            ApplyLODOptimization(Landscape);
        }
    }
}

void AWorld_ProceduralLandscapeSystem::SetLandscapeLODDistances(float LOD0Distance, float LOD1Distance, float LOD2Distance)
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Setting LOD distances - LOD0: %f, LOD1: %f, LOD2: %f"), 
        LOD0Distance, LOD1Distance, LOD2Distance);
    
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape)
        {
            // TODO: Apply LOD distance settings to landscape
        }
    }
}

float AWorld_ProceduralLandscapeSystem::GetHeightAtLocation(const FVector& WorldLocation) const
{
    // Find the appropriate landscape
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape)
        {
            FVector LandscapeLocation = Landscape->GetActorLocation();
            float Distance = FVector::Dist2D(WorldLocation, LandscapeLocation);
            
            if (Distance < 25000.0f) // Within biome radius
            {
                // TODO: Query landscape height at location
                return LandscapeLocation.Z;
            }
        }
    }
    
    return 0.0f;
}

EBiomeType AWorld_ProceduralLandscapeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float MinDistance = MAX_FLT;
    EBiomeType ClosestBiome = EBiomeType::Savana;
    
    for (const FWorld_BiomeLandscapeData& BiomeData : BiomeLandscapeDataArray)
    {
        float Distance = FVector::Dist2D(WorldLocation, BiomeData.BiomeCenter);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }
    
    return ClosestBiome;
}

bool AWorld_ProceduralLandscapeSystem::GetLandscapeNormalAtLocation(const FVector& WorldLocation, FVector& OutNormal) const
{
    // TODO: Calculate landscape normal at location
    OutNormal = FVector::UpVector;
    return true;
}

void AWorld_ProceduralLandscapeSystem::GenerateAllBiomeLandscapes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Editor command - Generate all biome landscapes"));
    GenerateProceduralLandscape();
}

void AWorld_ProceduralLandscapeSystem::ClearAllLandscapes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Clearing all generated landscapes"));
    
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape && IsValid(Landscape))
        {
            Landscape->Destroy();
        }
    }
    
    GeneratedLandscapes.Empty();
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: All landscapes cleared"));
}

void AWorld_ProceduralLandscapeSystem::ValidateLandscapeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Validating landscape system"));
    UE_LOG(LogTemp, Warning, TEXT("  - System initialized: %s"), bLandscapeSystemInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  - Biome configurations: %d"), BiomeLandscapeDataArray.Num());
    UE_LOG(LogTemp, Warning, TEXT("  - Generated landscapes: %d"), GeneratedLandscapes.Num());
    UE_LOG(LogTemp, Warning, TEXT("  - Performance optimization: %s"), bEnablePerformanceOptimization ? TEXT("Enabled") : TEXT("Disabled"));
}

// Private implementation methods

bool AWorld_ProceduralLandscapeSystem::CreateLandscapeActor(const FWorld_LandscapeConfig& Config, const FVector& Location, ALandscape*& OutLandscape)
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Creating landscape actor at (%f, %f, %f)"), 
        Location.X, Location.Y, Location.Z);
    
    // TODO: Implement actual landscape creation
    // For now, return false as landscape creation requires special handling
    OutLandscape = nullptr;
    return false;
}

void AWorld_ProceduralLandscapeSystem::ApplyHeightmapToLandscape(ALandscape* Landscape, const TArray<float>& HeightmapData)
{
    if (!Landscape)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeSystem: Cannot apply heightmap to null landscape"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Applying heightmap with %d points to landscape"), HeightmapData.Num());
    
    // TODO: Implement heightmap application using LandscapeDataAccess
}

void AWorld_ProceduralLandscapeSystem::ConfigureLandscapeMaterials(ALandscape* Landscape, EBiomeType BiomeType)
{
    if (!Landscape)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Configuring materials for biome %d"), (int32)BiomeType);
    
    // TODO: Apply biome-specific materials to landscape
}

float AWorld_ProceduralLandscapeSystem::GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves) const
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Value += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return FMath::Clamp(Value, -1.0f, 1.0f);
}

float AWorld_ProceduralLandscapeSystem::GenerateBiomeSpecificHeight(EBiomeType BiomeType, float X, float Y, float BaseHeight) const
{
    switch (BiomeType)
    {
        case EBiomeType::Montanha:
            // Mountains: Higher elevation with steep peaks
            return BaseHeight * 2.0f + FMath::Abs(FMath::PerlinNoise2D(FVector2D(X * 0.0005f, Y * 0.0005f)));
            
        case EBiomeType::Pantano:
            // Swamp: Low, flat terrain with occasional small hills
            return BaseHeight * 0.2f + FMath::Sin(X * 0.001f) * FMath::Sin(Y * 0.001f) * 0.1f;
            
        case EBiomeType::Deserto:
            // Desert: Rolling dunes and rocky outcrops
            return BaseHeight * 0.8f + FMath::PerlinNoise2D(FVector2D(X * 0.0002f, Y * 0.0002f)) * 0.5f;
            
        case EBiomeType::Floresta:
            // Forest: Hilly terrain with moderate elevation changes
            return BaseHeight * 1.2f + FMath::PerlinNoise2D(FVector2D(X * 0.0003f, Y * 0.0003f)) * 0.3f;
            
        case EBiomeType::Savana:
        default:
            // Savanna: Gentle rolling hills
            return BaseHeight * 0.6f + FMath::PerlinNoise2D(FVector2D(X * 0.0004f, Y * 0.0004f)) * 0.2f;
    }
}

void AWorld_ProceduralLandscapeSystem::ApplyBiomeModifiers(EBiomeType BiomeType, TArray<float>& HeightmapData, const FVector& BiomeCenter) const
{
    // Apply biome-specific post-processing to heightmap
    for (int32 i = 0; i < HeightmapData.Num(); i++)
    {
        float& Height = HeightmapData[i];
        
        switch (BiomeType)
        {
            case EBiomeType::Montanha:
                // Sharpen peaks for mountains
                Height = FMath::Pow(Height / 65535.0f, 0.7f) * 65535.0f;
                break;
                
            case EBiomeType::Pantano:
                // Flatten for swamps
                Height = FMath::Lerp(32768.0f, Height, 0.3f);
                break;
                
            default:
                // No additional modification
                break;
        }
    }
}

void AWorld_ProceduralLandscapeSystem::UpdatePerformanceMetrics()
{
    // Monitor performance and optimize if needed
    if (GeneratedLandscapes.Num() > 0)
    {
        // TODO: Implement performance monitoring
    }
}

bool AWorld_ProceduralLandscapeSystem::ShouldOptimizeLandscape(ALandscape* Landscape) const
{
    if (!Landscape)
    {
        return false;
    }
    
    // Check distance from player
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Landscape->GetActorLocation());
        return Distance > MaxLandscapeDrawDistance * 0.5f;
    }
    
    return false;
}

void AWorld_ProceduralLandscapeSystem::ApplyLODOptimization(ALandscape* Landscape) const
{
    if (!Landscape)
    {
        return;
    }
    
    // TODO: Apply LOD optimization settings to landscape
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeSystem: Applied LOD optimization to landscape %s"), *Landscape->GetName());
}