// Copyright Transpersonal Game Studio. All Rights Reserved.
// BuildIntegrationManager.h - Manages build integration and compilation validation

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

/**
 * Build configuration information
 */
UENUM(BlueprintType)
enum class EBuildConfiguration : uint8
{
    Debug           UMETA(DisplayName = "Debug"),
    DebugGame       UMETA(DisplayName = "DebugGame"),
    Development     UMETA(DisplayName = "Development"),
    Test            UMETA(DisplayName = "Test"),
    Shipping        UMETA(DisplayName = "Shipping")
};

/**
 * Build target platform
 */
UENUM(BlueprintType)
enum class EBuildTargetPlatform : uint8
{
    Win64           UMETA(DisplayName = "Windows 64-bit"),
    Mac             UMETA(DisplayName = "macOS"),
    Linux           UMETA(DisplayName = "Linux"),
    Android         UMETA(DisplayName = "Android"),
    IOS             UMETA(DisplayName = "iOS"),
    PS5             UMETA(DisplayName = "PlayStation 5"),
    XboxSeriesX     UMETA(DisplayName = "Xbox Series X|S")
};

/**
 * Build status information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bLinkingSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float BuildTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime BuildStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime BuildEndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EBuildConfiguration BuildConfiguration = EBuildConfiguration::Development;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EBuildTargetPlatform TargetPlatform = EBuildTargetPlatform::Win64;

    FBuildStatus()
    {
        bBuildSuccessful = false;
        bCompilationSuccessful = false;
        bLinkingSuccessful = false;
        ErrorCount = 0;
        WarningCount = 0;
        BuildTime = 0.0f;
        BuildStartTime = FDateTime::MinValue();
        BuildEndTime = FDateTime::MinValue();
    }
};

/**
 * Module compilation status
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FModuleCompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float CompilationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString AgentName;

    FModuleCompilationStatus()
    {
        ModuleName = TEXT("");
        bCompilationSuccessful = false;
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
        AgentNumber = TEXT("");
        AgentName = TEXT("");
    }
};

/**
 * Build integration report
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FBuildStatus OverallBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FModuleCompilationStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 TotalModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 SuccessfulModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 FailedModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float TotalBuildTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FDateTime ReportGeneratedTime;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FString> Recommendations;

    FBuildIntegrationReport()
    {
        TotalModules = 0;
        SuccessfulModules = 0;
        FailedModules = 0;
        TotalBuildTime = 0.0f;
        ReportGeneratedTime = FDateTime::Now();
    }
};

/**
 * Build Integration Manager
 * 
 * This subsystem manages the build integration process for the Transpersonal Game Studio.
 * It handles compilation validation, build status tracking, and integration reporting
 * for all agent-created modules and systems.
 */
UCLASS()
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool StartBuild(EBuildConfiguration Configuration = EBuildConfiguration::Development, 
                   EBuildTargetPlatform Platform = EBuildTargetPlatform::Win64);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CompileProject();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CompileModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAllModules();

    // Build Status
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuildStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FModuleCompilationStatus> GetModuleCompilationStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FModuleCompilationStatus GetModuleStatus(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsBuildInProgress() const { return bBuildInProgress; }

    // Integration Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetMissingDependencies(const FString& ModuleName);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuildIntegrationReport GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool SaveBuildReport(const FString& FilePath = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogBuildSummary();

    // Agent Module Registration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool RegisterAgentModule(const FString& ModuleName, const FString& AgentNumber, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetRegisteredModules() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetModulesForAgent(const FString& AgentNumber) const;

    // Build Configuration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetBuildConfiguration(EBuildConfiguration Configuration) { DefaultBuildConfiguration = Configuration; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuildConfiguration GetBuildConfiguration() const { return DefaultBuildConfiguration; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetTargetPlatform(EBuildTargetPlatform Platform) { DefaultTargetPlatform = Platform; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuildTargetPlatform GetTargetPlatform() const { return DefaultTargetPlatform; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStarted, EBuildConfiguration, Configuration);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildStarted OnBuildStarted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildCompleted, FBuildStatus, BuildStatus);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildCompleted OnBuildCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleCompiled, FString, ModuleName, bool, bSuccess);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleCompiled OnModuleCompiled;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildError, FString, ErrorMessage, FString, ModuleName);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildError OnBuildError;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildWarning, FString, WarningMessage, FString, ModuleName);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildWarning OnBuildWarning;

protected:
    // Internal build management
    bool ExecuteBuildCommand(const FString& Command, FString& Output, FString& Errors);
    bool ParseBuildOutput(const FString& Output, const FString& Errors);
    void UpdateBuildStatus();
    void ResetBuildStatus();

    // Module management
    bool CompileModuleInternal(const FString& ModuleName);
    void UpdateModuleStatus(const FString& ModuleName, bool bSuccess, const TArray<FString>& Errors, const TArray<FString>& Warnings, float CompileTime);
    FModuleCompilationStatus* FindModuleStatus(const FString& ModuleName);

    // Dependency validation
    bool ValidateModuleDependenciesInternal(const FString& ModuleName, TArray<FString>& MissingDeps);
    TArray<FString> GetModuleDependencies(const FString& ModuleName);

    // Report generation
    void GenerateReportContent(FString& ReportContent);
    void AnalyzeBuildIssues(FBuildIntegrationReport& Report);

private:
    // Build state
    UPROPERTY()
    bool bBuildInProgress = false;

    UPROPERTY()
    FBuildStatus CurrentBuildStatus;

    UPROPERTY()
    TArray<FModuleCompilationStatus> ModuleStatuses;

    UPROPERTY()
    TMap<FString, int32> ModuleNameToIndex;

    // Configuration
    UPROPERTY()
    EBuildConfiguration DefaultBuildConfiguration = EBuildConfiguration::Development;

    UPROPERTY()
    EBuildTargetPlatform DefaultTargetPlatform = EBuildTargetPlatform::Win64;

    // Agent module registry
    UPROPERTY()
    TMap<FString, FString> ModuleToAgentNumber;

    UPROPERTY()
    TMap<FString, FString> ModuleToAgentName;

    // Build process tracking
    UPROPERTY()
    FProcHandle CurrentBuildProcess;

    UPROPERTY()
    float BuildStartTime = 0.0f;

    // Build history
    UPROPERTY()
    TArray<FBuildIntegrationReport> BuildHistory;

    // Constants
    static const int32 MaxBuildHistoryEntries = 50;
    static const float BuildTimeoutSeconds = 600.0f; // 10 minutes
};