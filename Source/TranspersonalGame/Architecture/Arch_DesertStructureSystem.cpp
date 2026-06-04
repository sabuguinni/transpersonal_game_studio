#include "Arch_DesertStructureSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

AArch_DesertStructureSystem::AArch_DesertStructureSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    PrimaryStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryStructureMesh"));
    PrimaryStructureMesh->SetupAttachment(RootComponent);

    SandAccumulationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SandAccumulationMesh"));
    SandAccumulationMesh->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentWeatheringLevel = 0.0f;
    CurrentSandLevel = 0.0f;
    bIsInitialized = false;

    // Set default structure data
    StructureData.StructureName = TEXT("Desert Ruins");
    StructureData.StructureType = EArch_StructureType::Ruins;
    StructureData.WeatheringLevel = 0.5f;
    StructureData.SandAccumulation = 0.3f;
    StructureData.bIsPartiallyBuried = true;
    StructureData.OriginalDimensions = FVector(500.0f, 500.0f, 300.0f);

    // Set default environmental factors
    EnvironmentalFactors.SandstormIntensity = 0.4f;
    EnvironmentalFactors.TemperatureVariation = 45.0f;
    EnvironmentalFactors.WindErosionRate = 0.15f;
    EnvironmentalFactors.bIsOasisNearby = false;
    EnvironmentalFactors.DistanceToWater = 15000.0f;
}

void AArch_DesertStructureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bIsInitialized)
    {
        InitializeDesertStructure(StructureData);
    }
}

void AArch_DesertStructureSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsInitialized)
    {
        UpdateSandAccumulation(DeltaTime);
        ProcessEnvironmentalFactors();
    }
}

void AArch_DesertStructureSystem::InitializeDesertStructure(const FArch_DesertStructureData& InStructureData)
{
    StructureData = InStructureData;
    CurrentWeatheringLevel = StructureData.WeatheringLevel;
    CurrentSandLevel = StructureData.SandAccumulation;
    
    // Apply initial weathering effects
    UpdateMeshBasedOnWeathering();
    ApplySandAccumulationToMesh();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("Desert Structure System initialized: %s"), *StructureData.StructureName);
}

void AArch_DesertStructureSystem::ApplyWeatheringEffects(float WeatheringIntensity)
{
    if (!bIsInitialized) return;
    
    CurrentWeatheringLevel = FMath::Clamp(CurrentWeatheringLevel + WeatheringIntensity, 0.0f, 1.0f);
    StructureData.WeatheringLevel = CurrentWeatheringLevel;
    
    UpdateMeshBasedOnWeathering();
    
    // Create dynamic material instance for weathering effects
    if (PrimaryStructureMesh && WeatheredStoneMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(WeatheredStoneMaterial, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), CurrentWeatheringLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("ErosionIntensity"), EnvironmentalFactors.WindErosionRate);
            PrimaryStructureMesh->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AArch_DesertStructureSystem::UpdateSandAccumulation(float DeltaTime)
{
    if (!bIsInitialized) return;
    
    // Gradually accumulate sand based on environmental factors
    float SandAccumulationRate = EnvironmentalFactors.SandstormIntensity * EnvironmentalFactors.WindErosionRate * DeltaTime * 0.001f;
    CurrentSandLevel = FMath::Clamp(CurrentSandLevel + SandAccumulationRate, 0.0f, 1.0f);
    StructureData.SandAccumulation = CurrentSandLevel;
    
    // Update sand mesh visibility and scale
    ApplySandAccumulationToMesh();
}

void AArch_DesertStructureSystem::ProcessEnvironmentalFactors()
{
    if (!bIsInitialized) return;
    
    // Process temperature effects on structure
    float TemperatureStress = EnvironmentalFactors.TemperatureVariation / 100.0f;
    ApplyWeatheringEffects(TemperatureStress * 0.0001f);
    
    // Check if structure should be partially buried
    if (CurrentSandLevel > 0.6f && !StructureData.bIsPartiallyBuried)
    {
        StructureData.bIsPartiallyBuried = true;
        UE_LOG(LogTemp, Warning, TEXT("Structure %s is now partially buried by sand"), *StructureData.StructureName);
    }
}

bool AArch_DesertStructureSystem::IsStructureAccessible() const
{
    if (!bIsInitialized) return false;
    
    // Structure is accessible if not too buried and structurally sound
    return CurrentSandLevel < 0.8f && CalculateStructuralIntegrity() > 0.3f;
}

FVector AArch_DesertStructureSystem::GetOptimalExplorationPoint() const
{
    if (!bIsInitialized) return GetActorLocation();
    
    // Calculate best entry point based on sand accumulation and structural integrity
    FVector BaseLocation = GetActorLocation();
    FVector Offset = FVector(0.0f, 0.0f, 0.0f);
    
    // Prefer higher ground if structure is partially buried
    if (StructureData.bIsPartiallyBuried)
    {
        Offset.Z += 200.0f * (1.0f - CurrentSandLevel);
    }
    
    return BaseLocation + Offset;
}

void AArch_DesertStructureSystem::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;
    
    // Update mesh based on new type
    UpdateMeshBasedOnWeathering();
}

