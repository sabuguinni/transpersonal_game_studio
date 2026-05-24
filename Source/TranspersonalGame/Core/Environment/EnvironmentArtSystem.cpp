// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentArtSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Foliage/Public/InstancedFoliageActor.h"
#include "Foliage/Public/FoliageInstancedStaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnvironmentArt, Log, All);

UEnvironmentArtSystem::UEnvironmentArtSystem()
{
    // Initialize default values
    bEnableVegetationPlacement = true;
    bEnableAtmosphericEffects = true;
    bEnableProceduralProps = true;
    bEnableSeasonalVariation = false;
    
    GlobalVegetationDensityMultiplier = 1.0f;
    AtmosphericIntensity = 1.0f;
    
    // Initialize biome coordinates from global memory
    BiomeCoordinates.Add(EBiomeType::Swamp, FVector(-50000.0f, -45000.0f, 0.0f));
    BiomeCoordinates.Add(EBiomeType::Forest, FVector(-45000.0f, 40000.0f, 0.0f));
    BiomeCoordinates.Add(EBiomeType::Savanna, FVector(0.0f, 0.0f, 0.0f));
    BiomeCoordinates.Add(EBiomeType::Desert, FVector(55000.0f, 0.0f, 0.0f));
    BiomeCoordinates.Add(EBiomeType::Mountains, FVector(40000.0f, 50000.0f, 500.0f));
    
    // Initialize default vegetation assets
    InitializeDefaultVegetationAssets();
}

void UEnvironmentArtSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art System initialized"));
    
    // Find or create material parameter collection for global environment settings
    MaterialParameterCollection = LoadObject<UMaterialParameterCollection>(
        nullptr, 
        TEXT("/Game/TranspersonalGame/Materials/MPC_EnvironmentSettings")
    );
    
    if (!MaterialParameterCollection)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("Material Parameter Collection not found. Environment effects may not work correctly."));
    }
    
    // Cache landscape reference
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<ALandscape> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            LandscapeReference = *ActorIterator;
            break;
        }
        
        if (LandscapeReference)
        {
            UE_LOG(LogEnvironmentArt, Log, TEXT("Landscape reference cached: %s"), *LandscapeReference->GetName());
        }
    }
}

void UEnvironmentArtSystem::Deinitialize()
{
    // Clean up any spawned actors or components
    CleanupEnvironmentalActors();
    
    Super::Deinitialize();
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Environment Art System deinitialized"));
}

void UEnvironmentArtSystem::PopulateBiomeWithVegetation(EBiomeType BiomeType, const FVector& BiomeCenter, float Radius)
{
    if (!bEnableVegetationPlacement)
    {
        return;
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Populating %s biome with vegetation at %s (radius: %f)"), 
           *UEnum::GetValueAsString(BiomeType), *BiomeCenter.ToString(), Radius);
    
    // Get vegetation assets suitable for this biome
    TArray<FPrehistoricVegetationAsset> SuitableVegetation = GetVegetationForBiome(BiomeType);
    
    if (SuitableVegetation.Num() == 0)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No suitable vegetation found for biome %s"), *UEnum::GetValueAsString(BiomeType));
        return;
    }
    
    // Calculate placement grid
    int32 GridSize = FMath::CeilToInt(Radius / 500.0f); // 5m grid spacing
    float GridSpacing = Radius / GridSize;
    
    for (int32 X = -GridSize; X <= GridSize; X++)
    {
        for (int32 Y = -GridSize; Y <= GridSize; Y++)
        {
            FVector GridPosition = BiomeCenter + FVector(X * GridSpacing, Y * GridSpacing, 0.0f);
            
            // Check if position is within biome radius
            float DistanceFromCenter = FVector::Dist2D(GridPosition, BiomeCenter);
            if (DistanceFromCenter > Radius)
            {
                continue;
            }
            
            // Add some randomization to grid position
            FVector RandomOffset = FVector(
                FMath::RandRange(-GridSpacing * 0.3f, GridSpacing * 0.3f),
                FMath::RandRange(-GridSpacing * 0.3f, GridSpacing * 0.3f),
                0.0f
            );
            GridPosition += RandomOffset;
            
            // Attempt to place vegetation at this position
            PlaceVegetationAtLocation(GridPosition, SuitableVegetation, BiomeType);
        }
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Completed vegetation placement for %s biome"), *UEnum::GetValueAsString(BiomeType));
}

