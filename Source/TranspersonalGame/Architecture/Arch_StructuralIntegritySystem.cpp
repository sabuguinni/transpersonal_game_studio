#include "Arch_StructuralIntegritySystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UArch_StructuralIntegritySystem::UArch_StructuralIntegritySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize structural properties
    StructuralData.MaxIntegrity = 100.0f;
    StructuralData.CurrentIntegrity = 100.0f;
    StructuralData.MaterialType = EArch_MaterialType::Stone;
    StructuralData.WeatheringRate = 0.1f;
    StructuralData.LoadBearing = 50.0f;
    StructuralData.bIsFoundation = false;
    
    CurrentState = EArch_StructuralState::Pristine;
    bAffectedByWeather = true;
    RainDamageMultiplier = 1.5f;
    WindDamageMultiplier = 1.2f;
}

void UArch_StructuralIntegritySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Start weathering timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            WeatheringTimer,
            this,
            &UArch_StructuralIntegritySystem::ProcessWeathering,
            30.0f, // Every 30 seconds
            true
        );
        
        GetWorld()->GetTimerManager().SetTimer(
            IntegrityCheckTimer,
            this,
            &UArch_StructuralIntegritySystem::CheckStructuralChain,
            10.0f, // Every 10 seconds
            true
        );
    }
    
    UpdateStructuralState();
}

void UArch_StructuralIntegritySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Continuous integrity monitoring
    if (StructuralData.CurrentIntegrity <= 0.0f && CurrentState != EArch_StructuralState::Collapsed)
    {
        CurrentState = EArch_StructuralState::Collapsed;
        PropagateStructuralFailure();
        UpdateVisualState();
    }
}

void UArch_StructuralIntegritySystem::ApplyDamage(float DamageAmount, bool bFromWeather)
{
    float ActualDamage = DamageAmount;
    
    // Apply weather multipliers
    if (bFromWeather && bAffectedByWeather)
    {
        ActualDamage *= RainDamageMultiplier;
    }
    
    // Material resistance
    switch (StructuralData.MaterialType)
    {
        case EArch_MaterialType::Stone:
            ActualDamage *= 0.5f; // Stone is very resistant
            break;
        case EArch_MaterialType::Wood:
            ActualDamage *= 1.2f; // Wood is more vulnerable
            break;
        case EArch_MaterialType::Bone:
            ActualDamage *= 0.8f; // Bone is moderately resistant
            break;
        case EArch_MaterialType::Clay:
            ActualDamage *= 1.5f; // Clay is fragile
            break;
        case EArch_MaterialType::Thatch:
            ActualDamage *= 2.0f; // Thatch is very fragile
            break;
    }
    
    StructuralData.CurrentIntegrity = FMath::Max(0.0f, StructuralData.CurrentIntegrity - ActualDamage);
    UpdateStructuralState();
    UpdateVisualState();
    
    UE_LOG(LogTemp, Warning, TEXT("Structure %s took %.1f damage, integrity now %.1f%%"), 
           *GetOwner()->GetName(), ActualDamage, GetIntegrityPercentage());
}

void UArch_StructuralIntegritySystem::RepairStructure(float RepairAmount)
{
    StructuralData.CurrentIntegrity = FMath::Min(StructuralData.MaxIntegrity, 
                                                StructuralData.CurrentIntegrity + RepairAmount);
    UpdateStructuralState();
    UpdateVisualState();
    
    UE_LOG(LogTemp, Log, TEXT("Structure %s repaired by %.1f, integrity now %.1f%%"), 
           *GetOwner()->GetName(), RepairAmount, GetIntegrityPercentage());
}

float UArch_StructuralIntegritySystem::GetIntegrityPercentage() const
{
    if (StructuralData.MaxIntegrity <= 0.0f) return 0.0f;
    return (StructuralData.CurrentIntegrity / StructuralData.MaxIntegrity) * 100.0f;
}

EArch_StructuralState UArch_StructuralIntegritySystem::GetStructuralState() const
{
    return CurrentState;
}

bool UArch_StructuralIntegritySystem::CanSupportLoad(float AdditionalLoad) const
{
    float CurrentLoad = SupportedStructures.Num() * 10.0f; // Simplified load calculation
    float TotalLoad = CurrentLoad + AdditionalLoad;
    float EffectiveLoadBearing = StructuralData.LoadBearing * (GetIntegrityPercentage() / 100.0f);
    
    return TotalLoad <= EffectiveLoadBearing;
}

void UArch_StructuralIntegritySystem::AddSupportedStructure(AActor* Structure)
{
    if (Structure && !SupportedStructures.Contains(Structure))
    {
        SupportedStructures.Add(Structure);
        
        // Check if we can still support this load
        if (!CanSupportLoad(0.0f))
        {
            UE_LOG(LogTemp, Warning, TEXT("Structure %s is overloaded!"), *GetOwner()->GetName());
            ApplyDamage(5.0f); // Overload damage
        }
    }
}

void UArch_StructuralIntegritySystem::RemoveSupportedStructure(AActor* Structure)
{
    SupportedStructures.Remove(Structure);
}

