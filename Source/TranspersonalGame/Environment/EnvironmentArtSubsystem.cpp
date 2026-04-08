// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentArtSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Foliage/Public/FoliageType.h"
#include "Foliage/Public/FoliageInstancedStaticMeshComponent.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnvironmentArt, Log, All);

UEnvironmentArtSubsystem::UEnvironmentArtSubsystem()
{
    // Initialize default settings
    CurrentSettings.OverallDensity = 1.0f;
    CurrentSettings.Quality = EEnvironmentArtQuality::High;
    CurrentSettings.bEnableProceduralVariation = true;
    CurrentSettings.bEnableStorytellingProps = true;
    CurrentSettings.WorldBounds = FBox(FVector(-100000), FVector(100000));
}

void UEnvironmentArtSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art Subsystem initialized"));
    
    // Initialize vegetation and rock systems
    InitializeVegetationSystem();
    InitializeRockSystem();
    
    // Reset performance stats
    PerformanceStats = FEnvironmentArtPerformanceStats();
}

void UEnvironmentArtSubsystem::Deinitialize()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art Subsystem deinitialized"));
    
    // Clear all environment art
    ClearEnvironmentArt();
    
    Super::Deinitialize();
}

bool UEnvironmentArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UEnvironmentArtSubsystem::PopulateEnvironment(const FEnvironmentArtSettings& Settings)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Populating environment with density: %f"), Settings.OverallDensity);
    
    CurrentSettings = Settings;
    
    // Clear existing environment art
    ClearEnvironmentArt();
    
    // Get the world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to get world for environment population"));
        return;
    }
    
    // Find landscape actors
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    if (LandscapeActors.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No landscape found for environment population"));
        return;
    }
    
    // Populate each biome type
    for (int32 BiomeIndex = 0; BiomeIndex < (int32)EBiomeType::LakeShore + 1; BiomeIndex++)
    {
        EBiomeType BiomeType = (EBiomeType)BiomeIndex;
        
        FVegetationPlacementSettings VegSettings;
        VegSettings.TargetBiome = BiomeType;
        VegSettings.DensityMultiplier = Settings.OverallDensity;
        VegSettings.PlacementArea = Settings.WorldBounds;
        
        PlaceVegetation(VegSettings);
    }
    
    // Update performance statistics
    UpdatePerformanceStats();
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment population complete. Total instances: %d"), 
           PerformanceStats.TotalVegetationInstances + PerformanceStats.TotalRockInstances);
}

void UEnvironmentArtSubsystem::RefreshRegion(const FBox& WorldBounds)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Refreshing environment region: %s"), *WorldBounds.ToString());
    
    // Create temporary settings for this region
    FEnvironmentArtSettings RegionSettings = CurrentSettings;
    RegionSettings.WorldBounds = WorldBounds;
    
    // Remove existing instances in this region
    for (UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (Component && IsValid(Component))
        {
            // TODO: Implement region-specific instance removal
            // For now, we'll clear and repopulate the entire area
        }
    }
    
    // Repopulate the region
    PopulateEnvironment(RegionSettings);
}

void UEnvironmentArtSubsystem::ClearEnvironmentArt()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Clearing all environment art"));
    
    // Clear vegetation components
    for (UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->ClearInstances();
            Component->DestroyComponent();
        }
    }
    VegetationComponents.Empty();
    
    // Clear rock components
    for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->ClearInstances();
            Component->DestroyComponent();
        }
    }
    RockComponents.Empty();
    
    // Reset performance stats
    PerformanceStats = FEnvironmentArtPerformanceStats();
}

void UEnvironmentArtSubsystem::PlaceVegetation(const FVegetationPlacementSettings& Settings)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placing vegetation for biome: %d"), (int32)Settings.TargetBiome);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create vegetation component for this biome
    UHierarchicalInstancedStaticMeshComponent* VegComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(World);
    if (!VegComponent)
    {
        UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to create vegetation component"));
        return;
    }
    
    // Configure component settings
    VegComponent->SetMobility(EComponentMobility::Static);
    VegComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    VegComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    VegComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    
    // Set LOD settings for performance
    VegComponent->bUseAsOccluder = false;
    VegComponent->bAllowCPUAccess = false;
    VegComponent->bDisableCollision = false;
    
    // TODO: Load appropriate static mesh based on biome type
    // For now, we'll use a placeholder
    
    // Generate placement positions based on biome characteristics
    TArray<FTransform> PlacementTransforms;
    GenerateVegetationPlacements(Settings, PlacementTransforms);
    
    // Add instances to the component
    for (const FTransform& Transform : PlacementTransforms)
    {
        VegComponent->AddInstance(Transform);
    }
    
    // Register the component
    VegComponent->RegisterComponent();
    VegetationComponents.Add(VegComponent);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placed %d vegetation instances for biome %d"), 
           PlacementTransforms.Num(), (int32)Settings.TargetBiome);
}

