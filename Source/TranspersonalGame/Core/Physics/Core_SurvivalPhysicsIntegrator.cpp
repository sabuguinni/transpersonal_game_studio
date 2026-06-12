#include "Core_SurvivalPhysicsIntegrator.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCore_SurvivalPhysicsIntegrator::UCore_SurvivalPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize terrain movement modifiers
    TerrainMovementModifiers.Add(ETT_Normal, 1.0f);
    TerrainMovementModifiers.Add(ETT_Sand, 0.8f);
    TerrainMovementModifiers.Add(ETT_Mud, 0.6f);
    TerrainMovementModifiers.Add(ETT_Rock, 0.9f);
    TerrainMovementModifiers.Add(ETT_Grass, 1.1f);
    TerrainMovementModifiers.Add(ETT_Snow, 0.7f);
    TerrainMovementModifiers.Add(ETT_Water, 0.5f);
    TerrainMovementModifiers.Add(ETT_Ice, 0.4f);
    
    CurrentHazard = EEH_None;
}

void UCore_SurvivalPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SurvivalComp = Owner->FindComponentByClass<USurvivalComponent>();
        MovementComp = Owner->FindComponentByClass<UCharacterMovementComponent>();
        CapsuleComp = Owner->FindComponentByClass<UCapsuleComponent>();
        
        // Cache original movement values
        if (MovementComp)
        {
            OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;
            OriginalJumpZVelocity = MovementComp->JumpZVelocity;
        }
        
        if (!SurvivalComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_SurvivalPhysicsIntegrator: No SurvivalComponent found on %s"), *Owner->GetName());
        }
    }
}

void UCore_SurvivalPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance optimization - only update at intervals
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateInterval)
    {
        return;
    }
    LastUpdateTime = CurrentTime;
    
    // Update all survival physics integrations
    UpdateMovementFromSurvival();
    
    // Update limp timer for injury effects
    if (InjuryLevel > 0.0f && bEnableLimpEffect)
    {
        LimpTimer += DeltaTime;
    }
}

void UCore_SurvivalPhysicsIntegrator::UpdateMovementFromSurvival()
{
    if (!MovementComp || !SurvivalComp)
    {
        return;
    }
    
    // Apply all physics modifications
    ApplyStaminaPhysics();
    ApplyTemperaturePhysics();
    ApplyInjuryPhysics();
    ApplyCarryingCapacityPhysics();
    HandleEnvironmentalHazards();
    
    // Calculate final movement multiplier
    float FinalMultiplier = GetTotalMovementMultiplier();
    
    // Apply to movement component
    MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed * FinalMultiplier;
    MovementComp->JumpZVelocity = OriginalJumpZVelocity * FMath::Max(FinalMultiplier, StaminaJumpHeightMultiplier);
}

void UCore_SurvivalPhysicsIntegrator::ApplyStaminaPhysics()
{
    if (!SurvivalComp)
    {
        return;
    }
    
    float StaminaRatio = SurvivalComp->GetStaminaRatio();
    
    // Calculate stamina-based movement multiplier
    float StaminaMultiplier;
    if (StaminaRatio <= CriticalStaminaThreshold)
    {
        // Severe stamina depletion - significant movement penalty
        StaminaMultiplier = MinStaminaSpeedMultiplier;
    }
    else
    {
        // Linear interpolation between min and max based on stamina
        float NormalizedStamina = (StaminaRatio - CriticalStaminaThreshold) / (1.0f - CriticalStaminaThreshold);
        StaminaMultiplier = FMath::Lerp(MinStaminaSpeedMultiplier, BaseStaminaSpeedMultiplier, NormalizedStamina);
    }
    
    // Store for use in total calculation
    BaseStaminaSpeedMultiplier = StaminaMultiplier;
}

void UCore_SurvivalPhysicsIntegrator::ApplyTemperaturePhysics()
{
    if (!SurvivalComp)
    {
        return;
    }
    
    float Temperature = SurvivalComp->GetTemperature();
    float OptimalTemp = SurvivalComp->GetOptimalTemperature();
    float TempDifference = FMath::Abs(Temperature - OptimalTemp);
    
    // Apply temperature penalties if outside comfortable range
    if (TempDifference > TemperaturePenaltyThreshold)
    {
        if (Temperature < OptimalTemp - TemperaturePenaltyThreshold)
        {
            // Too cold - apply cold penalty
            BaseStaminaSpeedMultiplier *= ColdMovementPenalty;
        }
        else if (Temperature > OptimalTemp + TemperaturePenaltyThreshold)
        {
            // Too hot - apply heat penalty
            BaseStaminaSpeedMultiplier *= HeatMovementPenalty;
        }
    }
}

