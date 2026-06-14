#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "ProductionDirector.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString LastDeliverable;

    FDir_AgentStatus()
    {
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        bIsActive = false;
        LastDeliverable = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float Priority;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
        Priority = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProductionDirector : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentStatus> AgentStatuses;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CharacterCount;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, const FString& Task, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CoordinateAgentTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RunProductionAnalysis();

private:
    void SetupDefaultMilestones();
    void UpdateProductionMetrics();
    float CalculateOverallProgress();
    void LogProductionState();
};