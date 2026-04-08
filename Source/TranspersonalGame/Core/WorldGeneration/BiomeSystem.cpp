// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UBiomeData::UBiomeData()
{
    // Set default values for Dense Jungle biome
    BiomeType = EBiomeType::DenseJungle;
    BiomeName = FText::FromString(TEXT("Dense Jungle"));
    BiomeDescription = FText::FromString(TEXT("A thick prehistoric jungle with towering trees, dense undergrowth, and limited visibility. High humidity and abundant wildlife make this a challenging but resource-rich environment."));
    BiomeColor = FLinearColor(0.2f, 0.6f, 0.2f, 1.0f); // Dark green
    
    // Terrain settings
    TerrainRoughness = 0.7f;
    TerrainScale = 1.5f;
    ElevationRange = FVector2D(0.0f, 1500.0f);
    SlopePreference = 0.4f;
    
    // Vegetation settings
    VegetationDensity = EVegetationDensity::VeryDense;
    TreeHeightMultiplier = 2.0f;
    UndergrowthMultiplier = 3.0f;
    
    // Wildlife settings
    WildlifeFrequency = EWildlifeFrequency::Abundant;
    
    // Default vegetation assets
    FVegetationAsset TreeAsset;
    TreeAsset.SpawnWeight = 10.0f;
    TreeAsset.MinScale = 1.5f;
    TreeAsset.MaxScale = 3.0f;
    TreeAsset.MinSpacing = 300.0f;
    TreeAsset.bCanBeDestroyed = true;
    TreeAsset.bProvidesFood = false;
    TreeAsset.bProvidesShelter = true;
    VegetationAssets.Add(TreeAsset);
    
    FVegetationAsset UndergrowthAsset;
    UndergrowthAsset.SpawnWeight = 8.0f;
    UndergrowthAsset.MinScale = 0.8f;
    UndergrowthAsset.MaxScale = 1.5f;
    UndergrowthAsset.MinSpacing = 150.0f;
    UndergrowthAsset.bCanBeDestroyed = true;
    UndergrowthAsset.bProvidesFood = true;
    UndergrowthAsset.bProvidesShelter = true;
    VegetationAssets.Add(UndergrowthAsset);
    
    // Default resource spawns
    FResourceSpawnData StoneResource;
    StoneResource.ResourceType = TEXT("Stone");
    StoneResource.SpawnProbability = 0.3f;
    StoneResource.MinInstancesPerKm2 = 20;
    StoneResource.MaxInstancesPerKm2 = 50;
    StoneResource.QualityMultiplier = 1.0f;
    ResourceSpawns.Add(StoneResource);
    
    FResourceSpawnData WoodResource;
    WoodResource.ResourceType = TEXT("Wood");
    WoodResource.SpawnProbability = 0.8f;
    WoodResource.MinInstancesPerKm2 = 100;
    WoodResource.MaxInstancesPerKm2 = 300;
    WoodResource.QualityMultiplier = 1.2f;
    ResourceSpawns.Add(WoodResource);
    
    FResourceSpawnData FiberResource;
    FiberResource.ResourceType = TEXT("Plant Fiber");
    FiberResource.SpawnProbability = 0.9f;
    FiberResource.MinInstancesPerKm2 = 200;
    FiberResource.MaxInstancesPerKm2 = 500;
    FiberResource.QualityMultiplier = 1.1f;
    ResourceSpawns.Add(FiberResource);
    
    // Default environmental hazards
    FEnvironmentalHazard PoisonousPlants;
    PoisonousPlants.HazardType = TEXT("Poisonous Plants");
    PoisonousPlants.Intensity = 0.3f;
    PoisonousPlants.Frequency = 0.2f;
    PoisonousPlants.EffectRadius = 200.0f;
    PoisonousPlants.DamagePerSecond = 5.0f;
    EnvironmentalHazards.Add(PoisonousPlants);
    
    FEnvironmentalHazard UnstableGround;
    UnstableGround.HazardType = TEXT("Unstable Ground");
    UnstableGround.Intensity = 0.2f;
    UnstableGround.Frequency = 0.1f;
    UnstableGround.EffectRadius = 300.0f;
    UnstableGround.DamagePerSecond = 0.0f; // No damage, just movement penalty
    EnvironmentalHazards.Add(UnstableGround);
    
    // Climate settings
    TemperatureRange = FVector2D(22.0f, 32.0f); // Warm tropical climate
    HumidityRange = FVector2D(75.0f, 95.0f); // Very humid
    RainfallAmount = 0.8f; // High rainfall
    WindStrength = 0.3f; // Light wind due to dense vegetation
    
    // Lighting settings
    AmbientLightMultiplier = 0.6f; // Darker due to canopy
    SunlightPenetration = 0.4f; // Limited sunlight through canopy
    FogDensity = 0.7f; // High fog/mist
    
    // Audio settings
    AmbientSoundIntensity = 0.9f; // Loud jungle sounds
    EchoAmount = 0.5f; // Some echo from dense vegetation
    
    // Gameplay modifiers
    MovementSpeedMultiplier = 0.7f; // Slower movement through dense vegetation
    VisibilityRange = 0.5f; // Reduced visibility
    StealthBonusMultiplier = 1.5f; // Easier to hide
    ResourceGatheringMultiplier = 1.3f; // More resources available
    
    // Dinosaur preferences
    PreferredDinosaurTypes.Add(TEXT("Compsognathus"));
    PreferredDinosaurTypes.Add(TEXT("Dilophosaurus"));
    PreferredDinosaurTypes.Add(TEXT("Parasaurolophus"));
    PreferredDinosaurTypes.Add(TEXT("Brachiosaurus"));
    PreferredDinosaurTypes.Add(TEXT("Carnotaurus"));
    
    // Biome transition settings
    TransitionBlendDistance = 500.0f;
    EdgeSoftness = 0.3f;
    CanBlendWith.Add(EBiomeType::RiverValley);
    CanBlendWith.Add(EBiomeType::Swampland);
    CanBlendWith.Add(EBiomeType::Grasslands);
}

