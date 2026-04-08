// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ArchitectureManager.h"
#include "StructureGenerator.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGPoint.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitecture, Log, All);

UArchitectureManager::UArchitectureManager()
{
    // Initialize default values
    bEnableDebugVisualization = false;
    MaxStructuresPerBiome = 50;
    MinDistanceBetweenStructures = 100.0f;
    StructureSpawnRadius = 2000.0f;
    
    // Initialize biome structure weights
    BiomeStructureWeights.Add(EBiomeType::DenseJungle, {
        {EStructureType::TreeHouse, 0.3f},
        {EStructureType::PrimitiveHut, 0.4f},
        {EStructureType::HuntingBlind, 0.2f},
        {EStructureType::RitualPlatform, 0.1f}
    });
    
    BiomeStructureWeights.Add(EBiomeType::OpenPlains, {
        {EStructureType::TemporaryCamp, 0.4f},
        {EStructureType::Watchtower, 0.3f},
        {EStructureType::StoneCircle, 0.2f},
        {EStructureType::Windbreak, 0.1f}
    });
    
    BiomeStructureWeights.Add(EBiomeType::RiverDelta, {
        {EStructureType::LogCabin, 0.3f},
        {EStructureType::Smokehouse, 0.3f},
        {EStructureType::StoragePit, 0.2f},
        {EStructureType::CommunalKitchen, 0.2f}
    });
    
    BiomeStructureWeights.Add(EBiomeType::RockyOutcrops, {
        {EStructureType::CliffDwelling, 0.4f},
        {EStructureType::StoneHouse, 0.3f},
        {EStructureType::HiddenBunker, 0.2f},
        {EStructureType::OfferingAltar, 0.1f}
    });
    
    BiomeStructureWeights.Add(EBiomeType::Wetlands, {
        {EStructureType::UndergroundShelter, 0.4f},
        {EStructureType::Workshop, 0.3f},
        {EStructureType::BurialMound, 0.2f},
        {EStructureType::SacredGrove, 0.1f}
    });
}

void UArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchitecture, Log, TEXT("Architecture Manager initialized"));
    
    // Initialize structure templates
    InitializeStructureTemplates();
    
    // Initialize material libraries
    InitializeMaterialLibraries();
    
    // Initialize interior storytelling system
    InitializeInteriorStorytellingSystem();
    
    // Set up performance monitoring
    PerformanceMonitor = FArchitecturePerformanceMonitor();
    PerformanceMonitor.MaxDrawCalls = 1000;
    PerformanceMonitor.MaxTriangles = 500000;
    PerformanceMonitor.MaxInstances = 10000;
}

void UArchitectureManager::Deinitialize()
{
    // Clean up all generated structures
    ClearAllStructures();
    
    // Clean up PCG components
    for (auto& PCGComponent : ActivePCGComponents)
    {
        if (IsValid(PCGComponent))
        {
            PCGComponent->DestroyComponent();
        }
    }
    ActivePCGComponents.Empty();
    
    Super::Deinitialize();
}

void UArchitectureManager::GenerateStructuresForBiome(EBiomeType BiomeType, const FVector& CenterLocation, float Radius)
{
    UE_LOG(LogArchitecture, Log, TEXT("Generating structures for biome %s at location %s with radius %.2f"), 
           *UEnum::GetValueAsString(BiomeType), *CenterLocation.ToString(), Radius);
    
    // Get structure weights for this biome
    if (!BiomeStructureWeights.Contains(BiomeType))
    {
        UE_LOG(LogArchitecture, Warning, TEXT("No structure weights defined for biome %s"), *UEnum::GetValueAsString(BiomeType));
        return;
    }
    
    const TMap<EStructureType, float>& StructureWeights = BiomeStructureWeights[BiomeType];
    
    // Calculate number of structures to generate
    int32 NumStructures = FMath::RandRange(5, MaxStructuresPerBiome);
    
    // Generate structures using weighted random selection
    for (int32 i = 0; i < NumStructures; i++)
    {
        // Select structure type based on weights
        EStructureType SelectedType = SelectStructureTypeByWeight(StructureWeights);
        
        // Find suitable placement location
        FVector PlacementLocation;
        if (FindSuitablePlacementLocation(BiomeType, SelectedType, CenterLocation, Radius, PlacementLocation))
        {
            // Generate structure at location
            GenerateStructureAtLocation(SelectedType, PlacementLocation, BiomeType);
        }
    }
    
    UE_LOG(LogArchitecture, Log, TEXT("Generated %d structures for biome %s"), NumStructures, *UEnum::GetValueAsString(BiomeType));
}

