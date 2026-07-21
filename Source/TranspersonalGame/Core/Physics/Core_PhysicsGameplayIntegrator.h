#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Core_PhysicsGameplayIntegrator.generated.h"

/**
 * Core_PhysicsGameplayIntegrator
 * 
 * Master gameplay physics integration system for the prehistoric survival game.
 * Bridges core physics systems with gameplay mechanics to create realistic,
 * immersive survival experiences where physics directly impacts gameplay.
 * 
 * Key Responsibilities:
 * - Character physics integration with survival mechanics
 * - Dinosaur physics behavior and interaction systems
 * - Environmental physics affecting gameplay (terrain, weather, objects)
 * - Combat physics integration (impact, knockback, damage)
 * - Tool and weapon physics for crafting and hunting
 * - Performance optimization for gameplay-critical physics
 * 
 * This system ensures that physics feels natural and enhances the survival
 * experience rather than being purely cosmetic or disconnected from gameplay.
 */
UCLASS(BlueprintType, Blueprintable, Category = "Core Systems|Physics|Gameplay")
class TRANSPERSONALGAME_API ACore_PhysicsGameplayIntegrator : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsGameplayIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ========== CORE COMPONENTS ==========
    
    /** Root scene component for organization */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    /** Visual representation mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* IntegratorMesh;

    // ========== GAMEPLAY PHYSICS INTEGRATION ==========

    /** Enable character physics integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Integration")
    bool bEnableCharacterPhysicsIntegration;

    /** Enable dinosaur physics integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Integration")
    bool bEnableDinosaurPhysicsIntegration;

    /** Enable environmental physics integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Integration")
    bool bEnableEnvironmentalPhysicsIntegration;

    /** Enable combat physics integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Integration")
    bool bEnableCombatPhysicsIntegration;

    /** Enable tool physics integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Integration")
    bool bEnableToolPhysicsIntegration;

    // ========== CHARACTER PHYSICS INTEGRATION ==========

    /** Movement physics multiplier based on terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float TerrainMovementMultiplier;

    /** Stamina drain multiplier for physics-heavy actions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Physics", meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float PhysicsStaminaDrainMultiplier;

    /** Fall damage threshold (units per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Physics", meta = (ClampMin = "500.0", ClampMax = "2000.0"))
    float FallDamageThreshold;

    /** Maximum fall damage percentage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Physics", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float MaxFallDamagePercent;

    /** Enable realistic climbing physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Physics")
    bool bEnableRealisticClimbing;

    /** Climbing stamina cost per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Physics", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float ClimbingStaminaCost;

    // ========== DINOSAUR PHYSICS INTEGRATION ==========

    /** Dinosaur knockback force multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float DinosaurKnockbackMultiplier;

    /** Small dinosaur mass threshold (kg) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float SmallDinosaurMassThreshold;

    /** Large dinosaur mass threshold (kg) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float LargeDinosaurMassThreshold;

    /** Dinosaur charge impact force */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float DinosaurChargeImpactForce;

    /** Enable dinosaur environmental interaction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    bool bEnableDinosaurEnvironmentalInteraction;

    // ========== ENVIRONMENTAL PHYSICS ==========

    /** Water physics drag multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float WaterDragMultiplier;

    /** Mud physics slow multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float MudSlowMultiplier;

    /** Wind force multiplier for light objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float WindForceMultiplier;

    /** Temperature effect on physics (cold = brittle, heat = soft) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float TemperaturePhysicsMultiplier;

    // ========== COMBAT PHYSICS INTEGRATION ==========

    /** Weapon impact force multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Physics", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float WeaponImpactForceMultiplier;

    /** Projectile physics accuracy factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float ProjectileAccuracyFactor;

    /** Enable realistic weapon physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Physics")
    bool bEnableRealisticWeaponPhysics;

    /** Armor physics protection multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Physics", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float ArmorPhysicsProtection;

    // ========== TOOL PHYSICS INTEGRATION ==========

    /** Tool durability physics factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Physics", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float ToolDurabilityPhysicsFactor;

    /** Crafting physics precision requirement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Physics", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float CraftingPhysicsPrecision;

    /** Enable tool physics feedback */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Physics")
    bool bEnableToolPhysicsFeedback;

    // ========== PERFORMANCE MONITORING ==========

    /** Current physics gameplay integration load (0-100%) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float CurrentGameplayPhysicsLoad;

    /** Maximum allowed gameplay physics load */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "50.0", ClampMax = "100.0"))
    float MaxGameplayPhysicsLoad;

    /** Enable automatic performance optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoPerformanceOptimization;

    /** Physics integration update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "30.0", ClampMax = "120.0"))
    float PhysicsIntegrationUpdateFrequency;

    // ========== INTEGRATION STATUS ==========

    /** Number of active character physics integrations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Status")
    int32 ActiveCharacterPhysicsIntegrations;

    /** Number of active dinosaur physics integrations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Status")
    int32 ActiveDinosaurPhysicsIntegrations;

    /** Number of active environmental physics integrations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Status")
    int32 ActiveEnvironmentalPhysicsIntegrations;

    /** Number of active combat physics integrations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Status")
    int32 ActiveCombatPhysicsIntegrations;

    /** Total physics integration operations per second */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Status")
    float PhysicsIntegrationOperationsPerSecond;

