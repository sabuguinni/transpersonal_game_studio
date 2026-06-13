#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototyping     UMETA(DisplayName = "Prototyping"),
    Production      UMETA(DisplayName = "Production"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Active          UMETA(DisplayName = "Active"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Error           UMETA(DisplayName = "Error")
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
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercentage = 0.0f;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        LastUpdate = FDateTime::Now();
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
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime CycleStartTime;

    FDir_ProductionMetrics()
    {
        TotalAgents = 20;
        ActiveAgents = 0;
        CompletedTasks = 0;
        TotalFiles = 0;
        TotalActors = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
        CycleID = TEXT("");
        CycleStartTime = FDateTime::Now();
    }
};

/**
 * Studio Director - Central coordination system for all 20 AI agents
 * Manages production phases, agent coordination, and overall project metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentProductionPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentInfo> AgentRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionActive;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(int32 AgentID, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& CurrentTask);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentProgress(int32 AgentID, float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgents();

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentProductionPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceProductionPhase();

    // Metrics and Analysis
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetActiveAgentCount();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    // Coordination and Dependencies
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDependencies(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResolveAgentBlocking(int32 AgentID);

    // Quality Assurance Integration
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsQAApprovalRequired();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RequestQAApproval();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetQABlocked(bool bBlocked, const FString& Reason);

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void InitializeAgentRegistry();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void RefreshProductionMetrics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void GenerateProductionReport();

private:
    void InitializeDefaultAgents();
    void UpdateProductionMetrics();
    bool CheckAgentDependencies(int32 AgentID);
    void LogProductionStatus();

    // Internal state
    bool bQABlocked;
    FString QABlockReason;
    FDateTime CycleStartTime;
    TMap<int32, TArray<int32>> AgentDependencies;
};