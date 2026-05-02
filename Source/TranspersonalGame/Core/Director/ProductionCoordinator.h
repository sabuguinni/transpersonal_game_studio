#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordinator
 * Manages agent task dispatch and milestone tracking for the development pipeline
 * Ensures each agent produces concrete deliverables toward Milestone 1
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Production milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentMilestone = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID = "PROD_CYCLE_AUTO_20260502_004";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetUsedToday = 28.63f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetLimitDaily = 150.0f;

    // Agent coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> PendingTasks;

    // Milestone 1 requirements tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bCharacterMovementReady = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bTerrainComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bDinosaursPlaced = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bLightingSetup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bCameraSystemReady = false;

    // Production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void DispatchAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckPlayablePrototypeReady();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionStatusReport();

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void MarkTaskCompleted(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FString> GetPendingTasksForAgent(const FString& AgentName);

protected:
    // Internal coordination mesh for visualization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinationMesh;

    // Production timer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionTimer = 0.0f;

    // Internal functions
    void UpdateProductionMetrics();
    void CheckAgentDeliverables();
    void EnsureMinimalPlayableElements();
};