#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Central coordination and oversight for all game systems
 * Manages agent task distribution, production pipeline coordination, and milestone tracking
 * This is the master control system that ensures all 18 agents work in harmony
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    PrototypeDevelopment,
    VerticalSlice,
    FullProduction,
    Polish,
    GoldMaster
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    float ProgressPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FDateTime LastUpdateTime;

    FDir_AgentTaskInfo()
    {
        AgentID = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        LastUpdateTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    int32 TotalFilesCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    int32 TotalLinesOfCode = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    int32 ActiveAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    float OverallProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Production Metrics")
    FDateTime CycleStartTime;

    FDir_ProductionMetrics()
    {
        TotalFilesCreated = 0;
        TotalLinesOfCode = 0;
        CompilationErrors = 0;
        ActiveAgents = 0;
        OverallProgress = 0.0f;
        CycleStartTime = FDateTime::Now();
    }
};

/**
 * Studio Director World Subsystem
 * Manages the entire production pipeline and coordinates all agent activities
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, const FString& TaskDescription, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTaskInfo> GetAllAgentStatuses() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentProductionPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportCompilationError(const FString& ErrorMessage, const FString& SourceFile);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ClearCompilationErrors();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetCompilationErrorCount() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DispatchTaskToAgent(int32 AgentID, const FString& TaskDescription, int32 Priority = 1);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsAgentBlocked(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockAgent(int32 AgentID);

protected:
    // Production State
    UPROPERTY()
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY()
    int32 CurrentCycleNumber;

    UPROPERTY()
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY()
    TArray<FDir_AgentTaskInfo> AgentStatuses;

    UPROPERTY()
    TArray<FString> CompilationErrors;

    UPROPERTY()
    TMap<FString, bool> MilestoneCompletionStatus;

private:
    void InitializeAgentStatuses();
    void UpdateProductionMetrics();
    void LogProductionStatus() const;
};

/**
 * Studio Director Component
 * Can be attached to GameMode or other actors for direct production oversight
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MonitorProductionHealth();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckSystemIntegrity();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    UStudioDirectorSubsystem* DirectorSubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float MonitoringInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bAutoGenerateReports = true;

private:
    float LastMonitoringTime;
    void InitializeDirectorSubsystem();
};