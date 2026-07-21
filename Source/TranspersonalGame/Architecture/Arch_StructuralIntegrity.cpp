#include "Arch_StructuralIntegrity.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UArch_StructuralIntegrity::UArch_StructuralIntegrity()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance
    
    // Initialize structural data
    StructuralData.IntegrityPercentage = 100.0f;
    StructuralData.CurrentState = EArch_StructuralState::Pristine;
    StructuralData.MaterialType = EArch_MaterialType::Stone;
    StructuralData.WeatheringRate = 0.1f;
    StructuralData.LoadCapacity = 1000.0f;
    StructuralData.CurrentLoad = 0.0f;
    
    // Environmental settings
    bEnableWeathering = true;
    WeatheringMultiplier = 1.0f;
    RainDamageMultiplier = 1.5f;
    WindDamageMultiplier = 1.2f;
    TemperatureDamageMultiplier = 1.1f;
    
    // Timers
    WeatheringTimer = 0.0f;
    StateUpdateTimer = 0.0f;
}

void UArch_StructuralIntegrity::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structural state
    UpdateStructuralState();
    
    UE_LOG(LogTemp, Log, TEXT("StructuralIntegrity component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UArch_StructuralIntegrity::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Process weathering over time
    if (bEnableWeathering)
    {
        ProcessWeathering(DeltaTime);
    }
    
    // Update structural state periodically
    StateUpdateTimer += DeltaTime;
    if (StateUpdateTimer >= StateUpdateInterval)
    {
        UpdateStructuralState();
        CheckStructuralFailure();
        StateUpdateTimer = 0.0f;
    }
}

void UArch_StructuralIntegrity::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f)
    {
        return;
    }
    
    // Calculate damage based on material type
    float MaterialResistance = 1.0f;
    switch (StructuralData.MaterialType)
    {
        case EArch_MaterialType::Stone:
            MaterialResistance = 0.8f; // Stone is more resistant
            break;
        case EArch_MaterialType::Wood:
            MaterialResistance = 1.2f; // Wood is more vulnerable
            break;
        case EArch_MaterialType::Bone:
            MaterialResistance = 1.0f; // Bone is moderate
            break;
        case EArch_MaterialType::Clay:
            MaterialResistance = 1.5f; // Clay is fragile
            break;
        case EArch_MaterialType::Organic:
            MaterialResistance = 2.0f; // Organic materials are very fragile
            break;
    }
    
    float ActualDamage = DamageAmount * MaterialResistance;
    float DamagePercentage = (ActualDamage / StructuralData.LoadCapacity) * 100.0f;
    
    StructuralData.IntegrityPercentage = FMath::Clamp(
        StructuralData.IntegrityPercentage - DamagePercentage, 
        0.0f, 
        100.0f
    );
    
    UpdateStructuralState();
    
    UE_LOG(LogTemp, Warning, TEXT("Structural damage applied: %.2f (%.2f%% integrity remaining)"), 
           ActualDamage, StructuralData.IntegrityPercentage);
}

void UArch_StructuralIntegrity::ApplyLoad(float LoadAmount)
{
    if (LoadAmount <= 0.0f)
    {
        return;
    }
    
    StructuralData.CurrentLoad += LoadAmount;
    
    // Check if load exceeds capacity
    if (StructuralData.CurrentLoad > StructuralData.LoadCapacity)
    {
        float OverloadDamage = (StructuralData.CurrentLoad - StructuralData.LoadCapacity) * 0.1f;
        ApplyDamage(OverloadDamage);
        
        UE_LOG(LogTemp, Warning, TEXT("Structural overload! Load: %.2f / %.2f capacity"), 
               StructuralData.CurrentLoad, StructuralData.LoadCapacity);
    }
}

void UArch_StructuralIntegrity::RemoveLoad(float LoadAmount)
{
    if (LoadAmount <= 0.0f)
    {
        return;
    }
    
    StructuralData.CurrentLoad = FMath::Max(0.0f, StructuralData.CurrentLoad - LoadAmount);
}

bool UArch_StructuralIntegrity::CanSupportLoad(float TestLoad) const
{
    return (StructuralData.CurrentLoad + TestLoad) <= StructuralData.LoadCapacity && 
           StructuralData.IntegrityPercentage > 25.0f;
}

void UArch_StructuralIntegrity::UpdateStructuralState()
{
    EArch_StructuralState PreviousState = StructuralData.CurrentState;
    
    // Determine state based on integrity percentage
    if (StructuralData.IntegrityPercentage >= 90.0f)
    {
        StructuralData.CurrentState = EArch_StructuralState::Pristine;
    }
    else if (StructuralData.IntegrityPercentage >= 70.0f)
    {
        StructuralData.CurrentState = EArch_StructuralState::Weathered;
    }
    else if (StructuralData.IntegrityPercentage >= 40.0f)
    {
        StructuralData.CurrentState = EArch_StructuralState::Damaged;
    }
    else if (StructuralData.IntegrityPercentage >= 10.0f)
    {
        StructuralData.CurrentState = EArch_StructuralState::Crumbling;
    }
    else
    {
        StructuralData.CurrentState = EArch_StructuralState::Collapsed;
    }
    
    // Trigger visual update if state changed
    if (PreviousState != StructuralData.CurrentState)
    {
        OnStructuralStateChanged(StructuralData.CurrentState);
        
        UE_LOG(LogTemp, Log, TEXT("Structural state changed from %d to %d"), 
               (int32)PreviousState, (int32)StructuralData.CurrentState);
    }
}

bool UArch_StructuralIntegrity::IsStructurallySound() const
{
    return StructuralData.IntegrityPercentage > 25.0f && 
           StructuralData.CurrentState != EArch_StructuralState::Collapsed;
}

void UArch_StructuralIntegrity::ProcessWeathering(float DeltaTime)
{
    WeatheringTimer += DeltaTime;
    
    // Apply weathering every 10 seconds
    if (WeatheringTimer >= 10.0f)
    {
        float EnvironmentalDamage = CalculateEnvironmentalDamage();
        float WeatheringDamage = StructuralData.WeatheringRate * WeatheringMultiplier * EnvironmentalDamage;
        
        ApplyDamage(WeatheringDamage);
        WeatheringTimer = 0.0f;
    }
}

void UArch_StructuralIntegrity::CheckStructuralFailure()
{
    if (StructuralData.CurrentState == EArch_StructuralState::Collapsed)
    {
        OnStructuralCollapse();
        
        // Disable further processing for collapsed structures
        SetComponentTickEnabled(false);
        
        UE_LOG(LogTemp, Error, TEXT("Structure has collapsed: %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

float UArch_StructuralIntegrity::CalculateEnvironmentalDamage() const
{
    float EnvironmentalFactor = 1.0f;
    
    // In a real implementation, this would check weather conditions
    // For now, we'll use a base environmental factor
    
    // Simulate random environmental conditions
    float RandomFactor = FMath::RandRange(0.8f, 1.2f);
    
    return EnvironmentalFactor * RandomFactor;
}