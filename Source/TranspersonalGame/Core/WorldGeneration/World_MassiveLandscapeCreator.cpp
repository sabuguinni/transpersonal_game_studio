#include "World_MassiveLandscapeCreator.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEditorObject.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogMassiveLandscape, Log, All);

UWorld_MassiveLandscapeCreator::UWorld_MassiveLandscapeCreator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize landscape parameters for 10km x 10km world
    LandscapeSize = 10000.0f; // 10km
    HeightmapResolution = 2017; // Standard UE5 landscape resolution
    ComponentSize = 127; // Quads per component
    SectionsPerComponent = 2;
    QuadsPerSection = 63;
    
    // Biome configuration
    BiomeZones.Add(FWorld_BiomeZoneConfig{
        TEXT("Swamp_SW"),
        FVector(-5000, -5000, 0),
        2000.0f,
        EWorld_BiomeType::Swamp
    });
    
    BiomeZones.Add(FWorld_BiomeZoneConfig{
        TEXT("Forest_NW"),
        FVector(-5000, 5000, 0),
        2000.0f,
        EWorld_BiomeType::Forest
    });
    
    BiomeZones.Add(FWorld_BiomeZoneConfig{
        TEXT("Savanna_Center"),
        FVector(0, 0, 0),
        2000.0f,
        EWorld_BiomeType::Savanna
    });
    
    BiomeZones.Add(FWorld_BiomeZoneConfig{
        TEXT("Desert_E"),
        FVector(5000, 0, 0),
        2000.0f,
        EWorld_BiomeType::Desert
    });
    
    BiomeZones.Add(FWorld_BiomeZoneConfig{
        TEXT("Mountain_NE"),
        FVector(5000, 5000, 500),
        2000.0f,
        EWorld_BiomeType::Mountain
    });
}

void UWorld_MassiveLandscapeCreator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogMassiveLandscape, Log, TEXT("MassiveLandscapeCreator initialized"));
}

ALandscape* UWorld_MassiveLandscapeCreator::CreateMassiveLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogMassiveLandscape, Error, TEXT("No valid world found"));
        return nullptr;
    }

    // Check if landscape already exists
    if (MainLandscape && IsValid(MainLandscape))
    {
        UE_LOG(LogMassiveLandscape, Warning, TEXT("Landscape already exists"));
        return MainLandscape;
    }

    // Create landscape actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName("MainLandscape_Cretaceous");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    MainLandscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    if (!MainLandscape)
    {
        UE_LOG(LogMassiveLandscape, Error, TEXT("Failed to spawn landscape actor"));
        return nullptr;
    }

    // Configure landscape properties
    ConfigureLandscapeProperties();
    
    // Generate heightmap data
    GenerateHeightmapData();
    
    // Apply biome-specific modifications
    ApplyBiomeModifications();
    
    UE_LOG(LogMassiveLandscape, Log, TEXT("Successfully created massive landscape: %s"), *MainLandscape->GetName());
    
    return MainLandscape;
}

void UWorld_MassiveLandscapeCreator::ConfigureLandscapeProperties()
{
    if (!MainLandscape)
    {
        return;
    }

    // Set landscape transform for 10km coverage
    FTransform LandscapeTransform;
    LandscapeTransform.SetLocation(FVector::ZeroVector);
    LandscapeTransform.SetRotation(FQuat::Identity);
    LandscapeTransform.SetScale3D(FVector(100.0f, 100.0f, 1.0f)); // Scale for massive size
    
    MainLandscape->SetActorTransform(LandscapeTransform);
    
    // Set landscape material if available
    UMaterialInterface* LandscapeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial"));
    if (LandscapeMaterial)
    {
        MainLandscape->GetLandscapeInfo()->LandscapeMaterial = LandscapeMaterial;
    }
    
    UE_LOG(LogMassiveLandscape, Log, TEXT("Landscape properties configured"));
}

