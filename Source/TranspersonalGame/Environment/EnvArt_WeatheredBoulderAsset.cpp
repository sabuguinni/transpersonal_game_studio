#include "EnvArt_WeatheredBoulderAsset.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"

AEnvArt_WeatheredBoulderAsset::AEnvArt_WeatheredBoulderAsset()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    WeatheringState = EEnvArt_WeatheringState::Moderate;
    BoulderSize = EEnvArt_BoulderSize::Medium;
    RockType = EEnvArt_RockType::Sandstone;
    MossCoverage = 0.3f;
    LichenIntensity = 0.2f;
    BoulderAge = 1000;
    
    bHasAncientMarkings = false;
    bIsAnimalTerritory = false;
    bProvidesShel ter = true;
    NarrativeContext = TEXT("Ancient boulder from Cretaceous period");
    
    CurrentMeshVariant = 0;
    bIsClimbable = true;
    bCanHarvestMaterials = true;
    StoneYield = 15;
    
    OrganicGrowthTimer = 0.0f;
    CachedHumidity = 0.6f;
    CachedTemperature = 22.0f;
}

void AEnvArt_WeatheredBoulderAsset::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize boulder with random properties if not manually configured
    if (BoulderAge <= 0)
    {
        InitializeRandomBoulder();
    }
    
    // Apply initial weathering state
    ApplyWeatheringState();
    
    // Set initial size
    SetBoulderSize(BoulderSize);
    
    // Apply rock type properties
    ApplyRockTypeProperties();
}

void AEnvArt_WeatheredBoulderAsset::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update organic growth periodically
    OrganicGrowthTimer += DeltaTime;
    if (OrganicGrowthTimer >= 30.0f) // Update every 30 seconds
    {
        UpdateOrganicGrowth(CachedHumidity, CachedTemperature, 1.0f);
        OrganicGrowthTimer = 0.0f;
    }
}

void AEnvArt_WeatheredBoulderAsset::InitializeRandomBoulder()
{
    // Randomize weathering state
    int32 RandomWeathering = FMath::RandRange(0, 4);
    WeatheringState = static_cast<EEnvArt_WeatheringState>(RandomWeathering);
    
    // Randomize size
    int32 RandomSize = FMath::RandRange(0, 4);
    BoulderSize = static_cast<EEnvArt_BoulderSize>(RandomSize);
    
    // Randomize rock type
    int32 RandomRock = FMath::RandRange(0, 5);
    RockType = static_cast<EEnvArt_RockType>(RandomRock);
    
    // Randomize organic growth
    MossCoverage = FMath::FRandRange(0.0f, 0.8f);
    LichenIntensity = FMath::FRandRange(0.0f, 0.6f);
    
    // Randomize age (affects weathering)
    BoulderAge = FMath::RandRange(100, 5000);
    
    // Randomize storytelling elements
    bHasAncientMarkings = FMath::RandBool();
    bIsAnimalTerritory = FMath::RandBool();
    bProvidesShel ter = FMath::RandRange(0, 100) < 70; // 70% chance
    
    // Generate narrative context
    TArray<FString> NarrativeOptions = {
        TEXT("Weathered by countless seasons"),
        TEXT("Marked by ancient predators"),
        TEXT("Shelter for small creatures"),
        TEXT("Witness to prehistoric migrations"),
        TEXT("Carved by primordial floods"),
        TEXT("Resting place of ancient spirits")
    };
    
    int32 RandomNarrative = FMath::RandRange(0, NarrativeOptions.Num() - 1);
    NarrativeContext = NarrativeOptions[RandomNarrative];
    
    // Select random mesh variant
    SelectRandomMeshVariant();
}

void AEnvArt_WeatheredBoulderAsset::ApplyWeatheringState()
{
    UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Create dynamic material instance
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;
    
    switch (WeatheringState)
    {
        case EEnvArt_WeatheringState::Fresh:
            if (BaseMaterial)
            {
                DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.1f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.3f);
            }
            break;
            
        case EEnvArt_WeatheringState::Light:
            if (BaseMaterial)
            {
                DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.3f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.5f);
            }
            break;
            
        case EEnvArt_WeatheringState::Moderate:
            if (WeatheredMaterial)
            {
                DynamicMaterial = UMaterialInstanceDynamic::Create(WeatheredMaterial, this);
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.6f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.7f);
            }
            break;
            
        case EEnvArt_WeatheringState::Heavy:
            if (WeatheredMaterial)
            {
                DynamicMaterial = UMaterialInstanceDynamic::Create(WeatheredMaterial, this);
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 0.8f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.9f);
                DynamicMaterial->SetScalarParameterValue(TEXT("CrackIntensity"), 0.6f);
            }
            break;
            
        case EEnvArt_WeatheringState::Ancient:
            if (WeatheredMaterial)
            {
                DynamicMaterial = UMaterialInstanceDynamic::Create(WeatheredMaterial, this);
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), 1.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 1.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("CrackIntensity"), 1.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("ErosionDepth"), 0.8f);
            }
            break;
    }
    
    if (DynamicMaterial)
    {
        // Apply moss and lichen overlays
        DynamicMaterial->SetScalarParameterValue(TEXT("MossCoverage"), MossCoverage);
        DynamicMaterial->SetScalarParameterValue(TEXT("LichenIntensity"), LichenIntensity);
        
        MeshComp->SetMaterial(0, DynamicMaterial);
    }
    
    BlendMaterials();
}