UBiomeManager::UBiomeManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    
    // Default biome weights for world generation
    BiomeWeights.Add(EBiomeType::DenseJungle, 0.25f);
    BiomeWeights.Add(EBiomeType::RiverValley, 0.15f);
    BiomeWeights.Add(EBiomeType::RockyHighlands, 0.15f);
    BiomeWeights.Add(EBiomeType::CoastalPlains, 0.10f);
    BiomeWeights.Add(EBiomeType::VolcanicRegion, 0.05f);
    BiomeWeights.Add(EBiomeType::Swampland, 0.10f);
    BiomeWeights.Add(EBiomeType::Grasslands, 0.15f);
    BiomeWeights.Add(EBiomeType::CaveSystem, 0.05f);
    
    BiomeTransitionDistance = 1000.0f;
    BiomeNoiseScale = 0.01f;
    BiomeNoiseOctaves = 4;
    ElevationInfluence = 0.7f;
    TemperatureInfluence = 0.5f;
    HumidityInfluence = 0.6f;
}

void UBiomeManager::InitializeComponent()
{
    Super::InitializeComponent();
    
    // Load default biome data assets
    LoadBiomeDataAssets();
    
    // Initialize biome noise generator
    InitializeBiomeNoise();
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager initialized with %d biome types"), BiomeDataAssets.Num());
}

void UBiomeManager::LoadBiomeDataAssets()
{
    // This would typically load from content browser or data assets
    // For now, create default biome data programmatically
    
    for (int32 i = 0; i < (int32)EBiomeType::CaveSystem + 1; ++i)
    {
        EBiomeType BiomeType = (EBiomeType)i;
        UBiomeData* BiomeData = CreateBiomeData(BiomeType);
        if (BiomeData)
        {
            BiomeDataAssets.Add(BiomeType, BiomeData);
        }
    }
}

