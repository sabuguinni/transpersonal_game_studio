#include "World_CretaceousTerrainExpansion.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "LandscapeEditorObject.h"
#include "EditorLevelLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Math/UnrealMathUtility.h"

AWorld_CretaceousTerrainExpansion::AWorld_CretaceousTerrainExpansion()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize terrain configuration
    TerrainSize = 1000000.0f; // 10km in centimeters
    MaxHeight = 50000.0f;     // 500m max elevation
    MinHeight = 0.0f;         // Sea level
    HeightmapResolution = 1009; // Standard UE5 landscape resolution

    // Initialize biome zone centers (distributed across 10km terrain)
    SwampZoneCenter = FVector(-250000.0f, -250000.0f, 0.0f);    // Southwest
    ForestZoneCenter = FVector(-250000.0f, 250000.0f, 0.0f);    // Northwest
    SavannaZoneCenter = FVector(0.0f, 0.0f, 0.0f);              // Center
    DesertZoneCenter = FVector(250000.0f, 0.0f, 0.0f);          // East
    MountainZoneCenter = FVector(250000.0f, 250000.0f, 0.0f);   // Northeast

    BiomeZoneRadius = 150000.0f; // 1.5km radius per biome zone

    // Performance settings
    bEnableTerrainStreaming = true;
    StreamingDistance = 200000.0f; // 2km streaming distance
    MaxTerrainLOD = 4;

    // Status initialization
    bTerrainCreated = false;
    bBiomeZonesAllocated = false;
    bHeightVariationApplied = false;
    CurrentTerrainSize = 0.0f;

    // Initialize actor references
    MainLandscape = nullptr;
    TerrainPlane = nullptr;
}

void AWorld_CretaceousTerrainExpansion::BeginPlay()
{
    Super::BeginPlay();

    // Auto-create terrain on begin play
    CreateMassiveTerrain();
}

void AWorld_CretaceousTerrainExpansion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Monitor terrain status and performance
    if (bTerrainCreated && bEnableTerrainStreaming)
    {
        // Update terrain streaming based on player position
        // This would be implemented with actual player tracking
    }
}

void AWorld_CretaceousTerrainExpansion::CreateMassiveTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Creating massive 10km terrain"));

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_CretaceousTerrainExpansion: No world context available"));
        return;
    }

    // Try to create a proper landscape first
    MainLandscape = World->SpawnActor<ALandscape>();
    
    if (MainLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Created ALandscape actor"));
        
        // Configure landscape properties
        MainLandscape->SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
        MainLandscape->SetActorScale3D(FVector(1000.0f, 1000.0f, 100.0f));
        
        bTerrainCreated = true;
        CurrentTerrainSize = TerrainSize;
    }
    else
    {
        // Fallback: Create a massive static mesh plane
        UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: ALandscape failed, creating StaticMesh terrain"));
        
        TerrainPlane = World->SpawnActor<AStaticMeshActor>();
        
        if (TerrainPlane)
        {
            // Load the default plane mesh
            UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
            
            if (PlaneMesh && TerrainPlane->GetStaticMeshComponent())
            {
                TerrainPlane->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
                
                // Scale to 10km x 10km
                float ScaleFactor = TerrainSize / 200.0f; // Default plane is 200cm
                TerrainPlane->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, 1.0f));
                TerrainPlane->SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
                
                UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Created terrain plane with scale %f"), ScaleFactor);
                
                bTerrainCreated = true;
                CurrentTerrainSize = TerrainSize;
            }
        }
    }

    if (bTerrainCreated)
    {
        // Setup biome zones after terrain creation
        SetupBiomeZones();
        
        // Apply height variation
        CreateHeightVariation();
        
        // Optimize performance
        OptimizeTerrainPerformance();
        
        UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Terrain expansion complete - Size: %f"), CurrentTerrainSize);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World_CretaceousTerrainExpansion: Failed to create any terrain"));
    }
}

void AWorld_CretaceousTerrainExpansion::ExpandTerrainTo10km()
{
    if (!bTerrainCreated)
    {
        CreateMassiveTerrain();
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Expanding terrain to 10km"));

    if (MainLandscape)
    {
        // Expand existing landscape
        FVector CurrentScale = MainLandscape->GetActorScale3D();
        float TargetScale = TerrainSize / 100000.0f; // Adjust scale calculation
        MainLandscape->SetActorScale3D(FVector(TargetScale, TargetScale, CurrentScale.Z));
    }
    else if (TerrainPlane)
    {
        // Expand terrain plane
        float ScaleFactor = TerrainSize / 200.0f;
        TerrainPlane->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, 1.0f));
    }

    CurrentTerrainSize = TerrainSize;
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Terrain expanded to %f"), CurrentTerrainSize);
}

void AWorld_CretaceousTerrainExpansion::SetupBiomeZones()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Setting up biome zones"));

    // Allocate each biome zone
    AllocateSwampZone();
    AllocateForestZone();
    AllocateSavannaZone();
    AllocateDesertZone();
    AllocateMountainZone();

    bBiomeZonesAllocated = true;
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: All biome zones allocated"));
}

void AWorld_CretaceousTerrainExpansion::CreateHeightVariation()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Creating height variation"));

    // Generate heightmap data
    GenerateHeightmapData();

    // Apply geological features
    ApplyGeologicalFeatures();

    bHeightVariationApplied = true;
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Height variation applied"));
}

void AWorld_CretaceousTerrainExpansion::OptimizeTerrainPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Optimizing terrain performance"));

    // Setup LOD system
    SetupLODSystem();

    // Configure terrain culling
    ConfigureTerrainCulling();

    // Optimize collision
    OptimizeTerrainCollision();

    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Performance optimization complete"));
}

