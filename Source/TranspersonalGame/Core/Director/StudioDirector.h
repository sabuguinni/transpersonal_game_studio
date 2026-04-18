#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Agent #1 in the 19-agent production chain
 * Coordinates all agents, translates creative vision into technical tasks
 * Manages production cycles and ensures delivery quality
 */

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
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_Priority::Medium;
        Status = EDir_TaskStatus::Pending;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCycleCompleted;

    FDir_ProductionCycle()
    {
        CycleNumber = 0;
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        bCycleCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_StudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_StudioDirectorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionCycle> ProductionHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionCycle CurrentCycle;

    // Agent Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TMap<int32, FString> AgentNames;

    // Quality Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bQABlockActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    TArray<FString> CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 MinFilesPerAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 MinUE5CommandsPerAgent;

    // Studio Director Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_Priority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentID, int32 FilesCreated, int32 UE5Commands);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentOutput(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetQABlock(bool bBlocked, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    // Gameplay-First Directive Functions
    UFUNCTION(BlueprintCallable, Category = "Gameplay First")
    bool CheckMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Gameplay First")
    void PrioritizePlayableContent();

    UFUNCTION(BlueprintCallable, Category = "Gameplay First")
    TArray<FString> GetCriticalGameplayTasks();

private:
    void SetupAgentNames();
    void ValidateProductionRules();
    bool CheckAgentDependencies(int32 AgentID);
    void LogProductionMetrics();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirectorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDir_StudioDirectorComponent* DirectorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Studio Director")
    void ExecuteProductionCycle();

    UFUNCTION(CallInEditor, Category = "Studio Director")
    void GenerateAgentReport();

    UFUNCTION(CallInEditor, Category = "Studio Director")
    void ValidateProjectState();
};