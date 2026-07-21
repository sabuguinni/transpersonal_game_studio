#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsValidation.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsValidation, Log, All);

/**
 * Physics validation result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsValidationResult
{
    GENERATED_BODY()

    /** Whether the validation passed */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    /** Validation error message */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    /** Performance score (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float PerformanceScore = 0.0f;

    /** Number of physics objects validated */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ObjectCount = 0;

    /** Validation timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime ValidationTime;

    FCore_PhysicsValidationResult()
    {
        bIsValid = false;
        ErrorMessage = TEXT("Not validated");
        PerformanceScore = 0.0f;
        ObjectCount = 0;
        ValidationTime = FDateTime::Now();
    }
};

/**
 * Physics validation test type enumeration
 */
UENUM(BlueprintType)
enum class ECore_PhysicsValidationType : uint8
{
    CollisionDetection      UMETA(DisplayName = "Collision Detection"),
    RigidBodySimulation     UMETA(DisplayName = "Rigid Body Simulation"),
    ConstraintValidation    UMETA(DisplayName = "Constraint Validation"),
    PerformanceTest         UMETA(DisplayName = "Performance Test"),
    MemoryUsageTest         UMETA(DisplayName = "Memory Usage Test"),
    StabilityTest           UMETA(DisplayName = "Stability Test"),
    AccuracyTest            UMETA(DisplayName = "Accuracy Test"),
    IntegrationTest         UMETA(DisplayName = "Integration Test")
};

/**
 * Physics validation component for testing and validating physics systems
 * Ensures physics systems meet performance and accuracy requirements
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsValidation : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsValidation();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    // Validation Methods
    
    /** Run comprehensive physics validation */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FCore_PhysicsValidationResult RunPhysicsValidation();

    /** Run specific validation test */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FCore_PhysicsValidationResult RunValidationTest(ECore_PhysicsValidationType TestType);

    /** Validate collision detection system */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateCollisionDetection();

    /** Validate rigid body simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateRigidBodySimulation();

    /** Validate physics constraints */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidatePhysicsConstraints();

    /** Run physics performance test */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    float RunPerformanceTest();

    /** Check physics memory usage */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    float CheckMemoryUsage();

    /** Test physics system stability */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool TestSystemStability();

    /** Test physics accuracy */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool TestPhysicsAccuracy();

    /** Run integration tests */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool RunIntegrationTests();

    // Utility Methods

    /** Get validation statistics */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FString GetValidationStatistics() const;

    /** Reset validation data */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void ResetValidationData();

    /** Enable/disable continuous validation */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void SetContinuousValidation(bool bEnabled);

    /** Get last validation result */
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FCore_PhysicsValidationResult GetLastValidationResult() const { return LastValidationResult; }

protected:
    // Validation Configuration

    /** Enable automatic validation on startup */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidateOnStart = true;

    /** Enable continuous validation during gameplay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bContinuousValidation = false;

    /** Validation interval for continuous testing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings", 
        meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float ValidationInterval = 5.0f;

    /** Performance threshold for validation pass */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings",
        meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PerformanceThreshold = 60.0f;

    /** Memory usage threshold (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings",
        meta = (ClampMin = "1.0", ClampMax = "1000.0"))
    float MemoryThreshold = 100.0f;

    /** Enable detailed logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bDetailedLogging = true;

    // Validation State

    /** Last validation result */
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    FCore_PhysicsValidationResult LastValidationResult;

    /** Validation timer handle */
    FTimerHandle ValidationTimerHandle;

    /** Validation start time */
    double ValidationStartTime = 0.0;

    /** Total validation runs */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Statistics")
    int32 TotalValidationRuns = 0;

    /** Successful validation runs */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Statistics")
    int32 SuccessfulValidationRuns = 0;

    /** Failed validation runs */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Statistics")
    int32 FailedValidationRuns = 0;

    /** Average performance score */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Statistics")
    float AveragePerformanceScore = 0.0f;

private:
    // Internal Validation Methods

    /** Initialize validation system */
    void InitializeValidation();

    /** Cleanup validation system */
    void CleanupValidation();

    /** Perform continuous validation tick */
    void PerformContinuousValidation();

    /** Update validation statistics */
    void UpdateValidationStatistics(const FCore_PhysicsValidationResult& Result);

    /** Create test physics objects */
    TArray<AActor*> CreateTestPhysicsObjects();

    /** Cleanup test physics objects */
    void CleanupTestPhysicsObjects(const TArray<AActor*>& TestObjects);

    /** Measure physics simulation performance */
    float MeasureSimulationPerformance(const TArray<AActor*>& TestObjects);

    /** Test collision accuracy */
    bool TestCollisionAccuracy(const TArray<AActor*>& TestObjects);

    /** Test constraint stability */
    bool TestConstraintStability(const TArray<AActor*>& TestObjects);

    /** Validate physics settings */
    bool ValidatePhysicsSettings() const;

    /** Check for physics errors */
    bool CheckForPhysicsErrors() const;

    /** Generate validation report */
    FString GenerateValidationReport(const FCore_PhysicsValidationResult& Result) const;

    // Test Object Management
    TArray<TWeakObjectPtr<AActor>> ActiveTestObjects;
    
    // Performance Tracking
    TArray<float> PerformanceHistory;
    TArray<float> MemoryUsageHistory;
    
    // Error Tracking
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
};