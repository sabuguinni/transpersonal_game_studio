#include "Arch_StructuralWeathering.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UArch_StructuralWeathering::UArch_StructuralWeathering()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    WeatheringData = FArch_WeatheringData();
    TimeAccelerationFactor = 1.0f;
    bEnableRealTimeWeathering = true;
    WeatheringMPC = nullptr;
    AccumulatedWeatheringTime = 0.0f;
}

void UArch_StructuralWeathering::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize dynamic materials
    AActor* Owner = GetOwner();
    if (Owner)
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp && MeshComp->GetMaterial(0))
            {
                UMaterialInstanceDynamic* DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
                if (DynMat)
                {
                    DynamicMaterials.Add(DynMat);
                }
            }
        }
    }
    
    UpdateMaterialParameters();
}

void UArch_StructuralWeathering::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableRealTimeWeathering && WeatheringData.StructuralIntegrity > 0.0f)
    {
        AccumulatedWeatheringTime += DeltaTime * TimeAccelerationFactor;
        
        if (AccumulatedWeatheringTime >= 60.0f) // Update every minute of game time
        {
            UpdateWeatheringEffects();
            AccumulatedWeatheringTime = 0.0f;
        }
    }
}

void UArch_StructuralWeathering::ApplyWeatheringLevel(EArch_WeatheringLevel NewLevel)
{
    WeatheringData.CurrentLevel = NewLevel;
    
    // Update structural integrity based on weathering level
    switch (NewLevel)
    {
        case EArch_WeatheringLevel::Pristine:
            WeatheringData.StructuralIntegrity = 100.0f;
            WeatheringData.MossGrowthFactor = 0.0f;
            break;
        case EArch_WeatheringLevel::LightWeathering:
            WeatheringData.StructuralIntegrity = 80.0f;
            WeatheringData.MossGrowthFactor = 0.2f;
            break;
        case EArch_WeatheringLevel::Moderate:
            WeatheringData.StructuralIntegrity = 60.0f;
            WeatheringData.MossGrowthFactor = 0.5f;
            break;
        case EArch_WeatheringLevel::Heavy:
            WeatheringData.StructuralIntegrity = 30.0f;
            WeatheringData.MossGrowthFactor = 0.8f;
            break;
        case EArch_WeatheringLevel::Collapsed:
            WeatheringData.StructuralIntegrity = 0.0f;
            WeatheringData.MossGrowthFactor = 1.0f;
            break;
    }
    
    UpdateMaterialParameters();
}

void UArch_StructuralWeathering::AccelerateWeathering(float AccelerationMultiplier)
{
    TimeAccelerationFactor = FMath::Clamp(AccelerationMultiplier, 0.1f, 100.0f);
}

void UArch_StructuralWeathering::ApplySeismicDamage(float DamageAmount)
{
    float MaterialResistance = 1.0f;
    
    // Different materials have different seismic resistance
    switch (WeatheringData.MaterialType)
    {
        case EArch_MaterialType::Stone:
            MaterialResistance = 0.8f; // Stone is more resistant
            break;
        case EArch_MaterialType::Wood:
            MaterialResistance = 1.2f; // Wood is flexible but can break
            break;
        case EArch_MaterialType::Bone:
            MaterialResistance = 1.0f; // Moderate resistance
            break;
        case EArch_MaterialType::Clay:
            MaterialResistance = 1.5f; // Clay is brittle
            break;
        case EArch_MaterialType::Hide:
            MaterialResistance = 0.5f; // Hide is very flexible
            break;
    }
    
    float ActualDamage = DamageAmount * MaterialResistance;
    WeatheringData.StructuralIntegrity = FMath::Clamp(WeatheringData.StructuralIntegrity - ActualDamage, 0.0f, 100.0f);
    
    CheckStructuralCollapse();
    UpdateMaterialParameters();
}

void UArch_StructuralWeathering::GrowMoss(float GrowthRate)
{
    if (WeatheringData.bIsExposedToElements)
    {
        WeatheringData.MossGrowthFactor = FMath::Clamp(WeatheringData.MossGrowthFactor + GrowthRate, 0.0f, 1.0f);
        UpdateMaterialParameters();
    }
}

