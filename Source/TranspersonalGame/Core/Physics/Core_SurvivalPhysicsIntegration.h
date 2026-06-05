#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Engine/Eng_SurvivalArchitecture.h"
#include "SharedTypes.h"
#include "Core_SurvivalPhysicsIntegration.generated.h"

/**
 * Core Survival Physics Integration Component
 * Integrates survival mechanics with physics systems for realistic prehistoric gameplay
 * Connects survival stats (stamina, health, temperature) with movement, collision, and terrain physics
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_SurvivalPhysicsIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_SurvivalPhysicsIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === SURVIVAL ARCHITECTURE INTEGRATION ===
    
    /** Reference to survival architecture system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Integration")
    class UEng_SurvivalArchitecture* SurvivalArchitecture;

    /** Current survival stats */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Stats")
    FEng_SurvivalStats CurrentStats;

    /** Environmental factors affecting physics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental")
    FEng_EnvironmentalFactors EnvironmentalFactors;

    // === PHYSICS INTEGRATION PROPERTIES ===

    /** Stamina-based movement modifier (0.0 = exhausted, 1.0 = full energy) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Physics")
    float StaminaMovementModifier;

    /** Health-based physics response modifier */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Physics")
    float HealthPhysicsModifier;

    /** Temperature effect on movement speed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Physics")
    float TemperatureMovementPenalty;

    /** Fear level affecting movement precision */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychological Physics")
    float FearMovementImpact;

    // === TERRAIN INTERACTION ===

    /** Current terrain type affecting movement */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Physics")
    ETerrainType CurrentTerrainType;

    /** Terrain-based stamina drain multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TerrainStaminaDrainMultiplier;

    /** Terrain movement speed modifier */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Physics")
    float TerrainSpeedModifier;

    // === COLLISION INTEGRATION ===

    /** Impact damage threshold based on health */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float ImpactDamageThreshold;

    /** Fall damage multiplier based on health status */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision Physics")
    float FallDamageMultiplier;

    /** Collision recovery time based on stamina */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision Physics")
    float CollisionRecoveryTime;

    // === PERFORMANCE MONITORING ===

    /** Physics integration performance score (0-100) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float IntegrationPerformanceScore;

    /** Frame time impact of survival physics (milliseconds) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float FrameTimeImpact;

    /** Integration validation status */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bIntegrationValid;

public:
    // === CORE INTEGRATION METHODS ===

    /** Initialize survival physics integration with character */
    UFUNCTION(BlueprintCallable, Category = "Survival Integration")
    void InitializeSurvivalPhysics(class ACharacter* TargetCharacter);

    /** Update movement physics based on survival stats */
    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    void UpdateMovementPhysics(float DeltaTime);

    /** Process terrain interaction effects */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void ProcessTerrainInteraction(ETerrainType TerrainType, float DeltaTime);

    /** Handle collision impact with survival considerations */
    UFUNCTION(BlueprintCallable, Category = "Collision Integration")
    void ProcessSurvivalCollision(const FHitResult& HitResult, float ImpactForce);

    /** Calculate stamina drain for current activity */
    UFUNCTION(BlueprintCallable, Category = "Survival Calculation")
    float CalculateStaminaDrain(float MovementSpeed, ETerrainType Terrain, float DeltaTime);

    /** Apply environmental effects to physics */
    UFUNCTION(BlueprintCallable, Category = "Environmental Integration")
    void ApplyEnvironmentalPhysics(const FEng_EnvironmentalFactors& Environment);

    // === VALIDATION AND MONITORING ===

    /** Validate survival physics integration */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateIntegration();

    /** Get current integration performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceMetrics();

    /** Reset integration to default state */
    UFUNCTION(BlueprintCallable, Category = "Integration Control")
    void ResetIntegration();

    // === MILESTONE VALIDATION ===

    /** Validate Walk Around milestone physics requirements */
    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool ValidateWalkAroundPhysics();

    /** Get milestone completion status */
    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    float GetMilestoneCompletionPercentage();

protected:
    // === INTERNAL PROCESSING ===

    /** Update survival stats from architecture */
    void UpdateSurvivalStats();

    /** Calculate physics modifiers from survival state */
    void CalculatePhysicsModifiers();

    /** Process environmental impact on physics */
    void ProcessEnvironmentalImpact(float DeltaTime);

    /** Monitor integration performance */
    void MonitorPerformance(float DeltaTime);

    /** Validate architecture connection */
    bool ValidateArchitectureConnection();

private:
    /** Character reference for physics integration */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Character movement component reference */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Performance monitoring timer */
    float PerformanceTimer;

    /** Last frame time for performance calculation */
    float LastFrameTime;

    /** Integration initialization status */
    bool bInitialized;
};