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
    CoreSystems,
    WorldBuilding,
    CharacterSystems,
    GameplayMechanics,
    Polish,
    Testing,
    Release
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bIsBlocked;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString BlockingReason;

    FDir_AgentTaskStatus()
    {
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        bIsBlocked = false;
        BlockingReason = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TerrainActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bHasPlayableCharacter;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainActorCount = 0;
        AverageFrameRate = 0.0f;
        bHasPlayableCharacter = false;
    }
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Manages the 19-agent production pipeline and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    TArray<FDir_AgentTaskStatus> AgentStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    float CycleStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    int32 CurrentCycleNumber;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, const FString& Task, float Completion, bool bBlocked = false, const FString& BlockReason = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeCurrentGameState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void ResolveAgentBlocker(const FString& AgentName);

    // Milestone validation
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool CheckWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool CheckDinosaurPresenceMilestone();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool CheckBasicSurvivalMilestone();

private:
    void UpdateProductionMetrics();
    void LogProductionState();
    void CheckForCriticalBlockers();
};