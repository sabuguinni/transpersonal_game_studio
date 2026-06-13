#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Core/Engine/Eng_SurvivalSystemManager.h"
#include "Core_SurvivalPhysicsIntegrator.generated.h"

class ATranspersonalCharacter;
class UEng_SurvivalSystemManager;

/**
 * Core Survival Physics Integrator
 * 
 * Bridges the survival system with physics mechanics, modifying physics behavior
 * based on character survival stats (hunger, thirst, stamina, fear, health).
 * 
 * Key Features:
 * - Physics multipliers based on survival stats
 * - Movement speed modification based on hunger/stamina
 * - Collision response changes based on fear level
 * - Ragdoll threshold adjustments based on health
 * - Environmental physics interaction based on survival state
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SurvivalPhysicsState
{
    GENERATED_BODY()

    // Movement Physics Multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float JumpForceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AccelerationMultiplier = 1.0f;

    // Collision Physics Multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float CollisionDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float ImpactResistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float RagdollThresholdMultiplier = 1.0f;

    // Environmental Physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GroundFrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AirResistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScaleMultiplier = 1.0f;

    // Fear-based Physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float FearTremorIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bFearInducedClumsiness = false;

    FCore_SurvivalPhysicsState()
    {
        MovementSpeedMultiplier = 1.0f;
        JumpForceMultiplier = 1.0f;
        AccelerationMultiplier = 1.0f;
        CollisionDamageMultiplier = 1.0f;
        ImpactResistanceMultiplier = 1.0f;
        RagdollThresholdMultiplier = 1.0f;
        GroundFrictionMultiplier = 1.0f;
        AirResistanceMultiplier = 1.0f;
        GravityScaleMultiplier = 1.0f;
        FearTremorIntensity = 0.0f;
        bFearInducedClumsiness = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_SurvivalPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_SurvivalPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Component References
    UPROPERTY()
    ATranspersonalCharacter* OwnerCharacter;

    UPROPERTY()
    UEng_SurvivalSystemManager* SurvivalSystem;

    // Current Physics State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State")
    FCore_SurvivalPhysicsState CurrentPhysicsState;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDebugOutput = false;

    // Survival Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float LowStaminaThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float LowHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float HighFearThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalHungerThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalThirstThreshold = 0.15f;

    // Physics Multiplier Ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multipliers")
    FVector2D MovementSpeedRange = FVector2D(0.4f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multipliers")
    FVector2D JumpForceRange = FVector2D(0.5f, 1.1f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multipliers")
    FVector2D CollisionDamageRange = FVector2D(0.8f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multipliers")
    FVector2D RagdollThresholdRange = FVector2D(0.3f, 1.5f);

    // Update Timer
    float LastUpdateTime = 0.0f;

public:
    // Main Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsFromSurvival();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyMovementModifications();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyCollisionModifications();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyEnvironmentalModifications();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyFearEffects();

    // Calculation Functions
    UFUNCTION(BlueprintPure, Category = "Physics Calculations")
    float CalculateMovementSpeedMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Physics Calculations")
    float CalculateJumpForceMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Physics Calculations")
    float CalculateCollisionDamageMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Physics Calculations")
    float CalculateRagdollThresholdMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Physics Calculations")
    float CalculateFearTremorIntensity() const;

    // State Query Functions
    UFUNCTION(BlueprintPure, Category = "Physics State")
    FCore_SurvivalPhysicsState GetCurrentPhysicsState() const { return CurrentPhysicsState; }

    UFUNCTION(BlueprintPure, Category = "Physics State")
    bool IsInCriticalSurvivalState() const;

    UFUNCTION(BlueprintPure, Category = "Physics State")
    bool IsPhysicsCompromised() const;

    UFUNCTION(BlueprintCallable, Category = "Physics State")
    void ResetPhysicsState();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableDebugOutput(bool bEnable) { bEnableDebugOutput = bEnable; }

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCurrentPhysicsState() const;

protected:
    // Internal Helper Functions
    void InitializeComponent();
    void CacheComponentReferences();
    float GetSurvivalStatNormalized(const FString& StatName) const;
    float InterpolateMultiplier(float NormalizedValue, const FVector2D& Range) const;
    void ApplyPhysicsStateToCharacter();
    void UpdateMovementComponent();
    void UpdateCollisionSettings();
    void ProcessFearEffects(float DeltaTime);
    void ValidatePhysicsState();
};