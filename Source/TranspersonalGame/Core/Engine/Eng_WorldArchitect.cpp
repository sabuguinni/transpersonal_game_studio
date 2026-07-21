#include "Eng_WorldArchitect.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UEng_WorldArchitect::UEng_WorldArchitect()
{
    // Initialize default world bounds
    WorldBounds.MinBounds = FVector(-10000, -10000, -1000);
    WorldBounds.MaxBounds = FVector(10000, 10000, 2000);
    WorldBounds.BiomeTransitionZone = 500.0f;

    // Initialize default terrain settings
    TerrainSettings.HeightScale = 100.0f;
    TerrainSettings.NoiseScale = 0.001f;
    TerrainSettings.TerrainResolution = 1024;
    TerrainSettings.bEnableWorldPartition = true;
}

void UEng_WorldArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: Initializing world architecture subsystem"));
    
    // Calculate initial biome zones
    CalculateBiomeZones();
    
    bWorldInitialized = true;
}

void UEng_WorldArchitect::Deinitialize()
{
    BiomeManagerRef = nullptr;
    bWorldInitialized = false;
    
    Super::Deinitialize();
}

void UEng_WorldArchitect::InitializeWorldBounds(const FEng_WorldBounds& Bounds)
{
    WorldBounds = Bounds;
    CalculateBiomeZones();
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: World bounds initialized - Min: %s, Max: %s"), 
           *WorldBounds.MinBounds.ToString(), *WorldBounds.MaxBounds.ToString());
}

void UEng_WorldArchitect::SetupBiomeZones()
{
    if (!bWorldInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldArchitect: Cannot setup biome zones - world not initialized"));
        return;
    }

    CalculateBiomeZones();
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: Biome zones setup complete - %d zones created"), 
           BiomeZoneCenters.Num());
}

void UEng_WorldArchitect::ValidateWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldArchitect: Cannot validate world partition - no world"));
        return;
    }

    // Check if world partition is enabled
    if (TerrainSettings.bEnableWorldPartition)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: World Partition validation - Enabled"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: World Partition validation - Disabled"));
    }

    ValidateActorCounts();
}

FVector UEng_WorldArchitect::GetWorldCenter() const
{
    return (WorldBounds.MinBounds + WorldBounds.MaxBounds) * 0.5f;
}

bool UEng_WorldArchitect::IsLocationInBounds(const FVector& Location) const
{
    return Location.X >= WorldBounds.MinBounds.X && Location.X <= WorldBounds.MaxBounds.X &&
           Location.Y >= WorldBounds.MinBounds.Y && Location.Y <= WorldBounds.MaxBounds.Y &&
           Location.Z >= WorldBounds.MinBounds.Z && Location.Z <= WorldBounds.MaxBounds.Z;
}

EBiomeType UEng_WorldArchitect::GetBiomeAtLocation(const FVector& Location) const
{
    if (!IsLocationInBounds(Location))
    {
        return EBiomeType::Forest; // Default fallback
    }

    // Simple biome determination based on location
    FVector WorldCenter = GetWorldCenter();
    FVector Offset = Location - WorldCenter;
    
    // Determine biome based on distance from center and direction
    float DistanceFromCenter = Offset.Size2D();
    
    if (DistanceFromCenter < 2000.0f)
    {
        return EBiomeType::Forest; // Central forest
    }
    else if (Offset.X > 0 && Offset.Y > 0)
    {
        return EBiomeType::Desert; // Northeast quadrant
    }
    else if (Offset.X < 0 && Offset.Y > 0)
    {
        return EBiomeType::Tundra; // Northwest quadrant
    }
    else if (Offset.X < 0 && Offset.Y < 0)
    {
        return EBiomeType::Swamp; // Southwest quadrant
    }
    else
    {
        return EBiomeType::Grassland; // Southeast quadrant
    }
}

void UEng_WorldArchitect::RegisterBiomeManager(UBiomeManager* Manager)
{
    BiomeManagerRef = Manager;
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: Biome manager registered"));
}

void UEng_WorldArchitect::SetTerrainSettings(const FEng_TerrainSettings& Settings)
{
    TerrainSettings = Settings;
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: Terrain settings updated - Resolution: %d, Height Scale: %f"), 
           TerrainSettings.TerrainResolution, TerrainSettings.HeightScale);
}

void UEng_WorldArchitect::ValidateWorldConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: === WORLD CONFIGURATION VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("World Bounds: %s to %s"), 
           *WorldBounds.MinBounds.ToString(), *WorldBounds.MaxBounds.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Biome Zones: %d"), BiomeZoneCenters.Num());
    UE_LOG(LogTemp, Warning, TEXT("World Partition: %s"), 
           TerrainSettings.bEnableWorldPartition ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Resolution: %d"), TerrainSettings.TerrainResolution);
    
    ValidateActorCounts();
}

void UEng_WorldArchitect::CalculateBiomeZones()
{
    BiomeZoneCenters.Empty();
    
    FVector WorldCenter = GetWorldCenter();
    float ZoneRadius = (WorldBounds.MaxBounds.X - WorldBounds.MinBounds.X) * 0.3f;
    
    // Create 5 biome zone centers
    BiomeZoneCenters.Add(WorldCenter); // Central forest
    BiomeZoneCenters.Add(WorldCenter + FVector(ZoneRadius, ZoneRadius, 0)); // NE Desert
    BiomeZoneCenters.Add(WorldCenter + FVector(-ZoneRadius, ZoneRadius, 0)); // NW Tundra
    BiomeZoneCenters.Add(WorldCenter + FVector(-ZoneRadius, -ZoneRadius, 0)); // SW Swamp
    BiomeZoneCenters.Add(WorldCenter + FVector(ZoneRadius, -ZoneRadius, 0)); // SE Grassland
}

void UEng_WorldArchitect::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Count actors in the world (this would be implemented with proper actor counting)
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitect: Actor count validation - Implementation needed"));
}

// AEng_WorldAnchor Implementation
AEng_WorldAnchor::AEng_WorldAnchor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create anchor mesh component
    AnchorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnchorMesh"));
    RootComponent = AnchorMesh;

    // Set default properties
    AnchorBiome = EBiomeType::Forest;
    InfluenceRadius = 1000.0f;

    // Try to load a default mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        AnchorMesh->SetStaticMesh(SphereMesh.Object);
        AnchorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }
}

void AEng_WorldAnchor::SetAnchorType(EBiomeType BiomeType)
{
    AnchorBiome = BiomeType;
    
    // Update visual representation based on biome type
    if (AnchorMesh)
    {
        switch (AnchorBiome)
        {
        case EBiomeType::Forest:
            AnchorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 3.0f));
            break;
        case EBiomeType::Desert:
            AnchorMesh->SetWorldScale3D(FVector(3.0f, 3.0f, 1.0f));
            break;
        case EBiomeType::Tundra:
            AnchorMesh->SetWorldScale3D(FVector(1.5f, 1.5f, 4.0f));
            break;
        case EBiomeType::Swamp:
            AnchorMesh->SetWorldScale3D(FVector(4.0f, 4.0f, 1.5f));
            break;
        case EBiomeType::Grassland:
            AnchorMesh->SetWorldScale3D(FVector(2.5f, 2.5f, 2.0f));
            break;
        }
    }
}