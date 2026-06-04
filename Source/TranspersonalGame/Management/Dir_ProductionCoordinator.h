#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FDateTime LastUpdateTime;

    FDir_AgentStatus()
    {
        AgentName = TEXT("");
        AgentNumber = 0;
        bIsActive = false;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        LastUpdateTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasThirdPersonCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TotalAgentsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float OverallProgress;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("WALK AROUND PROTOTYPE");
        bHasThirdPersonCharacter = false;
        bHasLandscape = false;
        bHasMovement = false;
        bHasDinosaurs = false;
        bHasLighting = false;
        TotalAgentsActive = 0;
        OverallProgress = 0.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneProgress CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCycles;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void InitializeAgentTracking();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentNumber, const FString& TaskDescription, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CheckMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_MilestoneProgress GetCurrentMilestoneStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentStatus> GetAllAgentStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionMetrics();

private:
    void InitializeDefaultAgents();
    void ScanWorldForMilestoneComponents();
    float CalculateOverallProgress() const;
};