void UEnvironmentArtSystem::PlaceEnvironmentalProps(EBiomeType BiomeType, const FVector& BiomeCenter, float Radius)
{
    if (!bEnableProceduralProps)
    {
        return;
    }
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Placing environmental props in %s biome"), *UEnum::GetValueAsString(BiomeType));
    
    // Get props suitable for this biome
    TArray<EEnvironmentalPropType> SuitableProps = GetPropsForBiome(BiomeType);
    
    // Calculate number of props based on biome size
    int32 NumProps = FMath::RandRange(
        FMath::FloorToInt(Radius * 0.01f), 
        FMath::FloorToInt(Radius * 0.02f)
    );
    
    for (int32 i = 0; i < NumProps; i++)
    {
        // Random position within biome radius
        float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        float RandomDistance = FMath::RandRange(0.0f, Radius * 0.8f);
        
        FVector PropPosition = BiomeCenter + FVector(
            FMath::Cos(RandomAngle) * RandomDistance,
            FMath::Sin(RandomAngle) * RandomDistance,
            0.0f
        );
        
        // Select random prop type
        if (SuitableProps.Num() > 0)
        {
            EEnvironmentalPropType PropType = SuitableProps[FMath::RandRange(0, SuitableProps.Num() - 1)];
            PlacePropAtLocation(PropPosition, PropType, BiomeType);
        }
    }
}

void UEnvironmentArtSystem::SetupAtmosphericLighting(EBiomeType BiomeType, float TimeOfDay)
{
    if (!bEnableAtmosphericEffects)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light (sun)
    ADirectionalLight* Sun = nullptr;
    for (TActorIterator<ADirectionalLight> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        Sun = *ActorIterator;
        break;
    }
    
    if (!Sun)
    {
        UE_LOG(LogEnvironmentArt, Warning, TEXT("No directional light found for atmospheric setup"));
        return;
    }
    
    UDirectionalLightComponent* SunComponent = Sun->GetComponent();
    if (!SunComponent)
    {
        return;
    }
    
    // Calculate sun angle based on time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    float SunAngle = (TimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    
    // Set sun rotation
    FRotator SunRotation = FRotator(SunAngle, 45.0f, 0.0f); // 45 degree azimuth for golden hour feel
    Sun->SetActorRotation(SunRotation);
    
    // Adjust sun intensity based on time of day
    float SunIntensity = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(SunAngle)) * 2.0f, 0.1f, 3.0f);
    SunComponent->SetIntensity(SunIntensity * AtmosphericIntensity);
    
    // Adjust sun color based on time of day
    FLinearColor SunColor;
    if (FMath::Abs(SunAngle) > 60.0f) // Dawn/dusk
    {
        SunColor = FLinearColor(1.0f, 0.7f, 0.4f); // Warm orange
    }
    else if (FMath::Abs(SunAngle) < 30.0f) // Midday
    {
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f); // Bright white
    }
    else // Morning/afternoon
    {
        SunColor = FLinearColor(1.0f, 0.85f, 0.7f); // Warm white
    }
    
    SunComponent->SetLightColor(SunColor);
    
    // Setup biome-specific atmospheric effects
    SetupBiomeAtmosphere(BiomeType);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Atmospheric lighting setup complete for %s biome (Time: %f, Angle: %f)"), 
           *UEnum::GetValueAsString(BiomeType), TimeOfDay, SunAngle);
}

