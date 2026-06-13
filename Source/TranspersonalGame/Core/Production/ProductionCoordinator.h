#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator - Cycle 020
 * Manages agent task distribution and playable prototype coordination
 * Focus: Ensure all agents produce concrete implementations, not just headers
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Architecture    UMETA(DisplayName = "Architecture"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Characters      UMETA(DisplayName = "Characters"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Polish          UMETA(DisplayName = "Polish")
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
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> ExpectedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Pending;
        Priority = 0;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        CompilationErrors = 0;
        ActiveActors = 0;
        bPlayablePrototypeReady = false;
        OverallProgress = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CycleObjective;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeCycle020Tasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPlayablePrototypeReady() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateProductionReport();

private:
    void UpdateProductionMetrics();
    void CheckTaskDependencies();
    void LogProductionState();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDir_ProductionCoordinator* ProductionCoordinator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Settings")
    bool bAutoManageProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Settings")
    float ProductionUpdateInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void StartCycle020();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void EmergencyPrototypeValidation();

private:
    float LastUpdateTime;
};