#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director Component - Coordinates all game systems and agent outputs
 * Manages the integration of work from all 19 specialized agents
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentCoordination();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateGameSystems();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetupBiomeCoordinators();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetBiomeCenter(EDir_BiomeType BiomeType);

    // Agent Task Distribution
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDeliverables();

protected:
    // Agent Status Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TMap<FString, bool> AgentCompletionStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FString> AgentCurrentTasks;

    // Biome Coordination
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TMap<EDir_BiomeType, FVector> BiomeCenters;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> BiomeCoordinatorActors;

    // Production Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    int32 CompilationErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    float ProductionEfficiencyScore;

    // Critical System Flags
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bLandscapeExpanded;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bDinosaurActorsSpawned;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bSurvivalHUDActive;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bCompilationClean;

private:
    // Internal coordination helpers
    void UpdateProductionMetrics();
    void CheckCriticalSystems();
    void LogAgentProgress();
};

/**
 * Studio Director Game Mode - Manages overall game coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AStudioDirectorGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ExecuteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinimumViablePrototype();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    UStudioDirectorComponent* DirectorComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    int32 CurrentProductionCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bMinimumViablePrototypeReached;
};