UBiomeData* UBiomeManager::CreateBiomeData(EBiomeType BiomeType)
{
    UBiomeData* BiomeData = NewObject<UBiomeData>(this);
    if (!BiomeData)
    {
        return nullptr;
    }
    
    BiomeData->BiomeType = BiomeType;
    
    switch (BiomeType)
    {
        case EBiomeType::DenseJungle:
            // Already configured in UBiomeData constructor
            break;
            
        case EBiomeType::RiverValley:
            BiomeData->BiomeName = FText::FromString(TEXT("River Valley"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Fertile valleys carved by ancient rivers. Rich in water sources and diverse plant life."));
            BiomeData->BiomeColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::Dense;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Abundant;
            BiomeData->TemperatureRange = FVector2D(18.0f, 28.0f);
            BiomeData->HumidityRange = FVector2D(60.0f, 85.0f);
            BiomeData->MovementSpeedMultiplier = 0.9f;
            BiomeData->ResourceGatheringMultiplier = 1.4f;
            break;
            
        case EBiomeType::RockyHighlands:
            BiomeData->BiomeName = FText::FromString(TEXT("Rocky Highlands"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Elevated rocky terrain with sparse vegetation and excellent visibility. Dangerous predators patrol these areas."));
            BiomeData->BiomeColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::Sparse;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Uncommon;
            BiomeData->TemperatureRange = FVector2D(12.0f, 22.0f);
            BiomeData->HumidityRange = FVector2D(30.0f, 60.0f);
            BiomeData->MovementSpeedMultiplier = 0.8f;
            BiomeData->VisibilityRange = 1.5f;
            BiomeData->StealthBonusMultiplier = 0.7f;
            break;
            
        case EBiomeType::CoastalPlains:
            BiomeData->BiomeName = FText::FromString(TEXT("Coastal Plains"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Open coastal areas with palm trees and beach vegetation. Access to ocean resources."));
            BiomeData->BiomeColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::Light;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Common;
            BiomeData->TemperatureRange = FVector2D(20.0f, 30.0f);
            BiomeData->HumidityRange = FVector2D(65.0f, 80.0f);
            BiomeData->MovementSpeedMultiplier = 1.1f;
            BiomeData->VisibilityRange = 1.3f;
            break;
            
        case EBiomeType::VolcanicRegion:
            BiomeData->BiomeName = FText::FromString(TEXT("Volcanic Region"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Dangerous volcanic landscape with geothermal activity. Rare minerals but extreme hazards."));
            BiomeData->BiomeColor = FLinearColor(0.8f, 0.3f, 0.2f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::None;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Rare;
            BiomeData->TemperatureRange = FVector2D(35.0f, 50.0f);
            BiomeData->HumidityRange = FVector2D(20.0f, 40.0f);
            BiomeData->MovementSpeedMultiplier = 0.6f;
            
            // Add volcanic hazards
            FEnvironmentalHazard LavaFlow;
            LavaFlow.HazardType = TEXT("Lava Flow");
            LavaFlow.Intensity = 1.0f;
            LavaFlow.Frequency = 0.3f;
            LavaFlow.EffectRadius = 500.0f;
            LavaFlow.DamagePerSecond = 50.0f;
            BiomeData->EnvironmentalHazards.Add(LavaFlow);
            break;
            
        case EBiomeType::Swampland:
            BiomeData->BiomeName = FText::FromString(TEXT("Swampland"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Murky wetlands with standing water and dangerous creatures lurking beneath the surface."));
            BiomeData->BiomeColor = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::Medium;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Common;
            BiomeData->TemperatureRange = FVector2D(24.0f, 34.0f);
            BiomeData->HumidityRange = FVector2D(85.0f, 100.0f);
            BiomeData->MovementSpeedMultiplier = 0.5f;
            BiomeData->StealthBonusMultiplier = 1.3f;
            break;
            
        case EBiomeType::Grasslands:
            BiomeData->BiomeName = FText::FromString(TEXT("Grasslands"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Open grasslands with scattered trees. Excellent visibility but little cover from predators."));
            BiomeData->BiomeColor = FLinearColor(0.5f, 0.7f, 0.3f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::Light;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Abundant;
            BiomeData->TemperatureRange = FVector2D(16.0f, 26.0f);
            BiomeData->HumidityRange = FVector2D(45.0f, 70.0f);
            BiomeData->MovementSpeedMultiplier = 1.2f;
            BiomeData->VisibilityRange = 2.0f;
            BiomeData->StealthBonusMultiplier = 0.5f;
            break;
            
        case EBiomeType::CaveSystem:
            BiomeData->BiomeName = FText::FromString(TEXT("Cave System"));
            BiomeData->BiomeDescription = FText::FromString(TEXT("Underground cave networks with unique ecosystems and hidden treasures."));
            BiomeData->BiomeColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
            BiomeData->VegetationDensity = EVegetationDensity::None;
            BiomeData->WildlifeFrequency = EWildlifeFrequency::Uncommon;
            BiomeData->TemperatureRange = FVector2D(14.0f, 18.0f);
            BiomeData->HumidityRange = FVector2D(70.0f, 90.0f);
            BiomeData->MovementSpeedMultiplier = 0.8f;
            BiomeData->VisibilityRange = 0.3f;
            BiomeData->AmbientLightMultiplier = 0.1f;
            break;
    }
    
    return BiomeData;
}

void UBiomeManager::InitializeBiomeNoise()
{
    // Initialize noise parameters for biome generation
    // This would typically use a noise library like FastNoise
    BiomeNoiseOffset = FVector2D(FMath::RandRange(-10000.0f, 10000.0f), FMath::RandRange(-10000.0f, 10000.0f));
    
    UE_LOG(LogTemp, Log, TEXT("Biome noise initialized with offset: %s"), *BiomeNoiseOffset.ToString());
}

EBiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Convert world location to biome noise coordinates
    FVector2D NoiseCoord = FVector2D(WorldLocation.X, WorldLocation.Y) * BiomeNoiseScale + BiomeNoiseOffset;
    
    // Generate base noise value
    float NoiseValue = GenerateNoiseValue(NoiseCoord);
    
    // Apply elevation influence
    float Elevation = WorldLocation.Z;
    float ElevationFactor = FMath::Clamp(Elevation / 2000.0f, 0.0f, 1.0f); // Normalize to 0-1
    NoiseValue += ElevationFactor * ElevationInfluence;
    
    // Apply temperature and humidity influences (would be calculated from climate system)
    float Temperature = CalculateTemperatureAtLocation(WorldLocation);
    float Humidity = CalculateHumidityAtLocation(WorldLocation);
    
    NoiseValue += (Temperature / 40.0f - 0.5f) * TemperatureInfluence;
    NoiseValue += (Humidity / 100.0f - 0.5f) * HumidityInfluence;
    
    // Normalize noise value to 0-1 range
    NoiseValue = FMath::Clamp((NoiseValue + 1.0f) * 0.5f, 0.0f, 1.0f);
    
    // Map noise value to biome type based on weights
    return MapNoiseToBiome(NoiseValue);
}

float UBiomeManager::GenerateNoiseValue(const FVector2D& Coordinate) const
{
    // Simple Perlin-like noise implementation
    // In a real implementation, you'd use a proper noise library
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    
    for (int32 i = 0; i < BiomeNoiseOctaves; ++i)
    {
        Value += FMath::PerlinNoise2D(Coordinate * Frequency) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return Value;
}

EBiomeType UBiomeManager::MapNoiseToBiome(float NoiseValue) const
{
    // Accumulate weights to create probability ranges
    float AccumulatedWeight = 0.0f;
    
    for (const auto& BiomeWeight : BiomeWeights)
    {
        AccumulatedWeight += BiomeWeight.Value;
        if (NoiseValue <= AccumulatedWeight)
        {
            return BiomeWeight.Key;
        }
    }
    
    // Fallback to dense jungle if something goes wrong
    return EBiomeType::DenseJungle;
}

float UBiomeManager::CalculateTemperatureAtLocation(const FVector& WorldLocation) const
{
    // Simple temperature calculation based on elevation and latitude
    float BaseTemperature = 25.0f; // Base temperature in Celsius
    float ElevationEffect = -WorldLocation.Z * 0.0065f; // Temperature drops with elevation
    float LatitudeEffect = FMath::Abs(WorldLocation.Y) * 0.00001f; // Cooler towards edges
    
    return BaseTemperature + ElevationEffect - LatitudeEffect;
}

float UBiomeManager::CalculateHumidityAtLocation(const FVector& WorldLocation) const
{
    // Simple humidity calculation based on distance from water and elevation
    float BaseHumidity = 60.0f; // Base humidity percentage
    float ElevationEffect = -WorldLocation.Z * 0.01f; // Humidity drops with elevation
    
    // This would ideally calculate distance to nearest water body
    float WaterDistanceEffect = 0.0f; // Placeholder
    
    return FMath::Clamp(BaseHumidity + ElevationEffect + WaterDistanceEffect, 0.0f, 100.0f);
}

UBiomeData* UBiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const auto* FoundBiomeData = BiomeDataAssets.Find(BiomeType))
    {
        return *FoundBiomeData;
    }
    
    return nullptr;
}

TArray<EBiomeType> UBiomeManager::GetAvailableBiomes() const
{
    TArray<EBiomeType> AvailableBiomes;
    BiomeDataAssets.GetKeys(AvailableBiomes);
    return AvailableBiomes;
}

float UBiomeManager::GetBiomeTransitionWeight(const FVector& WorldLocation, EBiomeType BiomeType) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    
    if (CurrentBiome == BiomeType)
    {
        return 1.0f;
    }
    
    // Calculate distance-based transition weight
    // This is a simplified implementation - in reality you'd check biome boundaries
    float TransitionWeight = 0.0f;
    
    // Check if biomes can blend together
    if (const UBiomeData* CurrentBiomeData = GetBiomeData(CurrentBiome))
    {
        if (CurrentBiomeData->CanBlendWith.Contains(BiomeType))
        {
            // Calculate smooth transition based on distance to biome boundary
            // This is a placeholder - real implementation would use proper boundary detection
            float DistanceToBoundary = 500.0f; // Placeholder
            TransitionWeight = FMath::Clamp(1.0f - (DistanceToBoundary / BiomeTransitionDistance), 0.0f, 1.0f);
        }
    }
    
    return TransitionWeight;
}

void UBiomeManager::SetBiomeWeight(EBiomeType BiomeType, float Weight)
{
    BiomeWeights.Add(BiomeType, FMath::Clamp(Weight, 0.0f, 1.0f));
    
    // Normalize all weights to ensure they sum to 1.0
    NormalizeBiomeWeights();
}

void UBiomeManager::NormalizeBiomeWeights()
{
    float TotalWeight = 0.0f;
    for (const auto& BiomeWeight : BiomeWeights)
    {
        TotalWeight += BiomeWeight.Value;
    }
    
    if (TotalWeight > 0.0f)
    {
        for (auto& BiomeWeight : BiomeWeights)
        {
            BiomeWeight.Value /= TotalWeight;
        }
    }
}