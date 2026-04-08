// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentArtManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Classes/Landscape.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnvironmentArt, Log, All);

UEnvironmentArtManager::UEnvironmentArtManager()
{
    // Initialize default settings
    bAutoGenerateOnWorldLoad = true;
    GlobalVegetationDensityMultiplier = 1.0f;
    GlobalRockDensityMultiplier = 1.0f;
    PerformanceLODMultiplier = 1.0f;
    MaxInstancesPerComponent = 1000;
    StreamingCellSize = 10000.0f; // 100m cells
    
    // Initialize biome weights
    BiomeVegetationWeights.Add(EBiomeType::DenseForest, 1.0f);
    BiomeVegetationWeights.Add(EBiomeType::OpenWoodland, 0.7f);
    BiomeVegetationWeights.Add(EBiomeType::GrasslandPlains, 0.4f);
    BiomeVegetationWeights.Add(EBiomeType::RiverDelta, 0.8f);
    BiomeVegetationWeights.Add(EBiomeType::SwampMarshland, 0.9f);
    BiomeVegetationWeights.Add(EBiomeType::RockyOutcrops, 0.2f);
    BiomeVegetationWeights.Add(EBiomeType::MountainousRegion, 0.3f);
    BiomeVegetationWeights.Add(EBiomeType::CaveSystem, 0.1f);
    BiomeVegetationWeights.Add(EBiomeType::CoastalArea, 0.5f);
    BiomeVegetationWeights.Add(EBiomeType::VolcanicRegion, 0.2f);
}

void UEnvironmentArtManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art Manager initialized"));
    
    // Initialize streaming system
    InitializeStreamingSystem();
    
    // Setup material parameter collections
    InitializeMaterialParameterCollections();
    
    // Cache biome data
    CacheBiomeData();
}

void UEnvironmentArtManager::Deinitialize()
{
    // Cleanup streaming system
    CleanupStreamingSystem();
    
    // Clear cached data
    CachedBiomeData.Empty();
    ActiveStreamingCells.Empty();
    VegetationComponents.Empty();
    RockComponents.Empty();
    PropComponents.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art Manager deinitialized"));
}

void UEnvironmentArtManager::GenerateEnvironmentArt(const FVector& CenterLocation, float Radius)
{
    if (!GetWorld())
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("Cannot generate environment art: No valid world"));
        return;
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Generating environment art at location %s with radius %f"), 
           *CenterLocation.ToString(), Radius);
    
    // Get biome data for the area
    TArray<FBiomeInfluenceData> BiomeInfluences = GetBiomeInfluencesForArea(CenterLocation, Radius);
    
    if (BiomeInfluences.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No biome data found for area"));
        return;
    }
    
    // Generate vegetation
    GenerateVegetationForArea(CenterLocation, Radius, BiomeInfluences);
    
    // Generate rocks and geological features
    GenerateRocksForArea(CenterLocation, Radius, BiomeInfluences);
    
    // Generate environmental storytelling props
    GeneratePropsForArea(CenterLocation, Radius, BiomeInfluences);
    
    // Apply atmospheric effects
    ApplyAtmosphericEffects(CenterLocation, Radius, BiomeInfluences);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment art generation completed for area"));
}

