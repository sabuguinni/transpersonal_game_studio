// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentArtSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "InstancedFoliageActor.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnvironmentArt, Log, All);

UEnvironmentArtSubsystem::UEnvironmentArtSubsystem()
{
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
    
    // Initialize vegetation system
    InitializeVegetationSystem();
    
    // Initialize rock placement system
    InitializeRockSystem();
    
    // Reset performance stats
    PerformanceStats = FEnvironmentArtPerformanceStats();
}

void UEnvironmentArtSubsystem::Deinitialize()
{
    // Clean up all instanced components
    for (UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (IsValid(Component))
        {
            Component->ClearInstances();
            Component->DestroyComponent();
        }
    }
    VegetationComponents.Empty();
    
    for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
    {
        if (IsValid(Component))
        {
            Component->ClearInstances();
            Component->DestroyComponent();
        }
    }
    RockComponents.Empty();
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art Subsystem deinitialized"));
    
    Super::Deinitialize();
}

bool UEnvironmentArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UEnvironmentArtSubsystem::PopulateEnvironment(const FEnvironmentArtSettings& Settings)
{
    CurrentSettings = Settings;
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Populating environment with density %.2f, quality %d"), 
           Settings.OverallDensity, (int32)Settings.Quality);
    
    // Clear existing environment art
    ClearEnvironmentArt();
    
    // Get world reference
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Find landscape actors in the world
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    if (LandscapeActors.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No landscape actors found in world"));
        return;
    }
    
    // Process each landscape
    for (AActor* LandscapeActor : LandscapeActors)
    {
        if (ALandscape* Landscape = Cast<ALandscape>(LandscapeActor))
        {
            PopulateLandscape(Landscape, Settings);
        }
    }
    
    // Update performance statistics
    UpdatePerformanceStats();
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment population completed. Total vegetation instances: %d"), 
           PerformanceStats.TotalVegetationInstances);
}

void UEnvironmentArtSubsystem::RefreshRegion(const FBox& WorldBounds)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Refreshing environment region: %s"), *WorldBounds.ToString());
    
    // Remove existing instances in the region
    ClearRegion(WorldBounds);
    
    // Repopulate the region
    FEnvironmentArtSettings RegionSettings = CurrentSettings;
    RegionSettings.WorldBounds = WorldBounds;
    
    PopulateEnvironment(RegionSettings);
}

void UEnvironmentArtSubsystem::ClearEnvironmentArt()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Clearing all environment art"));
    
    // Clear vegetation instances
    for (UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (IsValid(Component))
        {
            Component->ClearInstances();
        }
    }
    
    // Clear rock instances
    for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
    {
        if (IsValid(Component))
        {
            Component->ClearInstances();
        }
    }
    
    // Reset performance stats
    PerformanceStats = FEnvironmentArtPerformanceStats();
}

void UEnvironmentArtSubsystem::PlaceVegetation(const FVegetationPlacementSettings& Settings)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placing vegetation for biome %d with density %.2f"), 
           (int32)Settings.TargetBiome, Settings.DensityMultiplier);
    
    // Implementation for vegetation placement
    // This would integrate with the PCG system and foliage tools
    
    // For now, create a simple vegetation placement
    CreateVegetationInstances(Settings);
}

void UEnvironmentArtSubsystem::UpdateVegetationDensity(EBiomeType BiomeType, float DensityMultiplier)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Updating vegetation density for biome %d: %.2f"), 
           (int32)BiomeType, DensityMultiplier);
    
    // Update density for specific biome type
    // This would modify existing vegetation instances
}

FEnvironmentArtPerformanceStats UEnvironmentArtSubsystem::GetPerformanceStats() const
{
    return PerformanceStats;
}

void UEnvironmentArtSubsystem::InitializeVegetationSystem()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initializing vegetation system"));
    
    // Clear existing components
    VegetationComponents.Empty();
    
    // Vegetation system initialization would go here
    // This includes setting up foliage types, PCG graphs, etc.
}

void UEnvironmentArtSubsystem::InitializeRockSystem()
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initializing rock placement system"));
    
    // Clear existing components
    RockComponents.Empty();
    
    // Rock system initialization would go here
}