void UArch_StructuralIntegritySystem::CheckStructuralChain()
{
    // Check if supporting structures are still valid
    for (int32 i = SupportingStructures.Num() - 1; i >= 0; i--)
    {
        AActor* Support = SupportingStructures[i];
        if (!IsValid(Support))
        {
            SupportingStructures.RemoveAt(i);
            continue;
        }
        
        // Check if support is collapsed
        UArch_StructuralIntegritySystem* SupportIntegrity = Support->FindComponentByClass<UArch_StructuralIntegritySystem>();
        if (SupportIntegrity && SupportIntegrity->GetStructuralState() == EArch_StructuralState::Collapsed)
        {
            UE_LOG(LogTemp, Warning, TEXT("Support structure %s collapsed, damaging %s"), 
                   *Support->GetName(), *GetOwner()->GetName());
            ApplyDamage(25.0f); // Significant damage from support failure
        }
    }
}

void UArch_StructuralIntegritySystem::ProcessWeathering()
{
    if (!bAffectedByWeather) return;
    
    float WeatheringDamage = GetMaterialWeatheringRate() * StructuralData.WeatheringRate;
    
    // TODO: Get actual weather conditions from weather system
    // For now, simulate random weather effects
    float WeatherIntensity = FMath::RandRange(0.5f, 2.0f);
    WeatheringDamage *= WeatherIntensity;
    
    ApplyDamage(WeatheringDamage, true);
}

void UArch_StructuralIntegritySystem::UpdateStructuralState()
{
    float IntegrityPercent = GetIntegrityPercentage();
    
    if (IntegrityPercent <= 0.0f)
    {
        CurrentState = EArch_StructuralState::Collapsed;
    }
    else if (IntegrityPercent <= 25.0f)
    {
        CurrentState = EArch_StructuralState::Crumbling;
    }
    else if (IntegrityPercent <= 50.0f)
    {
        CurrentState = EArch_StructuralState::Damaged;
    }
    else if (IntegrityPercent <= 80.0f)
    {
        CurrentState = EArch_StructuralState::Weathered;
    }
    else
    {
        CurrentState = EArch_StructuralState::Pristine;
    }
}

void UArch_StructuralIntegritySystem::PropagateStructuralFailure()
{
    // Damage supported structures when this one collapses
    for (AActor* SupportedActor : SupportedStructures)
    {
        if (IsValid(SupportedActor))
        {
            UArch_StructuralIntegritySystem* SupportedIntegrity = SupportedActor->FindComponentByClass<UArch_StructuralIntegritySystem>();
            if (SupportedIntegrity)
            {
                UE_LOG(LogTemp, Error, TEXT("Structure %s collapsed, damaging supported structure %s"), 
                       *GetOwner()->GetName(), *SupportedActor->GetName());
                SupportedIntegrity->ApplyDamage(50.0f); // Major damage from support collapse
            }
        }
    }
    
    // Clear supported structures list
    SupportedStructures.Empty();
}

void UArch_StructuralIntegritySystem::UpdateVisualState()
{
    // Update visual representation based on structural state
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp) return;
    
    // TODO: Apply different materials or effects based on CurrentState
    // This would typically involve material parameter changes or particle effects
    
    switch (CurrentState)
    {
        case EArch_StructuralState::Pristine:
            // Clean, new appearance
            break;
        case EArch_StructuralState::Weathered:
            // Slightly worn, moss/dirt
            break;
        case EArch_StructuralState::Damaged:
            // Visible cracks, missing pieces
            break;
        case EArch_StructuralState::Crumbling:
            // Major damage, unstable appearance
            break;
        case EArch_StructuralState::Collapsed:
            // Rubble, destroyed
            GetOwner()->SetActorHiddenInGame(true); // Hide the structure
            break;
    }
}

float UArch_StructuralIntegritySystem::GetMaterialWeatheringRate() const
{
    switch (StructuralData.MaterialType)
    {
        case EArch_MaterialType::Stone:
            return 0.05f; // Very slow weathering
        case EArch_MaterialType::Wood:
            return 0.2f;  // Moderate weathering
        case EArch_MaterialType::Bone:
            return 0.15f; // Slow-moderate weathering
        case EArch_MaterialType::Clay:
            return 0.3f;  // Fast weathering
        case EArch_MaterialType::Thatch:
            return 0.5f;  // Very fast weathering
        default:
            return 0.1f;
    }
}

float UArch_StructuralIntegritySystem::GetMaterialLoadCapacity() const
{
    switch (StructuralData.MaterialType)
    {
        case EArch_MaterialType::Stone:
            return 100.0f; // Very high load capacity
        case EArch_MaterialType::Wood:
            return 60.0f;  // Moderate load capacity
        case EArch_MaterialType::Bone:
            return 40.0f;  // Lower load capacity
        case EArch_MaterialType::Clay:
            return 20.0f;  // Low load capacity
        case EArch_MaterialType::Thatch:
            return 10.0f;  // Very low load capacity
        default:
            return 50.0f;
    }
}