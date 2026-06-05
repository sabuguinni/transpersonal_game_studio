#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntitySubsystem.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_EntityState : uint8
{
    Idle = 0,
    Moving,
    Gathering,
    Fleeing,
    Following,
    Hunting,
    Resting
};

UENUM(BlueprintType)
enum class ECrowd_DensityLevel : uint8
{
    VeryLow = 0,    // 0-10 entities per 100m²
    Low,            // 10-25 entities per 100m²
    Medium,         // 25-50 entities per 100m²
    High,           // 50-100 entities per 100m²
    VeryHigh        // 100+ entities per 100m²
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Config")
    int32 MaxEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Config")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Config")
    ECrowd_DensityLevel DefaultDensity = ECrowd_DensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Config")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Config")
    float DetectionRadius = 500.0f;

    FCrowd_EntityConfig()
    {
        MaxEntities = 50000;
        SpawnRadius = 5000.0f;
        DefaultDensity = ECrowd_DensityLevel::Medium;
        MovementSpeed = 150.0f;
        DetectionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD0_Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD1_Distance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD2_Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD3_Distance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    FCrowd_PerformanceSettings()
    {
        LOD0_Distance = 1000.0f;
        LOD1_Distance = 2500.0f;
        LOD2_Distance = 5000.0f;
        LOD3_Distance = 10000.0f;
        MaxVisibleEntities = 1000;
        UpdateFrequency = 0.1f;
    }
};

/**
 * Mass Entity Manager for massive crowd simulation (50k+ entities)
 * Handles prehistoric tribal crowds with advanced pathfinding and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // === CORE CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    FCrowd_EntityConfig EntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FCrowd_PerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    TArray<FVector> GatheringPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    TArray<FVector> PatrolRoutes;

    // === MASS ENTITY SYSTEM ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FMassEntityTemplate EntityTemplate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    TArray<FMassEntityHandle> SpawnedEntities;

    // === CROWD MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(int32 Count, FVector SpawnCenter, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDensity(ECrowd_DensityLevel NewDensity, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerCrowdBehavior(ECrowd_EntityState NewState, FVector TriggerLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdMovement(FVector TargetLocation, float MovementSpeed);

    // === PERFORMANCE OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEntities(FVector ViewerLocation, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    // === CROWD ANALYTICS ===
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    ECrowd_DensityLevel CalculateDensityAtLocation(FVector Location, float SampleRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    TArray<FVector> GetCrowdHotspots() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    float GetAverageCrowdSpeed() const;

protected:
    // === INTERNAL SYSTEMS ===
    void InitializeMassEntitySystem();
    void UpdateEntityStates(float DeltaTime);
    void ProcessCrowdBehaviors(float DeltaTime);
    void ManagePerformance(float DeltaTime);

    // === CROWD BEHAVIOR ===
    void ProcessIdleBehavior(FMassEntityHandle Entity);
    void ProcessMovementBehavior(FMassEntityHandle Entity);
    void ProcessGatheringBehavior(FMassEntityHandle Entity);
    void ProcessFleeingBehavior(FMassEntityHandle Entity);

    // === PERFORMANCE TRACKING ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActiveEntityCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastOptimizationTime;

private:
    // === INTERNAL STATE ===
    TMap<FMassEntityHandle, ECrowd_EntityState> EntityStates;
    TMap<FMassEntityHandle, FVector> EntityTargets;
    TArray<FVector> CrowdHotspots;
    float PerformanceTimer;
    bool bIsInitialized;
};