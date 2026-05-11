#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "SharedTypes.h"
#include "Dir_StudioDirector.generated.h"

/**
 * Studio Director Production Coordination System
 * Manages the 18-agent production pipeline and real-time status monitoring
 * This is the central command system that coordinates all game development agents
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Working     UMETA(DisplayName = "Working"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing"),
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
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Ready;
        CurrentTask = TEXT("");
        ProgressPercent = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float AverageFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlueprintsCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float CycleExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        CompilationErrors = 0;
        AverageFrameRate = 60.0f;
        FilesCreated = 0;
        BlueprintsCreated = 0;
        CycleExecutionTime = 0.0f;
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
    virtual void Tick(float DeltaTime) override;

    // Core Production Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    UStaticMeshComponent* DirectorHubMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    UTextRenderComponent* StatusDisplay;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    // Agent Status Actors (visual indicators in level)
    UPROPERTY(BlueprintReadOnly, Category = "Visual Coordination")
    TArray<ATextRenderActor*> AgentStatusActors;

public:
    // Production Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateVisualCoordinationBoard();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RefreshAgentStatusDisplay();

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Emergency Controls")
    void TriggerEmergencyStop();

    UFUNCTION(BlueprintCallable, Category = "Emergency Controls")
    void ResetAllAgentStatus();

    UFUNCTION(BlueprintCallable, Category = "Emergency Controls")
    void ForceCompilationCheck();

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    bool ValidateMinimumPlayableState();

    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    void GenerateProductionReport();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentMetrics.CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    float GetOverallProgress() const;

private:
    // Internal coordination functions
    void SetupAgentTaskList();
    void UpdateVisualIndicators();
    void CheckDependencies();
    void LogProductionStatus();

    // Agent names for initialization
    TArray<FString> AgentNames = {
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };

    // Timing and coordination
    float LastMetricsUpdate;
    float MetricsUpdateInterval;
    bool bEmergencyMode;
    bool bProductionActive;
};