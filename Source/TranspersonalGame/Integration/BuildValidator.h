#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/EngineSubsystem.h"
#include "HAL/PlatformFilemanager.h"
#include "BuildValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildValidator, Log, All);

/**
 * Build Validator for Transpersonal Game Studio
 * 
 * Validates build integrity across all systems:
 * - Module compilation and linking
 * - Asset reference validation
 * - Configuration file integrity
 * - Performance compliance
 * - Cross-system compatibility
 * 
 * Ensures that all 18 agent systems integrate properly
 * and the final build meets quality standards.
 */

UENUM(BlueprintType)
enum class EValidationSeverity : uint8
{
    Info,
    Warning,
    Error,
    Critical
};

UENUM(BlueprintType)
enum class EValidationType : uint8
{
    Module,
    Asset,
    Configuration,
    Performance,
    Compatibility,
    Integration
};

USTRUCT(BlueprintType)
struct FValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString TestName;

    UPROPERTY(BlueprintReadOnly)
    EValidationType Type;

    UPROPERTY(BlueprintReadOnly)
    EValidationSeverity Severity;

    UPROPERTY(BlueprintReadOnly)
    bool bPassed;

    UPROPERTY(BlueprintReadOnly)
    FString Message;

    UPROPERTY(BlueprintReadOnly)
    FString Details;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;

    FValidationResult()
    {
        TestName = TEXT("");
        Type = EValidationType::Module;
        Severity = EValidationSeverity::Info;
        bPassed = false;
        Message = TEXT("");
        Details = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FBuildValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<FValidationResult> Results;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalTests;

    UPROPERTY(BlueprintReadOnly)
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly)
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly)
    int32 CriticalErrors;

    UPROPERTY(BlueprintReadOnly)
    float TotalExecutionTime;

    UPROPERTY(BlueprintReadOnly)
    bool bBuildValid;

    UPROPERTY(BlueprintReadOnly)
    FDateTime GeneratedTime;

    FBuildValidationReport()
    {
        TotalTests = 0;
        PassedTests = 0;
        FailedTests = 0;
        CriticalErrors = 0;
        TotalExecutionTime = 0.0f;
        bBuildValid = false;
        GeneratedTime = FDateTime::Now();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, const FBuildValidationReport&, Report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnValidationProgress, int32, CompletedTests, int32, TotalTests);

/**
 * Build Validator Subsystem
 * Coordinates all build validation processes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidator : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Main Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunModuleValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunAssetValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunConfigurationValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunCompatibilityValidation();

    // Report Management
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuildValidationReport GetLastValidationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool SaveValidationReport(const FString& FilePath) const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ClearValidationHistory();

    // Validation Configuration
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetValidationEnabled(EValidationType Type, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsValidationEnabled(EValidationType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetPerformanceThresholds(float MinFPS, int32 MaxMemoryMB);

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnValidationComplete OnValidationComplete;

    UPROPERTY(BlueprintAssignable)
    FOnValidationProgress OnValidationProgress;

protected:
    // Validation State
    UPROPERTY()
    FBuildValidationReport CurrentReport;

    UPROPERTY()
    TArray<FBuildValidationReport> ValidationHistory;

    UPROPERTY()
    bool bValidationInProgress;

    // Configuration
    UPROPERTY()
    TMap<EValidationType, bool> ValidationTypeEnabled;

    UPROPERTY()
    float PerformanceMinFPS;

    UPROPERTY()
    int32 PerformanceMaxMemoryMB;

    // Module Validation
    void ValidateModuleCompilation();
    void ValidateModuleDependencies();
    void ValidateModuleLinking();
    FValidationResult ValidateIndividualModule(const FString& ModuleName);

    // Asset Validation
    void ValidateAssetReferences();
    void ValidateAssetIntegrity();
    void ValidateMissingAssets();
    FValidationResult ValidateIndividualAsset(const FString& AssetPath);

    // Configuration Validation
    void ValidateEngineConfiguration();
    void ValidateGameConfiguration();
    void ValidateInputConfiguration();
    void ValidateRenderingConfiguration();
    FValidationResult ValidateConfigurationFile(const FString& ConfigPath);

    // Performance Validation
    void ValidateFrameRate();
    void ValidateMemoryUsage();
    void ValidateRenderingPerformance();
    void ValidateLoadingTimes();

    // Compatibility Validation
    void ValidateSystemCompatibility();
    void ValidatePlatformCompatibility();
    void ValidateVersionCompatibility();

    // Utility Functions
    void AddValidationResult(const FValidationResult& Result);
    void UpdateValidationProgress();
    void FinalizeValidationReport();
    FString GetValidationSummary() const;

    // File System Utilities
    bool CheckFileExists(const FString& FilePath) const;
    bool CheckDirectoryExists(const FString& DirectoryPath) const;
    TArray<FString> GetFilesInDirectory(const FString& DirectoryPath, const FString& Extension = TEXT("")) const;
};

/**
 * Build Validation Utilities
 * Static helper functions for build validation
 */
UCLASS()
class TRANSPERSONALGAME_API UBuildValidationUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Module Utilities
    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static bool IsModuleCompiled(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static TArray<FString> GetAllProjectModules();

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static TArray<FString> GetModuleDependencies(const FString& ModuleName);

    // Asset Utilities
    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static bool ValidateAssetPath(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static TArray<FString> FindBrokenAssetReferences();

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static int32 GetAssetCount(const FString& AssetType = TEXT(""));

    // Performance Utilities
    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static float MeasureCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static int32 MeasureCurrentMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static float MeasureLoadTime(const FString& LevelPath);

    // System Utilities
    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static FString GetEngineVersion();

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static FString GetProjectVersion();

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static TArray<FString> GetInstalledPlatforms();

    // Report Utilities
    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static FString FormatValidationReport(const FBuildValidationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static bool ExportReportToHTML(const FBuildValidationReport& Report, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Build Validation|Utilities")
    static bool ExportReportToJSON(const FBuildValidationReport& Report, const FString& FilePath);
};