void UEnvironmentArtSystem::CreateVolumetricFog(const FVector& Location, float Radius, float Density)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find existing exponential height fog or create new one
    AExponentialHeightFog* FogActor = nullptr;
    for (TActorIterator<AExponentialHeightFog> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        FogActor = *ActorIterator;
        break;
    }
    
    if (!FogActor)
    {
        // Spawn new fog actor
        FogActor = World->SpawnActor<AExponentialHeightFog>(Location, FRotator::ZeroRotator);
        if (!FogActor)
        {
            UE_LOG(LogEnvironmentArt, Error, TEXT("Failed to spawn fog actor"));
            return;
        }
    }
    
    UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
    if (!FogComponent)
    {
        return;
    }
    
    // Configure fog properties
    FogComponent->SetFogDensity(Density * AtmosphericIntensity);
    FogComponent->SetFogHeightFalloff(0.02f); // Gradual falloff
    FogComponent->SetFogMaxOpacity(0.8f);
    FogComponent->SetStartDistance(1000.0f);
    FogComponent->SetFogCutoffDistance(50000.0f);
    
    // Set fog color based on time of day
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f); // Bluish-gray
    FogComponent->SetFogInscatteringColor(FogColor);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Volumetric fog created at %s (Density: %f)"), *Location.ToString(), Density);
}

void UEnvironmentArtSystem::SpawnParticleEffects(const FVector& Location, EEnvironmentalPropType EffectType)
{
    // This would spawn particle systems for atmospheric effects like dust, pollen, etc.
    // Implementation depends on available particle assets
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Particle effect spawned at %s (Type: %s)"), 
           *Location.ToString(), *UEnum::GetValueAsString(EffectType));
}

void UEnvironmentArtSystem::InitializeDefaultVegetationAssets()
{
    // Initialize with placeholder data - these would be replaced with actual asset references
    
    // Large Ferns
    FPrehistoricVegetationAsset LargeFern;
    LargeFern.VegetationType = EPrehistoricVegetationType::LargeFerns;
    LargeFern.DisplayName = TEXT("Large Prehistoric Fern");
    LargeFern.DensityPerSquareMeter = 0.3f;
    LargeFern.ScaleRange = FVector2D(0.8f, 1.5f);
    LargeFern.PlacementRules.SuitableBiomes = {EBiomeType::Forest, EBiomeType::Swamp};
    LargeFern.PlacementRules.PreferredSlopeRange = FVector2D(0.0f, 20.0f);
    LargeFern.bProvidesConcealment = true;
    DefaultVegetationAssets.Add(LargeFern);
    
    // Conifer Trees
    FPrehistoricVegetationAsset ConiferTree;
    ConiferTree.VegetationType = EPrehistoricVegetationType::ConiferTrees;
    ConiferTree.DisplayName = TEXT("Prehistoric Conifer");
    ConiferTree.DensityPerSquareMeter = 0.05f;
    ConiferTree.ScaleRange = FVector2D(1.0f, 2.0f);
    ConiferTree.PlacementRules.SuitableBiomes = {EBiomeType::Forest, EBiomeType::Mountains};
    ConiferTree.PlacementRules.ClusteringTendency = 0.7f;
    ConiferTree.bCanBeDestroyed = false; // Large trees are indestructible
    DefaultVegetationAssets.Add(ConiferTree);
    
    // Cycad Palms
    FPrehistoricVegetationAsset CycadPalm;
    CycadPalm.VegetationType = EPrehistoricVegetationType::CycadPalms;
    CycadPalm.DisplayName = TEXT("Cycad Palm");
    CycadPalm.DensityPerSquareMeter = 0.1f;
    CycadPalm.ScaleRange = FVector2D(0.7f, 1.3f);
    CycadPalm.PlacementRules.SuitableBiomes = {EBiomeType::Savanna, EBiomeType::Desert};
    CycadPalm.PlacementRules.MinDistanceFromWater = 500.0f;
    CycadPalm.bProvidesFood = true; // Herbivores can eat these
    DefaultVegetationAssets.Add(CycadPalm);
    
    // Horsetails
    FPrehistoricVegetationAsset Horsetail;
    Horsetail.VegetationType = EPrehistoricVegetationType::Horsetails;
    Horsetail.DisplayName = TEXT("Giant Horsetail");
    Horsetail.DensityPerSquareMeter = 0.8f;
    Horsetail.ScaleRange = FVector2D(0.5f, 1.2f);
    Horsetail.PlacementRules.SuitableBiomes = {EBiomeType::Swamp};
    Horsetail.PlacementRules.MaxDistanceFromWater = 1000.0f;
    Horsetail.bCanBeDestroyed = true;
    DefaultVegetationAssets.Add(Horsetail);
    
    UE_LOG(LogEnvironmentArt, Log, TEXT("Initialized %d default vegetation assets"), DefaultVegetationAssets.Num());
}

