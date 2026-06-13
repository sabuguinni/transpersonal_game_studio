#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float LODDistance_High = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float LODDistance_Medium = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float LODDistance_Low = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bEnablePathfinding = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bEnableLODSystem = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 EntityID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float LastUpdateTime = 0.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdEntities(int32 Count, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetCrowdBehaviorMode(ECrowd_BehaviorMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllCrowdEntities();

    // Pathfinding
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void CreatePathfindingZone(FVector Center, float Radius, FString ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdMovement(float DeltaTime);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetSpawnConfiguration(const FCrowd_EntitySpawnConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Crowd")
    FCrowd_EntitySpawnConfig GetSpawnConfiguration() const { return SpawnConfig; }

    // Statistics
    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetHighLODCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetMediumLODCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetLowLODCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FCrowd_EntitySpawnConfig SpawnConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TArray<FVector> PathfindingWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_BehaviorMode CurrentBehaviorMode = ECrowd_BehaviorMode::Wandering;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float LastLODUpdateTime = 0.0f;

private:
    void UpdateEntityLOD(FCrowd_EntityData& Entity, float DistanceToPlayer);
    void ApplyMovementBehavior(FCrowd_EntityData& Entity, float DeltaTime);
    FVector GetRandomWaypoint() const;
    void OptimizePerformance();
};