#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorCoordinator.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString LastOutput;

    FDir_AgentStatus()
    {
        AgentNumber = 0;
        AgentName = TEXT("Unknown");
        CurrentTask = TEXT("Idle");
        CompletionPercentage = 0.0f;
        bIsActive = false;
        LastOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float TerrainCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bPlayablePrototype;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        ActiveActors = 0;
        DinosaurCount = 0;
        TerrainCoverage = 0.0f;
        bPlayablePrototype = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Direction")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Direction")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Direction")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Direction")
    float CycleDuration;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Direction")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Direction")
    void UpdateAgentStatus(int32 AgentNumber, const FString& Task, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Studio Direction")
    void AnalyzeProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Direction")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Direction")
    void CoordinateAgentExecution();

    UFUNCTION(BlueprintCallable, Category = "Studio Direction")
    FString GenerateProductionReport();

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool CheckMilestone1Completion();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    TArray<FString> GetMissingMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void TriggerEmergencyPrototype();

private:
    void InitializeDefaultAgents();
    void UpdateProductionMetrics();
    void CheckAgentDependencies();
    
    UPROPERTY()
    float LastMetricsUpdate;

    UPROPERTY()
    bool bEmergencyMode;
};