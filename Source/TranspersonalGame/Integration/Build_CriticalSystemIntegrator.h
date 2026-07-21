#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Build_CriticalSystemIntegrator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildCriticalSystemIntegrator, Log, All);

/**
 * Integration phases for critical system validation
 */
UENUM(BlueprintType)
enum class EBuild_CriticalIntegrationPhase : uint8
{
    Initializing        UMETA(DisplayName = "Initializing"),
    ValidatingCore      UMETA(DisplayName = "Validating Core"),
    CheckingDependencies UMETA(DisplayName = "Checking Dependencies"),
    TestingInteractions UMETA(DisplayName = "Testing Interactions"),
    ValidatingBuild     UMETA(DisplayName = "Validating Build"),
    GeneratingReport    UMETA(DisplayName = "Generating Report"),
    Complete           UMETA(DisplayName = "Complete"),
    Failed             UMETA(DisplayName = "Failed")
};

/**
 * Critical system integration metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CriticalIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 CoreSystemsValidated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 DependenciesResolved = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 InteractionTestsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 BuildValidationsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 CriticalErrorsFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 WarningsGenerated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    float IntegrationProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    bool bIntegrationSuccessful = false;

    FBuild_CriticalIntegrationMetrics()
    {
        CoreSystemsValidated = 0;
        DependenciesResolved = 0;
        InteractionTestsPassed = 0;
        BuildValidationsPassed = 0;
        CriticalErrorsFound = 0;
        WarningsGenerated = 0;
        IntegrationProgress = 0.0f;
        bIntegrationSuccessful = false;
    }
};

/**
 * Critical system integration result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CriticalIntegrationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Result")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Result")
    FString ResultMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Result")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Result")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Result")
    FBuild_CriticalIntegrationMetrics Metrics;

    FBuild_CriticalIntegrationResult()
    {
        bSuccess = false;
        ResultMessage = TEXT("Integration not started");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCriticalIntegrationComplete, const FBuild_CriticalIntegrationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalIntegrationPhaseChanged, EBuild_CriticalIntegrationPhase, NewPhase, float, Progress);

/**
 * Critical System Integrator - Handles integration of all critical game systems
 * Ensures all core systems work together properly and validates build integrity
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_CriticalSystemIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_CriticalSystemIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === INTEGRATION CONTROL ===
    
    /**
     * Start critical system integration process
     */
    UFUNCTION(BlueprintCallable, Category = "Critical Integration")
    void StartCriticalIntegration();

    /**
     * Stop integration process
     */
    UFUNCTION(BlueprintCallable, Category = "Critical Integration")
    void StopIntegration();

    /**
     * Force complete integration (emergency use)
     */
    UFUNCTION(BlueprintCallable, Category = "Critical Integration")
    void ForceCompleteIntegration();

    /**
     * Reset integration state
     */
    UFUNCTION(BlueprintCallable, Category = "Critical Integration")
    void ResetIntegration();

    // === INTEGRATION STATUS ===
    
    /**
     * Check if integration is complete
     */
    UFUNCTION(BlueprintPure, Category = "Critical Integration")
    bool IsIntegrationComplete() const;

    /**
     * Check if integration was successful
     */
    UFUNCTION(BlueprintPure, Category = "Critical Integration")
    bool IsIntegrationSuccessful() const;

    /**
     * Get current integration phase
     */
    UFUNCTION(BlueprintPure, Category = "Critical Integration")
    EBuild_CriticalIntegrationPhase GetCurrentPhase() const;

    /**
     * Get integration progress (0.0 to 1.0)
     */
    UFUNCTION(BlueprintPure, Category = "Critical Integration")
    float GetIntegrationProgress() const;

    /**
     * Get integration metrics
     */
    UFUNCTION(BlueprintPure, Category = "Critical Integration")
    FBuild_CriticalIntegrationMetrics GetIntegrationMetrics() const;

    /**
     * Get integration result
     */
    UFUNCTION(BlueprintPure, Category = "Critical Integration")
    FBuild_CriticalIntegrationResult GetIntegrationResult() const;

    // === EVENTS ===
    
    UPROPERTY(BlueprintAssignable, Category = "Critical Integration")
    FOnCriticalIntegrationComplete OnIntegrationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Critical Integration")
    FOnCriticalIntegrationPhaseChanged OnPhaseChanged;

protected:
    // === INTEGRATION PHASES ===
    
    void ProcessIntegrationPhase();
    void ValidateCoreSystemsPhase();
    void CheckDependenciesPhase();
    void TestInteractionsPhase();
    void ValidateBuildPhase();
    void GenerateReportPhase();
    void CompleteIntegration();
    void FailIntegration(const FString& Reason);

    // === VALIDATION METHODS ===
    
    bool ValidateCharacterSystem();
    bool ValidateWorldGeneration();
    bool ValidateFoliageSystem();
    bool ValidateCrowdSimulation();
    bool ValidateGameState();
    bool ValidateModuleDependencies();
    bool ValidateAssetReferences();
    bool ValidateSystemInteractions();
    bool ValidateBuildIntegrity();

    // === UTILITY METHODS ===
    
    void UpdateProgress();
    void LogIntegrationStep(const FString& Step, bool bSuccess);
    void AddCriticalError(const FString& Error);
    void AddWarning(const FString& Warning);

protected:
    // === INTEGRATION STATE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    EBuild_CriticalIntegrationPhase CurrentPhase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    bool bIntegrationActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    bool bIntegrationComplete;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    bool bIntegrationSuccessful;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration State")
    float IntegrationProgress;

    // === INTEGRATION METRICS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Metrics")
    FBuild_CriticalIntegrationMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Result")
    FBuild_CriticalIntegrationResult IntegrationResult;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Config")
    float PhaseTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Config")
    int32 MaxAllowedCriticalErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Config")
    int32 MaxAllowedWarnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Config")
    bool bAutoStartOnBeginPlay;

    // === INTERNAL STATE ===
    
    float PhaseStartTime;
    int32 CurrentPhaseStep;
    int32 TotalPhaseSteps;
};