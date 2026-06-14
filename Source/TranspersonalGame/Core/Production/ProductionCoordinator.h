#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototype,
    Alpha,
    Beta,
    Gold
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TerrainCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionPercentage;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainCount = 0;
        CompletionPercentage = 0.0f;
    }
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
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CycleID;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDesc, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeProductionCycle();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateProductionReport();
};

#include "ProductionCoordinator.generated.h"