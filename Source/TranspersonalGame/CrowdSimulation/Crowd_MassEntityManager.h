#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FMassEntityHandle EntityHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 GroupID;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        BehaviorState = ECrowd_BehaviorState::Idle;
        Health = 100.0f;
        Fear = 0.0f;
        GroupID = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    FVector SpawnCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    ECrowd_EntityType EntityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float MaxDistance;

    FCrowd_SpawnConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        SpawnCenter = FVector::ZeroVector;
        EntityType = ECrowd_EntityType::Human;
        MinDistance = 100.0f;
        MaxDistance = 200.0f;
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

    // Mass Entity System Integration
    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void SpawnMassEntities(const FCrowd_SpawnConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void UpdateEntityBehaviors(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void ProcessEntityCollisions();

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void UpdateEntityLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Mass Entity")
    void DestroyAllEntities();

    // Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Entity Management")
    FMassEntityHandle CreateEntity(const FVector& Position, ECrowd_EntityType Type);

    UFUNCTION(BlueprintCallable, Category = "Crowd Entity Management")
    void DestroyEntity(FMassEntityHandle EntityHandle);

    UFUNCTION(BlueprintCallable, Category = "Crowd Entity Management")
    void SetEntityBehavior(FMassEntityHandle EntityHandle, ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Entity Management")
    FCrowd_EntityData GetEntityData(FMassEntityHandle EntityHandle);

    // Group Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Group Management")
    int32 CreateGroup(const TArray<FMassEntityHandle>& Entities);

    UFUNCTION(BlueprintCallable, Category = "Crowd Group Management")
    void SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Group Management")
    void MoveGroup(int32 GroupID, const FVector& TargetLocation);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    float GetAverageFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void SetLODDistance(float Distance);

protected:
    // Mass Entity System
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    // Entity Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Data")
    TArray<FCrowd_EntityData> ActiveEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Data")
    TMap<int32, TArray<FMassEntityHandle>> Groups;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    FCrowd_SpawnConfig DefaultSpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    bool bEnableCollisions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    bool bEnableLOD;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Performance")
    int32 CurrentEntityCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Performance")
    float AverageFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Performance")
    float LastUpdateTime;

private:
    // Internal helpers
    void UpdateEntityPositions(float DeltaTime);
    void ProcessEntityInteractions();
    void OptimizeEntityCount();
    bool IsEntityInLODRange(const FVector& EntityPosition) const;
    void UpdatePerformanceMetrics(float DeltaTime);

    // Group management helpers
    int32 NextGroupID;
    TArray<int32> AvailableGroupIDs;
};