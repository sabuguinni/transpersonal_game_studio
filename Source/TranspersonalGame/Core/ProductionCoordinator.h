#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    GameplayCore    UMETA(DisplayName = "Gameplay Core"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Error           UMETA(DisplayName = "Error")
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
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 FilesRequired;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        FilesCreated = 0;
        FilesRequired = 8;
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
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TargetFileCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FString CurrentCycle;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        TotalTasks = 0;
        OverallProgress = 0.0f;
        TotalFilesCreated = 0;
        TargetFileCount = 152; // 19 agents * 8 files each
        CurrentCycle = TEXT("CYCLE_003");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float MetricsUpdateInterval;

    // Production management functions
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void InitializeProductionChain();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void CheckDependencies();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(const FString& AgentName) const;

private:
    float LastMetricsUpdate;
    
    void UpdateMetricsInternal();
    void ValidateAgentChain();
    void LogProductionState();
};