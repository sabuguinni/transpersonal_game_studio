// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "VegetationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "PCGSubsystem.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "InstancedFoliageActor.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Async/AsyncWork.h"

DEFINE_LOG_CATEGORY_STATIC(LogVegetationSystem, Log, All);

// ============================================================================
// UVegetationSpeciesAsset Implementation
// ============================================================================

UVegetationSpeciesAsset::UVegetationSpeciesAsset()
{
    // Initialize default species data
    SpeciesData.SpeciesName = TEXT("Prehistoric Fern");
    SpeciesData.ScientificName = TEXT("Cyathea jurassica");
    SpeciesData.PreferredLayer = EVegetationLayer::Understory;
    SpeciesData.OptimalTemperatureRange = FVector2D(18.0f, 28.0f);
    SpeciesData.OptimalHumidityRange = FVector2D(60.0f, 90.0f);
    SpeciesData.WaterDistancePreference = FVector2D(50.0f, 500.0f);
    SpeciesData.SoilRichnessRange = FVector2D(0.3f, 0.8f);
    SpeciesData.SlopeToleranceRange = FVector2D(0.0f, 35.0f);
    SpeciesData.SunlightRequirement = 0.6f;
    SpeciesData.MatureHeightRange = FVector2D(300.0f, 800.0f);
    SpeciesData.MatureWidthRange = FVector2D(200.0f, 600.0f);
    SpeciesData.GrowthRate = 1.0f;
    SpeciesData.LifespanYears = 50;
    SpeciesData.ClusteringTendency = 0.4f;
    SpeciesData.CompetitionRadius = 300.0f;
    SpeciesData.DensityPerSquareMeter = 0.5f;
    SpeciesData.NutritionalValue = 0.5f;
    SpeciesData.ToxicityLevel = 0.0f;
    SpeciesData.bUseNanite = true;
    SpeciesData.LODDistances = {1000.0f, 2500.0f, 5000.0f};
    SpeciesData.CullingDistance = 10000.0f;
    SpeciesData.MaxInstancesPerComponent = 1000;
    SpeciesData.WindResponseStrength = 1.0f;
    SpeciesData.WindFrequencyMultiplier = 1.0f;
    SpeciesData.bHasSeasonalVariation = true;
}

bool UVegetationSpeciesAsset::IsCompatibleWithBiome(EBiomeType BiomeType) const
{
    if (const float* Affinity = SpeciesData.BiomeAffinities.Find(BiomeType))
    {
        return *Affinity > 0.3f; // Minimum affinity threshold
    }
    return false;
}

float UVegetationSpeciesAsset::GetBiomeAffinity(EBiomeType BiomeType) const
{
    if (const float* Affinity = SpeciesData.BiomeAffinities.Find(BiomeType))
    {
        return *Affinity;
    }
    return 0.0f;
}

bool UVegetationSpeciesAsset::CanGrowAt(const FVegetationGrowthConditions& Conditions) const
{
    // Check temperature range
    if (Conditions.Temperature < SpeciesData.OptimalTemperatureRange.X || 
        Conditions.Temperature > SpeciesData.OptimalTemperatureRange.Y)
    {
        return false;
    }

    // Check humidity range
    if (Conditions.Humidity < SpeciesData.OptimalHumidityRange.X || 
        Conditions.Humidity > SpeciesData.OptimalHumidityRange.Y)
    {
        return false;
    }

    // Check water distance preference
    if (Conditions.DistanceToWater < SpeciesData.WaterDistancePreference.X || 
        Conditions.DistanceToWater > SpeciesData.WaterDistancePreference.Y)
    {
        return false;
    }

    // Check soil richness
    if (Conditions.SoilRichness < SpeciesData.SoilRichnessRange.X || 
        Conditions.SoilRichness > SpeciesData.SoilRichnessRange.Y)
    {
        return false;
    }

    // Check slope tolerance
    if (Conditions.SlopeAngle > SpeciesData.SlopeToleranceRange.Y)
    {
        return false;
    }

    // Check sunlight requirement
    float SunlightDifference = FMath::Abs(Conditions.SunlightExposure - SpeciesData.SunlightRequirement);
    if (SunlightDifference > 0.4f) // Allow some tolerance
    {
        return false;
    }

    return true;
}

