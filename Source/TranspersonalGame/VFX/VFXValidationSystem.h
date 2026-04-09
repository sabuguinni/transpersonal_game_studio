// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "VFXValidationSystem.generated.h"

/**
 * Validation result structure for VFX systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly)
    float PerformanceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly)
    FDateTime ValidationTime;

    FVFXValidationResult()
    {
        ValidationTime = FDateTime::Now();
    }
};

/**
 * VFX validation rules and criteria
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXValidationCriteria
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFPSImpact = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxMemoryUsageMB = 256.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticleCount = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxGPUUsagePercent = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequireNiagaraCompatibility = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequireLODSupport = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequirePoolingSupport = true;
};

/**
 * Automated VFX validation and quality assurance system
 * Continuously monitors VFX systems for performance and quality issues
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFXValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FVFXValidationResult ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FVFXValidationResult ValidateNiagaraSystem(UNiagaraSystem* NiagaraSystem);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FVFXValidationResult ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FVFXValidationResult ValidateVFXMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool RunComprehensiveValidation();

    // Automated monitoring
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    void StartContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    void StopContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool IsContinuousValidationActive() const { return bContinuousValidationActive; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    void SetValidationCriteria(const FVFXValidationCriteria& NewCriteria);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FVFXValidationCriteria GetValidationCriteria() const { return ValidationCriteria; }

    // Results and reporting
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    TArray<FVFXValidationResult> GetValidationHistory() const { return ValidationHistory; }

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FVFXValidationResult GetLastValidationResult() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    void SaveValidationReport(const FString& Filename);

    // Quality metrics
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    float GetOverallVFXQualityScore();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    TArray<FString> GetActiveVFXWarnings();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    TArray<FString> GetActiveVFXErrors();

protected:
    // Validation criteria
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    FVFXValidationCriteria ValidationCriteria;

    // Continuous validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    int32 MaxValidationHistorySize = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoFixIssues = false;

    // Validation state
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    bool bContinuousValidationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    TArray<FVFXValidationResult> ValidationHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    FDateTime LastValidationTime;

private:
    // Internal validation functions
    FVFXValidationResult ValidateSystemIntegrity();
    FVFXValidationResult ValidatePerformanceMetrics();
    FVFXValidationResult ValidateAssetQuality();
    FVFXValidationResult ValidateLODConfiguration();
    FVFXValidationResult ValidatePoolingSystem();

    // Performance monitoring
    float GetCurrentFPS();
    float GetVFXMemoryUsage();
    float GetGPUUsage();
    int32 GetActiveParticleCount();

    // Issue detection and resolution
    void DetectPerformanceIssues(FVFXValidationResult& Result);
    void DetectMemoryLeaks(FVFXValidationResult& Result);
    void DetectAssetIssues(FVFXValidationResult& Result);
    bool AttemptAutoFix(const FVFXValidationResult& Result);

    // Validation history management
    void AddValidationResult(const FVFXValidationResult& Result);
    void TrimValidationHistory();

    // Timer for continuous validation
    FTimerHandle ValidationTimerHandle;
    void PerformScheduledValidation();
};

/**
 * VFX validation utilities and helper functions
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFXValidationUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool ValidateNiagaraAsset(UNiagaraSystem* NiagaraSystem, FString& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool ValidateMaterialAsset(UMaterialInterface* Material, FString& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static TArray<FString> GetMissingVFXAssets();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool IsVFXPerformanceAcceptable(float FPSThreshold = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static float CalculateVFXPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool IsVFXMemoryUsageAcceptable(float MemoryThresholdMB = 256.0f);

    // System validation
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool AreVFXSystemsInitialized();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool IsNiagaraSystemAvailable();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static TArray<FString> GetVFXSystemStatus();

    // Quality assurance
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static bool RunVFXQualityCheck();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static FString GenerateVFXQualityReport();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    static void ApplyVFXQualitySettings();
};

/**
 * VFX validation event delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVFXValidationComplete, const FVFXValidationResult&, ValidationResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVFXValidationError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVFXPerformanceIssue, float, PerformanceImpact);

/**
 * VFX validation event handler
 * Responds to validation events and can trigger automated responses
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFXValidationEventHandler : public UObject
{
    GENERATED_BODY()

public:
    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "VFX Validation Events")
    FOnVFXValidationComplete OnValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "VFX Validation Events")
    FOnVFXValidationError OnValidationError;

    UPROPERTY(BlueprintAssignable, Category = "VFX Validation Events")
    FOnVFXPerformanceIssue OnPerformanceIssue;

    // Event handlers
    UFUNCTION(BlueprintCallable, Category = "VFX Validation Events")
    void HandleValidationComplete(const FVFXValidationResult& Result);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation Events")
    void HandleValidationError(const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "VFX Validation Events")
    void HandlePerformanceIssue(float PerformanceImpact);

    // Automated responses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automated Responses")
    bool bAutoReduceQualityOnPerformanceIssue = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automated Responses")
    bool bAutoRestartVFXSystemOnError = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automated Responses")
    bool bAutoSaveValidationLogs = true;

private:
    void TriggerAutomatedResponse(const FVFXValidationResult& Result);
};