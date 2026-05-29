#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Misc/DateTime.h"
#include "../Core/QABuildValidator.h"
#include "QAIntegrationManager.generated.h"

/**
 * Integration Manager for Transpersonal Game Studio
 * 
 * Coordinates integration of outputs from all 18 specialized agents into a cohesive game.
 * Manages build validation, dependency resolution, and integration testing.
 * 
 * This is the Integration Agent's primary coordination system.
 */

DECLARE_LOG_CATEGORY_EXTERN(LogQAIntegration, Log, All);

/**
 * Agent output status tracking
 */
UENUM(BlueprintType)
enum class EAgentOutputStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    NeedsRevision   UMETA(DisplayName = "Needs Revision"),
    Integrated      UMETA(DisplayName = "Integrated")
};

/**
 * Agent output information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAgentOutputInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly)
    EAgentOutputStatus Status = EAgentOutputStatus::NotStarted;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastUpdateTime;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> OutputFiles;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly)
    FString IntegrationNotes;

    UPROPERTY(BlueprintReadOnly)
    float IntegrationPriority = 1.0f;
};

/**
 * Integration pipeline stage
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FIntegrationStage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString StageName;

    UPROPERTY(BlueprintReadOnly)
    TArray<int32> RequiredAgents;

    UPROPERTY(BlueprintReadOnly)
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadOnly)
    FDateTime CompletionTime;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ValidationResults;
};

/**
 * Build integration result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildIntegrationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bIntegrationSuccessful = false;

    UPROPERTY(BlueprintReadOnly)
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly)
    FDateTime IntegrationTime;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> IntegratedModules;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> FailedModules;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ConflictingFiles;

    UPROPERTY(BlueprintReadOnly)
    FString IntegrationReport;
};

/**
 * Main integration manager subsystem
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQAIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Agent output management
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    void RegisterAgentOutput(int32 AgentID, const FString& AgentName, const TArray<FString>& OutputFiles);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    void UpdateAgentStatus(int32 AgentID, EAgentOutputStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ValidateAgentOutput(int32 AgentID, TArray<FString>& OutValidationErrors);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    FAgentOutputInfo GetAgentOutputInfo(int32 AgentID);

    /**
     * Integration pipeline management
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool StartIntegrationPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ProcessIntegrationStage(const FString& StageName);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool IsIntegrationStageReady(const FString& StageName);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    TArray<FIntegrationStage> GetIntegrationStages();

    /**
     * Build integration
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    FBuildIntegrationResult IntegrateAllAgentOutputs();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ValidateIntegratedBuild();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ResolveIntegrationConflicts(const TArray<FString>& ConflictingFiles);

    /**
     * Dependency management
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ResolveDependencyOrder(TArray<int32>& OutAgentOrder);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ValidateAgentDependencies();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    TArray<int32> GetDependentAgents(int32 AgentID);

    /**
     * Quality assurance
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ValidateGameplayIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ValidatePerformanceIntegration();

    /**
     * Reporting and monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    FString GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool IsIntegrationComplete();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    float GetIntegrationProgress();

    /**
     * Rollback and recovery
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool CreateIntegrationCheckpoint(const FString& CheckpointName);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool RollbackToCheckpoint(const FString& CheckpointName);

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    TArray<FString> GetAvailableCheckpoints();

protected:
    /**
     * Internal integration methods
     */
    void InitializeAgentRegistry();
    void SetupIntegrationPipeline();
    bool ValidateAgentOutputFiles(const TArray<FString>& Files);
    bool CheckForFileConflicts(const TArray<FString>& Files1, const TArray<FString>& Files2);
    bool MergeAgentOutputs(const TArray<int32>& AgentIDs);
    void UpdateIntegrationProgress();

    /**
     * Agent-specific validation
     */
    bool ValidateEngineArchitectOutput(const FAgentOutputInfo& AgentInfo);
    bool ValidateCoreSystemsOutput(const FAgentOutputInfo& AgentInfo);
    bool ValidateWorldGenerationOutput(const FAgentOutputInfo& AgentInfo);
    bool ValidateAISystemsOutput(const FAgentOutputInfo& AgentInfo);
    bool ValidateAudioSystemsOutput(const FAgentOutputInfo& AgentInfo);
    bool ValidateVFXSystemsOutput(const FAgentOutputInfo& AgentInfo);

private:
    UPROPERTY()
    TMap<int32, FAgentOutputInfo> AgentOutputRegistry;

    UPROPERTY()
    TArray<FIntegrationStage> IntegrationPipeline;

    UPROPERTY()
    UQABuildValidator* BuildValidator;

    UPROPERTY()
    FString CurrentBuildVersion;

    UPROPERTY()
    bool bIntegrationInProgress = false;

    UPROPERTY()
    float IntegrationProgress = 0.0f;

    UPROPERTY()
    TMap<FString, FDateTime> IntegrationCheckpoints;

    // Agent dependency mapping (Agent ID -> Required Agent IDs)
    TMap<int32, TArray<int32>> AgentDependencies;

    // Integration stage definitions
    void DefineIntegrationStages();
    
    // Agent registry with all 19 agents
    static const TMap<int32, FString> AgentNames;
};

/**
 * Integration automation test
 */
class TRANSPERSONALGAME_API FIntegrationValidationTest : public FAutomationTestBase
{
public:
    FIntegrationValidationTest(const FString& InName)
        : FAutomationTestBase(InName, false)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
    virtual EAutomationTestFlags GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter; }
    virtual FString GetBeautifiedTestName() const override { return TEXT("Integration Validation Test"); }
};