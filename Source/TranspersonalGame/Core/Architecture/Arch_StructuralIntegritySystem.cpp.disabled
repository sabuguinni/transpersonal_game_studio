#include "Arch_StructuralIntegritySystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

UArch_StructuralIntegritySystem::UArch_StructuralIntegritySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance

    // Initialize default structural properties
    StructuralProperties = FArch_StructuralProperties();
    bAffectedByWeather = true;
    bAffectedBySeismic = false;
    MossGrowthRate = 0.05f;
    CurrentMossLevel = 0.0f;
}

void UArch_StructuralIntegritySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize weathering based on material type
    UpdateWeatheringLevel();
    
    UE_LOG(LogTemp, Log, TEXT("Structural Integrity System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UArch_StructuralIntegritySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAffectedByWeather)
    {
        UpdateWeathering(DeltaTime);
        UpdateMossGrowth(DeltaTime);
    }

    CheckForCollapse();
}

void UArch_StructuralIntegritySystem::ApplyDamage(float DamageAmount, bool bIsSeismic)
{
    if (DamageAmount <= 0.0f)
        return;

    // Apply material resistance
    float MaterialMultiplier = GetMaterialWeatheringMultiplier();
    float ActualDamage = DamageAmount * MaterialMultiplier;

    // Seismic damage is more severe for certain materials
    if (bIsSeismic && bAffectedBySeismic)
    {
        if (StructuralProperties.Material == EArch_StructuralMaterial::Stone)
        {
            ActualDamage *= 0.5f; // Stone resists seismic better
        }
        else if (StructuralProperties.Material == EArch_StructuralMaterial::Wood)
        {
            ActualDamage *= 1.5f; // Wood is more vulnerable to seismic
        }
    }

    StructuralProperties.CurrentIntegrity = FMath::Max(0.0f, 
        StructuralProperties.CurrentIntegrity - ActualDamage);

    UpdateWeatheringLevel();
    UpdateVisualState();

    UE_LOG(LogTemp, Warning, TEXT("Structure %s took %.1f damage, integrity now %.1f%%"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           ActualDamage, GetIntegrityPercentage());
}

void UArch_StructuralIntegritySystem::RepairStructure(float RepairAmount)
{
    if (RepairAmount <= 0.0f)
        return;

    StructuralProperties.CurrentIntegrity = FMath::Min(StructuralProperties.MaxIntegrity,
        StructuralProperties.CurrentIntegrity + RepairAmount);

    UpdateWeatheringLevel();
    UpdateVisualState();

    UE_LOG(LogTemp, Log, TEXT("Structure %s repaired by %.1f, integrity now %.1f%%"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           RepairAmount, GetIntegrityPercentage());
}

bool UArch_StructuralIntegritySystem::IsStructureStable() const
{
    return StructuralProperties.CurrentIntegrity > StructuralProperties.CollapseThreshold;
}

float UArch_StructuralIntegritySystem::GetIntegrityPercentage() const
{
    if (StructuralProperties.MaxIntegrity <= 0.0f)
        return 0.0f;

    return (StructuralProperties.CurrentIntegrity / StructuralProperties.MaxIntegrity) * 100.0f;
}

void UArch_StructuralIntegritySystem::UpdateWeathering(float DeltaTime)
{
    if (StructuralProperties.WeatheringRate <= 0.0f)
        return;

    float WeatheringDamage = StructuralProperties.WeatheringRate * DeltaTime * GetMaterialWeatheringMultiplier();
    
    // Environmental factors
    if (CurrentMossLevel > 0.5f)
    {
        WeatheringDamage *= 1.2f; // Moss accelerates weathering
    }

    StructuralProperties.CurrentIntegrity = FMath::Max(0.0f,
        StructuralProperties.CurrentIntegrity - WeatheringDamage);

    UpdateWeatheringLevel();
}

void UArch_StructuralIntegritySystem::UpdateMossGrowth(float DeltaTime)
{
    if (MossGrowthRate <= 0.0f)
        return;

    // Moss grows faster on damaged structures
    float GrowthMultiplier = 1.0f;
    if (GetIntegrityPercentage() < 75.0f)
    {
        GrowthMultiplier = 1.5f;
    }

    CurrentMossLevel = FMath::Min(1.0f, CurrentMossLevel + (MossGrowthRate * DeltaTime * GrowthMultiplier));

    // Update visual state when moss level changes significantly
    static float LastMossLevel = 0.0f;
    if (FMath::Abs(CurrentMossLevel - LastMossLevel) > 0.1f)
    {
        UpdateVisualState();
        LastMossLevel = CurrentMossLevel;
    }
}

void UArch_StructuralIntegritySystem::CheckForCollapse()
{
    if (!StructuralProperties.bCanCollapse)
        return;

    if (StructuralProperties.CurrentIntegrity <= StructuralProperties.CollapseThreshold)
    {
        if (StructuralProperties.WeatheringLevel != EArch_WeatheringLevel::Collapsed)
        {
            StructuralProperties.WeatheringLevel = EArch_WeatheringLevel::Collapsed;
            OnStructureCollapsed();
            
            UE_LOG(LogTemp, Error, TEXT("Structure %s has collapsed!"), 
                   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
        }
    }
}

void UArch_StructuralIntegritySystem::UpdateWeatheringLevel()
{
    EArch_WeatheringLevel OldLevel = StructuralProperties.WeatheringLevel;
    float IntegrityPercent = GetIntegrityPercentage();

    if (IntegrityPercent > 90.0f)
    {
        StructuralProperties.WeatheringLevel = EArch_WeatheringLevel::Pristine;
    }
    else if (IntegrityPercent > 70.0f)
    {
        StructuralProperties.WeatheringLevel = EArch_WeatheringLevel::Weathered;
    }
    else if (IntegrityPercent > 40.0f)
    {
        StructuralProperties.WeatheringLevel = EArch_WeatheringLevel::Damaged;
    }
    else if (IntegrityPercent > StructuralProperties.CollapseThreshold)
    {
        StructuralProperties.WeatheringLevel = EArch_WeatheringLevel::Ruined;
    }
    else
    {
        StructuralProperties.WeatheringLevel = EArch_WeatheringLevel::Collapsed;
    }

    if (OldLevel != StructuralProperties.WeatheringLevel)
    {
        OnWeatheringLevelChanged(StructuralProperties.WeatheringLevel);
    }
}

float UArch_StructuralIntegritySystem::GetMaterialWeatheringMultiplier() const
{
    switch (StructuralProperties.Material)
    {
        case EArch_StructuralMaterial::Stone:
            return 0.3f; // Stone weathers slowly
        case EArch_StructuralMaterial::Wood:
            return 1.5f; // Wood weathers faster
        case EArch_StructuralMaterial::Bone:
            return 1.2f; // Bone weathers moderately
        case EArch_StructuralMaterial::Clay:
            return 2.0f; // Clay weathers quickly
        case EArch_StructuralMaterial::Hide:
            return 3.0f; // Hide degrades rapidly
        default:
            return 1.0f;
    }
}