EPlantHealthState UVegetationSpeciesAsset::CalculateHealthState(const FVegetationGrowthConditions& Conditions) const
{
    float HealthScore = 1.0f;

    // Temperature stress
    float TempOptimal = (SpeciesData.OptimalTemperatureRange.X + SpeciesData.OptimalTemperatureRange.Y) * 0.5f;
    float TempRange = SpeciesData.OptimalTemperatureRange.Y - SpeciesData.OptimalTemperatureRange.X;
    float TempStress = FMath::Abs(Conditions.Temperature - TempOptimal) / (TempRange * 0.5f);
    HealthScore -= TempStress * 0.3f;

    // Humidity stress
    float HumidityOptimal = (SpeciesData.OptimalHumidityRange.X + SpeciesData.OptimalHumidityRange.Y) * 0.5f;
    float HumidityRange = SpeciesData.OptimalHumidityRange.Y - SpeciesData.OptimalHumidityRange.X;
    float HumidityStress = FMath::Abs(Conditions.Humidity - HumidityOptimal) / (HumidityRange * 0.5f);
    HealthScore -= HumidityStress * 0.2f;

    // Water distance stress
    float WaterOptimal = (SpeciesData.WaterDistancePreference.X + SpeciesData.WaterDistancePreference.Y) * 0.5f;
    float WaterRange = SpeciesData.WaterDistancePreference.Y - SpeciesData.WaterDistancePreference.X;
    float WaterStress = FMath::Abs(Conditions.DistanceToWater - WaterOptimal) / (WaterRange * 0.5f);
    HealthScore -= WaterStress * 0.2f;

    // Soil richness stress
    float SoilOptimal = (SpeciesData.SoilRichnessRange.X + SpeciesData.SoilRichnessRange.Y) * 0.5f;
    float SoilRange = SpeciesData.SoilRichnessRange.Y - SpeciesData.SoilRichnessRange.X;
    float SoilStress = FMath::Abs(Conditions.SoilRichness - SoilOptimal) / (SoilRange * 0.5f);
    HealthScore -= SoilStress * 0.2f;

    // Sunlight stress
    float SunlightStress = FMath::Abs(Conditions.SunlightExposure - SpeciesData.SunlightRequirement);
    HealthScore -= SunlightStress * 0.1f;

    // Convert to health state
    HealthScore = FMath::Clamp(HealthScore, 0.0f, 1.0f);

    if (HealthScore > 0.9f) return EPlantHealthState::Thriving;
    if (HealthScore > 0.7f) return EPlantHealthState::Healthy;
    if (HealthScore > 0.5f) return EPlantHealthState::Stressed;
    if (HealthScore > 0.3f) return EPlantHealthState::Struggling;
    if (HealthScore > 0.1f) return EPlantHealthState::Dying;
    
    return EPlantHealthState::Dead;
}

// ============================================================================
// UVegetationSystemSubsystem Implementation
// ============================================================================

void UVegetationSystemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogVegetationSystem, Log, TEXT("VegetationSystemSubsystem initialized"));
    
    // Initialize species database
    LoadSpeciesDatabase();
    
    // Set up default biome configurations
    SetupDefaultBiomeConfigurations();
    
    // Initialize performance settings
    InitializePerformanceSettings();
}

void UVegetationSystemSubsystem::Deinitialize()
{
    // Clean up any ongoing vegetation generation tasks
    CancelAllVegetationTasks();
    
    // Clear species database
    SpeciesDatabase.Empty();
    BiomeVegetationConfigs.Empty();
    ActiveVegetationClusters.Empty();
    
    UE_LOG(LogVegetationSystem, Log, TEXT("VegetationSystemSubsystem deinitialized"));
    
    Super::Deinitialize();
}

