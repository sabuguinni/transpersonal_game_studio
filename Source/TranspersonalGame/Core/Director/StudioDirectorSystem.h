#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Alpha          UMETA(DisplayName = "Alpha"),
    Beta           UMETA(DisplayName = "Beta"),
    Gold           UMETA(DisplayName = "Gold Master"),
    PostLaunch     UMETA(DisplayName = "Post-Launch")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Working        UMETA(DisplayName = "Working"),
    Blocked        UMETA(DisplayName = "Blocked"),
    Complete       UMETA(DisplayName = "Complete"),
    Error          UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentBottleneck;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float EstimatedCompletionHours;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        BlockedAgents = 0;
        OverallProgress = 0.0f;
        CurrentBottleneck = TEXT("None");
        EstimatedCompletionHours = 0.0f;
    }
};

/**
 * Studio Director System - Central coordination and oversight component
 * Manages the 19-agent production pipeline and ensures creative vision integrity
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Pipeline Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentInfo> AgentChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    int32 CurrentActiveAgent;

    // Creative Vision Protection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creative Vision")
    FString CreativeDirectorVision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creative Vision")
    TArray<FString> CorePillars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creative Vision")
    bool bVisionIntegrityMaintained;

    // Quality Gates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Control")
    bool bQAApprovalRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Control")
    TArray<FString> QualityGatesCriteria;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Control")
    int32 FailedQualityChecks;

public:
    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AdvanceToNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UnblockAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Creative Vision Functions
    UFUNCTION(BlueprintCallable, Category = "Creative Vision")
    void SetCreativeVision(const FString& Vision);

    UFUNCTION(BlueprintCallable, Category = "Creative Vision")
    bool ValidateAgentOutput(int32 AgentID, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Creative Vision")
    void EnforceVisionCompliance();

    // Quality Control Functions
    UFUNCTION(BlueprintCallable, Category = "Quality Control")
    bool RunQualityGate();

    UFUNCTION(BlueprintCallable, Category = "Quality Control")
    void TriggerQAReview();

    UFUNCTION(BlueprintCallable, Category = "Quality Control")
    void HandleQABlock(const FString& BlockReason);

    // Agent Communication Functions
    UFUNCTION(BlueprintCallable, Category = "Agent Communication")
    void SendTaskToAgent(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Communication")
    void ReceiveAgentReport(int32 AgentID, const FString& Report);

    UFUNCTION(BlueprintCallable, Category = "Agent Communication")
    void BroadcastToAllAgents(const FString& Message);

    // Debug and Monitoring Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintPipelineStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void SimulateAgentWork(int32 AgentID, float ProgressAmount);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetProductionPipeline();

protected:
    // Internal helper functions
    void UpdateProductionMetrics();
    void CheckForBottlenecks();
    void ValidateAgentDependencies();
    bool IsAgentReadyToWork(int32 AgentID) const;
    void LogProductionEvent(const FString& Event);

private:
    // Internal state tracking
    float LastMetricsUpdate;
    TArray<FString> ProductionLog;
    bool bPipelineInitialized;
};