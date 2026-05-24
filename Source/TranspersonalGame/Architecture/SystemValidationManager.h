#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "SystemValidationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnValidationComplete, bool, bSuccess, int32, ErrorCount, FString, Summary);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalValidationFailure, FString, SystemName, FString, FailureReason);

/**
 * System Validation Manager
 * Validates system integrity, dependencies, and compliance with architecture rules
 * Works in conjunction with EngineArchitectureManager to ensure system health
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USystemValidationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    USystemValidationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateSystemPerformance(const FString& SystemName);

    // Specific System Validations
    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateWorldPartitionSetup();

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateLumenConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateNiagaraSystemsCompliance();

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateMetaSoundsCompliance();

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool ValidateMassAICompliance();

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool TestSystemInteraction(const FString& SystemA, const FString& SystemB);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting", CallInEditor)
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting", CallInEditor)
    FString GetLastValidationSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting", CallInEditor)
    TArray<FString> GetValidationErrors() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableContinuousValidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationIntervalSeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bFailOnFirstError = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bGenerateDetailedReports = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxValidationErrors = 50;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnValidationComplete OnValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCriticalValidationFailure OnCriticalValidationFailure;

protected:
    // Validation state tracking
    UPROPERTY()
    TMap<FString, FEng_ValidationResult> ValidationResults;

    UPROPERTY()
    TArray<FString> ValidationErrors;

    UPROPERTY()
    TArray<FString> ValidationWarnings;

    UPROPERTY()
    FString LastValidationSummary;

    UPROPERTY()
    bool bLastValidationSuccessful = true;

    UPROPERTY()
    int32 TotalValidationRuns = 0;

    // Internal validation methods
    bool ValidateActorComponents(AActor* Actor);
    bool ValidateComponentDependencies(UActorComponent* Component);
    bool ValidateSystemConfiguration(const FString& SystemName);
    bool CheckArchitectureCompliance(const FString& SystemName);
    bool ValidatePerformanceConstraints(const FString& SystemName);

    // Specific validation implementations
    bool ValidateWorldSettings();
    bool ValidateRenderingPipeline();
    bool ValidatePhysicsConfiguration();
    bool ValidateAudioConfiguration();
    bool ValidateAIConfiguration();

    // Error handling
    void LogValidationError(const FString& SystemName, const FString& ErrorMessage);
    void LogValidationWarning(const FString& SystemName, const FString& WarningMessage);
    void HandleCriticalFailure(const FString& SystemName, const FString& FailureReason);

    // Timer handle for continuous validation
    FTimerHandle ValidationTimerHandle;

private:
    // Validation constants
    static constexpr int32 MAX_VALIDATION_DEPTH = 10;
    static constexpr float CRITICAL_PERFORMANCE_THRESHOLD = 100.0f; // 10 FPS
    
    int32 CurrentValidationDepth = 0;
    bool bValidationInProgress = false;
};