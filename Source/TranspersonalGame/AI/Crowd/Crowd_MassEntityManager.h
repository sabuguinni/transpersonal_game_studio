#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxEntities = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    ECrowd_BehaviorType DefaultBehavior = ECrowd_BehaviorType::Wandering;

    FCrowd_EntitySpawnConfig()
    {
        MaxEntities = 100;
        SpawnRadius = 1000.0f;
        SpawnRate = 5.0f;
        DefaultBehavior = ECrowd_BehaviorType::Wandering;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 5000.0f;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1500.0f;
        LowDetailDistance = 3000.0f;
        CullDistance = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(const FVector& Location, const FCrowd_EntitySpawnConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void DespawnCrowdEntities(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetActiveCrowdEntityCount() const;

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetCrowdBehavior(ECrowd_BehaviorType NewBehavior, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void AddGatheringPoint(const FVector& Location, float Radius, ECrowd_BehaviorType BehaviorType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void RemoveGatheringPoint(const FVector& Location);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCrowdSimulationFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(float TargetFPS);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_EntitySpawnConfig DefaultSpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PerformanceTargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceMonitoring = true;

private:
    // Mass Entity System References
    UMassEntitySubsystem* MassEntitySubsystem;
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Crowd Management
    TArray<FVector> GatheringPoints;
    TArray<ECrowd_BehaviorType> GatheringPointBehaviors;
    TArray<float> GatheringPointRadii;

    // Performance Tracking
    float CurrentSimulationFPS;
    float LastFrameTime;
    int32 ActiveEntityCount;

    // Internal Methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void AdaptLODBasedOnPerformance();
    bool IsMassEntitySystemAvailable() const;
    void CleanupMassEntitySystem();
};