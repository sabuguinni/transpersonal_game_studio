#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CharacterActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 DinosaurActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TerrainActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 LightingActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float ProductionProgress = 0.0f;
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    float Priority = 1.0f;
};

/**
 * Studio Director's production coordination system
 * Tracks progress across all 19 agents and manages task dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Production metrics tracking
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Agent task management
    UPROPERTY(BlueprintReadOnly, Category = "Tasks")
    TArray<FDir_AgentTask> AgentTasks;

    // Production phase tracking
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CurrentCycle = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentPhase = TEXT("Milestone1_WalkAround");

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPhaseComplete() const;

    // Milestone 1 specific tracking
    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    bool HasPlayableCharacter() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    bool HasTerrainWithVariation() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    bool HasDinosaurActors() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    bool HasBasicLighting() const;

private:
    // Internal tracking
    UPROPERTY()
    float LastMetricsUpdate = 0.0f;

    UPROPERTY()
    bool bInitialized = false;

    void InitializeProductionTasks();
    void ValidateGameState();
};