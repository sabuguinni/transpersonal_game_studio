#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "StudioDirectorCoordinator.generated.h"

/**
 * Studio Director Coordinator - Central command system for managing
 * the 19-agent production pipeline and ensuring milestone delivery
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent coordination system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    TArray<FDir_AgentTaskInfo> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    TArray<FDir_MilestoneInfo> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    FDir_BuildStatus CurrentBuildStatus;

    // Production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, float ProgressPercent);

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool ValidateMinPlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void TriggerAgentSequence();

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Director")
    void ValidateBuildHealth();

    UFUNCTION(BlueprintCallable, Category = "Director")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<AActor*> GetDinosaurActors();

    // Emergency coordination
    UFUNCTION(BlueprintCallable, Category = "Director")
    void HandleAgentTimeout(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void RequestProductionHalt(const FString& Reason);

private:
    // Internal state tracking
    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    int32 CurrentCycleNumber;

    UPROPERTY()
    bool bProductionHalted;

    // Helper functions
    void LogProductionStatus();
    void CheckMemoryUsage();
    bool IsAgentResponsive(int32 AgentNumber);
};