bool UArchitectureManager::GenerateStructureAtLocation(EStructureType StructureType, const FVector& Location, EBiomeType BiomeType)
{
    // Create placement context
    FStructurePlacementContext PlacementContext = CreatePlacementContext(Location, BiomeType);
    
    // Create generation parameters
    FStructureGenerationParams GenerationParams = CreateGenerationParameters(StructureType, PlacementContext);
    
    // Generate the structure using the structure generator
    UStructureGenerator* StructureGenerator = NewObject<UStructureGenerator>(this);
    if (!StructureGenerator)
    {
        UE_LOG(LogArchitecture, Error, TEXT("Failed to create StructureGenerator"));
        return false;
    }
    
    FGeneratedStructureData StructureData = StructureGenerator->GenerateStructure(PlacementContext, GenerationParams);
    
    // Create the actual structure actor in the world
    AActor* StructureActor = CreateStructureActor(StructureData);
    if (!StructureActor)
    {
        UE_LOG(LogArchitecture, Error, TEXT("Failed to create structure actor"));
        return false;
    }
    
    // Register the structure
    RegisterGeneratedStructure(StructureData, StructureActor);
    
    // Generate interior if applicable
    if (GenerationParams.bIncludeInterior && HasInterior(StructureType))
    {
        GenerateInteriorForStructure(StructureData, StructureActor);
    }
    
    // Add environmental storytelling elements
    AddEnvironmentalStorytelling(StructureData, StructureActor);
    
    UE_LOG(LogArchitecture, Log, TEXT("Successfully generated %s at location %s"), 
           *UEnum::GetValueAsString(StructureType), *Location.ToString());
    
    return true;
}

EStructureType UArchitectureManager::SelectStructureTypeByWeight(const TMap<EStructureType, float>& Weights)
{
    float TotalWeight = 0.0f;
    for (const auto& WeightPair : Weights)
    {
        TotalWeight += WeightPair.Value;
    }
    
    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const auto& WeightPair : Weights)
    {
        CurrentWeight += WeightPair.Value;
        if (RandomValue <= CurrentWeight)
        {
            return WeightPair.Key;
        }
    }
    
    // Fallback to first structure type
    return Weights.begin()->Key;
}

bool UArchitectureManager::FindSuitablePlacementLocation(EBiomeType BiomeType, EStructureType StructureType, 
                                                        const FVector& CenterLocation, float SearchRadius, FVector& OutLocation)
{
    const int32 MaxAttempts = 50;
    
    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        // Generate random location within search radius
        FVector2D RandomPoint = FMath::RandPointInCircle(SearchRadius);
        FVector TestLocation = CenterLocation + FVector(RandomPoint.X, RandomPoint.Y, 0.0f);
        
        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = TestLocation + FVector(0, 0, 1000);
        FVector TraceEnd = TestLocation - FVector(0, 0, 1000);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            TestLocation = HitResult.Location;
            
            // Check if location is suitable
            if (IsLocationSuitableForStructure(TestLocation, StructureType, BiomeType))
            {
                OutLocation = TestLocation;
                return true;
            }
        }
    }
    
    UE_LOG(LogArchitecture, Warning, TEXT("Failed to find suitable placement location for %s after %d attempts"), 
           *UEnum::GetValueAsString(StructureType), MaxAttempts);
    return false;
}

