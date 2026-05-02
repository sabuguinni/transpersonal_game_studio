#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director Component - Coordinates all game systems and agent tasks
 * Manages the production pipeline and ensures milestone completion
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_StudioDirector : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_StudioDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckMilestone1Completion();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateEngineArchitect();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateCharacterSystems();

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RunProductionChecks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CleanupDuplicateActors();

protected:
    // Production State
    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    bool bProductionInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    bool bMilestone1Complete;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    int32 CurrentCycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    float ProductionBudgetUsed;

    UPROPERTY(BlueprintReadOnly, Category = "Production State")
    float ProductionBudgetLimit;

    // Agent Task Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Agent Tasks")
    TMap<FString, FString> AgentTaskAssignments;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Tasks")
    TMap<FString, bool> AgentTaskCompletion;

    // Map State Validation
    UPROPERTY(BlueprintReadOnly, Category = "Map Validation")
    int32 TerrainActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Map Validation")
    int32 DinosaurActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Map Validation")
    int32 LightingActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Map Validation")
    int32 PlayerCharacterCount;

private:
    // Internal Production Management
    void UpdateProductionMetrics();
    void ValidateActorCounts();
    void GenerateProductionReport();
    void ScheduleNextAgentCycle();

    // Milestone Validation
    bool ValidatePlayerMovement();
    bool ValidateTerrainPresence();
    bool ValidateDinosaurPresence();
    bool ValidateLightingSetup();
    bool ValidateGameModeActive();
};