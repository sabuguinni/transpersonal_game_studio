#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Dir_StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
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
    Failed          UMETA(DisplayName = "Failed")
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
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CompletionPercentage = 0.0f;
        Dependencies = TEXT("");
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 VegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BuildingCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        TerrainActors = 0;
        VegetationCount = 0;
        BuildingCount = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    float ProductionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    bool bAutoManageProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    int32 MaxActorsPerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    int32 MaxDinosaursPerLevel;

private:
    FTimerHandle ProductionUpdateTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, int32 Priority = 1);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float CompletionPercentage = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    EDir_ProductionPhase GetCurrentProductionPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool ValidateProductionState() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management", CallInEditor)
    void DebugPrintProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void OptimizeLevel();

protected:
    UFUNCTION()
    void OnProductionUpdate();

    void CountLevelActors();
    void ValidateAgentDependencies();
    void UpdateProductionPhase();
};

#include "Dir_StudioDirector.generated.h"