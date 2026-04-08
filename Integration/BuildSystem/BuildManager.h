#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Containers/Map.h"
#include "Containers/Array.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildManager, Log, All);

/**
 * Build configuration types supported by the integration system
 */
UENUM(BlueprintType)
enum class EBuildConfiguration : uint8
{
    Debug       UMETA(DisplayName = "Debug"),
    DebugGame   UMETA(DisplayName = "DebugGame"),
    Development UMETA(DisplayName = "Development"),
    Test        UMETA(DisplayName = "Test"),
    Shipping    UMETA(DisplayName = "Shipping")
};

/**
 * Build status tracking
 */
UENUM(BlueprintType)
enum class EBuildStatus : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Cancelled   UMETA(DisplayName = "Cancelled")
};

/**
 * Agent integration status
 */
UENUM(BlueprintType)
enum class EAgentStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Validated   UMETA(DisplayName = "Validated"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
};

/**
 * Build information structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString BuildId;

    UPROPERTY(BlueprintReadOnly)
    EBuildConfiguration Configuration;

    UPROPERTY(BlueprintReadOnly)
    EBuildStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly)
    FDateTime EndTime;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly)
    TMap<FString, EAgentStatus> AgentStatuses;

    UPROPERTY(BlueprintReadOnly)
    FString PackagePath;

    FBuildInfo()
    {
        Configuration = EBuildConfiguration::Development;
        Status = EBuildStatus::NotStarted;
        StartTime = FDateTime::Now();
        EndTime = FDateTime::MinValue();
    }
};

/**
 * Agent output validation result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString AgentId;

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly)
    FDateTime ValidationTime;

    FAgentValidationResult()
    {
        bIsValid = false;
        ValidationTime = FDateTime::Now();
    }
};

/**
 * Main build manager class responsible for coordinating integration and builds
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildManager : public UObject
{
    GENERATED_BODY()

public:
    UBuildManager();

    /**
     * Initialize the build manager with project configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    void Initialize(const FString& ProjectRoot);

    /**
     * Start a new integration cycle
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool StartIntegrationCycle(const FString& CycleId, EBuildConfiguration Configuration = EBuildConfiguration::Development);

    /**
     * Validate output from a specific agent
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    FAgentValidationResult ValidateAgentOutput(const FString& AgentId, const FString& OutputPath);

    /**
     * Build the project with specified configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool BuildProject(EBuildConfiguration Configuration);

    /**
     * Package the project for distribution
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    FString CreateBuildPackage(EBuildConfiguration Configuration);

    /**
     * Get current build status
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    FBuildInfo GetCurrentBuildInfo() const { return CurrentBuild; }

    /**
     * Get build history (last 10 builds)
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    TArray<FBuildInfo> GetBuildHistory() const { return BuildHistory; }

    /**
     * Rollback to previous build
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool RollbackToPreviousBuild();

    /**
     * Check if QA has blocked the build
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool IsQABlocked() const { return bQABlocked; }

    /**
     * Get validation results for all agents
     */
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    TArray<FAgentValidationResult> GetAgentValidationResults() const { return AgentValidationResults; }

protected:
    /**
     * Validate architecture files from Engine Architect (Agent #02)
     */
    bool ValidateArchitectureFiles(const FString& OutputPath, TArray<FString>& OutErrors);

    /**
     * Validate core systems from Core Systems Programmer (Agent #03)
     */
    bool ValidateCoreSystemsOutput(const FString& OutputPath, TArray<FString>& OutErrors);

    /**
     * Validate QA results from QA & Testing Agent (Agent #18)
     */
    bool ValidateQAResults(const FString& OutputPath, TArray<FString>& OutErrors);

    /**
     * Validate performance optimization from Performance Optimizer (Agent #04)
     */
    bool ValidatePerformanceOutput(const FString& OutputPath, TArray<FString>& OutErrors);

    /**
     * Execute build process
     */
    bool ExecuteBuild(EBuildConfiguration Configuration);

    /**
     * Create build manifest
     */
    void CreateBuildManifest(const FString& PackagePath, const FBuildInfo& BuildInfo);

    /**
     * Load agent configuration
     */
    void LoadAgentConfiguration();

    /**
     * Save build history to disk
     */
    void SaveBuildHistory();

    /**
     * Load build history from disk
     */
    void LoadBuildHistory();

private:
    UPROPERTY()
    FString ProjectRootPath;

    UPROPERTY()
    FBuildInfo CurrentBuild;

    UPROPERTY()
    TArray<FBuildInfo> BuildHistory;

    UPROPERTY()
    TArray<FAgentValidationResult> AgentValidationResults;

    UPROPERTY()
    TMap<FString, FString> AgentNames;

    UPROPERTY()
    bool bQABlocked;

    UPROPERTY()
    int32 MaxBuildHistory;

    static const TMap<FString, FString> DefaultAgentNames;
};