void AEnvArt_WeatheredBoulderAsset::UpdateOrganicGrowth(float Humidity, float Temperature, float TimeMultiplier)
{
    CachedHumidity = Humidity;
    CachedTemperature = Temperature;
    
    // Moss grows better in humid conditions
    if (Humidity > 0.7f)
    {
        MossCoverage = FMath::Clamp(MossCoverage + (0.01f * TimeMultiplier), 0.0f, 1.0f);
    }
    else if (Humidity < 0.3f)
    {
        MossCoverage = FMath::Clamp(MossCoverage - (0.005f * TimeMultiplier), 0.0f, 1.0f);
    }
    
    // Lichen grows in moderate conditions
    if (Temperature > 15.0f && Temperature < 25.0f && Humidity > 0.4f)
    {
        LichenIntensity = FMath::Clamp(LichenIntensity + (0.008f * TimeMultiplier), 0.0f, 1.0f);
    }
    
    // Reapply weathering with updated organic growth
    ApplyWeatheringState();
}

void AEnvArt_WeatheredBoulderAsset::SetBoulderSize(EEnvArt_BoulderSize NewSize)
{
    BoulderSize = NewSize;
    
    FVector Scale = FVector(1.0f);
    
    switch (BoulderSize)
    {
        case EEnvArt_BoulderSize::Tiny:
            Scale = FVector(0.3f, 0.3f, 0.3f);
            StoneYield = 3;
            bIsClimbable = false;
            break;
            
        case EEnvArt_BoulderSize::Small:
            Scale = FVector(0.6f, 0.6f, 0.6f);
            StoneYield = 8;
            bIsClimbable = false;
            break;
            
        case EEnvArt_BoulderSize::Medium:
            Scale = FVector(1.0f, 1.0f, 1.0f);
            StoneYield = 15;
            bIsClimbable = true;
            break;
            
        case EEnvArt_BoulderSize::Large:
            Scale = FVector(1.5f, 1.5f, 1.5f);
            StoneYield = 25;
            bIsClimbable = true;
            bProvidesShel ter = true;
            break;
            
        case EEnvArt_BoulderSize::Massive:
            Scale = FVector(2.2f, 2.2f, 2.2f);
            StoneYield = 40;
            bIsClimbable = true;
            bProvidesShel ter = true;
            break;
    }
    
    SetActorScale3D(Scale);
}

void AEnvArt_WeatheredBoulderAsset::ApplyRockTypeProperties()
{
    switch (RockType)
    {
        case EEnvArt_RockType::Granite:
            StoneYield = FMath::RoundToInt(StoneYield * 1.3f); // Harder rock, more yield
            MossCoverage *= 0.7f; // Less moss on harder surfaces
            break;
            
        case EEnvArt_RockType::Sandstone:
            StoneYield = FMath::RoundToInt(StoneYield * 0.9f); // Softer rock
            MossCoverage *= 1.2f; // More moss on softer surfaces
            break;
            
        case EEnvArt_RockType::Limestone:
            LichenIntensity *= 1.4f; // Lichen loves limestone
            break;
            
        case EEnvArt_RockType::Basalt:
            StoneYield = FMath::RoundToInt(StoneYield * 1.1f);
            break;
            
        case EEnvArt_RockType::Slate:
            bHasAncientMarkings = true; // Slate often has natural patterns
            break;
            
        case EEnvArt_RockType::Quartzite:
            StoneYield = FMath::RoundToInt(StoneYield * 1.5f); // Very hard rock
            MossCoverage *= 0.5f; // Very little organic growth
            LichenIntensity *= 0.6f;
            break;
    }
    
    // Clamp values
    MossCoverage = FMath::Clamp(MossCoverage, 0.0f, 1.0f);
    LichenIntensity = FMath::Clamp(LichenIntensity, 0.0f, 1.0f);
}

