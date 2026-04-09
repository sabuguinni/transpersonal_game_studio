// Copyright Transpersonal Game Studio. All Rights Reserved.
// BuildIntegrationManager.h - Manages build integration and deployment pipeline

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/EngineSubsystem.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

/**
 * Build configuration information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildConfiguration
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString TargetPlatform;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildConfiguration_Type; // Development, Shipping, Debug

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> IncludedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> AgentSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsStableBuild = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bPassedQA = false;

    FBuildConfiguration()
    {
        BuildName = TEXT("");
        BuildVersion = TEXT("1.0.0");
        BuildNumber = TEXT("0");
        TargetPlatform = TEXT("Windows");
        BuildConfiguration_Type = TEXT("Development");
        BuildTimestamp = FDateTime::Now();
        bIsStableBuild = false;
        bPassedQA = false;
    }
};

/**
 * Build validation result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> MissingDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationSteps;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime = 0.0f;

    FBuildValidationResult()
    {
        bIsValid = false;
        ValidationTime = 0.0f;
    }
};

/**
 * Agent integration status for build
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentBuildStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    bool bSystemsReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    bool bAssetsReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    bool bCodeReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    TArray<FString> PendingTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    TArray<FString> CompletedTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    float CompletionPercentage = 0.0f;

    FAgentBuildStatus()
    {
        AgentNumber = TEXT("");
        AgentName = TEXT("");
        bSystemsReady = false;
        bAssetsReady = false;
        bCodeReady = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Build deployment information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildDeploymentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    FString DeploymentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    FString DeploymentPath;

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    FDateTime DeploymentTime;

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    bool bDeploymentSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    TArray<FString> DeployedFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    int64 TotalSize = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Deployment")
    FString DeploymentNotes;

    FBuildDeploymentInfo()
    {
        DeploymentTarget = TEXT("");
        DeploymentPath = TEXT("");
        DeploymentTime = FDateTime::Now();
        bDeploymentSuccessful = false;
        TotalSize = 0;
        DeploymentNotes = TEXT("");
    }
};

/**
 * Build Integration Manager
 * 
 * This subsystem manages the integration of all agent outputs into cohesive builds,
 * handles build validation, deployment, and maintains build history for rollback.
 * It ensures that all 19 agents' work is properly integrated and tested.
 */
UCLASS()
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CreateNewBuild(const FBuildConfiguration& BuildConfig);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateBuild(const FString& BuildName, FBuildValidationResult& ValidationResult);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool DeployBuild(const FString& BuildName, const FString& DeploymentTarget, FBuildDeploymentInfo& DeploymentInfo);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuildConfiguration> GetAvailableBuilds() const { return BuildHistory; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuildConfiguration GetBuildConfiguration(const FString& BuildName) const;

    // Agent Integration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IntegrateAgentOutput(const FString& AgentNumber, const TArray<FString>& OutputPaths);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FAgentBuildStatus GetAgentBuildStatus(const FString& AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FAgentBuildStatus> GetAllAgentStatuses() const { return AgentStatuses; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAgentIntegration(const FString& AgentNumber);

    // Build History and Rollback
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool RollbackToBuild(const FString& BuildName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetStableBuilds() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool MarkBuildAsStable(const FString& BuildName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ArchiveOldBuilds(int32 MaxBuildsToKeep = 10);

    // QA Integration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool SubmitBuildForQA(const FString& BuildName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ProcessQAResults(const FString& BuildName, bool bPassed, const TArray<FString>& QANotes);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetBuildsAwaitingQA() const;

    // Continuous Integration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool SetupContinuousIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TriggerAutomatedBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsAutomatedBuildRunning() const { return bAutomatedBuildInProgress; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport(const FString& BuildName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetAverageBuildTime() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildCreated, FString, BuildName);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildCreated OnBuildCreated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildValidated, FString, BuildName, bool, bIsValid);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildValidated OnBuildValidated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildDeployed, FString, BuildName, bool, bSuccessful);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildDeployed OnBuildDeployed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentIntegrated, FString, AgentNumber, bool, bSuccessful);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnAgentIntegrated OnAgentIntegrated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQACompleted, FString, BuildName, bool, bPassed);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnQACompleted OnQACompleted;

protected:
    // Internal build management
    bool CreateBuildDirectory(const FString& BuildName);
    bool CopyAgentOutputs(const FString& BuildName);
    bool CompileProject(const FString& BuildName);
    bool PackageProject(const FString& BuildName, const FString& Platform);

    // Validation helpers
    bool ValidateProjectStructure(const FString& BuildName, TArray<FString>& Errors);
    bool ValidateAgentDependencies(const FString& BuildName, TArray<FString>& Errors);
    bool ValidateAssetIntegrity(const FString& BuildName, TArray<FString>& Errors);
    bool ValidateCodeCompilation(const FString& BuildName, TArray<FString>& Errors);

    // Agent integration helpers
    bool ProcessAgentAssets(const FString& AgentNumber, const TArray<FString>& AssetPaths);
    bool ProcessAgentCode(const FString& AgentNumber, const TArray<FString>& CodePaths);
    bool ProcessAgentConfigurations(const FString& AgentNumber, const TArray<FString>& ConfigPaths);
    void UpdateAgentStatus(const FString& AgentNumber);

    // Build history management
    void SaveBuildConfiguration(const FBuildConfiguration& BuildConfig);
    void LoadBuildHistory();
    void CleanupOldBuilds();

    // Metrics and reporting
    void RecordBuildMetrics(const FString& BuildName, float BuildTime, bool bSuccessful);
    void GenerateBuildStatistics();

private:
    // Build management
    UPROPERTY()
    TArray<FBuildConfiguration> BuildHistory;

    UPROPERTY()
    TMap<FString, int32> BuildNameToIndex;

    UPROPERTY()
    FString CurrentBuildName;

    UPROPERTY()
    bool bBuildInProgress = false;

    // Agent integration tracking
    UPROPERTY()
    TArray<FAgentBuildStatus> AgentStatuses;

    UPROPERTY()
    TMap<FString, int32> AgentNumberToIndex;

    // QA integration
    UPROPERTY()
    TArray<FString> BuildsAwaitingQA;

    UPROPERTY()
    TMap<FString, bool> QAResults;

    // Continuous integration
    UPROPERTY()
    bool bContinuousIntegrationEnabled = false;

    UPROPERTY()
    bool bAutomatedBuildInProgress = false;

    UPROPERTY()
    float LastAutomatedBuildTime = 0.0f;

    // Build metrics
    UPROPERTY()
    TArray<float> BuildTimes;

    UPROPERTY()
    TArray<bool> BuildSuccessRates;

    UPROPERTY()
    int32 TotalBuildsCreated = 0;

    UPROPERTY()
    int32 SuccessfulBuilds = 0;

    // Configuration
    UPROPERTY()
    FString BuildOutputDirectory;

    UPROPERTY()
    FString DeploymentDirectory;

    UPROPERTY()
    int32 MaxBuildHistorySize = 50;

    // Agent chain definition (for validation)
    static const TArray<FString> RequiredAgents;
};