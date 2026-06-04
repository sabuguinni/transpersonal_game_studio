#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator - Cycle 004
 * Manages agent task distribution and milestone tracking for playable prototype
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 ActiveActorsInMap = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeCycle004Tasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& ExpectedDeliverable);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkTaskCompleted(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RefreshProductionStatus();
};