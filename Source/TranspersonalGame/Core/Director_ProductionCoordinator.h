#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Director_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldBuilding,
    CharacterSystems,
    GameplayFeatures,
    Polish,
    Testing,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 EstimatedCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString DeliverablePath;

    FDir_AgentTask()
    {
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        EstimatedCycles = 1;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeDistribution
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentActorCount;

    FDir_BiomeDistribution()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        TargetActorCount = 0;
        CurrentActorCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleDuration;

    // Biome Distribution Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FDir_BiomeDistribution> BiomeDistributions;

    // Critical Path Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    TArray<FString> CriticalPathTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bPlayerControllerReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bSurvivalSystemReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bDinosaurAIReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bMinPlayableMapReady;

    // Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProduction();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateCriticalPath();

    // Biome Management Functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void UpdateBiomeActorCount(const FString& BiomeName, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool ValidateBiomeDistribution();

    // Quality Assurance Functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateMinimumPlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FString> GetBlockingIssues();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateProductionReport();

private:
    void SetupCriticalPathTasks();
    void ValidateAgentDependencies();
    float CalculateProductionProgress();
    void LogProductionMetrics();
};