float AArch_DesertStructureSystem::CalculateStructuralIntegrity() const
{
    if (!bIsInitialized) return 0.0f;
    
    // Calculate integrity based on weathering and environmental factors
    float BaseIntegrity = 1.0f - CurrentWeatheringLevel;
    float EnvironmentalDamage = (EnvironmentalFactors.SandstormIntensity + EnvironmentalFactors.WindErosionRate) * 0.5f;
    float SandSupport = CurrentSandLevel * 0.2f; // Sand can provide some structural support
    
    return FMath::Clamp(BaseIntegrity - EnvironmentalDamage + SandSupport, 0.0f, 1.0f);
}

void AArch_DesertStructureSystem::SimulateSandstormDamage(float StormIntensity)
{
    if (!bIsInitialized) return;
    
    // Apply immediate weathering from sandstorm
    float WeatheringDamage = StormIntensity * 0.1f;
    ApplyWeatheringEffects(WeatheringDamage);
    
    // Increase sand accumulation
    float SandIncrease = StormIntensity * 0.3f;
    CurrentSandLevel = FMath::Clamp(CurrentSandLevel + SandIncrease, 0.0f, 1.0f);
    StructureData.SandAccumulation = CurrentSandLevel;
    
    ApplySandAccumulationToMesh();
    
    UE_LOG(LogTemp, Warning, TEXT("Sandstorm hit structure %s - Integrity now: %f"), 
           *StructureData.StructureName, CalculateStructuralIntegrity());
}

TArray<FVector> AArch_DesertStructureSystem::GetShelterPoints() const
{
    TArray<FVector> ShelterPoints;
    
    if (!bIsInitialized || !IsStructureAccessible())
    {
        return ShelterPoints;
    }
    
    FVector BaseLocation = GetActorLocation();
    
    // Add shelter points based on structure type
    switch (StructureData.StructureType)
    {
        case EArch_StructureType::Ruins:
            ShelterPoints.Add(BaseLocation + FVector(100.0f, 0.0f, 0.0f));
            ShelterPoints.Add(BaseLocation + FVector(-100.0f, 0.0f, 0.0f));
            break;
            
        case EArch_StructureType::Temple:
            ShelterPoints.Add(BaseLocation); // Center of temple
            ShelterPoints.Add(BaseLocation + FVector(0.0f, 150.0f, 0.0f));
            ShelterPoints.Add(BaseLocation + FVector(0.0f, -150.0f, 0.0f));
            break;
            
        default:
            ShelterPoints.Add(BaseLocation);
            break;
    }
    
    return ShelterPoints;
}

void AArch_DesertStructureSystem::UpdateMeshBasedOnWeathering()
{
    if (!PrimaryStructureMesh) return;
    
    // Select appropriate mesh based on structure type and weathering level
    UStaticMesh* TargetMesh = nullptr;
    
    if (CurrentWeatheringLevel < 0.3f)
    {
        TargetMesh = DesertPillarMesh; // Well-preserved
    }
    else if (CurrentWeatheringLevel < 0.7f)
    {
        TargetMesh = DesertArchMesh; // Partially weathered
    }
    else
    {
        // Heavily weathered - reduce scale
        PrimaryStructureMesh->SetRelativeScale3D(FVector(1.0f - CurrentWeatheringLevel * 0.3f));
    }
    
    if (TargetMesh)
    {
        PrimaryStructureMesh->SetStaticMesh(TargetMesh);
    }
}

void AArch_DesertStructureSystem::ApplySandAccumulationToMesh()
{
    if (!SandAccumulationMesh) return;
    
    // Show/hide and scale sand mesh based on accumulation level
    bool bShouldShowSand = CurrentSandLevel > 0.1f;
    SandAccumulationMesh->SetVisibility(bShouldShowSand);
    
    if (bShouldShowSand)
    {
        float SandScale = CurrentSandLevel;
        SandAccumulationMesh->SetRelativeScale3D(FVector(SandScale, SandScale, SandScale * 0.5f));
        
        // Position sand mesh at base of structure
        FVector SandOffset = FVector(0.0f, 0.0f, -50.0f * CurrentSandLevel);
        SandAccumulationMesh->SetRelativeLocation(SandOffset);
        
        if (SandMaterial)
        {
            SandAccumulationMesh->SetMaterial(0, SandMaterial);
        }
    }
}

void AArch_DesertStructureSystem::CalculateOptimalSandPlacement()
{
    // Calculate where sand should naturally accumulate based on wind patterns and structure geometry
    // This could be expanded to use more complex physics simulation
}