void UCore_SurvivalPhysicsIntegrator::ApplyInjuryPhysics()
{
    if (!SurvivalComp)
    {
        return;
    }
    
    // Get injury level from health ratio (lower health = more injuries)
    float HealthRatio = SurvivalComp->GetHealthRatio();
    InjuryLevel = 1.0f - HealthRatio;
    
    // Apply injury movement penalty
    float InjuryMultiplier = 1.0f - (InjuryLevel * InjuryMovementPenalty);
    BaseStaminaSpeedMultiplier *= FMath::Max(InjuryMultiplier, 0.1f); // Minimum 10% speed
    
    // Apply limp effect for visual/gameplay impact
    if (bEnableLimpEffect && InjuryLevel > 0.3f && MovementComp)
    {
        // Create irregular movement pattern
        float LimpEffect = FMath::Sin(LimpTimer * 3.0f) * LimpIntensity * InjuryLevel;
        float CurrentSpeed = MovementComp->MaxWalkSpeed * (1.0f + LimpEffect * 0.1f);
        MovementComp->MaxWalkSpeed = FMath::Max(CurrentSpeed, OriginalMaxWalkSpeed * 0.2f);
    }
}

void UCore_SurvivalPhysicsIntegrator::ApplyCarryingCapacityPhysics()
{
    float CurrentWeight = CalculateCurrentWeight();
    float Overweight = FMath::Max(0.0f, CurrentWeight - BaseCarryingCapacity);
    
    if (Overweight > 0.0f)
    {
        if (Overweight >= MaxOverweight)
        {
            // Cannot move when severely overloaded
            BaseStaminaSpeedMultiplier *= 0.1f;
        }
        else
        {
            // Apply overweight penalty
            float WeightPenalty = 1.0f - (Overweight * OverweightPenalty);
            BaseStaminaSpeedMultiplier *= FMath::Max(WeightPenalty, 0.2f);
        }
    }
}

void UCore_SurvivalPhysicsIntegrator::HandleEnvironmentalHazards()
{
    // This would typically be called by environmental systems
    // For now, apply terrain-based modifiers
    
    // Get current terrain type (simplified - would normally use terrain detection)
    ECore_TerrainType CurrentTerrain = ETT_Normal; // Default
    
    // Apply terrain modifier
    if (TerrainMovementModifiers.Contains(CurrentTerrain))
    {
        BaseStaminaSpeedMultiplier *= TerrainMovementModifiers[CurrentTerrain];
    }
    
    // Handle specific environmental hazards
    switch (CurrentHazard)
    {
        case EEH_Quicksand:
            BaseStaminaSpeedMultiplier *= 0.3f;
            break;
        case EEH_UnstableTerrain:
            BaseStaminaSpeedMultiplier *= 0.7f;
            break;
        case EEH_WaterCurrent:
            BaseStaminaSpeedMultiplier *= 0.5f;
            break;
        case EEH_Muddy:
            BaseStaminaSpeedMultiplier *= 0.6f;
            break;
        case EEH_Rocky:
            BaseStaminaSpeedMultiplier *= 0.8f;
            break;
        case EEH_Steep:
            BaseStaminaSpeedMultiplier *= 0.4f;
            break;
        default:
            // No additional penalty
            break;
    }
}

float UCore_SurvivalPhysicsIntegrator::CalculateCurrentWeight() const
{
    // Simplified weight calculation
    // In a full implementation, this would sum inventory items, equipment, etc.
    float BaseWeight = 70.0f; // Base character weight in kg
    
    // Add equipment weight (placeholder)
    float EquipmentWeight = 10.0f;
    
    // Add inventory weight (placeholder)
    float InventoryWeight = 5.0f;
    
    return BaseWeight + EquipmentWeight + InventoryWeight;
}

float UCore_SurvivalPhysicsIntegrator::GetTotalMovementMultiplier() const
{
    // BaseStaminaSpeedMultiplier already contains all applied modifiers
    return FMath::Clamp(BaseStaminaSpeedMultiplier, 0.1f, 2.0f);
}

bool UCore_SurvivalPhysicsIntegrator::CanPerformPhysicalAction() const
{
    if (!SurvivalComp)
    {
        return true; // Default to allowing actions if no survival component
    }
    
    // Check if character has enough stamina for physical actions
    float StaminaRatio = SurvivalComp->GetStaminaRatio();
    if (StaminaRatio < 0.1f)
    {
        return false; // Too exhausted
    }
    
    // Check if injury level is too high
    if (InjuryLevel > 0.8f)
    {
        return false; // Too injured
    }
    
    // Check if overweight prevents actions
    float CurrentWeight = CalculateCurrentWeight();
    if (CurrentWeight - BaseCarryingCapacity >= MaxOverweight)
    {
        return false; // Too overloaded
    }
    
    return true;
}