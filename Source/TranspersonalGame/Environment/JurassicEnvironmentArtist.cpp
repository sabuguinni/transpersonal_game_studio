#include "JurassicEnvironmentArtist.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

AJurassicEnvironmentArtist::AJurassicEnvironmentArtist()
{
    PrimaryActorTick.bCanEverTick = false;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;
    
    // Initialize foliage component for procedural vegetation
    FoliageComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("FoliageComponent"));
    
    // Initialize material system component
    MaterialSystemComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MaterialSystem"));
    MaterialSystemComponent->SetupAttachment(RootComponent);
    
    // Initialize prop system component
    PropSystemComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PropSystem"));
    PropSystemComponent->SetupAttachment(RootComponent);
    
    // Initialize narrative system component
    NarrativeSystemComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NarrativeSystem"));
    NarrativeSystemComponent->SetupAttachment(RootComponent);
    
    // Default settings
    VegetationDensityMultiplier = 1.0f;
    PropDensityMultiplier = 1.0f;
    MaterialBlendRadius = 500.0f;
    NarrativeElementDensity = 0.1f;
    WindStrength = 0.5f;
    SeasonalVariation = 0.0f;
    
    // Performance settings
    MaxRenderDistance = 10000.0f;
    LODDistanceMultiplier = 1.0f;
    CullingEnabled = true;
    
    // Initialize biome-specific settings
    InitializeBiomeVegetationSettings();
    InitializeBiomeMaterialSettings();
    InitializeBiomeNarrativeSettings();
}

void AJurassicEnvironmentArtist::BeginPlay()
{
    Super::BeginPlay();
    
    // Find biome manager in the world
    BiomeManager = Cast<AJurassicBiomeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AJurassicBiomeManager::StaticClass()));
    
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicEnvironmentArtist: No BiomeManager found in world"));
        return;
    }
    
    // Setup environment art systems
    SetupVegetationSystem();
    SetupMaterialSystem();
    SetupPropSystem();
    SetupNarrativeSystem();
    
    // Generate initial environment art
    GenerateEnvironmentArt();
}

