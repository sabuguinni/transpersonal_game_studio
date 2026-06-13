#include "Arch_StructuralIntegritySystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

UArch_StructuralIntegritySystem::UArch_StructuralIntegritySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize default values
    StructuralData = FArch_StructuralData();
    DegradationRate = 0.1f;
    WeatherDamageMultiplier = 1.0f;
    bEnableTimeDegradation = true;
    bEnableWeatherDegradation = true;
    bAutoUpdateMaterials = true;
    
    LastDegradationTime = 0.0f;
}

void UArch_StructuralIntegritySystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMaterialArrays();
    
    // Start degradation timer
    if (bEnableTimeDegradation && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DegradationTimerHandle,
            this,
            &UArch_StructuralIntegritySystem::ProcessDegradation,
            60.0f, // Process every minute
            true
        );
    }
    
    // Initial visual update
    UpdateVisualCondition();
}

void UArch_StructuralIntegritySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check for structural failure
    CheckStructuralFailure();
    
    // Update age
    StructuralData.AgeInYears += DeltaTime / (365.0f * 24.0f * 3600.0f); // Convert seconds to years
}

void UArch_StructuralIntegritySystem::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f) return;
    
    float PreviousIntegrity = StructuralData.IntegrityValue;
    StructuralData.IntegrityValue = FMath::Clamp(StructuralData.IntegrityValue - DamageAmount, 0.0f, 100.0f);
    
    // Update condition based on new integrity
    EArch_StructuralCondition NewCondition = GetConditionFromIntegrity(StructuralData.IntegrityValue);
    if (NewCondition != StructuralData.Condition)
    {
        StructuralData.Condition = NewCondition;
        UpdateVisualCondition();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Structure damaged: %.1f -> %.1f integrity"), PreviousIntegrity, StructuralData.IntegrityValue);
}

