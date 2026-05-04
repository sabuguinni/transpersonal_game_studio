#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProductionCoordinator, Log, All);

UENUM(BlueprintType)
enum class ETaskPriority : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FMilestone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    TArray<FString> RequiredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    FDateTime StartDate;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone")
    FDateTime CompletionDate;

    FMilestone()
    {
        Name = TEXT("Unnamed Milestone");
        Description = TEXT("No description");
        StartDate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FAgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    ETaskPriority Priority = ETaskPriority::Normal;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    bool bBlocking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    float ProgressPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FDateTime LastUpdate;

    FAgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Studio Director's oversight system
 * Tracks milestone progress, coordinates agent tasks, identifies blocking issues
 * Critical for ensuring Milestone 1 (Walk Around prototype) completion
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === MILESTONE TRACKING ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Milestones")
    FMilestone Milestone1_WalkAround;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestone1Progress();

    UFUNCTION(BlueprintPure, Category = "Production")
    float GetMilestone1Progress() const { return Milestone1_WalkAround.CompletionPercentage; }

    UFUNCTION(BlueprintPure, Category = "Production")
    bool IsMilestone1Complete() const { return Milestone1_WalkAround.bCompleted; }

    // === AGENT COORDINATION ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Agent Coordination")
    TMap<int32, FAgentTask> AgentTasks;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportAgentProgress(int32 AgentID, const FString& TaskUpdate, float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockingIssueDetected(const FString& IssueDescription, int32 ResponsibleAgentID);

    // === SYSTEM VALIDATION ===
    
    UFUNCTION(BlueprintPure, Category = "Validation")
    bool CheckPlayerCharacterExists();

    UFUNCTION(BlueprintPure, Category = "Validation")
    bool CheckTerrainExists();

    UFUNCTION(BlueprintPure, Category = "Validation")
    bool CheckDinosaurActorsExist();

    UFUNCTION(BlueprintPure, Category = "Validation")
    bool CheckLightingSystemExists();

    UFUNCTION(BlueprintPure, Category = "Validation")
    bool CheckMovementInputWorks();

protected:
    // === INTERNAL SYSTEMS ===
    
    void InitializeAgentTasks();
    void CheckProductionState();
    void OnMilestone1Completed();
    void InitializeMilestone2();

    FTimerHandle ProductionCheckTimer;
};