void UEnvironmentArtSubsystem::UpdatePerformanceStats()
{
    PerformanceStats.TotalVegetationInstances = 0;
    PerformanceStats.TotalRockInstances = 0;
    PerformanceStats.TotalStorytellingProps = 0;
    
    // Count vegetation instances
    for (const UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (IsValid(Component))
        {
            PerformanceStats.TotalVegetationInstances += Component->GetInstanceCount();
        }
    }
    
    // Count rock instances
    for (const UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
    {
        if (IsValid(Component))
        {
            PerformanceStats.TotalRockInstances += Component->GetInstanceCount();
        }
    }
    
    // Estimate memory usage (rough calculation)
    PerformanceStats.MemoryUsageMB = (PerformanceStats.TotalVegetationInstances + PerformanceStats.TotalRockInstances) * 0.001f;
    
    // Frame time impact is measured elsewhere
    PerformanceStats.FrameTimeImpactMS = 0.0f;
}

void UEnvironmentArtSubsystem::PopulateLandscape(ALandscape* Landscape, const FEnvironmentArtSettings& Settings)
{
    if (!IsValid(Landscape))
    {
        return;
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Populating landscape: %s"), *Landscape->GetName());
    
    // Get landscape bounds
    FBox LandscapeBounds = Landscape->GetComponentsBoundingBox();
    
    // Intersect with world bounds from settings
    FBox EffectiveBounds = LandscapeBounds.Overlap(Settings.WorldBounds);
    
    if (!EffectiveBounds.IsValid)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No overlap between landscape and world bounds"));
        return;
    }
    
    // Place vegetation based on landscape data
    PlaceVegetationOnLandscape(Landscape, EffectiveBounds, Settings);
    
    // Place geological features
    PlaceGeologicalFeatures(Landscape, EffectiveBounds, Settings);
    
    // Place storytelling props if enabled
    if (Settings.bEnableStorytellingProps)
    {
        PlaceStorytellingProps(Landscape, EffectiveBounds, Settings);
    }
}

void UEnvironmentArtSubsystem::PlaceVegetationOnLandscape(ALandscape* Landscape, const FBox& Bounds, const FEnvironmentArtSettings& Settings)
{
    // This would implement the actual vegetation placement logic
    // using the landscape heightfield, slope data, and biome information
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placing vegetation on landscape within bounds: %s"), *Bounds.ToString());
    
    // For now, create some test vegetation instances
    CreateTestVegetation(Bounds, Settings);
}

void UEnvironmentArtSubsystem::PlaceGeologicalFeatures(ALandscape* Landscape, const FBox& Bounds, const FEnvironmentArtSettings& Settings)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placing geological features within bounds: %s"), *Bounds.ToString());
    
    // Implementation for rock and geological feature placement
}

void UEnvironmentArtSubsystem::PlaceStorytellingProps(ALandscape* Landscape, const FBox& Bounds, const FEnvironmentArtSettings& Settings)
{
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placing storytelling props within bounds: %s"), *Bounds.ToString());
    
    // Implementation for environmental storytelling props
}

void UEnvironmentArtSubsystem::ClearRegion(const FBox& WorldBounds)
{
    // Remove instances within the specified bounds
    for (UHierarchicalInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (IsValid(Component))
        {
            // This would remove instances within the bounds
            // For now, just log the operation
            UE_LOG(LogEnvironmentArt, Log, TEXT("Clearing vegetation instances in region"));
        }
    }
    
    for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
    {
        if (IsValid(Component))
        {
            // This would remove instances within the bounds
            UE_LOG(LogEnvironmentArt, Log, TEXT("Clearing rock instances in region"));
        }
    }
}

void UEnvironmentArtSubsystem::CreateVegetationInstances(const FVegetationPlacementSettings& Settings)
{
    // Create hierarchical instanced static mesh component for vegetation
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find or create a parent actor for the components
    AActor* ParentActor = World->SpawnActor<AActor>();
    if (!ParentActor)
    {
        return;
    }
    
    ParentActor->SetActorLabel(TEXT("EnvironmentArt_Vegetation"));
    
    // Create HISM component
    UHierarchicalInstancedStaticMeshComponent* VegetationComponent = 
        NewObject<UHierarchicalInstancedStaticMeshComponent>(ParentActor);
    
    if (VegetationComponent)
    {
        VegetationComponent->SetupAttachment(ParentActor->GetRootComponent());
        VegetationComponent->RegisterComponent();
        
        // Configure the component for performance
        VegetationComponent->SetCullDistances(5000, 25000);
        VegetationComponent->SetCanEverAffectNavigation(false);
        VegetationComponent->SetCastShadow(true);
        VegetationComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // Add to our tracking array
        VegetationComponents.Add(VegetationComponent);
        
        UE_LOG(LogEnvironmentArt, Log, TEXT("Created vegetation component"));
    }
}

void UEnvironmentArtSubsystem::CreateTestVegetation(const FBox& Bounds, const FEnvironmentArtSettings& Settings)
{
    // Create some test vegetation instances for demonstration
    // In a real implementation, this would be driven by biome data and PCG
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Creating test vegetation in bounds: %s"), *Bounds.ToString());
    
    // This is where we would place actual vegetation instances
    // based on the terrain analysis and biome data
}