#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsValidationComponent.generated.h"

/** Physics validation test types */
UENUM(BlueprintType)
enum class ECore_PhysicsValidationType : uint8
{
    CollisionAccuracy       UMETA(DisplayName = "Collision Accuracy"),
    RigidBodyStability      UMETA(DisplayName = "Rigid Body Stability"),
    DestructionIntegrity    UMETA(DisplayName = "Destruction Integrity"),
    PerformanceMetrics      UMETA(DisplayName = "Performance Metrics"),
    NetworkSynchronization  UMETA(DisplayName = "Network Sync"),
    MemoryUsage            UMETA(DisplayName = "Memory Usage")
};

/** Physics validation result structure */
USTRUCT(BlueprintType)
struct FPhysicsValidationResult
{
    GENERATED_BODY()

    /** Whether validation passed */
    UPROPERTY(BlueprintReadOnly)
    bool bValidationPassed = false;

    /** Validation score (0-100) */
    UPROPERTY(BlueprintReadOnly)
    float ValidationScore = 0.0f;

    /** Error messages if validation failed */
    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ErrorMessages;

    /** Performance metrics */
    UPROPERTY(BlueprintReadOnly)
    float FrameTime = 0.0f;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly)
    float MemoryUsage = 0.0f;

    /** Number of active physics bodies */
    UPROPERTY(BlueprintReadOnly)
    int32 ActiveBodies = 0;
};

/** Delegate for validation completion */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPhysicsValidationDelegate, ECore_PhysicsValidationType, ValidationType, const FPhysicsValidationResult&, Result);

/**
 * @brief Physics validation component for testing and debugging physics systems
 * 
 * Provides comprehensive validation and testing capabilities for:
 * - Collision detection accuracy
 * - Rigid body simulation stability
 * - Destruction system integrity
 * - Performance metrics monitoring
 * - Network synchronization validation
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsValidationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsValidationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Run comprehensive physics validation
     * 
     * @param ValidationType Type of validation to perform
     * @return Validation result with detailed metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FPhysicsValidationResult RunPhysicsValidation(ECore_PhysicsValidationType ValidationType);

    /**
     * @brief Run all physics validation tests
     * 
     * @return Array of all validation results
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    TArray<FPhysicsValidationResult> RunAllValidationTests();

    /**
     * @brief Test collision detection accuracy
     * 
     * @param TestActorA First test actor
     * @param TestActorB Second test actor
     * @param ExpectedResult Expected collision result
     * @return Validation result for collision test
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FPhysicsValidationResult TestCollisionAccuracy(AActor* TestActorA, AActor* TestActorB, bool ExpectedResult);

    /**
     * @brief Test rigid body simulation stability
     * 
     * @param TestActor Actor to test stability for
     * @param TestDuration Duration of stability test in seconds
     * @return Validation result for stability test
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FPhysicsValidationResult TestRigidBodyStability(AActor* TestActor, float TestDuration = 5.0f);

    /**
     * @brief Test destruction system integrity
     * 
     * @param DestructibleActor Actor with destruction components
     * @param ImpactForce Force to apply for destruction test
     * @return Validation result for destruction test
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FPhysicsValidationResult TestDestructionIntegrity(AActor* DestructibleActor, float ImpactForce);

    /**
     * @brief Monitor performance metrics
     * 
     * @param MonitorDuration Duration to monitor in seconds
     * @return Validation result with performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FPhysicsValidationResult MonitorPerformanceMetrics(float MonitorDuration = 10.0f);

    /**
     * @brief Set validation thresholds
     * 
     * @param FrameTimeThreshold Maximum acceptable frame time in ms
     * @param MemoryThreshold Maximum acceptable memory usage in MB
     * @param BodyCountThreshold Maximum acceptable active body count
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void SetValidationThresholds(float FrameTimeThreshold, float MemoryThreshold, int32 BodyCountThreshold);

    /**
     * @brief Enable continuous validation monitoring
     * 
     * @param bEnabled Whether to enable continuous monitoring
     * @param MonitorInterval Interval between validation checks in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void SetContinuousValidation(bool bEnabled, float MonitorInterval = 1.0f);

protected:
    /** Maximum acceptable frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float MaxFrameTime = 16.67f; // 60 FPS

    /** Maximum acceptable memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float MaxMemoryUsage = 512.0f;

    /** Maximum acceptable active physics body count */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    int32 MaxActiveBodyCount = 1000;

    /** Enable detailed logging of validation results */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bEnableDetailedLogging = true;

    /** Validation completion event */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPhysicsValidationDelegate OnValidationComplete;

private:
    /** Validate collision detection system */
    FPhysicsValidationResult ValidateCollisionSystem();
    
    /** Validate rigid body simulation */
    FPhysicsValidationResult ValidateRigidBodySimulation();
    
    /** Validate destruction system */
    FPhysicsValidationResult ValidateDestructionSystem();
    
    /** Collect performance metrics */
    FPhysicsValidationResult CollectPerformanceMetrics();
    
    /** Validate network synchronization */
    FPhysicsValidationResult ValidateNetworkSync();
    
    /** Check memory usage */
    FPhysicsValidationResult CheckMemoryUsage();

    /** Log validation result */
    void LogValidationResult(ECore_PhysicsValidationType ValidationType, const FPhysicsValidationResult& Result);

    /** Continuous validation timer */
    FTimerHandle ValidationTimer;
    
    /** Current validation in progress */
    bool bValidationInProgress = false;
    
    /** Validation start time */
    double ValidationStartTime = 0.0;
    
    /** Performance monitoring data */
    TArray<float> FrameTimeHistory;
    TArray<float> MemoryUsageHistory;
    TArray<int32> BodyCountHistory;
};