void UEnvironmentArtSubsystem::UpdateVegetationDensity(EBiomeType BiomeType, float DensityMultiplier)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Updating vegetation density for biome %d: %f"), 
           (int32)BiomeType, DensityMultiplier);
    
    // Update current settings
    CurrentSettings.OverallDensity = DensityMultiplier;
    
    // Refresh the specific biome
    FVegetationPlacementSettings Settings;
    Settings.TargetBiome = BiomeType;
    Settings.DensityMultiplier = DensityMultiplier;
    Settings.PlacementArea = CurrentSettings.WorldBounds;
    
    PlaceVegetation(Settings);
}

FEnvironmentArtPerformanceStats UEnvironmentArtSubsystem::GetPerformanceStats() const
{
    return PerformanceStats;
}

void UEnvironmentArtSubsystem::InitializeVegetationSystem()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initializing vegetation system"));
    
    // Clear existing vegetation components
    VegetationComponents.Empty();
    
    // TODO: Load vegetation asset database
    // TODO: Initialize PCG components for procedural placement
    // TODO: Setup LOD chains for different vegetation types
}

void UEnvironmentArtSubsystem::InitializeRockSystem()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initializing rock placement system"));
    
    // Clear existing rock components
    RockComponents.Empty();
    
    // TODO: Load rock asset database
    // TODO: Initialize geological feature placement rules
    // TODO: Setup material variation system
}

void UEnvironmentArtSubsystem::UpdatePerformanceStats()
{
    // Count total instances
    PerformanceStats.TotalVegetationInstances = 0;
    for (UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (Component && IsValid(Component))
        {
            PerformanceStats.TotalVegetationInstances += Component->GetInstanceCount();
        }
    }
    
    PerformanceStats.TotalRockInstances = 0;
    for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
    {
        if (Component && IsValid(Component))
        {
            PerformanceStats.TotalRockInstances += Component->GetInstanceCount();
        }
    }
    
    // TODO: Calculate actual memory usage
    PerformanceStats.MemoryUsageMB = (PerformanceStats.TotalVegetationInstances + PerformanceStats.TotalRockInstances) * 0.001f;
    
    // TODO: Measure frame time impact
    PerformanceStats.FrameTimeImpactMS = 0.0f;
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Performance Stats - Vegetation: %d, Rocks: %d, Memory: %.2f MB"), 
           PerformanceStats.TotalVegetationInstances, 
           PerformanceStats.TotalRockInstances, 
           PerformanceStats.MemoryUsageMB);
}

void UEnvironmentArtSubsystem::GenerateVegetationPlacements(const FVegetationPlacementSettings& Settings, TArray<FTransform>& OutTransforms)
{
    OutTransforms.Empty();
    
    // Generate grid-based placement with variation
    const float GridSize = 500.0f; // 5 meters
    const FVector BoundsMin = Settings.PlacementArea.Min;
    const FVector BoundsMax = Settings.PlacementArea.Max;
    
    for (float X = BoundsMin.X; X < BoundsMax.X; X += GridSize)
    {
        for (float Y = BoundsMin.Y; Y < BoundsMax.Y; Y += GridSize)
        {
            // Add random variation to placement
            float RandomX = X + FMath::RandRange(-GridSize * 0.4f, GridSize * 0.4f);
            float RandomY = Y + FMath::RandRange(-GridSize * 0.4f, GridSize * 0.4f);
            
            // Skip placement based on density
            if (FMath::RandRange(0.0f, 1.0f) > Settings.DensityMultiplier)
            {
                continue;
            }
            
            // TODO: Sample landscape height at this position
            float Z = 0.0f; // Placeholder
            
            // Create transform with variation
            FVector Location(RandomX, RandomY, Z);
            FRotator Rotation(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            FVector Scale = FVector(FMath::RandRange(Settings.SizeVariation.X, Settings.SizeVariation.Y));
            
            OutTransforms.Add(FTransform(Rotation, Location, Scale));
        }
    }
}