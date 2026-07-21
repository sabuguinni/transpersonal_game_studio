#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator
 * Manages the overall production pipeline and agent coordination
 * Tracks development milestones and ensures quality gates
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Error           UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CharacterActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TerrainActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString LastUpdateTime;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        TerrainActors = 0;
        CompletionPercentage = 0.0f;
        LastUpdateTime = TEXT("Never");
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString LastOutput;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("Unknown");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("No Task");
        Progress = 0.0f;
        LastOutput = TEXT("No Output");
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Prototype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CycleID = TEXT("PROD_CYCLE_AUTO_20260611_009");

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const { return ProductionMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAgentTasks() const { return AgentTasks; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void RunProductionDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

protected:
    UFUNCTION()
    void OnProductionPhaseChanged();

private:
    void CountActorsByType();
    void ValidateMinimumViablePrototype();
    void LogProductionStatus();
};