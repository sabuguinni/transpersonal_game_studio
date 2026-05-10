#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Production Coordination System
 * 
 * Manages overall game production state, agent coordination, and milestone tracking.
 * This is the central command system that oversees all 18 specialized agents
 * and ensures the project stays on track for commercial release.
 * 
 * Key Responsibilities:
 * - Track compilation status and technical health
 * - Coordinate agent priorities and dependencies
 * - Monitor milestone progress (Milestone 1: "Walk Around")
 * - Enforce production rules and quality gates
 * - Generate production reports and metrics
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Assessment      UMETA(DisplayName = "Assessment Phase"),
    Development     UMETA(DisplayName = "Active Development"),
    Integration     UMETA(DisplayName = "Integration Phase"),
    Testing         UMETA(DisplayName = "Quality Assurance"),
    Release         UMETA(DisplayName = "Release Candidate")
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

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    NearComplete    UMETA(DisplayName = "Near Complete"),
    Complete        UMETA(DisplayName = "Complete"),
    Verified        UMETA(DisplayName = "Verified")
};

USTRUCT(BlueprintType)
struct FDir_AgentMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    float CycleCompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FString LastError;

    FDir_AgentMetrics()
    {
        AgentID = 0;
        AgentName = TEXT("Unknown");
        Status = EDir_AgentStatus::Idle;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        CycleCompletionTime = 0.0f;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("Unknown Milestone");
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 OrphanedHeaders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompilationSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bMinPlayableMapReady;

    FDir_ProductionMetrics()
    {
        TotalHeaderFiles = 0;
        TotalCppFiles = 0;
        OrphanedHeaders = 0;
        bCompilationSuccess = false;
        CompilationErrors = 0;
        TotalActorsInLevel = 0;
        bMinPlayableMapReady = false;
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
    USceneComponent* RootSceneComponent;

    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleName;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentMetrics> AgentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    int32 ActiveAgentCount;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_MilestoneProgress> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    FDir_MilestoneProgress CurrentMilestone;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TotalBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float DailyBudgetLimit;

public:
    virtual void Tick(float DeltaTime) override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionCycle(const FString& CycleName, int32 CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RecordAgentMetrics(int32 AgentID, int32 FilesCreated, int32 UE5Commands, float CompletionTime);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, const FString& CompletedDeliverable);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float CalculateMilestoneCompletion(const FString& MilestoneName);

    // Production Assessment
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssessProductionHealth();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanProceedToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsCompilationHealthy();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EnforceProductionRules();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const { return ProductionMetrics; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    TArray<FDir_AgentMetrics> GetAgentMetrics() const { return AgentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FDir_MilestoneProgress GetCurrentMilestone() const { return CurrentMilestone; }

private:
    void UpdateProductionMetrics();
    void CheckAgentDependencies();
    void LogProductionStatus();
};