void AJurassicEnvironmentArtist::InitializeBiomeVegetationSettings()
{
    BiomeVegetationSettings.Empty();
    
    // Dense Forest - Lush Jurassic vegetation
    FJurassicVegetationSettings DenseForestSettings;
    DenseForestSettings.TreeDensity = 0.8f;
    DenseForestSettings.UndergrowthDensity = 0.9f;
    DenseForestSettings.FernDensity = 0.7f;
    DenseForestSettings.MossDensity = 0.6f;
    DenseForestSettings.FlowerDensity = 0.3f;
    DenseForestSettings.DeadVegetationDensity = 0.1f;
    DenseForestSettings.VegetationScale = FVector2D(0.8f, 1.2f);
    DenseForestSettings.VegetationVariation = 0.4f;
    BiomeVegetationSettings.Add(EJurassicBiomeType::DenseForest, DenseForestSettings);
    
    // River Valley - Riparian vegetation
    FJurassicVegetationSettings RiverValleySettings;
    RiverValleySettings.TreeDensity = 0.4f;
    RiverValleySettings.UndergrowthDensity = 0.6f;
    RiverValleySettings.FernDensity = 0.8f;
    RiverValleySettings.MossDensity = 0.9f;
    RiverValleySettings.FlowerDensity = 0.5f;
    RiverValleySettings.DeadVegetationDensity = 0.2f;
    RiverValleySettings.VegetationScale = FVector2D(1.0f, 1.4f);
    RiverValleySettings.VegetationVariation = 0.3f;
    BiomeVegetationSettings.Add(EJurassicBiomeType::RiverValley, RiverValleySettings);
    
    // Open Plains - Sparse vegetation
    FJurassicVegetationSettings OpenPlainsSettings;
    OpenPlainsSettings.TreeDensity = 0.1f;
    OpenPlainsSettings.UndergrowthDensity = 0.3f;
    OpenPlainsSettings.FernDensity = 0.2f;
    OpenPlainsSettings.MossDensity = 0.1f;
    OpenPlainsSettings.FlowerDensity = 0.4f;
    OpenPlainsSettings.DeadVegetationDensity = 0.3f;
    OpenPlainsSettings.VegetationScale = FVector2D(0.6f, 1.0f);
    OpenPlainsSettings.VegetationVariation = 0.6f;
    BiomeVegetationSettings.Add(EJurassicBiomeType::OpenPlains, OpenPlainsSettings);
    
    // Rocky Outcrops - Hardy vegetation
    FJurassicVegetationSettings RockyOutcropsSettings;
    RockyOutcropsSettings.TreeDensity = 0.2f;
    RockyOutcropsSettings.UndergrowthDensity = 0.1f;
    RockyOutcropsSettings.FernDensity = 0.3f;
    RockyOutcropsSettings.MossDensity = 0.4f;
    RockyOutcropsSettings.FlowerDensity = 0.1f;
    RockyOutcropsSettings.DeadVegetationDensity = 0.4f;
    RockyOutcropsSettings.VegetationScale = FVector2D(0.5f, 0.8f);
    RockyOutcropsSettings.VegetationVariation = 0.8f;
    BiomeVegetationSettings.Add(EJurassicBiomeType::RockyOutcrops, RockyOutcropsSettings);
    
    // Swamp Lands - Wetland vegetation
    FJurassicVegetationSettings SwampLandsSettings;
    SwampLandsSettings.TreeDensity = 0.6f;
    SwampLandsSettings.UndergrowthDensity = 0.8f;
    SwampLandsSettings.FernDensity = 0.9f;
    SwampLandsSettings.MossDensity = 0.95f;
    SwampLandsSettings.FlowerDensity = 0.2f;
    SwampLandsSettings.DeadVegetationDensity = 0.5f;
    SwampLandsSettings.VegetationScale = FVector2D(1.2f, 1.6f);
    SwampLandsSettings.VegetationVariation = 0.2f;
    BiomeVegetationSettings.Add(EJurassicBiomeType::SwampLands, SwampLandsSettings);
    
    // Continue for other biomes...
}

void AJurassicEnvironmentArtist::InitializeBiomeMaterialSettings()
{
    BiomeMaterialSettings.Empty();
    
    // Dense Forest - Rich, dark soils
    FJurassicMaterialSettings DenseForestMaterials;
    DenseForestMaterials.PrimaryMaterialWeight = 0.7f; // Dark forest soil
    DenseForestMaterials.SecondaryMaterialWeight = 0.2f; // Leaf litter
    DenseForestMaterials.TertiaryMaterialWeight = 0.1f; // Moss patches
    DenseForestMaterials.MaterialRoughness = 0.8f;
    DenseForestMaterials.MaterialMoisture = 0.9f;
    DenseForestMaterials.MaterialColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
    BiomeMaterialSettings.Add(EJurassicBiomeType::DenseForest, DenseForestMaterials);
    
    // River Valley - Sedimentary deposits
    FJurassicMaterialSettings RiverValleyMaterials;
    RiverValleyMaterials.PrimaryMaterialWeight = 0.5f; // River sediment
    RiverValleyMaterials.SecondaryMaterialWeight = 0.3f; // Sandy soil
    RiverValleyMaterials.TertiaryMaterialWeight = 0.2f; // Rocky patches
    RiverValleyMaterials.MaterialRoughness = 0.6f;
    RiverValleyMaterials.MaterialMoisture = 0.8f;
    RiverValleyMaterials.MaterialColor = FLinearColor(0.4f, 0.35f, 0.25f, 1.0f);
    BiomeMaterialSettings.Add(EJurassicBiomeType::RiverValley, RiverValleyMaterials);
    
    // Continue for other biomes...
}

