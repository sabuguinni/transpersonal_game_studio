#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Manages the 19-agent development pipeline and ensures proper task distribution
 */

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 FilesCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 UE5CommandsExecuted = 0;

    FDir_AgentStatus()
    {
        AgentID = 0;
        AgentName = TEXT("");
        bIsActive = false;
        CompletionPercentage = 0.0f;
        CurrentTask = TEXT("");
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FString CycleID = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAgents = 19;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 SystemsImplemented = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 PlayableFeatures = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bCompilationSuccessful = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float CycleStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float EstimatedCompletionTime = 0.0f;

    FDir_ProductionMetrics()
    {
        CycleID = TEXT("");
        TotalAgents = 19;
        ActiveAgents = 0;
        SystemsImplemented = 0;
        PlayableFeatures = 0;
        bCompilationSuccessful = false;
        CycleStartTime = 0.0f;
        EstimatedCompletionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentStatus> AgentStatusList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    bool bPipelineActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    int32 CurrentAgentIndex = 1;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateAgentStatus(int32 AgentID, const FString& TaskName, float Completion, int32 FilesCreated, int32 CommandsExecuted);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void AdvanceToNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FDir_AgentStatus GetAgentStatus(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    bool ValidateAgentDeliverable(int32 AgentID, const FString& DeliverableType);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Coordination")
    void ResetProductionPipeline();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Coordination")
    void ForceAdvanceAgent();

private:
    void SetupAgentList();
    void CalculateProductionMetrics();
    bool CheckSystemCompilation();
    void LogProductionStatus();
};