#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Survival/SurvivalComponent.h"
#include "Core_SurvivalPhysicsIntegrator.generated.h"

/**
 * Core_SurvivalPhysicsIntegrator
 * 
 * Integrates survival mechanics with physics systems for realistic prehistoric survival gameplay.
 * Handles stamina-based movement, temperature physics, injury physics, and environmental hazards.
 * 
 * Key Features:
 * - Stamina affects movement speed, jump height, and physics interactions
 * - Temperature physics (hypothermia/hyperthermia affects movement)
 * - Injury system with realistic physics consequences (limping, reduced mobility)
 * - Environmental hazard physics (quicksand, unstable terrain, water currents)
 * - Realistic carrying capacity affecting movement and physics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_SurvivalPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_SurvivalPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === COMPONENT REFERENCES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class USurvivalComponent* SurvivalComp;
    
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComp;
    
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCapsuleComponent* CapsuleComp;

    // === STAMINA PHYSICS INTEGRATION ===
    
    /** Base movement speed multiplier when at full stamina */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float BaseStaminaSpeedMultiplier = 1.0f;
    
    /** Minimum movement speed multiplier when stamina is depleted */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Physics", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float MinStaminaSpeedMultiplier = 0.3f;
    
    /** Jump height reduction when stamina is low */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Physics", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float StaminaJumpHeightMultiplier = 0.5f;
    
    /** Stamina threshold below which movement is significantly impaired */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CriticalStaminaThreshold = 0.2f;

    // === TEMPERATURE PHYSICS ===
    
    /** Movement speed reduction in extreme cold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Physics", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float ColdMovementPenalty = 0.7f;
    
    /** Movement speed reduction in extreme heat */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Physics", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float HeatMovementPenalty = 0.8f;
    
    /** Temperature threshold for movement penalties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Physics")
    float TemperaturePenaltyThreshold = 15.0f;

    // === INJURY PHYSICS SYSTEM ===
    
    /** Current injury level affecting movement */
    UPROPERTY(BlueprintReadOnly, Category = "Injury Physics")
    float InjuryLevel = 0.0f;
    
    /** Movement speed reduction per injury level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury Physics", meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float InjuryMovementPenalty = 0.1f;
    
    /** Limp effect - irregular movement pattern when injured */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury Physics")
    bool bEnableLimpEffect = true;
    
    /** Limp intensity - how pronounced the limping movement is */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float LimpIntensity = 1.0f;

    // === CARRYING CAPACITY PHYSICS ===
    
    /** Base carrying capacity in kg */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carrying Physics")
    float BaseCarryingCapacity = 50.0f;
    
    /** Movement penalty per kg over capacity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carrying Physics", meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float OverweightPenalty = 0.02f;
    
    /** Maximum overweight before movement becomes impossible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carrying Physics")
    float MaxOverweight = 25.0f;

    // === ENVIRONMENTAL HAZARD PHYSICS ===
    
    /** Current environmental hazard affecting movement */
    UPROPERTY(BlueprintReadOnly, Category = "Environmental Physics")
    TEnumAsByte<ECore_EnvironmentalHazard> CurrentHazard;
    
    /** Movement modifier for different terrain types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    TMap<TEnumAsByte<ECore_TerrainType>, float> TerrainMovementModifiers;

    // === PHYSICS INTEGRATION METHODS ===
    
    /** Update movement parameters based on survival stats */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void UpdateMovementFromSurvival();
    
    /** Apply stamina-based movement modifications */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyStaminaPhysics();
    
    /** Apply temperature-based movement modifications */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyTemperaturePhysics();
    
    /** Apply injury-based movement modifications */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyInjuryPhysics();
    
    /** Apply carrying capacity physics */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyCarryingCapacityPhysics();
    
    /** Handle environmental hazard physics */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void HandleEnvironmentalHazards();

    // === UTILITY METHODS ===
    
    /** Calculate current total weight being carried */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    float CalculateCurrentWeight() const;
    
    /** Get movement speed multiplier based on all survival factors */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    float GetTotalMovementMultiplier() const;
    
    /** Check if character can perform physically demanding actions */
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    bool CanPerformPhysicalAction() const;

private:
    /** Internal tracking of base movement speed */
    float OriginalMaxWalkSpeed;
    float OriginalJumpZVelocity;
    
    /** Limp effect timer for irregular movement */
    float LimpTimer = 0.0f;
    
    /** Cache for performance optimization */
    float LastUpdateTime = 0.0f;
    float UpdateInterval = 0.1f; // Update 10 times per second
};

/** Environmental hazard types that affect movement physics */
UENUM(BlueprintType)
enum ECore_EnvironmentalHazard
{
    EEH_None            UMETA(DisplayName = "None"),
    EEH_Quicksand       UMETA(DisplayName = "Quicksand"),
    EEH_UnstableTerrain UMETA(DisplayName = "Unstable Terrain"),
    EEH_WaterCurrent    UMETA(DisplayName = "Water Current"),
    EEH_Muddy           UMETA(DisplayName = "Muddy Ground"),
    EEH_Rocky           UMETA(DisplayName = "Rocky Terrain"),
    EEH_Steep           UMETA(DisplayName = "Steep Slope")
};

/** Terrain types with different physics properties */
UENUM(BlueprintType)
enum ECore_TerrainType
{
    ETT_Normal          UMETA(DisplayName = "Normal Ground"),
    ETT_Sand            UMETA(DisplayName = "Sand"),
    ETT_Mud             UMETA(DisplayName = "Mud"),
    ETT_Rock            UMETA(DisplayName = "Rock"),
    ETT_Grass           UMETA(DisplayName = "Grass"),
    ETT_Snow            UMETA(DisplayName = "Snow"),
    ETT_Water           UMETA(DisplayName = "Shallow Water"),
    ETT_Ice             UMETA(DisplayName = "Ice")
};

#include "Core_SurvivalPhysicsIntegrator.generated.h"