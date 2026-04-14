#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "DirectorSystemManager.generated.h"

// Forward declarations
class UDir_AgentCoordinator;
class UDir_ProductionPipeline;
class UDir_QualityAssurance;

/**
 * Director System Manager - Central coordination hub for all 18 specialized agents
 * This is the core system that ensures Miguel's creative vision flows intact through
 * the entire production pipeline from concept to final build.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDirectorSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDirectorSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core Director Functions
    UFUNCTION(BlueprintCallable, Category = "Director System")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Director System")
    void ProcessCreativeVision(const FString& VisionDescription);

    UFUNCTION(BlueprintCallable, Category = "Director System")
    bool ValidateAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Director System")
    void TriggerAgentSequence(EDir_AgentType StartingAgent);

    UFUNCTION(BlueprintCallable, Category = "Director System")
    EDir_ProductionStatus GetCurrentProductionStatus() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void RegisterAgent(EDir_AgentType AgentType, const FString& AgentIdentifier);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void DeactivateAgent(EDir_AgentType AgentType, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentStatus> GetActiveAgents() const;

    // Quality Control
    UFUNCTION(BlueprintCallable, Category = "Quality Control")
    bool CanProceedToNextAgent(EDir_AgentType CurrentAgent) const;

    UFUNCTION(BlueprintCallable, Category = "Quality Control")
    void ReportAgentCompletion(EDir_AgentType CompletedAgent, bool bSuccess, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Quality Control")
    void TriggerQualityAssessment();

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void CreateBuildSnapshot(const FString& BuildDescription);

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    bool RollbackToPreviousBuild(int32 BuildsBack = 1);

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    TArray<FDir_BuildInfo> GetBuildHistory() const;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director Components")
    TObjectPtr<UDir_AgentCoordinator> AgentCoordinator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director Components")
    TObjectPtr<UDir_ProductionPipeline> ProductionPipeline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director Components")
    TObjectPtr<UDir_QualityAssurance> QualityAssurance;

    // Production State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production State")
    EDir_ProductionStatus CurrentStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production State")
    EDir_AgentType ActiveAgent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production State")
    TArray<FDir_AgentStatus> AgentStatusList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production State")
    TArray<FDir_BuildInfo> BuildHistory;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Config")
    int32 MaxBuildHistoryCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Config")
    float AgentTimeoutSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Config")
    bool bEnableAutomaticQualityChecks;

private:
    // Internal state management
    void UpdateProductionStatus();
    void ValidateAgentDependencies();
    void CleanupOldBuilds();
    
    // Event handling
    UFUNCTION()
    void OnAgentCompleted(EDir_AgentType AgentType, bool bSuccess);

    UFUNCTION()
    void OnQualityCheckFailed(const FString& FailureReason);

    UFUNCTION()
    void OnBuildCreated(const FDir_BuildInfo& NewBuild);

    // Utility functions
    bool IsAgentSequenceValid() const;
    FString GenerateBuildIdentifier() const;
    void LogDirectorActivity(const FString& Activity, EDir_LogLevel LogLevel = EDir_LogLevel::Info);
};