bool UArchitectureManager::IsLocationSuitableForStructure(const FVector& Location, EStructureType StructureType, EBiomeType BiomeType)
{
    // Check minimum distance to other structures
    for (const auto& ExistingStructure : GeneratedStructures)
    {
        float Distance = FVector::Dist(Location, ExistingStructure.Value.WorldTransform.GetLocation());
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    // Check terrain slope
    float TerrainSlope = CalculateTerrainSlope(Location);
    if (TerrainSlope > GetMaxSlopeForStructure(StructureType))
    {
        return false;
    }
    
    // Check water proximity requirements
    float WaterDistance = CalculateDistanceToWater(Location);
    if (!IsWaterDistanceSuitable(StructureType, WaterDistance))
    {
        return false;
    }
    
    // Check biome-specific requirements
    if (!IsBiomeSuitableForStructure(BiomeType, StructureType))
    {
        return false;
    }
    
    return true;
}

FStructurePlacementContext UArchitectureManager::CreatePlacementContext(const FVector& Location, EBiomeType BiomeType)
{
    FStructurePlacementContext Context;
    Context.WorldLocation = Location;
    Context.BiomeType = BiomeType;
    Context.TerrainSlope = CalculateTerrainSlope(Location);
    Context.DistanceToWater = CalculateDistanceToWater(Location);
    Context.Elevation = Location.Z;
    Context.VegetationDensity = CalculateVegetationDensity(Location);
    Context.WindExposure = CalculateWindExposure(Location);
    Context.VisibilityRange = CalculateVisibilityRange(Location);
    Context.DrainageQuality = CalculateDrainageQuality(Location);
    Context.NaturalShelter = CalculateNaturalShelter(Location);
    Context.ThreatLevel = CalculateThreatLevel(Location);
    
    return Context;
}

FStructureGenerationParams UArchitectureManager::CreateGenerationParameters(EStructureType StructureType, const FStructurePlacementContext& Context)
{
    FStructureGenerationParams Params;
    Params.StructureType = StructureType;
    Params.SizeCategory = DetermineSizeCategory(StructureType, Context);
    Params.ConstructionQuality = DetermineConstructionQuality(StructureType, Context);
    Params.StructureAge = FMath::RandRange(1.0f, 365.0f); // 1 day to 1 year
    Params.MaintenanceLevel = DetermineMaintenanceLevel(StructureType, Context);
    Params.PrimaryMaterial = DeterminePrimaryMaterial(StructureType, Context.BiomeType);
    Params.SecondaryMaterial = DetermineSecondaryMaterial(StructureType, Context.BiomeType);
    Params.StructuralCondition = DetermineStructuralCondition(Params.StructureAge, Params.MaintenanceLevel);
    Params.RandomSeed = FMath::Rand();
    Params.bIncludeInterior = HasInterior(StructureType);
    Params.InteriorStoryTheme = GenerateInteriorStoryTheme(StructureType, Context);
    Params.InhabitantCount = DetermineInhabitantCount(StructureType);
    
    return Params;
}

AActor* UArchitectureManager::CreateStructureActor(const FGeneratedStructureData& StructureData)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Create the main structure actor
    AActor* StructureActor = GetWorld()->SpawnActor<AActor>();
    if (!StructureActor)
    {
        return nullptr;
    }
    
    // Set transform
    StructureActor->SetActorTransform(StructureData.WorldTransform);
    StructureActor->SetActorLabel(FString::Printf(TEXT("%s_%s"), 
        *UEnum::GetValueAsString(StructureData.StructureType),
        *StructureData.StructureID.ToString()));
    
    // Create root component
    USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("StructureRoot"));
    StructureActor->SetRootComponent(RootComponent);
    
    // Add building components
    for (const FBuildingComponent& Component : StructureData.BuildingComponents)
    {
        CreateBuildingComponentMesh(StructureActor, Component);
    }
    
    // Add collision components
    CreateStructureCollision(StructureActor, StructureData);
    
    return StructureActor;
}