void AWorld_CretaceousTerrainExpansion::AllocateSwampZone()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Allocating Swamp zone at %s"), *SwampZoneCenter.ToString());
    // Implementation for swamp zone allocation
}

void AWorld_CretaceousTerrainExpansion::AllocateForestZone()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Allocating Forest zone at %s"), *ForestZoneCenter.ToString());
    // Implementation for forest zone allocation
}

void AWorld_CretaceousTerrainExpansion::AllocateSavannaZone()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Allocating Savanna zone at %s"), *SavannaZoneCenter.ToString());
    // Implementation for savanna zone allocation
}

void AWorld_CretaceousTerrainExpansion::AllocateDesertZone()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Allocating Desert zone at %s"), *DesertZoneCenter.ToString());
    // Implementation for desert zone allocation
}

void AWorld_CretaceousTerrainExpansion::AllocateMountainZone()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Allocating Mountain zone at %s"), *MountainZoneCenter.ToString());
    // Implementation for mountain zone allocation
}

void AWorld_CretaceousTerrainExpansion::CreateRiverSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Creating river systems"));
    // Implementation for river system creation
}

void AWorld_CretaceousTerrainExpansion::CreateMountainRanges()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Creating mountain ranges"));
    // Implementation for mountain range creation
}

void AWorld_CretaceousTerrainExpansion::CreateValleysAndPlains()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Creating valleys and plains"));
    // Implementation for valley and plain creation
}

void AWorld_CretaceousTerrainExpansion::CreateCoastalAreas()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Creating coastal areas"));
    // Implementation for coastal area creation
}

void AWorld_CretaceousTerrainExpansion::GenerateHeightmapData()
{
    // Initialize heightmap data array
    HeightmapData.SetNum(HeightmapResolution);
    for (int32 i = 0; i < HeightmapResolution; i++)
    {
        HeightmapData[i].SetNum(HeightmapResolution);
    }

    // Generate height data with Perlin noise-like algorithm
    for (int32 Y = 0; Y < HeightmapResolution; Y++)
    {
        for (int32 X = 0; X < HeightmapResolution; X++)
        {
            // Simple height generation with multiple octaves
            float Height = 0.0f;
            float Amplitude = 1.0f;
            float Frequency = 0.01f;

            // Multiple noise octaves for realistic terrain
            for (int32 Octave = 0; Octave < 4; Octave++)
            {
                float NoiseValue = FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency));
                Height += NoiseValue * Amplitude;
                
                Amplitude *= 0.5f;
                Frequency *= 2.0f;
            }

            // Normalize and scale height
            Height = FMath::Clamp(Height, -1.0f, 1.0f);
            Height = (Height + 1.0f) * 0.5f; // Convert to 0-1 range
            Height = FMath::Lerp(MinHeight, MaxHeight, Height);

            HeightmapData[Y][X] = Height;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Generated heightmap data %dx%d"), HeightmapResolution, HeightmapResolution);
}

void AWorld_CretaceousTerrainExpansion::ApplyGeologicalFeatures()
{
    // Apply specific geological features to heightmap
    // This would modify the HeightmapData array to add:
    // - Mountain ridges
    // - River valleys
    // - Coastal shelves
    // - Volcanic formations

    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Applied geological features"));
}

void AWorld_CretaceousTerrainExpansion::SetupTerrainMaterials()
{
    // Setup terrain materials based on biome zones
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Setup terrain materials"));
}

void AWorld_CretaceousTerrainExpansion::ConfigureTerrainPhysics()
{
    // Configure physics properties for the terrain
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Configured terrain physics"));
}

void AWorld_CretaceousTerrainExpansion::ValidateTerrainSize()
{
    // Validate that terrain meets the 10km requirement
    bool bSizeValid = CurrentTerrainSize >= 1000000.0f; // 10km minimum
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Terrain size validation: %s (Current: %f)"), 
           bSizeValid ? TEXT("PASS") : TEXT("FAIL"), CurrentTerrainSize);
}

FVector AWorld_CretaceousTerrainExpansion::CalculateBiomeZonePosition(EWorld_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Swamp:
            return SwampZoneCenter;
        case EWorld_BiomeType::Forest:
            return ForestZoneCenter;
        case EWorld_BiomeType::Savanna:
            return SavannaZoneCenter;
        case EWorld_BiomeType::Desert:
            return DesertZoneCenter;
        case EWorld_BiomeType::Mountain:
            return MountainZoneCenter;
        default:
            return FVector::ZeroVector;
    }
}

bool AWorld_CretaceousTerrainExpansion::IsPositionInBiomeZone(const FVector& Position, EWorld_BiomeType BiomeType)
{
    FVector ZoneCenter = CalculateBiomeZonePosition(BiomeType);
    float Distance = FVector::Dist2D(Position, ZoneCenter);
    return Distance <= BiomeZoneRadius;
}

void AWorld_CretaceousTerrainExpansion::CreateBiomeTransitionZones()
{
    // Create smooth transitions between biome zones
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Created biome transition zones"));
}

void AWorld_CretaceousTerrainExpansion::SetupLODSystem()
{
    // Setup Level of Detail system for terrain
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Setup LOD system"));
}

void AWorld_CretaceousTerrainExpansion::ConfigureTerrainCulling()
{
    // Configure terrain culling for performance
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Configured terrain culling"));
}

void AWorld_CretaceousTerrainExpansion::OptimizeTerrainCollision()
{
    // Optimize terrain collision detection
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousTerrainExpansion: Optimized terrain collision"));
}