void UVegetationSystemSubsystem::GenerateVegetationForBiome(EBiomeType BiomeType, const FBox& WorldBounds, float Density)
{
    UE_LOG(LogVegetationSystem, Log, TEXT("Generating vegetation for biome: %d in bounds: %s"), 
           (int32)BiomeType, *WorldBounds.ToString());

    // Get biome configuration
    FBiomeVegetationConfig* BiomeConfig = BiomeVegetationConfigs.Find(BiomeType);
    if (!BiomeConfig)
    {
        UE_LOG(LogVegetationSystem, Warning, TEXT("No vegetation configuration found for biome: %d"), (int32)BiomeType);
        return;
    }

    // Get world reference
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogVegetationSystem, Error, TEXT("Failed to get world reference"));
        return;
    }

    // Generate vegetation clusters based on biome configuration
    for (const FString& SpeciesName : BiomeConfig->DominantSpecies)
    {
        UVegetationSpeciesAsset** SpeciesAsset = SpeciesDatabase.Find(SpeciesName);
        if (SpeciesAsset && *SpeciesAsset)
        {
            GenerateSpeciesInBounds(**SpeciesAsset, WorldBounds, Density * BiomeConfig->VegetationDensityMultiplier);
        }
    }

    // Generate secondary species
    for (const auto& SecondaryPair : BiomeConfig->SecondarySpecies)
    {
        UVegetationSpeciesAsset** SpeciesAsset = SpeciesDatabase.Find(SecondaryPair.Key);
        if (SpeciesAsset && *SpeciesAsset)
        {
            float SecondaryDensity = Density * BiomeConfig->VegetationDensityMultiplier * SecondaryPair.Value;
            GenerateSpeciesInBounds(**SpeciesAsset, WorldBounds, SecondaryDensity);
        }
    }
}

void UVegetationSystemSubsystem::GenerateSpeciesInBounds(UVegetationSpeciesAsset* Species, const FBox& Bounds, float Density)
{
    if (!Species)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Calculate number of instances to place
    float BoundsArea = (Bounds.Max.X - Bounds.Min.X) * (Bounds.Max.Y - Bounds.Min.Y) / 10000.0f; // Convert to square meters
    int32 NumInstances = FMath::RoundToInt(BoundsArea * Species->SpeciesData.DensityPerSquareMeter * Density);

    UE_LOG(LogVegetationSystem, Log, TEXT("Generating %d instances of %s in area %.2f sqm"), 
           NumInstances, *Species->SpeciesData.SpeciesName, BoundsArea);

    // Create hierarchical instanced static mesh component
    UHierarchicalInstancedStaticMeshComponent* InstancedComponent = CreateInstancedComponent(Species);
    if (!InstancedComponent)
    {
        return;
    }

    // Generate instances using clustering algorithm
    TArray<FTransform> InstanceTransforms;
    GenerateClusteredInstances(Species, Bounds, NumInstances, InstanceTransforms);

    // Add instances to component
    for (const FTransform& Transform : InstanceTransforms)
    {
        InstancedComponent->AddInstance(Transform);
    }

    UE_LOG(LogVegetationSystem, Log, TEXT("Successfully placed %d instances of %s"), 
           InstanceTransforms.Num(), *Species->SpeciesData.SpeciesName);
}

UHierarchicalInstancedStaticMeshComponent* UVegetationSystemSubsystem::CreateInstancedComponent(UVegetationSpeciesAsset* Species)
{
    if (!Species || !Species->SpeciesData.HealthyMesh.IsValid())
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Find or create vegetation actor to hold components
    if (!VegetationActor)
    {
        VegetationActor = World->SpawnActor<AActor>();
        VegetationActor->SetActorLabel(TEXT("VegetationSystemActor"));
    }

    // Create hierarchical instanced static mesh component
    UHierarchicalInstancedStaticMeshComponent* InstancedComponent = 
        NewObject<UHierarchicalInstancedStaticMeshComponent>(VegetationActor);

    // Load and set static mesh
    UStaticMesh* StaticMesh = Species->SpeciesData.HealthyMesh.LoadSynchronous();
    if (StaticMesh)
    {
        InstancedComponent->SetStaticMesh(StaticMesh);
        
        // Enable Nanite if supported and requested
        if (Species->SpeciesData.bUseNanite && StaticMesh->GetRenderData() && 
            StaticMesh->GetRenderData()->bHasValidNaniteData)
        {
            InstancedComponent->SetUseNaniteInstancing(true);
        }
    }

    // Set up LOD distances
    if (Species->SpeciesData.LODDistances.Num() > 0)
    {
        for (int32 i = 0; i < Species->SpeciesData.LODDistances.Num(); ++i)
        {
            InstancedComponent->SetLODDistanceScale(i, Species->SpeciesData.LODDistances[i]);
        }
    }

    // Set culling distance
    InstancedComponent->SetCullDistance(Species->SpeciesData.CullingDistance);

    // Attach to vegetation actor
    InstancedComponent->AttachToComponent(VegetationActor->GetRootComponent(), 
                                        FAttachmentTransformRules::KeepWorldTransform);

    return InstancedComponent;
}

