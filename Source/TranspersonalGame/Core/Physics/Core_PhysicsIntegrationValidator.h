#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Core_PhysicsIntegrationValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsIntegrationValidator, Log, All);

/**
 * Performance validation levels for physics integration
 */
UENUM(BlueprintType)
enum class ECore_ValidationLevel : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Physics integration test types
 */
UENUM(BlueprintType)
enum class ECore_IntegrationTestType : uint8
{
    CharacterMovement       UMETA(DisplayName = "Character Movement"),
    TerrainInteraction     UMETA(DisplayName = "Terrain Interaction"),
    CollisionDetection     UMETA(DisplayName = "Collision Detection"),
    RigidBodySimulation    UMETA(DisplayName = "Rigid Body Simulation"),
    ConstraintSystem       UMETA(DisplayName = "Constraint System"),
    DestructionSystem      UMETA(DisplayName = "Destruction System"),
    FluidDynamics          UMETA(DisplayName = "Fluid Dynamics"),
    PerformanceValidation  UMETA(DisplayName = "Performance Validation")
};

/**
 * Integration test result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_IntegrationTestResult
{
    GENERATED_BODY()

    /** Test type that was executed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Result")
    ECore_IntegrationTestType TestType;

    /** Validation level of the result */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Result")
    ECore_ValidationLevel ValidationLevel;

    /** Test execution time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Result")
    float ExecutionTime;

    /** Success rate as percentage (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Result")
    float SuccessRate;

    /** Detailed test message */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Result")
    FString TestMessage;

    /** Performance metrics collected during test */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Result")
    TMap<FString, float> PerformanceMetrics;

    FCore_IntegrationTestResult()
    {
        TestType = ECore_IntegrationTestType::CharacterMovement;
        ValidationLevel = ECore_ValidationLevel::Pass;
        ExecutionTime = 0.0f;
        SuccessRate = 100.0f;
        TestMessage = TEXT("Test not executed");
    }
};

/**
 * Physics integration validation configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_ValidationConfig
{
    GENERATED_BODY()

    /** Enable automatic validation on level start */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bAutoValidateOnStart;

    /** Enable continuous validation during gameplay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bContinuousValidation;

    /** Validation interval in seconds for continuous mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    float ValidationInterval;

    /** Performance threshold for warning level (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    float PerformanceWarningThreshold;

    /** Performance threshold for error level (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    float PerformanceErrorThreshold;

    /** Maximum allowed memory usage (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    float MaxMemoryUsage;

    /** Target frame rate for performance validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    float TargetFrameRate;

    /** Enable detailed logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bEnableDetailedLogging;

    /** Save validation results to file */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bSaveResultsToFile;

    FCore_ValidationConfig()
    {
        bAutoValidateOnStart = true;
        bContinuousValidation = false;
        ValidationInterval = 10.0f;
        PerformanceWarningThreshold = 8.33f; // 120 FPS
        PerformanceErrorThreshold = 16.67f; // 60 FPS
        MaxMemoryUsage = 512.0f; // 512 MB
        TargetFrameRate = 60.0f;
        bEnableDetailedLogging = true;
        bSaveResultsToFile = true;
    }
};

/**
 * Delegate for validation events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationCompleted, const TArray<FCore_IntegrationTestResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnValidationFailed, ECore_IntegrationTestType, TestType, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceThresholdExceeded, ECore_ValidationLevel, Level, const FString&, MetricName);

/**
 * Physics Integration Validator Component
 * 
 * Validates the integration and performance of all physics systems in the game.
 * Ensures that character movement, terrain interaction, collision detection,
 * and other physics systems work correctly together and meet performance targets.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegrationValidator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Start comprehensive physics integration validation */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    void StartValidation();

    /** Stop ongoing validation */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    void StopValidation();

    /** Run specific integration test */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    FCore_IntegrationTestResult RunIntegrationTest(ECore_IntegrationTestType TestType);

    /** Get all validation results */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    TArray<FCore_IntegrationTestResult> GetValidationResults() const;

    /** Get validation configuration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    FCore_ValidationConfig GetValidationConfig() const;

    /** Set validation configuration */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    void SetValidationConfig(const FCore_ValidationConfig& NewConfig);

    /** Check if validation is currently running */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    bool IsValidationRunning() const;

    /** Get overall validation status */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    ECore_ValidationLevel GetOverallValidationStatus() const;

    /** Reset validation results */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    void ResetValidation();

    /** Export validation results to file */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Validator")
    bool ExportValidationResults(const FString& FilePath = TEXT(""));

protected:
    /** Run character movement integration test */
    FCore_IntegrationTestResult ValidateCharacterMovement();

    /** Run terrain interaction integration test */
    FCore_IntegrationTestResult ValidateTerrainInteraction();

    /** Run collision detection integration test */
    FCore_IntegrationTestResult ValidateCollisionDetection();

    /** Run rigid body simulation integration test */
    FCore_IntegrationTestResult ValidateRigidBodySimulation();

    /** Run constraint system integration test */
    FCore_IntegrationTestResult ValidateConstraintSystem();

    /** Run destruction system integration test */
    FCore_IntegrationTestResult ValidateDestructionSystem();

    /** Run fluid dynamics integration test */
    FCore_IntegrationTestResult ValidateFluidDynamics();

    /** Run performance validation test */
    FCore_IntegrationTestResult ValidatePerformance();

    /** Update continuous validation */
    void UpdateContinuousValidation(float DeltaTime);

    /** Check performance thresholds */
    void CheckPerformanceThresholds();

    /** Log validation results */
    void LogValidationResults(const TArray<FCore_IntegrationTestResult>& Results);

    /** Save validation results to file */
    void SaveValidationResultsToFile();

    /** Get current frame time */
    float GetCurrentFrameTime() const;

    /** Get current memory usage */
    float GetCurrentMemoryUsage() const;

    /** Get physics system status */
    bool IsPhysicsSystemHealthy() const;

protected:
    /** Validation configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration Validator", meta = (AllowPrivateAccess = "true"))
    FCore_ValidationConfig ValidationConfig;

    /** Current validation results */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration Validator", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_IntegrationTestResult> ValidationResults;

    /** Is validation currently running */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration Validator", meta = (AllowPrivateAccess = "true"))
    bool bIsValidationRunning;

    /** Time since last validation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration Validator", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastValidation;

    /** Overall validation status */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration Validator", meta = (AllowPrivateAccess = "true"))
    ECore_ValidationLevel OverallValidationStatus;

    /** Validation start time */
    FDateTime ValidationStartTime;

    /** Frame time history for performance analysis */
    TArray<float> FrameTimeHistory;

    /** Memory usage history */
    TArray<float> MemoryUsageHistory;

public:
    /** Event fired when validation is completed */
    UPROPERTY(BlueprintAssignable, Category = "Physics Integration Validator")
    FOnValidationCompleted OnValidationCompleted;

    /** Event fired when validation fails */
    UPROPERTY(BlueprintAssignable, Category = "Physics Integration Validator")
    FOnValidationFailed OnValidationFailed;

    /** Event fired when performance threshold is exceeded */
    UPROPERTY(BlueprintAssignable, Category = "Physics Integration Validator")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;
};