void AJurassicEnvironmentArtist::InitializeBiomeNarrativeSettings()
{
    BiomeNarrativeSettings.Empty();
    
    // Dense Forest - Signs of large herbivore activity
    FJurassicNarrativeSettings DenseForestNarrative;
    DenseForestNarrative.DinosaurTrackDensity = 0.3f;
    DenseForestNarrative.NestedAreaDensity = 0.1f;
    DenseForestNarrative.FeedingSignDensity = 0.4f;
    DenseForestNarrative.BoneScatterDensity = 0.05f;
    DenseForestNarrative.TerritoralMarkingDensity = 0.2f;
    DenseForestNarrative.WeatheringIntensity = 0.6f;
    BiomeNarrativeSettings.Add(EJurassicBiomeType::DenseForest, DenseForestNarrative);
    
    // River Valley - Water-related activity
    FJurassicNarrativeSettings RiverValleyNarrative;
    RiverValleyNarrative.DinosaurTrackDensity = 0.6f; // High traffic area
    RiverValleyNarrative.NestedAreaDensity = 0.2f;
    RiverValleyNarrative.FeedingSignDensity = 0.3f;
    RiverValleyNarrative.BoneScatterDensity = 0.1f;
    RiverValleyNarrative.TerritoralMarkingDensity = 0.4f;
    RiverValleyNarrative.WeatheringIntensity = 0.8f; // High moisture
    BiomeNarrativeSettings.Add(EJurassicBiomeType::RiverValley, RiverValleyNarrative);
    
    // Continue for other biomes...
}

void AJurassicEnvironmentArtist::SetupVegetationSystem()
{
    if (!FoliageComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicEnvironmentArtist: FoliageComponent not found"));
        return;
    }
    
    // Setup PCG graph for vegetation generation
    if (VegetationPCGGraph)
    {
        FoliageComponent->SetGraph(VegetationPCGGraph);
    }
    
    // Configure vegetation parameters
    ConfigureVegetationParameters();
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Vegetation system setup complete"));
}

void AJurassicEnvironmentArtist::SetupMaterialSystem()
{
    // Find landscape in the world
    ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(GetWorld(), ALandscape::StaticClass()));
    
    if (!Landscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicEnvironmentArtist: No Landscape found in world"));
        return;
    }
    
    // Setup dynamic material blending
    SetupLandscapeMaterialBlending(Landscape);
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Material system setup complete"));
}

void AJurassicEnvironmentArtist::SetupPropSystem()
{
    // Initialize prop placement system
    PropPlacementSeeds.Empty();
    
    // Generate seeds for prop placement
    for (int32 i = 0; i < 1000; ++i)
    {
        FVector2D Seed = FVector2D(
            FMath::RandRange(-WorldBounds.X * 0.5f, WorldBounds.X * 0.5f),
            FMath::RandRange(-WorldBounds.Y * 0.5f, WorldBounds.Y * 0.5f)
        );
        PropPlacementSeeds.Add(Seed);
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Prop system setup complete"));
}

void AJurassicEnvironmentArtist::SetupNarrativeSystem()
{
    // Initialize narrative element placement
    NarrativeElementSeeds.Empty();
    
    // Generate seeds for narrative elements
    for (int32 i = 0; i < 500; ++i)
    {
        FVector2D Seed = FVector2D(
            FMath::RandRange(-WorldBounds.X * 0.5f, WorldBounds.X * 0.5f),
            FMath::RandRange(-WorldBounds.Y * 0.5f, WorldBounds.Y * 0.5f)
        );
        NarrativeElementSeeds.Add(Seed);
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Narrative system setup complete"));
}

void AJurassicEnvironmentArtist::GenerateEnvironmentArt()
{
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicEnvironmentArtist: Cannot generate without BiomeManager"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Starting environment art generation..."));
    
    // Generate vegetation
    GenerateVegetation();
    
    // Apply material blending
    ApplyMaterialBlending();
    
    // Place environmental props
    PlaceEnvironmentalProps();
    
    // Add narrative elements
    AddNarrativeElements();
    
    // Apply weathering and aging
    ApplyWeatheringEffects();
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Environment art generation complete"));
}

void AJurassicEnvironmentArtist::GenerateVegetation()
{
    if (!FoliageComponent || !BiomeManager)
    {
        return;
    }
    
    // Trigger PCG vegetation generation
    FoliageComponent->Generate();
    
    // Apply biome-specific vegetation modifications
    ApplyBiomeSpecificVegetation();
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Vegetation generation complete"));
}