void UArch_StructuralWeathering::UpdateWeatheringEffects()
{
    float WeatheringRate = CalculateWeatheringRate();
    
    // Apply gradual weathering
    WeatheringData.StructuralIntegrity = FMath::Clamp(WeatheringData.StructuralIntegrity - WeatheringRate, 0.0f, 100.0f);
    
    // Update weathering level based on integrity
    if (WeatheringData.StructuralIntegrity > 80.0f)
        WeatheringData.CurrentLevel = EArch_WeatheringLevel::Pristine;
    else if (WeatheringData.StructuralIntegrity > 60.0f)
        WeatheringData.CurrentLevel = EArch_WeatheringLevel::LightWeathering;
    else if (WeatheringData.StructuralIntegrity > 30.0f)
        WeatheringData.CurrentLevel = EArch_WeatheringLevel::Moderate;
    else if (WeatheringData.StructuralIntegrity > 0.0f)
        WeatheringData.CurrentLevel = EArch_WeatheringLevel::Heavy;
    else
        WeatheringData.CurrentLevel = EArch_WeatheringLevel::Collapsed;
    
    // Moss grows over time on exposed structures
    if (WeatheringData.bIsExposedToElements && WeatheringData.CurrentLevel != EArch_WeatheringLevel::Pristine)
    {
        GrowMoss(0.01f);
    }
    
    CheckStructuralCollapse();
    UpdateMaterialParameters();
}

void UArch_StructuralWeathering::UpdateMaterialParameters()
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            // Update weathering parameters
            DynMat->SetScalarParameterValue(TEXT("WeatheringAmount"), 1.0f - (WeatheringData.StructuralIntegrity / 100.0f));
            DynMat->SetScalarParameterValue(TEXT("MossGrowth"), WeatheringData.MossGrowthFactor);
            DynMat->SetScalarParameterValue(TEXT("StructuralIntegrity"), WeatheringData.StructuralIntegrity / 100.0f);
            
            // Material-specific parameters
            switch (WeatheringData.MaterialType)
            {
                case EArch_MaterialType::Stone:
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
                    break;
                case EArch_MaterialType::Wood:
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.3f, 0.2f, 0.1f, 1.0f));
                    break;
                case EArch_MaterialType::Bone:
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.8f, 0.7f, 0.6f, 1.0f));
                    break;
                case EArch_MaterialType::Clay:
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.5f, 0.3f, 0.2f, 1.0f));
                    break;
                case EArch_MaterialType::Hide:
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
                    break;
            }
        }
    }
    
    // Update global material parameter collection if available
    if (WeatheringMPC)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            UMaterialParameterCollectionInstance* MPCInstance = World->GetParameterCollectionInstance(WeatheringMPC);
            if (MPCInstance)
            {
                MPCInstance->SetScalarParameterValue(TEXT("GlobalWeatheringFactor"), WeatheringData.StructuralIntegrity / 100.0f);
                MPCInstance->SetScalarParameterValue(TEXT("GlobalMossGrowth"), WeatheringData.MossGrowthFactor);
            }
        }
    }
}

float UArch_StructuralWeathering::CalculateWeatheringRate() const
{
    float BaseRate = WeatheringData.WeatheringRate;
    
    // Material-specific weathering rates
    switch (WeatheringData.MaterialType)
    {
        case EArch_MaterialType::Stone:
            BaseRate *= 0.5f; // Stone weathers slowly
            break;
        case EArch_MaterialType::Wood:
            BaseRate *= 2.0f; // Wood weathers faster
            break;
        case EArch_MaterialType::Bone:
            BaseRate *= 1.5f; // Bone weathers moderately fast
            break;
        case EArch_MaterialType::Clay:
            BaseRate *= 3.0f; // Clay is very susceptible to weathering
            break;
        case EArch_MaterialType::Hide:
            BaseRate *= 4.0f; // Hide degrades quickly
            break;
    }
    
    // Exposure factor
    if (!WeatheringData.bIsExposedToElements)
    {
        BaseRate *= 0.1f; // Protected structures weather much slower
    }
    
    return BaseRate;
}

void UArch_StructuralWeathering::CheckStructuralCollapse()
{
    if (WeatheringData.StructuralIntegrity <= 0.0f && WeatheringData.CurrentLevel != EArch_WeatheringLevel::Collapsed)
    {
        WeatheringData.CurrentLevel = EArch_WeatheringLevel::Collapsed;
        
        // Trigger collapse effects
        AActor* Owner = GetOwner();
        if (Owner)
        {
            // Could trigger particle effects, sound, physics simulation, etc.
            UE_LOG(LogTemp, Warning, TEXT("Structure %s has collapsed due to weathering!"), *Owner->GetName());
        }
    }
}