TArray<FPrehistoricVegetationAsset> UEnvironmentArtSystem::GetVegetationForBiome(EBiomeType BiomeType) const
{
    TArray<FPrehistoricVegetationAsset> SuitableVegetation;
    
    for (const FPrehistoricVegetationAsset& Asset : DefaultVegetationAssets)
    {
        if (Asset.PlacementRules.SuitableBiomes.Contains(BiomeType))
        {
            SuitableVegetation.Add(Asset);
        }
    }
    
    return SuitableVegetation;
}

TArray<EEnvironmentalPropType> UEnvironmentArtSystem::GetPropsForBiome(EBiomeType BiomeType) const
{
    TArray<EEnvironmentalPropType> SuitableProps;
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            SuitableProps = {
                EEnvironmentalPropType::FallenLogs,
                EEnvironmentalPropType::DeadTrees,
                EEnvironmentalPropType::BrokenBranches,
                EEnvironmentalPropType::DinosaurBones,
                EEnvironmentalPropType::ClawMarks
            };
            break;
            
        case EBiomeType::Swamp:
            SuitableProps = {
                EEnvironmentalPropType::Driftwood,
                EEnvironmentalPropType::MudPatches,
                EEnvironmentalPropType::WaterPlants,
                EEnvironmentalPropType::Mist
            };
            break;
            
        case EBiomeType::Savanna:
            SuitableProps = {
                EEnvironmentalPropType::ScatteredStones,
                EEnvironmentalPropType::DinosaurBones,
                EEnvironmentalPropType::AbandonedNests,
                EEnvironmentalPropType::LeafLitter
            };
            break;
            
        case EBiomeType::Desert:
            SuitableProps = {
                EEnvironmentalPropType::Boulders,
                EEnvironmentalPropType::RockOutcrops,
                EEnvironmentalPropType::FossilizedRemains,
                EEnvironmentalPropType::ScatteredStones
            };
            break;
            
        case EBiomeType::Mountains:
            SuitableProps = {
                EEnvironmentalPropType::Boulders,
                EEnvironmentalPropType::RockOutcrops,
                EEnvironmentalPropType::ScatteredStones,
                EEnvironmentalPropType::LightShafts
            };
            break;
            
        default:
            break;
    }
    
    return SuitableProps;
}

