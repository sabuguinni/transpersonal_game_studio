#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Complete    UMETA(DisplayName = "Complete"),
    Error       UMETA(DisplayName = "Error")
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
    FDateTime AssignedTime;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        AssignedTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float TerrainCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompilationHealthy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float FrameRate;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurActors = 0;
        TerrainActors = 0;
        TerrainCoverage = 0.0f;
        bCompilationHealthy = true;
        FrameRate = 60.0f;
    }
};

/**
 * Studio Director System - Coordinates all agent activities and tracks production metrics
 * This system manages the 19-agent pipeline and ensures proper task delegation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    // Coordination Commands
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IssueTerrainExpansionCommand();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IssueDinosaurCreationCommand();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IssueSurvivalHUDCommand();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IssuePerformanceOptimizationCommand();

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayableMap() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCriticalIssues() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bAutoCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float CoordinationInterval;

private:
    float LastCoordinationTime;
    
    void PerformAutomaticCoordination();
    void AnalyzeAgentPerformance();
    void OptimizeTaskDistribution();
};