void AJurassicEnvironmentArtist::ApplyBiomeSpecificVegetation()
{
    // This would be implemented with specific PCG logic
    // For now, we log the process
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Applying biome-specific vegetation modifications"));
}

void AJurassicEnvironmentArtist::ApplyMaterialBlending()
{
    // Apply dynamic material blending based on biomes
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Applying material blending"));
}

void AJurassicEnvironmentArtist::PlaceEnvironmentalProps()
{
    // Place rocks, fallen logs, debris, etc.
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Placing environmental props"));
}

void AJurassicEnvironmentArtist::AddNarrativeElements()
{
    // Add dinosaur tracks, feeding signs, territorial markings
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Adding narrative elements"));
}

void AJurassicEnvironmentArtist::ApplyWeatheringEffects()
{
    // Apply aging, wear, and environmental storytelling
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Applying weathering effects"));
}

void AJurassicEnvironmentArtist::ConfigureVegetationParameters()
{
    // Configure PCG parameters based on current settings
    if (FoliageComponent && FoliageComponent->GetGraph())
    {
        // Set global vegetation parameters
        // This would be implemented with PCG parameter overrides
        UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Vegetation parameters configured"));
    }
}

void AJurassicEnvironmentArtist::SetupLandscapeMaterialBlending(ALandscape* Landscape)
{
    if (!Landscape)
    {
        return;
    }
    
    // Setup dynamic material instance for landscape
    if (LandscapeMasterMaterial)
    {
        LandscapeDynamicMaterial = UMaterialInstanceDynamic::Create(LandscapeMasterMaterial, this);
        
        if (LandscapeDynamicMaterial)
        {
            // Set initial material parameters
            LandscapeDynamicMaterial->SetScalarParameterValue(TEXT("GlobalMoisture"), 0.65f);
            LandscapeDynamicMaterial->SetScalarParameterValue(TEXT("GlobalTemperature"), 0.75f);
            LandscapeDynamicMaterial->SetScalarParameterValue(TEXT("SeasonalVariation"), SeasonalVariation);
            LandscapeDynamicMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), 0.5f);
            
            // Apply to landscape
            Landscape->GetLandscapeMaterial() = LandscapeDynamicMaterial;
        }
    }
}

FJurassicVegetationSettings AJurassicEnvironmentArtist::GetVegetationSettingsForBiome(EJurassicBiomeType BiomeType) const
{
    if (const FJurassicVegetationSettings* Settings = BiomeVegetationSettings.Find(BiomeType))
    {
        return *Settings;
    }
    
    // Return default settings if biome not found
    FJurassicVegetationSettings DefaultSettings;
    DefaultSettings.TreeDensity = 0.3f;
    DefaultSettings.UndergrowthDensity = 0.4f;
    DefaultSettings.FernDensity = 0.3f;
    DefaultSettings.MossDensity = 0.2f;
    DefaultSettings.FlowerDensity = 0.2f;
    DefaultSettings.DeadVegetationDensity = 0.1f;
    DefaultSettings.VegetationScale = FVector2D(0.8f, 1.2f);
    DefaultSettings.VegetationVariation = 0.5f;
    
    return DefaultSettings;
}

FJurassicMaterialSettings AJurassicEnvironmentArtist::GetMaterialSettingsForBiome(EJurassicBiomeType BiomeType) const
{
    if (const FJurassicMaterialSettings* Settings = BiomeMaterialSettings.Find(BiomeType))
    {
        return *Settings;
    }
    
    // Return default settings if biome not found
    FJurassicMaterialSettings DefaultSettings;
    DefaultSettings.PrimaryMaterialWeight = 0.6f;
    DefaultSettings.SecondaryMaterialWeight = 0.3f;
    DefaultSettings.TertiaryMaterialWeight = 0.1f;
    DefaultSettings.MaterialRoughness = 0.7f;
    DefaultSettings.MaterialMoisture = 0.5f;
    DefaultSettings.MaterialColor = FLinearColor(0.3f, 0.25f, 0.2f, 1.0f);
    
    return DefaultSettings;
}