void UEnvironmentArtSystem::PlaceVegetationAtLocation(const FVector& Location, const TArray<FPrehistoricVegetationAsset>& AvailableVegetation, EBiomeType BiomeType)
{
    if (AvailableVegetation.Num() == 0)
    {
        return;
    }
    
    // Get landscape height at this location
    FVector AdjustedLocation = Location;
    if (LandscapeReference)
    {
        float LandscapeHeight = 0.0f;
        if (LandscapeReference->GetHeightAtLocation(Location, LandscapeHeight))
        {
            AdjustedLocation.Z = LandscapeHeight;
        }
    }
    
    // Select random vegetation type
    const FPrehistoricVegetationAsset& SelectedVegetation = AvailableVegetation[FMath::RandRange(0, AvailableVegetation.Num() - 1)];
    
    // Apply density check
    if (FMath::RandRange(0.0f, 1.0f) > SelectedVegetation.DensityPerSquareMeter * GlobalVegetationDensityMultiplier)
    {
        return; // Skip this placement
    }
    
    // Calculate scale and rotation
    float Scale = FMath::RandRange(SelectedVegetation.ScaleRange.X, SelectedVegetation.ScaleRange.Y);
    FRotator Rotation = FRotator::ZeroRotator;
    
    if (SelectedVegetation.bRandomizeRotation)
    {
        Rotation.Yaw = FMath::RandRange(0.0f, 360.0f);
    }
    
    if (SelectedVegetation.MaxTiltAngle > 0.0f)
    {
        Rotation.Pitch = FMath::RandRange(-SelectedVegetation.MaxTiltAngle, SelectedVegetation.MaxTiltAngle);
        Rotation.Roll = FMath::RandRange(-SelectedVegetation.MaxTiltAngle, SelectedVegetation.MaxTiltAngle);
    }
    
    // For now, just log the placement (actual mesh spawning would require asset references)
    UE_LOG(LogEnvironmentArt, VeryVerbose, TEXT("Placed %s at %s (Scale: %f, Rotation: %s)"), 
           *SelectedVegetation.DisplayName, *AdjustedLocation.ToString(), Scale, *Rotation.ToString());
    
    // In a full implementation, this would spawn the actual mesh using:
    // - Hierarchical Instanced Static Mesh Component for performance
    // - Foliage system integration
    // - LOD management
}

void UEnvironmentArtSystem::PlacePropAtLocation(const FVector& Location, EEnvironmentalPropType PropType, EBiomeType BiomeType)
{
    // Get landscape height at this location
    FVector AdjustedLocation = Location;
    if (LandscapeReference)
    {
        float LandscapeHeight = 0.0f;
        if (LandscapeReference->GetHeightAtLocation(Location, LandscapeHeight))
        {
            AdjustedLocation.Z = LandscapeHeight;
        }
    }
    
    // Add some random variation to placement
    FVector RandomOffset = FVector(
        FMath::RandRange(-100.0f, 100.0f),
        FMath::RandRange(-100.0f, 100.0f),
        0.0f
    );
    AdjustedLocation += RandomOffset;
    
    // Random rotation
    FRotator PropRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    
    // Random scale
    float PropScale = FMath::RandRange(0.8f, 1.5f);
    
    UE_LOG(LogEnvironmentArt, VeryVerbose, TEXT("Placed %s prop at %s (Scale: %f)"), 
           *UEnum::GetValueAsString(PropType), *AdjustedLocation.ToString(), PropScale);
    
    // In a full implementation, this would spawn the actual prop mesh
}

void UEnvironmentArtSystem::SetupBiomeAtmosphere(EBiomeType BiomeType)
{
    // Configure atmosphere based on biome type
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            // Dense, green-tinted atmosphere with dappled lighting
            CreateVolumetricFog(BiomeCoordinates[BiomeType], 30000.0f, 0.002f);
            break;
            
        case EBiomeType::Swamp:
            // Heavy, misty atmosphere with low visibility
            CreateVolumetricFog(BiomeCoordinates[BiomeType], 25000.0f, 0.005f);
            break;
            
        case EBiomeType::Desert:
            // Clear, harsh atmosphere with heat shimmer
            CreateVolumetricFog(BiomeCoordinates[BiomeType], 40000.0f, 0.001f);
            break;
            
        case EBiomeType::Mountains:
            // Thin, clear atmosphere with distant haze
            CreateVolumetricFog(BiomeCoordinates[BiomeType], 35000.0f, 0.0015f);
            break;
            
        case EBiomeType::Savanna:
            // Moderate atmosphere with dust particles
            CreateVolumetricFog(BiomeCoordinates[BiomeType], 45000.0f, 0.0025f);
            break;
            
        default:
            break;
    }
}

void UEnvironmentArtSystem::CleanupEnvironmentalActors()
{
    // Clean up any actors spawned by this system
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would iterate through and clean up spawned environmental actors
    // For now, just log the cleanup
    UE_LOG(LogEnvironmentArt, Log, TEXT("Cleaning up environmental actors"));
}