public:
    // ========== CORE INTEGRATION FUNCTIONS ==========

    /** Initialize all physics gameplay integrations */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void InitializePhysicsGameplayIntegration();

    /** Update character physics integration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateCharacterPhysicsIntegration(float DeltaTime);

    /** Update dinosaur physics integration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateDinosaurPhysicsIntegration(float DeltaTime);

    /** Update environmental physics integration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateEnvironmentalPhysicsIntegration(float DeltaTime);

    /** Update combat physics integration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateCombatPhysicsIntegration(float DeltaTime);

    /** Update tool physics integration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateToolPhysicsIntegration(float DeltaTime);

    // ========== CHARACTER PHYSICS FUNCTIONS ==========

    /** Apply terrain physics effects to character movement */
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void ApplyTerrainPhysicsToCharacter(ACharacter* Character, float TerrainType);

    /** Calculate fall damage based on physics */
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    float CalculatePhysicsFallDamage(float FallVelocity, float CharacterMass);

    /** Apply climbing physics constraints */
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    bool ApplyClimbingPhysicsConstraints(ACharacter* Character, float ClimbAngle);

    // ========== DINOSAUR PHYSICS FUNCTIONS ==========

    /** Apply dinosaur charge physics */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyDinosaurChargePhysics(AActor* Dinosaur, AActor* Target, float ChargeForce);

    /** Calculate dinosaur knockback force */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    float CalculateDinosaurKnockback(float DinosaurMass, float ImpactVelocity);

    // ========== ENVIRONMENTAL PHYSICS FUNCTIONS ==========

    /** Apply water physics effects */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyWaterPhysicsEffects(AActor* Actor, float WaterDepth);

    /** Apply weather physics effects */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyWeatherPhysicsEffects(AActor* Actor, float WindStrength, float Temperature);

    // ========== COMBAT PHYSICS FUNCTIONS ==========

    /** Apply weapon impact physics */
    UFUNCTION(BlueprintCallable, Category = "Combat Physics")
    void ApplyWeaponImpactPhysics(AActor* Weapon, AActor* Target, float ImpactForce);

    /** Calculate projectile physics trajectory */
    UFUNCTION(BlueprintCallable, Category = "Combat Physics")
    FVector CalculateProjectilePhysicsTrajectory(FVector StartLocation, FVector TargetLocation, float ProjectileSpeed);

    // ========== PERFORMANCE FUNCTIONS ==========

    /** Optimize physics integration performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsIntegrationPerformance();

    /** Get current physics integration performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsIntegrationPerformanceMetrics();

    // ========== VALIDATION FUNCTIONS ==========

    /** Validate all physics gameplay integrations */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidatePhysicsGameplayIntegrations();

    /** Generate physics integration performance report */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void GeneratePhysicsIntegrationReport();

private:
    /** Internal update timer for performance optimization */
    float PhysicsIntegrationUpdateTimer;

    /** Internal performance tracking */
    float LastFramePhysicsTime;
    int32 PhysicsOperationsThisFrame;
};