FJurassicNarrativeSettings AJurassicEnvironmentArtist::GetNarrativeSettingsForBiome(EJurassicBiomeType BiomeType) const
{
    if (const FJurassicNarrativeSettings* Settings = BiomeNarrativeSettings.Find(BiomeType))
    {
        return *Settings;
    }
    
    // Return default settings if biome not found
    FJurassicNarrativeSettings DefaultSettings;
    DefaultSettings.DinosaurTrackDensity = 0.2f;
    DefaultSettings.NestedAreaDensity = 0.05f;
    DefaultSettings.FeedingSignDensity = 0.1f;
    DefaultSettings.BoneScatterDensity = 0.02f;
    DefaultSettings.TerritoralMarkingDensity = 0.1f;
    DefaultSettings.WeatheringIntensity = 0.5f;
    
    return DefaultSettings;
}

void AJurassicEnvironmentArtist::RefreshEnvironmentArt()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Refreshing environment art..."));
    
    // Clear existing generation
    ClearEnvironmentArt();
    
    // Regenerate everything
    GenerateEnvironmentArt();
}

void AJurassicEnvironmentArtist::ClearEnvironmentArt()
{
    if (FoliageComponent)
    {
        FoliageComponent->CleanupLocalImmediate(true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Environment art cleared"));
}

void AJurassicEnvironmentArtist::SetVegetationDensity(float NewDensity)
{
    VegetationDensityMultiplier = FMath::Clamp(NewDensity, 0.0f, 2.0f);
    
    // Update PCG parameters
    ConfigureVegetationParameters();
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Vegetation density set to %f"), VegetationDensityMultiplier);
}

void AJurassicEnvironmentArtist::SetSeasonalVariation(float NewVariation)
{
    SeasonalVariation = FMath::Clamp(NewVariation, 0.0f, 1.0f);
    
    // Update material parameters
    if (LandscapeDynamicMaterial)
    {
        LandscapeDynamicMaterial->SetScalarParameterValue(TEXT("SeasonalVariation"), SeasonalVariation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Seasonal variation set to %f"), SeasonalVariation);
}

void AJurassicEnvironmentArtist::SetWindStrength(float NewWindStrength)
{
    WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 2.0f);
    
    // Update wind parameters for vegetation
    if (LandscapeDynamicMaterial)
    {
        LandscapeDynamicMaterial->SetScalarParameterValue(TEXT("WindStrength"), WindStrength);
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicEnvironmentArtist: Wind strength set to %f"), WindStrength);
}

bool AJurassicEnvironmentArtist::IsLocationSuitableForVegetation(FVector Location, EJurassicBiomeType BiomeType) const
{
    if (!BiomeManager)
    {
        return false;
    }
    
    // Get biome data at location
    FJurassicBiomeData BiomeData = BiomeManager->GetBiomeDataAtLocation(Location);
    
    // Check if location matches the requested biome type
    if (BiomeData.BiomeType != BiomeType)
    {
        return false;
    }
    
    // Additional suitability checks could be added here
    // (slope, moisture, temperature, etc.)
    
    return true;
}

float AJurassicEnvironmentArtist::CalculateVegetationDensityAtLocation(FVector Location) const
{
    if (!BiomeManager)
    {
        return 0.0f;
    }
    
    // Get biome type at location
    EJurassicBiomeType BiomeType = BiomeManager->GetBiomeAtLocation(Location);
    
    // Get vegetation settings for this biome
    FJurassicVegetationSettings VegSettings = GetVegetationSettingsForBiome(BiomeType);
    
    // Calculate base density from biome settings
    float BaseDensity = (VegSettings.TreeDensity + VegSettings.UndergrowthDensity + VegSettings.FernDensity) / 3.0f;
    
    // Apply global multiplier
    float FinalDensity = BaseDensity * VegetationDensityMultiplier;
    
    return FMath::Clamp(FinalDensity, 0.0f, 1.0f);
}