void UArchitectureManager::CreateBuildingComponentMesh(AActor* StructureActor, const FBuildingComponent& Component)
{
    if (!StructureActor || !Component.ComponentMesh)
    {
        return;
    }
    
    // Create static mesh component
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(StructureActor);
    if (!MeshComponent)
    {
        return;
    }
    
    // Set mesh and material
    MeshComponent->SetStaticMesh(Component.ComponentMesh);
    if (Component.ComponentMaterial)
    {
        MeshComponent->SetMaterial(0, Component.ComponentMaterial);
    }
    
    // Set transform
    MeshComponent->SetRelativeTransform(Component.RelativeTransform);
    
    // Configure collision
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
    
    // Attach to structure
    MeshComponent->AttachToComponent(StructureActor->GetRootComponent(), 
        FAttachmentTransformRules::KeepRelativeTransform);
}

void UArchitectureManager::GenerateInteriorForStructure(const FGeneratedStructureData& StructureData, AActor* StructureActor)
{
    if (!StructureActor)
    {
        return;
    }
    
    // Generate interior props based on structure narrative
    for (const FInteriorPropConfig& PropConfig : StructureData.InteriorProps)
    {
        CreateInteriorProp(StructureActor, PropConfig);
    }
    
    // Add narrative elements
    AddNarrativeElements(StructureActor, StructureData);
    
    UE_LOG(LogArchitecture, Log, TEXT("Generated interior for structure %s with %d props"), 
           *StructureData.StructureID.ToString(), StructureData.InteriorProps.Num());
}

void UArchitectureManager::CreateInteriorProp(AActor* StructureActor, const FInteriorPropConfig& PropConfig)
{
    if (!StructureActor || !PropConfig.PropMesh)
    {
        return;
    }
    
    // Create prop mesh component
    UStaticMeshComponent* PropComponent = NewObject<UStaticMeshComponent>(StructureActor);
    if (!PropComponent)
    {
        return;
    }
    
    // Configure prop
    PropComponent->SetStaticMesh(PropConfig.PropMesh);
    if (PropConfig.PropMaterial)
    {
        PropComponent->SetMaterial(0, PropConfig.PropMaterial);
    }
    
    PropComponent->SetRelativeTransform(PropConfig.RelativeTransform);
    PropComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Attach to structure
    PropComponent->AttachToComponent(StructureActor->GetRootComponent(), 
        FAttachmentTransformRules::KeepRelativeTransform);
}

void UArchitectureManager::AddNarrativeElements(AActor* StructureActor, const FGeneratedStructureData& StructureData)
{
    // Add wear patterns based on inhabitant behavior
    AddWearPatterns(StructureActor, StructureData);
    
    // Add tool marks and usage signs
    AddUsageSigns(StructureActor, StructureData);
    
    // Add personal belongings based on departure circumstances
    AddPersonalBelongings(StructureActor, StructureData);
    
    // Add hidden caches and secrets
    AddHiddenElements(StructureActor, StructureData);
}

void UArchitectureManager::InitializeStructureTemplates()
{
    // Initialize templates for each structure type
    CreatePrimitiveHutTemplate();
    CreateLogCabinTemplate();
    CreateTreeHouseTemplate();
    CreateCliffDwellingTemplate();
    CreateUndergroundShelterTemplate();
    CreateWatchtowerTemplate();
    CreateRitualPlatformTemplate();
    CreateAbandonedCampTemplate();
    
    UE_LOG(LogArchitecture, Log, TEXT("Initialized %d structure templates"), StructureTemplates.Num());
}

void UArchitectureManager::InitializeMaterialLibraries()
{
    // Initialize material libraries for different construction materials
    // This would typically load materials from asset references
    
    UE_LOG(LogArchitecture, Log, TEXT("Initialized material libraries"));
}

