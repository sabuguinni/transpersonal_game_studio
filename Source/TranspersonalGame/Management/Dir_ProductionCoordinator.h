#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    Architecture    UMETA(DisplayName = "Architecture"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character"),
    Animation       UMETA(DisplayName = "Animation"),
    AI              UMETA(DisplayName = "AI"),
    Narrative       UMETA(DisplayName = "Narrative"),
    QA              UMETA(DisplayName = "QA")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_MilestoneType MilestoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime DeadlineTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Pending;
        MilestoneType = EDir_MilestoneType::Architecture;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        LastUpdate = FDateTime::Now();
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
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinatorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    float MetricsUpdateInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AddAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FDir_AgentTask GetAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FDir_AgentTask> GetTasksByMilestone(EDir_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool ValidateMilestoneDependencies(EDir_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Management")
    void RefreshCoordinatorData();

private:
    FTimerHandle MetricsUpdateTimer;
    
    void SetupCoordinatorMesh();
    void UpdateMeshMaterial();
    FLinearColor GetStatusColor(EDir_AgentStatus Status);
};