void UEnvironmentArtManager::GenerateVegetationForArea(const FVector& CenterLocation, float Radius, 
                                                       const TArray<FBiomeInfluenceData>& BiomeInfluences)
{
    if (VegetationAssets.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No vegetation assets configured"));
        return;
    }
    
    // Create vegetation component if needed
    UHierarchicalInstancedStaticMeshComponent* VegetationComponent = GetOrCreateVegetationComponent(CenterLocation);
    if (!VegetationComponent)
    {
        UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to create vegetation component"));
        return;
    }
    
    // Calculate grid size based on density
    float GridSize = 100.0f; // 1 meter grid
    int32 GridCount = FMath::CeilToInt((Radius * 2.0f) / GridSize);
    
    FVector StartLocation = CenterLocation - FVector(Radius, Radius, 0.0f);
    
    for (int32 X = 0; X < GridCount; X++)
    {
        for (int32 Y = 0; Y < GridCount; Y++)
        {
            FVector GridLocation = StartLocation + FVector(X * GridSize, Y * GridSize, 0.0f);
            
            // Check if within radius
            if (FVector::Dist2D(GridLocation, CenterLocation) > Radius)
                continue;
            
            // Get terrain height
            FVector TerrainLocation = GetTerrainLocationAtPosition(GridLocation);
            if (TerrainLocation.Z < -9999.0f) // Invalid terrain
                continue;
            
            // Calculate biome influence at this location
            float TotalVegetationWeight = 0.0f;
            for (const FBiomeInfluenceData& BiomeInfluence : BiomeInfluences)
            {
                float Distance = FVector::Dist2D(GridLocation, BiomeInfluence.Location);
                float InfluenceWeight = FMath::Clamp(1.0f - (Distance / BiomeInfluence.Radius), 0.0f, 1.0f);
                
                if (BiomeVegetationWeights.Contains(BiomeInfluence.BiomeType))
                {
                    TotalVegetationWeight += InfluenceWeight * BiomeVegetationWeights[BiomeInfluence.BiomeType];
                }
            }
            
            // Apply global density multiplier
            TotalVegetationWeight *= GlobalVegetationDensityMultiplier;
            
            // Random chance based on weight
            if (FMath::RandRange(0.0f, 1.0f) > TotalVegetationWeight)
                continue;
            
            // Select vegetation type based on biome
            FVegetationAssetData* SelectedVegetation = SelectVegetationForBiomes(BiomeInfluences, TerrainLocation);
            if (!SelectedVegetation)
                continue;
            
            // Place vegetation instance
            PlaceVegetationInstance(*SelectedVegetation, TerrainLocation, VegetationComponent);
        }
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Generated vegetation for area - Total instances: %d"), 
           VegetationComponent->GetInstanceCount());
}

void UEnvironmentArtManager::GenerateRocksForArea(const FVector& CenterLocation, float Radius, 
                                                  const TArray<FBiomeInfluenceData>& BiomeInfluences)
{
    if (RockAssets.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No rock assets configured"));
        return;
    }
    
    // Create rock component if needed
    UHierarchicalInstancedStaticMeshComponent* RockComponent = GetOrCreateRockComponent(CenterLocation);
    if (!RockComponent)
    {
        UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to create rock component"));
        return;
    }
    
    // Calculate number of rocks based on area and biome
    float AreaSquareMeters = PI * Radius * Radius / 10000.0f; // Convert to square meters
    int32 TargetRockCount = FMath::RoundToInt(AreaSquareMeters * GlobalRockDensityMultiplier * 0.1f);
    
    for (int32 i = 0; i < TargetRockCount; i++)
    {
        // Random location within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector RockLocation = CenterLocation + RandomOffset;
        
        // Check if within radius
        if (FVector::Dist2D(RockLocation, CenterLocation) > Radius)
            continue;
        
        // Get terrain height
        FVector TerrainLocation = GetTerrainLocationAtPosition(RockLocation);
        if (TerrainLocation.Z < -9999.0f) // Invalid terrain
            continue;
        
        // Select rock type based on biome
        FRockAssetData* SelectedRock = SelectRockForBiomes(BiomeInfluences, TerrainLocation);
        if (!SelectedRock)
            continue;
        
        // Place rock instance
        PlaceRockInstance(*SelectedRock, TerrainLocation, RockComponent);
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Generated rocks for area - Total instances: %d"), 
           RockComponent->GetInstanceCount());
}

void UEnvironmentArtManager::GeneratePropsForArea(const FVector& CenterLocation, float Radius, 
                                                  const TArray<FBiomeInfluenceData>& BiomeInfluences)
{
    if (EnvironmentalProps.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No environmental props configured"));
        return;
    }
    
    // Create prop component if needed
    UHierarchicalInstancedStaticMeshComponent* PropComponent = GetOrCreatePropComponent(CenterLocation);
    if (!PropComponent)
    {
        UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to create prop component"));
        return;
    }
    
    // Calculate number of props based on area (much sparser than vegetation)
    float AreaSquareMeters = PI * Radius * Radius / 10000.0f;
    int32 TargetPropCount = FMath::RoundToInt(AreaSquareMeters * 0.01f); // Very sparse
    
    for (int32 i = 0; i < TargetPropCount; i++)
    {
        // Random location within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector PropLocation = CenterLocation + RandomOffset;
        
        // Check if within radius
        if (FVector::Dist2D(PropLocation, CenterLocation) > Radius)
            continue;
        
        // Get terrain height
        FVector TerrainLocation = GetTerrainLocationAtPosition(PropLocation);
        if (TerrainLocation.Z < -9999.0f) // Invalid terrain
            continue;
        
        // Select prop type based on biome and storytelling rules
        FEnvironmentalPropData* SelectedProp = SelectPropForBiomes(BiomeInfluences, TerrainLocation);
        if (!SelectedProp)
            continue;
        
        // Place prop instance
        PlacePropInstance(*SelectedProp, TerrainLocation, PropComponent);
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Generated environmental props for area - Total instances: %d"), 
           PropComponent->GetInstanceCount());
}