void UArchitectureManager::InitializeInteriorStorytellingSystem()
{
    // Initialize narrative themes and story elements
    InteriorNarrativeThemes.Add("Abandoned in Haste", {
        "Scattered belongings suggest a hurried departure",
        "Unfinished meals indicate sudden interruption",
        "Tools left mid-use show abrupt abandonment"
    });
    
    InteriorNarrativeThemes.Add("Peaceful Departure", {
        "Neatly arranged belongings show planned leaving",
        "Covered fire pit indicates careful preparation",
        "Hidden food caches suggest intention to return"
    });
    
    InteriorNarrativeThemes.Add("Seasonal Migration", {
        "Lightweight items taken, heavy items cached",
        "Weather protection measures in place",
        "Markers indicating return timing"
    });
    
    UE_LOG(LogArchitecture, Log, TEXT("Initialized interior storytelling system with %d narrative themes"), 
           InteriorNarrativeThemes.Num());
}

// Helper functions implementation continues...

float UArchitectureManager::CalculateTerrainSlope(const FVector& Location)
{
    // Sample terrain height at multiple points to calculate slope
    const float SampleDistance = 100.0f;
    TArray<FVector> SamplePoints = {
        Location + FVector(SampleDistance, 0, 0),
        Location + FVector(-SampleDistance, 0, 0),
        Location + FVector(0, SampleDistance, 0),
        Location + FVector(0, -SampleDistance, 0)
    };
    
    float MaxHeightDiff = 0.0f;
    for (const FVector& SamplePoint : SamplePoints)
    {
        FHitResult HitResult;
        FVector TraceStart = SamplePoint + FVector(0, 0, 1000);
        FVector TraceEnd = SamplePoint - FVector(0, 0, 1000);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            float HeightDiff = FMath::Abs(HitResult.Location.Z - Location.Z);
            MaxHeightDiff = FMath::Max(MaxHeightDiff, HeightDiff);
        }
    }
    
    return MaxHeightDiff / SampleDistance; // Return slope as rise/run
}

float UArchitectureManager::CalculateDistanceToWater(const FVector& Location)
{
    // This would integrate with the water system to find nearest water source
    // For now, return a placeholder value
    return FMath::RandRange(50.0f, 500.0f);
}

bool UArchitectureManager::HasInterior(EStructureType StructureType)
{
    switch (StructureType)
    {
        case EStructureType::PrimitiveHut:
        case EStructureType::LogCabin:
        case EStructureType::StoneHouse:
        case EStructureType::UndergroundShelter:
        case EStructureType::TreeHouse:
        case EStructureType::CliffDwelling:
        case EStructureType::GatheringHall:
        case EStructureType::Workshop:
        case EStructureType::Smokehouse:
        case EStructureType::CommunalKitchen:
            return true;
        default:
            return false;
    }
}

void UArchitectureManager::RegisterGeneratedStructure(const FGeneratedStructureData& StructureData, AActor* StructureActor)
{
    GeneratedStructures.Add(StructureData.StructureID, StructureData);
    StructureActors.Add(StructureData.StructureID, StructureActor);
    
    // Update performance monitoring
    PerformanceMonitor.CurrentInstances++;
    PerformanceMonitor.CurrentDrawCalls += StructureData.BuildingComponents.Num();
    
    UE_LOG(LogArchitecture, Verbose, TEXT("Registered structure %s"), *StructureData.StructureID.ToString());
}

void UArchitectureManager::ClearAllStructures()
{
    // Destroy all structure actors
    for (auto& ActorPair : StructureActors)
    {
        if (IsValid(ActorPair.Value))
        {
            ActorPair.Value->Destroy();
        }
    }
    
    // Clear data structures
    GeneratedStructures.Empty();
    StructureActors.Empty();
    
    // Reset performance monitoring
    PerformanceMonitor = FArchitecturePerformanceMonitor();
    
    UE_LOG(LogArchitecture, Log, TEXT("Cleared all generated structures"));
}