FString AEnvArt_WeatheredBoulderAsset::GetEnvironmentalStory() const
{
    FString Story = NarrativeContext;
    
    if (bHasAncientMarkings)
    {
        Story += TEXT(". Ancient claw marks tell of territorial disputes.");
    }
    
    if (bIsAnimalTerritory)
    {
        Story += TEXT(". Scent markers indicate active animal territory.");
    }
    
    if (bProvidesShel ter)
    {
        Story += TEXT(". Natural hollows provide refuge for small creatures.");
    }
    
    if (MossCoverage > 0.6f)
    {
        Story += TEXT(". Thick moss blanket indicates high humidity.");
    }
    
    if (LichenIntensity > 0.5f)
    {
        Story += TEXT(". Vibrant lichen suggests clean air and stable climate.");
    }
    
    return Story;
}

bool AEnvArt_WeatheredBoulderAsset::CanProvideFunction(EEnvArt_BoulderFunction Function) const
{
    switch (Function)
    {
        case EEnvArt_BoulderFunction::Climbing:
            return bIsClimbable && (BoulderSize >= EEnvArt_BoulderSize::Medium);
            
        case EEnvArt_BoulderFunction::Shelter:
            return bProvidesShel ter && (BoulderSize >= EEnvArt_BoulderSize::Large);
            
        case EEnvArt_BoulderFunction::MaterialHarvesting:
            return bCanHarvestMaterials && (StoneYield > 0);
            
        case EEnvArt_BoulderFunction::Landmark:
            return (BoulderSize >= EEnvArt_BoulderSize::Large) || bHasAncientMarkings;
            
        case EEnvArt_BoulderFunction::TerritoryMarker:
            return bIsAnimalTerritory;
            
        default:
            return false;
    }
}

void AEnvArt_WeatheredBoulderAsset::SelectRandomMeshVariant()
{
    if (BoulderMeshVariants.Num() > 0)
    {
        CurrentMeshVariant = FMath::RandRange(0, BoulderMeshVariants.Num() - 1);
        
        UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
        if (MeshComp && BoulderMeshVariants[CurrentMeshVariant])
        {
            MeshComp->SetStaticMesh(BoulderMeshVariants[CurrentMeshVariant]);
        }
    }
}

void AEnvArt_WeatheredBoulderAsset::CalculateWeatheringEffects()
{
    // Age affects weathering intensity
    float AgeMultiplier = FMath::Clamp(BoulderAge / 2000.0f, 0.1f, 2.0f);
    
    // Environmental factors
    float HumidityEffect = CachedHumidity * 0.5f;
    float TemperatureEffect = FMath::Abs(CachedTemperature - 20.0f) / 20.0f;
    
    // Calculate overall weathering intensity
    float WeatheringIntensity = (AgeMultiplier + HumidityEffect + TemperatureEffect) / 3.0f;
    
    // Update weathering state based on calculated intensity
    if (WeatheringIntensity < 0.2f)
    {
        WeatheringState = EEnvArt_WeatheringState::Fresh;
    }
    else if (WeatheringIntensity < 0.4f)
    {
        WeatheringState = EEnvArt_WeatheringState::Light;
    }
    else if (WeatheringIntensity < 0.6f)
    {
        WeatheringState = EEnvArt_WeatheringState::Moderate;
    }
    else if (WeatheringIntensity < 0.8f)
    {
        WeatheringState = EEnvArt_WeatheringState::Heavy;
    }
    else
    {
        WeatheringState = EEnvArt_WeatheringState::Ancient;
    }
}

void AEnvArt_WeatheredBoulderAsset::BlendMaterials()
{
    UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Get current material
    UMaterialInstanceDynamic* CurrentMaterial = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(0));
    if (!CurrentMaterial)
    {
        return;
    }
    
    // Blend moss material if coverage is significant
    if (MossCoverage > 0.3f && MossMaterial)
    {
        CurrentMaterial->SetScalarParameterValue(TEXT("MossBlend"), MossCoverage);
        CurrentMaterial->SetTextureParameterValue(TEXT("MossTexture"), 
            Cast<UTexture>(MossMaterial->GetDefaultObject()));
    }
    
    // Blend lichen material if intensity is significant
    if (LichenIntensity > 0.2f && LichenMaterial)
    {
        CurrentMaterial->SetScalarParameterValue(TEXT("LichenBlend"), LichenIntensity);
        CurrentMaterial->SetTextureParameterValue(TEXT("LichenTexture"), 
            Cast<UTexture>(LichenMaterial->GetDefaultObject()));
    }
}