#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director System - Agent #1
 * Coordinates all 18 AI agents in the development pipeline
 * Manages task distribution, progress tracking, and quality assurance
 * Ensures the vision of Miguel Martins reaches the final product intact
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskPriority Priority = EDir_TaskPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskStatus Status = EDir_TaskStatus::Pending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime Deadline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString OutputFiles;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("Unknown");
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        EstimatedHours = 1.0f;
        Deadline = FDateTime::Now() + FTimespan::FromHours(24);
        OutputFiles = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCycles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 PendingTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 BlockedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompilationSuccessRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInMap = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 PlayablePrototypeScore = 0;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        BlockedTasks = 0;
        CompilationSuccessRate = 0.0f;
        TotalActorsInMap = 0;
        PlayablePrototypeScore = 0;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskStatus(int32 AgentID, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetTaskForAgent(int32 AgentID) const;

    // Production Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 CalculatePrototypeScore();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DispatchToEngineArchitect();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReceiveFromIntegrationAgent(const FString& BuildReport);

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void LogCurrentStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void CreateTaskMarkers();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentCycleID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bProductionCycleActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDateTime CycleStartTime;

    // Agent chain coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentAgentInChain = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bChainExecutionActive = false;

private:
    void InitializeAgentTasks();
    void UpdateProductionMetrics();
    bool CheckCompilationStatus();
    int32 CountActorsInLevel();
    void LogAgentChainStatus();
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStudioDirectorComponent* DirectorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualizationMesh;
};