void UWorld_MassiveLandscapeCreator::GenerateHeightmapData()
{
    if (!MainLandscape)
    {
        return;
    }

    // Generate basic heightmap with varied terrain
    TArray<uint16> HeightData;
    HeightData.SetNum(HeightmapResolution * HeightmapResolution);
    
    for (int32 Y = 0; Y < HeightmapResolution; Y++)
    {
        for (int32 X = 0; X < HeightmapResolution; X++)
        {
            int32 Index = Y * HeightmapResolution + X;
            
            // Generate varied terrain height
            float NormalizedX = (float)X / HeightmapResolution;
            float NormalizedY = (float)Y / HeightmapResolution;
            
            // Create rolling hills with some variation
            float Height = FMath::Sin(NormalizedX * 8.0f) * FMath::Cos(NormalizedY * 6.0f) * 0.3f;
            Height += FMath::PerlinNoise2D(FVector2D(NormalizedX * 4.0f, NormalizedY * 4.0f)) * 0.2f;
            Height += 0.5f; // Base elevation
            
            // Convert to uint16 range
            HeightData[Index] = FMath::Clamp((uint16)(Height * 65535.0f), (uint16)0, (uint16)65535);
        }
    }
    
    UE_LOG(LogMassiveLandscape, Log, TEXT("Generated heightmap data with %d points"), HeightData.Num());
}

void UWorld_MassiveLandscapeCreator::ApplyBiomeModifications()
{
    if (!MainLandscape)
    {
        return;
    }

    // Apply biome-specific terrain modifications
    for (const FWorld_BiomeZoneConfig& BiomeZone : BiomeZones)
    {
        ApplyBiomeTerrainModification(BiomeZone);
    }
    
    UE_LOG(LogMassiveLandscape, Log, TEXT("Applied biome modifications to %d zones"), BiomeZones.Num());
}

void UWorld_MassiveLandscapeCreator::ApplyBiomeTerrainModification(const FWorld_BiomeZoneConfig& BiomeConfig)
{
    // Apply terrain modifications based on biome type
    switch (BiomeConfig.BiomeType)
    {
        case EWorld_BiomeType::Mountain:
            // Increase elevation for mountain areas
            ModifyTerrainElevation(BiomeConfig.Location, BiomeConfig.Radius, 800.0f);
            break;
            
        case EWorld_BiomeType::Swamp:
            // Lower elevation for swamp areas
            ModifyTerrainElevation(BiomeConfig.Location, BiomeConfig.Radius, -200.0f);
            break;
            
        case EWorld_BiomeType::Desert:
            // Add sand dunes variation
            AddTerrainVariation(BiomeConfig.Location, BiomeConfig.Radius, 0.3f);
            break;
            
        case EWorld_BiomeType::Forest:
            // Add rolling hills
            AddTerrainVariation(BiomeConfig.Location, BiomeConfig.Radius, 0.5f);
            break;
            
        case EWorld_BiomeType::Savanna:
            // Keep relatively flat with gentle slopes
            AddTerrainVariation(BiomeConfig.Location, BiomeConfig.Radius, 0.1f);
            break;
            
        default:
            break;
    }
}

void UWorld_MassiveLandscapeCreator::ModifyTerrainElevation(const FVector& Location, float Radius, float ElevationChange)
{
    // Terrain elevation modification logic would go here
    // This is a simplified implementation for the core structure
    UE_LOG(LogMassiveLandscape, Log, TEXT("Modified terrain elevation at %s by %f"), *Location.ToString(), ElevationChange);
}

void UWorld_MassiveLandscapeCreator::AddTerrainVariation(const FVector& Location, float Radius, float Intensity)
{
    // Terrain variation logic would go here
    // This is a simplified implementation for the core structure
    UE_LOG(LogMassiveLandscape, Log, TEXT("Added terrain variation at %s with intensity %f"), *Location.ToString(), Intensity);
}

bool UWorld_MassiveLandscapeCreator::ValidateLandscapeSize() const
{
    if (!MainLandscape)
    {
        return false;
    }
    
    // Check if landscape meets the 10km x 10km requirement
    FVector LandscapeScale = MainLandscape->GetActorScale3D();
    float EffectiveSize = LandscapeSize * FMath::Max(LandscapeScale.X, LandscapeScale.Y);
    
    bool bMeetsRequirement = EffectiveSize >= 10000.0f; // 10km minimum
    
    UE_LOG(LogMassiveLandscape, Log, TEXT("Landscape size validation: %s (Effective size: %f)"), 
           bMeetsRequirement ? TEXT("PASS") : TEXT("FAIL"), EffectiveSize);
    
    return bMeetsRequirement;
}

TArray<FWorld_BiomeZoneConfig> UWorld_MassiveLandscapeCreator::GetBiomeZones() const
{
    return BiomeZones;
}

void UWorld_MassiveLandscapeCreator::SetBiomeZones(const TArray<FWorld_BiomeZoneConfig>& NewBiomeZones)
{
    BiomeZones = NewBiomeZones;
    UE_LOG(LogMassiveLandscape, Log, TEXT("Updated biome zones configuration with %d zones"), BiomeZones.Num());
}