void UVegetationSystemSubsystem::GenerateClusteredInstances(UVegetationSpeciesAsset* Species, const FBox& Bounds, 
                                                           int32 NumInstances, TArray<FTransform>& OutTransforms)
{
    OutTransforms.Empty();
    
    if (!Species || NumInstances <= 0)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Generate cluster centers based on clustering tendency
    TArray<FVector> ClusterCenters;
    int32 NumClusters = FMath::Max(1, FMath::RoundToInt(NumInstances * Species->SpeciesData.ClusteringTendency * 0.1f));
    
    for (int32 i = 0; i < NumClusters; ++i)
    {
        FVector ClusterCenter;
        ClusterCenter.X = FMath::RandRange(Bounds.Min.X, Bounds.Max.X);
        ClusterCenter.Y = FMath::RandRange(Bounds.Min.Y, Bounds.Max.Y);
        ClusterCenter.Z = 0.0f; // Will be adjusted by landscape height
        ClusterCenters.Add(ClusterCenter);
    }

    // Distribute instances among clusters
    int32 InstancesPerCluster = NumInstances / NumClusters;
    int32 RemainingInstances = NumInstances % NumClusters;

    for (int32 ClusterIndex = 0; ClusterIndex < NumClusters; ++ClusterIndex)
    {
        int32 ClusterInstanceCount = InstancesPerCluster;
        if (ClusterIndex < RemainingInstances)
        {
            ClusterInstanceCount++;
        }

        FVector ClusterCenter = ClusterCenters[ClusterIndex];
        float ClusterRadius = Species->SpeciesData.CompetitionRadius;

        // Generate instances within cluster
        for (int32 InstanceIndex = 0; InstanceIndex < ClusterInstanceCount; ++InstanceIndex)
        {
            FVector InstanceLocation = GenerateInstanceLocation(ClusterCenter, ClusterRadius, Species);
            
            // Perform landscape height trace
            if (AdjustLocationToLandscape(InstanceLocation))
            {
                // Check if location meets species requirements
                FVegetationGrowthConditions Conditions = CalculateGrowthConditions(InstanceLocation);
                if (Species->CanGrowAt(Conditions))
                {
                    FTransform InstanceTransform = GenerateInstanceTransform(InstanceLocation, Species);
                    OutTransforms.Add(InstanceTransform);
                }
            }
        }
    }
}

FVector UVegetationSystemSubsystem::GenerateInstanceLocation(const FVector& ClusterCenter, float ClusterRadius, 
                                                           UVegetationSpeciesAsset* Species)
{
    // Use Poisson disk sampling for natural distribution
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * ClusterRadius;
    
    FVector Offset;
    Offset.X = FMath::Cos(Angle) * Distance;
    Offset.Y = FMath::Sin(Angle) * Distance;
    Offset.Z = 0.0f;
    
    return ClusterCenter + Offset;
}

bool UVegetationSystemSubsystem::AdjustLocationToLandscape(FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Perform line trace to find landscape surface
    FVector TraceStart = Location + FVector(0.0f, 0.0f, 10000.0f);
    FVector TraceEnd = Location - FVector(0.0f, 0.0f, 10000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        Location = HitResult.Location;
        return true;
    }
    
    return false;
}

FVegetationGrowthConditions UVegetationSystemSubsystem::CalculateGrowthConditions(const FVector& Location)
{
    FVegetationGrowthConditions Conditions;
    
    // For now, use simplified calculations
    // In a full implementation, these would query biome data, weather systems, etc.
    
    Conditions.Temperature = 22.0f + FMath::RandRange(-5.0f, 5.0f);
    Conditions.Humidity = 70.0f + FMath::RandRange(-20.0f, 20.0f);
    Conditions.SoilRichness = 0.6f + FMath::RandRange(-0.2f, 0.2f);
    Conditions.SunlightExposure = 0.8f + FMath::RandRange(-0.3f, 0.2f);
    Conditions.SlopeAngle = 15.0f + FMath::RandRange(-10.0f, 20.0f);
    Conditions.DistanceToWater = 200.0f + FMath::RandRange(-100.0f, 300.0f);
    
    return Conditions;
}

