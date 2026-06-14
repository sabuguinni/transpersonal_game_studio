#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

/**
 * Production coordination system for managing agent workflows and build state
 * Tracks production metrics, agent task completion, and build quality
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction    UMETA(DisplayName = "Pre-Production"),
    Prototype        UMETA(DisplayName = "Prototype"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
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
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CompletionPercentage = 0.0f;
        LastOutput = TEXT("");
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TerrainElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CharacterSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 LightingSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 EnvironmentProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastAnalysis;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        TerrainElements = 0;
        CharacterSystems = 0;
        LightingSystems = 0;
        EnvironmentProps = 0;
        OverallCompletion = 0.0f;
        LastAnalysis = FDateTime::Now();
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

public:
    virtual void Tick(float DeltaTime) override;

    // Production phase management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_AgentStatus Status, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateAgentPriorities();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateBuildQuality();

    // Metrics and reporting
    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallCompletion();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionEvent(const FString& EventDescription);

private:
    UPROPERTY()
    float AnalysisTimer;

    UPROPERTY()
    TArray<FString> ProductionLog;

    void InitializeAgentTasks();
    void UpdateProductionMetrics();
    void CheckAgentDependencies();
};