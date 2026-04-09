// Copyright Transpersonal Game Studio. All Rights Reserved.
// EngineArchitectureValidator.h - Validates engine configuration and architectural compliance

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TechnicalArchitecture.h"
#include "EngineArchitectureValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitectureValidator, Log, All);

/**
 * Validation result structure for architecture compliance checks
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchitectureValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTimestamp = 0.0f;

    FArchitectureValidationResult()
    {
        bIsValid = false;
        ValidationMessage = TEXT("Not validated");
        ValidationTimestamp = 0.0f;
    }
};

/**
 * Engine feature validation status
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngineFeatureStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bNaniteEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bLumenEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bVirtualShadowMapsEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bWorldPartitionEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bMassEntityEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bChaosPhysicsEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bMetaSoundsEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Features")
    bool bNiagaraEnabled = false;

    FEngineFeatureStatus()
    {
        bNaniteEnabled = false;
        bLumenEnabled = false;
        bVirtualShadowMapsEnabled = false;
        bWorldPartitionEnabled = false;
        bMassEntityEnabled = false;
        bChaosPhysicsEnabled = false;
        bMetaSoundsEnabled = false;
        bNiagaraEnabled = false;
    }
};

/**
 * Performance validation metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceValidationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetMemoryMB = 8192.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxDrawCalls = 5000;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bWithinTargets = false;

    FPerformanceValidationMetrics()
    {
        CurrentFrameRate = 0.0f;
        TargetFrameRate = TPSArchitecture::TARGET_FRAMERATE_PC;
        MemoryUsageMB = 0.0f;
        TargetMemoryMB = TPSArchitecture::TARGET_MEMORY_CONSOLE_MB;
        ActiveDrawCalls = 0;
        MaxDrawCalls = 5000;
        bWithinTargets = false;
    }
};

/**
 * Engine Architecture Validator Subsystem
 * 
 * This subsystem validates that the engine configuration meets the architectural
 * requirements defined by the Engine Architect. It performs continuous monitoring
 * and validation of critical engine features and performance metrics.
 */
UCLASS()
class TRANSPERSONALGAME_API UEngineArchitectureValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FArchitectureValidationResult ValidateFullArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FEngineFeatureStatus ValidateEngineFeatures();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FPerformanceValidationMetrics ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateNaniteConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateLumenConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateVirtualShadowMapsConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateWorldPartitionConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateMassEntityConfiguration();

    // Continuous Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    void StartContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    void StopContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool IsContinuousValidationActive() const { return bContinuousValidationActive; }

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    void GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    TArray<FString> GetValidationHistory() const { return ValidationHistory; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationCompleted, FArchitectureValidationResult, Result);
    UPROPERTY(BlueprintAssignable, Category = "Architecture Validation")
    FOnValidationCompleted OnValidationCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationFailed, FString, ErrorMessage);
    UPROPERTY(BlueprintAssignable, Category = "Architecture Validation")
    FOnValidationFailed OnValidationFailed;

protected:
    // Internal validation methods
    bool ValidateProjectSettings();
    bool ValidateRenderingSettings();
    bool ValidatePhysicsSettings();
    bool ValidateAudioSettings();
    bool ValidateWorldSettings();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();

    // Logging and reporting
    void LogValidationResult(const FArchitectureValidationResult& Result);
    void AddValidationEntry(const FString& Message);

private:
    // Validation state
    UPROPERTY()
    bool bValidationInitialized = false;

    UPROPERTY()
    bool bContinuousValidationActive = false;

    UPROPERTY()
    float ValidationInterval = 5.0f; // Validate every 5 seconds

    UPROPERTY()
    FTimerHandle ValidationTimerHandle;

    // Cached validation results
    UPROPERTY()
    FArchitectureValidationResult LastValidationResult;

    UPROPERTY()
    FEngineFeatureStatus LastFeatureStatus;

    UPROPERTY()
    FPerformanceValidationMetrics LastPerformanceMetrics;

    // Validation history
    UPROPERTY()
    TArray<FString> ValidationHistory;

    UPROPERTY()
    int32 MaxHistoryEntries = 100;

    // Internal timer callback
    void PerformScheduledValidation();
};