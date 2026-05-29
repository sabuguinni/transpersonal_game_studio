// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsSystemValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsValidator, Log, All);

// Physics validation test types
UENUM(BlueprintType)
enum class EPhysicsValidationType : uint8
{
    ChaosEngine = 0,           // Chaos physics engine validation
    CollisionDetection = 1,    // Collision system validation
    RigidBodyDynamics = 2,     // Rigid body physics validation
    RagdollPhysics = 3,        // Ragdoll system validation
    DestructionSystem = 4,     // Destruction physics validation
    FluidSimulation = 5,       // Fluid physics validation
    ClothSimulation = 6,       // Cloth physics validation
    VehiclePhysics = 7,        // Vehicle physics validation
    PerformanceTest = 8,       // Physics performance validation
    NetworkedPhysics = 9       // Networked physics validation
};

// Physics validation results
UENUM(BlueprintType)
enum class EPhysicsValidationResult : uint8
{
    Passed = 0,                // Test passed successfully
    Failed = 1,                // Test failed
    Warning = 2,               // Test passed with warnings
    NotImplemented = 3,        // Test not implemented yet
    Skipped = 4                // Test skipped due to conditions
};

// Physics validation report entry
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsValidationEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EPhysicsValidationType TestType;

    UPROPERTY(BlueprintReadOnly)
    EPhysicsValidationResult Result;

    UPROPERTY(BlueprintReadOnly)
    FString TestName;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;

    FPhysicsValidationEntry()
        : TestType(EPhysicsValidationType::ChaosEngine)
        , Result(EPhysicsValidationResult::NotImplemented)
        , ExecutionTime(0.0f)
        , Timestamp(FDateTime::Now())
    {}
};

// Physics performance metrics
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConstraints = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CPUUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bWithinTargetFramerate = false;
};

// Delegate for validation completion
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsValidationComplete, 
    bool, bAllTestsPassed, const TArray<FPhysicsValidationEntry>&, ValidationResults);

/**
 * Physics System Validator
 * Comprehensive validation and testing system for all physics components
 * Validates Chaos physics, collision, ragdoll, destruction, and performance
 * Ensures physics systems meet AAA game standards for the dinosaur survival game
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsSystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemValidator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Run all physics validation tests */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void RunAllValidationTests();

    /** Run specific validation test */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void RunValidationTest(EPhysicsValidationType TestType);

    /** Get validation results */
    UFUNCTION(BlueprintPure, Category = "Physics Validation")
    const TArray<FPhysicsValidationEntry>& GetValidationResults() const { return ValidationResults; }

    /** Get performance metrics */
    UFUNCTION(BlueprintPure, Category = "Physics Validation")
    FPhysicsPerformanceMetrics GetPerformanceMetrics() const { return CurrentPerformanceMetrics; }

    /** Check if all tests passed */
    UFUNCTION(BlueprintPure, Category = "Physics Validation")
    bool AllTestsPassed() const;

    /** Get test count by result type */
    UFUNCTION(BlueprintPure, Category = "Physics Validation")
    int32 GetTestCountByResult(EPhysicsValidationResult Result) const;

    /** Clear validation results */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void ClearValidationResults();

    /** Set validation enabled/disabled */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void SetValidationEnabled(bool bEnabled) { bValidationEnabled = bEnabled; }

    /** Generate validation report */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FString GenerateValidationReport() const;

    /** Save validation report to file */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool SaveValidationReportToFile(const FString& FilePath) const;

    /** Validation completion event */
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsValidationComplete OnValidationComplete;

protected:
    /** Validation results array */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    TArray<FPhysicsValidationEntry> ValidationResults;

    /** Current performance metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPhysicsPerformanceMetrics CurrentPerformanceMetrics;

    /** Whether validation is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bValidationEnabled = true;

    /** Auto-run validation on begin play */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoRunValidation = true;

    /** Continuous performance monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bContinuousMonitoring = true;

    /** Performance monitoring interval */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float MonitoringInterval = 1.0f;

    /** Target framerate for performance validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFramerate = 60.0f;

    /** Maximum allowed physics tick time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MaxPhysicsTickTime = 16.67f; // 60fps

    /** Maximum allowed rigid bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 MaxRigidBodies = 1000;

    /** Maximum allowed memory usage (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MaxMemoryUsage = 512.0f;

private:
    // Validation test methods
    FPhysicsValidationEntry ValidateChaosEngine();
    FPhysicsValidationEntry ValidateCollisionDetection();
    FPhysicsValidationEntry ValidateRigidBodyDynamics();
    FPhysicsValidationEntry ValidateRagdollPhysics();
    FPhysicsValidationEntry ValidateDestructionSystem();
    FPhysicsValidationEntry ValidateFluidSimulation();
    FPhysicsValidationEntry ValidateClothSimulation();
    FPhysicsValidationEntry ValidateVehiclePhysics();
    FPhysicsValidationEntry ValidatePerformance();
    FPhysicsValidationEntry ValidateNetworkedPhysics();

    // Helper methods
    void UpdatePerformanceMetrics();
    bool CreateTestRigidBody();
    bool CreateTestRagdoll();
    bool CreateTestDestruction();
    bool TestCollisionResponse();
    bool TestPhysicsConstraints();
    bool TestPhysicsMaterials();
    
    // Performance monitoring
    void MonitorPerformance();
    float GetCurrentFramerate() const;
    float GetPhysicsTickTime() const;
    int32 GetActiveRigidBodyCount() const;
    float GetPhysicsMemoryUsage() const;

    // Internal state
    float LastMonitoringTime = 0.0f;
    TArray<float> FrameTimeHistory;
    TArray<float> PhysicsTimeHistory;
    
    // Test objects for validation
    UPROPERTY()
    TArray<TObjectPtr<AActor>> TestActors;
    
    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> TestMeshComponents;
};