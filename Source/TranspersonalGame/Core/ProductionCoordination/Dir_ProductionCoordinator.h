#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Working     UMETA(DisplayName = "Working"), 
    Waiting     UMETA(DisplayName = "Waiting"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Foundation  UMETA(DisplayName = "Foundation"),
    WorldBuild  UMETA(DisplayName = "World Building"),
    Characters  UMETA(DisplayName = "Characters"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    Polish      UMETA(DisplayName = "Polish")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime CompletionTime;

    FDir_AgentTask()
    {
        Priority = 0;
        Status = EDir_AgentStatus::Ready;
        EstimatedHours = 1.0f;
        StartTime = FDateTime::Now();
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestonePhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CriticalPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString BlockingReason;

    FDir_MilestoneProgress()
    {
        Phase = EDir_MilestonePhase::Foundation;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
        bIsBlocked = false;
    }
};

/**
 * Studio Director's Production Coordination Component
 * Manages the 19-agent production pipeline for Milestone 1: "Walk Around" prototype
 * Tracks dependencies, critical path, and ensures each agent produces concrete deliverables
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === MILESTONE MANAGEMENT ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneProgress CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_MilestoneProgress> MilestoneHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetLimit;

    // === AGENT COORDINATION ===

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentStart(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkAgentStarted(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkAgentComplete(const FString& AgentName, const TArray<FString>& CompletedDeliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetNextReadyAgent();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalPathStatus();

    // === MILESTONE VALIDATION ===

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidatePlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateTerrainVariation();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateDinosaurModels();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateSurvivalHUD();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateMilestoneCompletion();

    // === REPORTING ===

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionMetrics();

    UFUNCTION(CallInEditor, Category = "Production")
    void DebugPrintStatus();

private:
    // Internal tracking
    TMap<FString, int32> AgentTaskIndices;
    FDateTime LastStatusUpdate;
    
    void UpdateMilestoneProgress();
    void CheckDependencies();
    void ValidateCriticalPath();
    
    // Milestone 1 specific validation helpers
    bool CheckCharacterController();
    bool CheckLandscapeActors();
    bool CheckDinosaurActors();
    bool CheckUIElements();
};