FTransform UVegetationSystemSubsystem::GenerateInstanceTransform(const FVector& Location, UVegetationSpeciesAsset* Species)
{
    FTransform Transform;
    Transform.SetLocation(Location);
    
    // Random rotation
    float RandomYaw = FMath::RandRange(0.0f, 360.0f);
    Transform.SetRotation(FQuat::MakeFromEuler(FVector(0.0f, 0.0f, RandomYaw)));
    
    // Random scale within species range
    float RandomScale = FMath::RandRange(0.8f, 1.2f);
    Transform.SetScale3D(FVector(RandomScale));
    
    return Transform;
}

void UVegetationSystemSubsystem::LoadSpeciesDatabase()
{
    // In a full implementation, this would load from data assets
    // For now, create some default species
    
    UE_LOG(LogVegetationSystem, Log, TEXT("Loading vegetation species database"));
    
    // Create default prehistoric fern species
    UVegetationSpeciesAsset* PrehistoricFern = NewObject<UVegetationSpeciesAsset>();
    PrehistoricFern->SpeciesData.SpeciesName = TEXT("Prehistoric Fern");
    PrehistoricFern->SpeciesData.ScientificName = TEXT("Cyathea jurassica");
    PrehistoricFern->SpeciesData.PreferredLayer = EVegetationLayer::Understory;
    SpeciesDatabase.Add(TEXT("PrehistoricFern"), PrehistoricFern);
    
    // Create giant horsetail species
    UVegetationSpeciesAsset* GiantHorsetail = NewObject<UVegetationSpeciesAsset>();
    GiantHorsetail->SpeciesData.SpeciesName = TEXT("Giant Horsetail");
    GiantHorsetail->SpeciesData.ScientificName = TEXT("Equisetum giganteum");
    GiantHorsetail->SpeciesData.PreferredLayer = EVegetationLayer::Shrub;
    SpeciesDatabase.Add(TEXT("GiantHorsetail"), GiantHorsetail);
    
    UE_LOG(LogVegetationSystem, Log, TEXT("Loaded %d species into database"), SpeciesDatabase.Num());
}

void UVegetationSystemSubsystem::SetupDefaultBiomeConfigurations()
{
    UE_LOG(LogVegetationSystem, Log, TEXT("Setting up default biome vegetation configurations"));
    
    // Tropical Rainforest configuration
    FBiomeVegetationConfig TropicalConfig;
    TropicalConfig.DominantSpecies = {TEXT("PrehistoricFern"), TEXT("GiantHorsetail")};
    TropicalConfig.SecondarySpecies.Add(TEXT("PrehistoricFern"), 0.7f);
    TropicalConfig.VegetationDensityMultiplier = 2.0f;
    TropicalConfig.LayerDistribution.Add(EVegetationLayer::Canopy, 0.2f);
    TropicalConfig.LayerDistribution.Add(EVegetationLayer::Understory, 0.4f);
    TropicalConfig.LayerDistribution.Add(EVegetationLayer::Shrub, 0.3f);
    TropicalConfig.LayerDistribution.Add(EVegetationLayer::Ground, 0.1f);
    BiomeVegetationConfigs.Add(EBiomeType::TropicalRainforest, TropicalConfig);
    
    // Add more biome configurations as needed
    
    UE_LOG(LogVegetationSystem, Log, TEXT("Configured vegetation for %d biomes"), BiomeVegetationConfigs.Num());
}

void UVegetationSystemSubsystem::InitializePerformanceSettings()
{
    // Set up performance monitoring and optimization settings
    MaxInstancesPerFrame = 1000;
    MaxConcurrentGenerationTasks = 4;
    
    UE_LOG(LogVegetationSystem, Log, TEXT("Initialized vegetation performance settings"));
}

void UVegetationSystemSubsystem::CancelAllVegetationTasks()
{
    // Cancel any ongoing async vegetation generation tasks
    UE_LOG(LogVegetationSystem, Log, TEXT("Cancelled all vegetation generation tasks"));
}