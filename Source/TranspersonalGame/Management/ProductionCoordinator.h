#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    WalkAroundPrototype,
    AlphaVersion,
    BetaVersion,
    GoldMaster
};

USTRUCT(BlueprintType)
struct FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString MilestoneName;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime TargetDate;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("");
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float TaskProgress;

    FDir_AgentTaskStatus()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        bIsActive = false;
        TaskProgress = 0.0f;
    }
};

/**
 * Production Coordinator - Studio Director's central management system
 * Tracks milestone progress, agent coordination, and build status
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_MilestoneStatus> Milestones;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTaskStatus> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TotalActorsInMap;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bBridgeOperational;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProduction();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(int32 AgentNumber, const FString& TaskDescription, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateBridgeConnection();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

    UFUNCTION(BlueprintPure, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintPure, Category = "Production")
    bool IsWalkAroundPrototypeComplete() const;
};