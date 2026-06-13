#include "Arch_StructuralIntegrity.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UArch_StructuralIntegrity::UArch_StructuralIntegrity()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CurrentState = EArch_StructuralState::Pristine;
    StructuralHealth = 100.0f;
    MaxStructuralHealth = 100.0f;
    WeatheringRate = 0.1f;
    RainDamageMultiplier = 2.0f;
    WindDamageMultiplier = 1.5f;
    bCanCollapse = true;
    CollapseThreshold = 10.0f;
}

void UArch_StructuralIntegrity::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize damage pattern
    DamagePattern.CrackSeverity = 0.0f;
    DamagePattern.MossGrowth = FMath::RandRange(0.0f, 0.2f);
    DamagePattern.WeatheringIntensity = 0.0f;
    
    UpdateStructuralState();
}

void UArch_StructuralIntegrity::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessWeathering(DeltaTime);
    UpdateStructuralState();
}

void UArch_StructuralIntegrity::ApplyWeatherDamage(float DamageAmount, EWeatherType WeatherType)
{
    float MultipliedDamage = DamageAmount;
    
    switch (WeatherType)
    {
        case EWeatherType::Rain:
        case EWeatherType::Storm:
            MultipliedDamage *= RainDamageMultiplier;
            DamagePattern.MossGrowth += 0.1f;
            break;
        case EWeatherType::Wind:
            MultipliedDamage *= WindDamageMultiplier;
            break;
        default:
            break;
    }
    
    StructuralHealth = FMath::Clamp(StructuralHealth - MultipliedDamage, 0.0f, MaxStructuralHealth);
    UpdateDamagePattern();
    ApplyVisualDamage();
}

void UArch_StructuralIntegrity::ApplyImpactDamage(float DamageAmount, FVector ImpactLocation)
{
    StructuralHealth = FMath::Clamp(StructuralHealth - DamageAmount, 0.0f, MaxStructuralHealth);
    
    // Add crack at impact location
    DamagePattern.CrackLocations.Add(ImpactLocation);
    DamagePattern.CrackSeverity += DamageAmount * 0.01f;
    
    UpdateDamagePattern();
    ApplyVisualDamage();
    
    if (StructuralHealth <= CollapseThreshold && bCanCollapse)
    {
        TriggerCollapse();
    }
}

void UArch_StructuralIntegrity::UpdateStructuralState()
{
    float HealthPercentage = GetStructuralHealthPercentage();
    
    if (HealthPercentage > 80.0f)
    {
        CurrentState = EArch_StructuralState::Pristine;
    }
    else if (HealthPercentage > 60.0f)
    {
        CurrentState = EArch_StructuralState::Weathered;
    }
    else if (HealthPercentage > 30.0f)
    {
        CurrentState = EArch_StructuralState::Damaged;
    }
    else if (HealthPercentage > 10.0f)
    {
        CurrentState = EArch_StructuralState::Ruined;
    }
    else
    {
        CurrentState = EArch_StructuralState::Collapsed;
        if (bCanCollapse)
        {
            TriggerCollapse();
        }
    }
}

void UArch_StructuralIntegrity::TriggerCollapse()
{
    CurrentState = EArch_StructuralState::Collapsed;
    
    // Disable collision and physics
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            MeshComp->SetSimulatePhysics(false);
        }
        
        // Mark for destruction after delay
        Owner->SetLifeSpan(5.0f);
    }
}

bool UArch_StructuralIntegrity::IsStructureStable() const
{
    return CurrentState != EArch_StructuralState::Collapsed && StructuralHealth > CollapseThreshold;
}

float UArch_StructuralIntegrity::GetStructuralHealthPercentage() const
{
    return (StructuralHealth / MaxStructuralHealth) * 100.0f;
}

void UArch_StructuralIntegrity::ProcessWeathering(float DeltaTime)
{
    if (CurrentState == EArch_StructuralState::Collapsed)
    {
        return;
    }
    
    // Apply gradual weathering over time
    float WeatheringDamage = WeatheringRate * DeltaTime;
    StructuralHealth = FMath::Clamp(StructuralHealth - WeatheringDamage, 0.0f, MaxStructuralHealth);
    
    // Increase weathering intensity
    DamagePattern.WeatheringIntensity += WeatheringDamage * 0.001f;
    DamagePattern.WeatheringIntensity = FMath::Clamp(DamagePattern.WeatheringIntensity, 0.0f, 1.0f);
    
    // Moss growth over time
    DamagePattern.MossGrowth += DeltaTime * 0.001f;
    DamagePattern.MossGrowth = FMath::Clamp(DamagePattern.MossGrowth, 0.0f, 1.0f);
}

void UArch_StructuralIntegrity::UpdateDamagePattern()
{
    float HealthPercentage = GetStructuralHealthPercentage();
    
    // Update crack severity based on health
    DamagePattern.CrackSeverity = FMath::Lerp(0.0f, 1.0f, 1.0f - (HealthPercentage / 100.0f));
    
    // Clamp all damage values
    DamagePattern.CrackSeverity = FMath::Clamp(DamagePattern.CrackSeverity, 0.0f, 1.0f);
    DamagePattern.MossGrowth = FMath::Clamp(DamagePattern.MossGrowth, 0.0f, 1.0f);
    DamagePattern.WeatheringIntensity = FMath::Clamp(DamagePattern.WeatheringIntensity, 0.0f, 1.0f);
}

void UArch_StructuralIntegrity::ApplyVisualDamage()
{
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            // Create dynamic material instance if needed
            UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateDynamicMaterialInstance(0);
            if (DynMaterial)
            {
                // Apply damage parameters to material
                DynMaterial->SetScalarParameterValue(TEXT("CrackSeverity"), DamagePattern.CrackSeverity);
                DynMaterial->SetScalarParameterValue(TEXT("MossGrowth"), DamagePattern.MossGrowth);
                DynMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), DamagePattern.WeatheringIntensity);
            }
        }
    }
}