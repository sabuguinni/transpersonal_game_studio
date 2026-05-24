#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

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
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 FailedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float AverageTaskTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        FailedTasks = 0;
        AverageTaskTime = 0.0f;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DirectorMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Management")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Management")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Management")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Management")
    bool bPlayablePrototypeReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Management")
    TArray<FString> CriticalBlockers;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void InitializeProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    bool IsAgentBlocked(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Management")
    void CreateCoordinationMarkers();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Management")
    void ValidateMinPlayableMap();

private:
    void UpdateProductionMetrics();
    void CheckTaskDependencies();
    void HandleCriticalBlockers();
};