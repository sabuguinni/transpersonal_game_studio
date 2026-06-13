#include "Arch_StructuralIntegrity.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"

UArch_StructuralIntegrity::UArch_StructuralIntegrity()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MaxIntegrity = 100.0f;
    WeatherResistance = 50.0f;
    ImpactThreshold = 25.0f;
    
    DamageState.IntegrityPercentage = 100.0f;
    DamageState.WeatheringRate = 0.1f;
    DamageState.LastDamageTime = 0.0f;
    DamageState.bIsCollapsed = false;
}

void UArch_StructuralIntegrity::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all static mesh components in the owner actor
    if (AActor* Owner = GetOwner())
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp && MeshComp->GetStaticMesh())
            {
                StructuralElements.Add(MeshComp);
            }
        }
    }
    
    // Initialize damage state
    DamageState.LastDamageTime = GetWorld()->GetTimeSeconds();
}

void UArch_StructuralIntegrity::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!DamageState.bIsCollapsed)
    {
        ProcessWeathering(DeltaTime);
        
        if (CheckStructuralCollapse())
        {
            DamageState.bIsCollapsed = true;
            UpdateVisualDamage();
        }
    }
}

void UArch_StructuralIntegrity::ApplyDamage(float DamageAmount, const FVector& ImpactPoint)
{
    if (DamageState.bIsCollapsed)
    {
        return;
    }
    
    // Apply damage based on impact threshold
    float ActualDamage = FMath::Max(0.0f, DamageAmount - ImpactThreshold);
    DamageState.IntegrityPercentage = FMath::Max(0.0f, DamageState.IntegrityPercentage - ActualDamage);
    DamageState.LastDamageTime = GetWorld()->GetTimeSeconds();
    
    UpdateVisualDamage();
    
    // Check for immediate collapse
    if (DamageState.IntegrityPercentage <= 0.0f)
    {
        DamageState.bIsCollapsed = true;
    }
}

void UArch_StructuralIntegrity::ProcessWeathering(float DeltaTime)
{
    if (DamageState.bIsCollapsed)
    {
        return;
    }
    
    // Calculate weathering based on resistance
    float WeatheringDamage = (DamageState.WeatheringRate * DeltaTime) / FMath::Max(1.0f, WeatherResistance / 10.0f);
    DamageState.IntegrityPercentage = FMath::Max(0.0f, DamageState.IntegrityPercentage - WeatheringDamage);
    
    // Update visual state periodically
    float TimeSinceLastDamage = GetWorld()->GetTimeSeconds() - DamageState.LastDamageTime;
    if (TimeSinceLastDamage > 5.0f)
    {
        UpdateVisualDamage();
        DamageState.LastDamageTime = GetWorld()->GetTimeSeconds();
    }
}

bool UArch_StructuralIntegrity::CheckStructuralCollapse()
{
    return DamageState.IntegrityPercentage <= 15.0f;
}

void UArch_StructuralIntegrity::UpdateVisualDamage()
{
    float DamageRatio = 1.0f - (DamageState.IntegrityPercentage / 100.0f);
    
    for (UStaticMeshComponent* Element : StructuralElements)
    {
        if (!Element)
        {
            continue;
        }
        
        // Create dynamic material instance if needed
        UMaterialInterface* CurrentMaterial = Element->GetMaterial(0);
        if (CurrentMaterial)
        {
            UMaterialInstanceDynamic* DynMaterial = Element->CreateDynamicMaterialInstance(0, CurrentMaterial);
            if (DynMaterial)
            {
                // Set damage parameters
                DynMaterial->SetScalarParameterValue(TEXT("DamageAmount"), DamageRatio);
                DynMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), DamageRatio * 0.8f);
                DynMaterial->SetScalarParameterValue(TEXT("MossGrowth"), DamageRatio * 0.6f);
            }
        }
        
        // Adjust collision if heavily damaged
        if (DamageState.IntegrityPercentage < 30.0f)
        {
            Element->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        
        // Hide elements if collapsed
        if (DamageState.bIsCollapsed)
        {
            Element->SetVisibility(false);
            Element->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void UArch_StructuralIntegrity::RepairStructure(float RepairAmount)
{
    if (DamageState.bIsCollapsed && RepairAmount < 50.0f)
    {
        return; // Cannot repair collapsed structure with small repair amount
    }
    
    DamageState.IntegrityPercentage = FMath::Min(MaxIntegrity, DamageState.IntegrityPercentage + RepairAmount);
    
    // Restore structure if repaired enough
    if (DamageState.bIsCollapsed && DamageState.IntegrityPercentage > 25.0f)
    {
        DamageState.bIsCollapsed = false;
        
        // Restore visibility and collision
        for (UStaticMeshComponent* Element : StructuralElements)
        {
            if (Element)
            {
                Element->SetVisibility(true);
                Element->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
    
    UpdateVisualDamage();
}