FVector UEnvironmentArtManager::GetTerrainLocationAtPosition(const FVector& WorldPosition)
{
    if (!GetWorld())
        return FVector(WorldPosition.X, WorldPosition.Y, -10000.0f);
    
    // Line trace to find terrain height
    FVector TraceStart = FVector(WorldPosition.X, WorldPosition.Y, 10000.0f);
    FVector TraceEnd = FVector(WorldPosition.X, WorldPosition.Y, -10000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return FVector(WorldPosition.X, WorldPosition.Y, -10000.0f);
}

TArray<FBiomeInfluenceData> UEnvironmentArtManager::GetBiomeInfluencesForArea(const FVector& CenterLocation, float Radius)
{
    TArray<FBiomeInfluenceData> BiomeInfluences;
    
    // For now, create a simple biome influence based on location
    // In a real implementation, this would query the biome system
    FBiomeInfluenceData DefaultBiome;
    DefaultBiome.BiomeType = EBiomeType::DenseForest;
    DefaultBiome.Location = CenterLocation;
    DefaultBiome.Radius = Radius;
    DefaultBiome.Influence = 1.0f;
    
    BiomeInfluences.Add(DefaultBiome);
    
    return BiomeInfluences;
}

FVegetationAssetData* UEnvironmentArtManager::SelectVegetationForBiomes(const TArray<FBiomeInfluenceData>& BiomeInfluences, 
                                                                        const FVector& Location)
{
    if (VegetationAssets.Num() == 0)
        return nullptr;
    
    // Simple selection for now - would be more sophisticated in real implementation
    int32 RandomIndex = FMath::RandRange(0, VegetationAssets.Num() - 1);
    return &VegetationAssets[RandomIndex];
}

FRockAssetData* UEnvironmentArtManager::SelectRockForBiomes(const TArray<FBiomeInfluenceData>& BiomeInfluences, 
                                                            const FVector& Location)
{
    if (RockAssets.Num() == 0)
        return nullptr;
    
    // Simple selection for now
    int32 RandomIndex = FMath::RandRange(0, RockAssets.Num() - 1);
    return &RockAssets[RandomIndex];
}

FEnvironmentalPropData* UEnvironmentArtManager::SelectPropForBiomes(const TArray<FBiomeInfluenceData>& BiomeInfluences, 
                                                                    const FVector& Location)
{
    if (EnvironmentalProps.Num() == 0)
        return nullptr;
    
    // Simple selection for now
    int32 RandomIndex = FMath::RandRange(0, EnvironmentalProps.Num() - 1);
    return &EnvironmentalProps[RandomIndex];
}

void UEnvironmentArtManager::PlaceVegetationInstance(const FVegetationAssetData& VegetationData, 
                                                     const FVector& Location, 
                                                     UHierarchicalInstancedStaticMeshComponent* Component)
{
    if (!Component || !VegetationData.StaticMesh.IsValid())
        return;
    
    // Calculate transform
    FVector Scale = FVector(FMath::RandRange(VegetationData.ScaleRange.X, VegetationData.ScaleRange.Y));
    FRotator Rotation = FRotator(0.0f, FMath::RandRange(0.0f, VegetationData.RotationVariation), 0.0f);
    FTransform InstanceTransform = FTransform(Rotation, Location, Scale);
    
    // Add instance
    Component->AddInstance(InstanceTransform);
}

void UEnvironmentArtManager::PlaceRockInstance(const FRockAssetData& RockData, 
                                               const FVector& Location, 
                                               UHierarchicalInstancedStaticMeshComponent* Component)
{
    if (!Component || !RockData.StaticMesh.IsValid())
        return;
    
    // Calculate transform with sink
    float SinkAmount = FMath::RandRange(RockData.SinkRange.X, RockData.SinkRange.Y);
    FVector AdjustedLocation = Location - FVector(0.0f, 0.0f, SinkAmount * 100.0f); // Convert to cm
    
    FVector Scale = FVector(FMath::RandRange(RockData.ScaleRange.X, RockData.ScaleRange.Y));
    FRotator Rotation = FRotator(0.0f, FMath::RandRange(0.0f, RockData.RotationVariation), 0.0f);
    FTransform InstanceTransform = FTransform(Rotation, AdjustedLocation, Scale);
    
    // Add instance
    Component->AddInstance(InstanceTransform);
}

void UEnvironmentArtManager::PlacePropInstance(const FEnvironmentalPropData& PropData, 
                                               const FVector& Location, 
                                               UHierarchicalInstancedStaticMeshComponent* Component)
{
    if (!Component || !PropData.StaticMesh.IsValid())
        return;
    
    // Calculate transform
    FVector Scale = FVector(FMath::RandRange(PropData.ScaleRange.X, PropData.ScaleRange.Y));
    FRotator Rotation = FRotator(0.0f, FMath::RandRange(0.0f, PropData.RotationVariation), 0.0f);
    FTransform InstanceTransform = FTransform(Rotation, Location, Scale);
    
    // Add instance
    Component->AddInstance(InstanceTransform);
}

UHierarchicalInstancedStaticMeshComponent* UEnvironmentArtManager::GetOrCreateVegetationComponent(const FVector& Location)
{
    // Find existing component or create new one
    FString ComponentKey = FString::Printf(TEXT("Vegetation_%d_%d"), 
                                          FMath::FloorToInt(Location.X / StreamingCellSize),
                                          FMath::FloorToInt(Location.Y / StreamingCellSize));
    
    if (VegetationComponents.Contains(ComponentKey))
    {
        return VegetationComponents[ComponentKey];
    }
    
    // Create new component
    UHierarchicalInstancedStaticMeshComponent* NewComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>();
    if (NewComponent)
    {
        VegetationComponents.Add(ComponentKey, NewComponent);
    }
    
    return NewComponent;
}

UHierarchicalInstancedStaticMeshComponent* UEnvironmentArtManager::GetOrCreateRockComponent(const FVector& Location)
{
    // Find existing component or create new one
    FString ComponentKey = FString::Printf(TEXT("Rock_%d_%d"), 
                                          FMath::FloorToInt(Location.X / StreamingCellSize),
                                          FMath::FloorToInt(Location.Y / StreamingCellSize));
    
    if (RockComponents.Contains(ComponentKey))
    {
        return RockComponents[ComponentKey];
    }
    
    // Create new component
    UHierarchicalInstancedStaticMeshComponent* NewComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>();
    if (NewComponent)
    {
        RockComponents.Add(ComponentKey, NewComponent);
    }
    
    return NewComponent;
}

UHierarchicalInstancedStaticMeshComponent* UEnvironmentArtManager::GetOrCreatePropComponent(const FVector& Location)
{
    // Find existing component or create new one
    FString ComponentKey = FString::Printf(TEXT("Prop_%d_%d"), 
                                          FMath::FloorToInt(Location.X / StreamingCellSize),
                                          FMath::FloorToInt(Location.Y / StreamingCellSize));
    
    if (PropComponents.Contains(ComponentKey))
    {
        return PropComponents[ComponentKey];
    }
    
    // Create new component
    UHierarchicalInstancedStaticMeshComponent* NewComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>();
    if (NewComponent)
    {
        PropComponents.Add(ComponentKey, NewComponent);
    }
    
    return NewComponent;
}

void UEnvironmentArtManager::InitializeStreamingSystem()
{
    // Initialize streaming cell management
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initializing environment art streaming system"));
}

void UEnvironmentArtManager::CleanupStreamingSystem()
{
    // Cleanup streaming system
    UE_LOG(LogEnvironmentArt, Log, TEXT("Cleaning up environment art streaming system"));
}

void UEnvironmentArtManager::InitializeMaterialParameterCollections()
{
    // Initialize material parameter collections for dynamic effects
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initializing material parameter collections"));
}

void UEnvironmentArtManager::CacheBiomeData()
{
    // Cache biome data for quick access
    UE_LOG(LogEnvironmentArt, Log, TEXT("Caching biome data"));
}

void UEnvironmentArtManager::ApplyAtmosphericEffects(const FVector& CenterLocation, float Radius, 
                                                     const TArray<FBiomeInfluenceData>& BiomeInfluences)
{
    // Apply atmospheric effects like particles, fog, etc.
    UE_LOG(LogEnvironmentArt, Log, TEXT("Applying atmospheric effects for area"));
}