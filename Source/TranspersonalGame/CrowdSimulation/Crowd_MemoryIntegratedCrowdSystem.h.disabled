#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MemoryIntegratedCrowdSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MemoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> RememberedLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<float> ThreatLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    FCrowd_MemoryData()
    {
        MemoryDecayRate = 0.1f;
        MaxMemoryEntries = 100;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TerritorialBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float DefenseAggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsAlpha;

    FCrowd_TerritorialBehavior()
    {
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 1000.0f;
        DefenseAggression = 0.5f;
        bIsAlpha = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LearningBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float LearningRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    TMap<FString, float> BehaviorWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    TArray<FVector> SuccessfulPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    TArray<FVector> FailedPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    int32 ExperiencePoints;

    FCrowd_LearningBehavior()
    {
        LearningRate = 0.05f;
        ExperiencePoints = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MemoryIntegratedCrowdSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MemoryIntegratedCrowdSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SystemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float MemoryUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float TerritorialUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float LearningUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    TArray<FCrowd_MemoryData> CrowdMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territorial System")
    TArray<FCrowd_TerritorialBehavior> TerritorialBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning System")
    TArray<FCrowd_LearningBehavior> LearningBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Integration")
    bool bEnableCombatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Integration")
    float CombatMemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Integration")
    TArray<FVector> CombatHotspots;

private:
    float MemoryTimer;
    float TerritorialTimer;
    float LearningTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void InitializeMemorySystem();

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void UpdateCrowdMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void AddMemoryEntry(int32 EntityIndex, FVector Location, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void InitializeTerritorialBehaviors();

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void UpdateTerritorialBehaviors(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void DefendTerritory(int32 EntityIndex, FVector IntruderLocation);

    UFUNCTION(BlueprintCallable, Category = "Learning System")
    void InitializeLearningSystem();

    UFUNCTION(BlueprintCallable, Category = "Learning System")
    void UpdateLearningBehaviors(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Learning System")
    void RecordBehaviorOutcome(int32 EntityIndex, FString BehaviorType, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void IntegrateWithCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void ProcessCombatMemory(FVector CombatLocation, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetOptimalPath(FVector Start, FVector End, int32 EntityIndex);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsLocationSafe(FVector Location, int32 EntityIndex);

    UFUNCTION(BlueprintCallable, Category = "Massive Simulation")
    void SpawnMassiveCrowd(int32 EntityCount, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Massive Simulation")
    void UpdateMassiveSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void ProcessEcosystemInteractions();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdatePredatorPreyDynamics();
};