void UArch_StructuralIntegritySystem::RepairStructure(float RepairAmount)
{
    if (RepairAmount <= 0.0f) return;
    
    float PreviousIntegrity = StructuralData.IntegrityValue;
    StructuralData.IntegrityValue = FMath::Clamp(StructuralData.IntegrityValue + RepairAmount, 0.0f, 100.0f);
    
    // Update condition based on new integrity
    EArch_StructuralCondition NewCondition = GetConditionFromIntegrity(StructuralData.IntegrityValue);
    if (NewCondition != StructuralData.Condition)
    {
        StructuralData.Condition = NewCondition;
        UpdateVisualCondition();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Structure repaired: %.1f -> %.1f integrity"), PreviousIntegrity, StructuralData.IntegrityValue);
}

bool UArch_StructuralIntegritySystem::CanSupportLoad(float AdditionalLoad) const
{
    if (!StructuralData.bIsLoadBearing) return true;
    
    float TotalLoad = StructuralData.CurrentLoad + AdditionalLoad;
    float EffectiveMaxLoad = StructuralData.MaxLoad * (StructuralData.IntegrityValue / 100.0f);
    
    return TotalLoad <= EffectiveMaxLoad;
}

void UArch_StructuralIntegritySystem::AddLoad(float LoadAmount)
{
    if (!StructuralData.bIsLoadBearing || LoadAmount <= 0.0f) return;
    
    StructuralData.CurrentLoad += LoadAmount;
    
    // Check if overloaded
    float EffectiveMaxLoad = StructuralData.MaxLoad * (StructuralData.IntegrityValue / 100.0f);
    if (StructuralData.CurrentLoad > EffectiveMaxLoad)
    {
        float OverloadDamage = (StructuralData.CurrentLoad - EffectiveMaxLoad) * 0.1f;
        ApplyDamage(OverloadDamage);
        UE_LOG(LogTemp, Warning, TEXT("Structure overloaded! Applying %.1f damage"), OverloadDamage);
    }
}

void UArch_StructuralIntegritySystem::RemoveLoad(float LoadAmount)
{
    if (!StructuralData.bIsLoadBearing || LoadAmount <= 0.0f) return;
    
    StructuralData.CurrentLoad = FMath::Max(0.0f, StructuralData.CurrentLoad - LoadAmount);
}

EArch_StructuralCondition UArch_StructuralIntegritySystem::GetConditionFromIntegrity(float Integrity) const
{
    if (Integrity >= 90.0f) return EArch_StructuralCondition::Pristine;
    if (Integrity >= 75.0f) return EArch_StructuralCondition::Good;
    if (Integrity >= 50.0f) return EArch_StructuralCondition::Weathered;
    if (Integrity >= 25.0f) return EArch_StructuralCondition::Damaged;
    if (Integrity >= 10.0f) return EArch_StructuralCondition::Crumbling;
    return EArch_StructuralCondition::Ruined;
}

void UArch_StructuralIntegritySystem::UpdateVisualCondition()
{
    if (!bAutoUpdateMaterials) return;
    
    UpdateMaterialBasedOnCondition();
}

bool UArch_StructuralIntegritySystem::IsStructureStable() const
{
    return StructuralData.IntegrityValue > 10.0f && 
           StructuralData.Condition != EArch_StructuralCondition::Ruined;
}

void UArch_StructuralIntegritySystem::SetMaterialType(EArch_MaterialType NewMaterialType)
{
    StructuralData.MaterialType = NewMaterialType;
    
    // Update weather resistance based on material
    switch (NewMaterialType)
    {
        case EArch_MaterialType::Stone:
            StructuralData.WeatherResistance = 0.9f;
            break;
        case EArch_MaterialType::Wood:
            StructuralData.WeatherResistance = 0.6f;
            break;
        case EArch_MaterialType::Bone:
            StructuralData.WeatherResistance = 0.7f;
            break;
        case EArch_MaterialType::Clay:
            StructuralData.WeatherResistance = 0.4f;
            break;
        case EArch_MaterialType::Hide:
            StructuralData.WeatherResistance = 0.3f;
            break;
        case EArch_MaterialType::Thatch:
            StructuralData.WeatherResistance = 0.2f;
            break;
    }
}

void UArch_StructuralIntegritySystem::ProcessDegradation()
{
    if (!bEnableTimeDegradation) return;
    
    float DegradationAmount = GetMaterialDegradationRate() * DegradationRate;
    
    if (bEnableWeatherDegradation)
    {
        DegradationAmount *= GetWeatherImpact();
    }
    
    ApplyDamage(DegradationAmount);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processed degradation: %.2f damage applied"), DegradationAmount);
}

void UArch_StructuralIntegritySystem::CheckStructuralFailure()
{
    if (StructuralData.IntegrityValue <= 0.0f && StructuralData.Condition != EArch_StructuralCondition::Ruined)
    {
        StructuralData.Condition = EArch_StructuralCondition::Ruined;
        UpdateVisualCondition();
        
        UE_LOG(LogTemp, Warning, TEXT("Structure has completely failed!"));
        
        // Notify owner actor of structural failure
        if (AActor* Owner = GetOwner())
        {
            // Could trigger collapse effects, sound, particles, etc.
        }
    }
}

void UArch_StructuralIntegritySystem::UpdateMaterialBasedOnCondition()
{
    if (ConditionMaterials.Num() == 0) return;
    
    // Find static mesh component to update
    if (AActor* Owner = GetOwner())
    {
        UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && ConditionMaterials.IsValidIndex(static_cast<int32>(StructuralData.Condition)))
        {
            UMaterialInterface* NewMaterial = ConditionMaterials[static_cast<int32>(StructuralData.Condition)];
            if (NewMaterial)
            {
                MeshComp->SetMaterial(0, NewMaterial);
            }
        }
    }
}

float UArch_StructuralIntegritySystem::GetMaterialDegradationRate() const
{
    switch (StructuralData.MaterialType)
    {
        case EArch_MaterialType::Stone: return 0.1f;
        case EArch_MaterialType::Wood: return 0.5f;
        case EArch_MaterialType::Bone: return 0.3f;
        case EArch_MaterialType::Clay: return 0.8f;
        case EArch_MaterialType::Hide: return 1.2f;
        case EArch_MaterialType::Thatch: return 1.5f;
        default: return 0.5f;
    }
}

float UArch_StructuralIntegritySystem::GetWeatherImpact() const
{
    // Base weather impact (could be enhanced with actual weather system)
    float BaseWeatherDamage = 1.0f - StructuralData.WeatherResistance;
    return BaseWeatherDamage * WeatherDamageMultiplier;
}

void UArch_StructuralIntegritySystem::InitializeMaterialArrays()
{
    // Initialize with default materials if empty
    if (ConditionMaterials.Num() == 0)
    {
        ConditionMaterials.SetNum(6); // One for each condition state
        // Materials will be set up in Blueprint or by other systems
    }
}