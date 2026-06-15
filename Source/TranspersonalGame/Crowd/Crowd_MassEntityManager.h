#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Entity")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Entity")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Entity")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(BlueprintReadWrite, Category = "Entity")
    int32 GroupID;

    UPROPERTY(BlueprintReadWrite, Category = "Entity")
    float LODDistance;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        BehaviorState = ECrowd_BehaviorState::Idle;
        GroupID = -1;
        LODDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 MaxHighDetailEntities;

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 MaxMediumDetailEntities;

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 MaxLowDetailEntities;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 2500.0f;
        LowDetailDistance = 5000.0f;
        MaxHighDetailEntities = 50;
        MaxMediumDetailEntities = 100;
        MaxLowDetailEntities = 200;
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

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(BlueprintReadWrite, Category = "Mass Entity")
    TArray<FCrowd_EntityData> Entities;

    UPROPERTY(BlueprintReadWrite, Category = "Mass Entity")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntities;

    UPROPERTY(BlueprintReadWrite, Category = "Mass Entity")
    float UpdateFrequency;

    UPROPERTY(BlueprintReadWrite, Category = "Mass Entity")
    bool bEnableLODSystem;

    UPROPERTY(BlueprintReadWrite, Category = "Mass Entity")
    bool bEnablePathfinding;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnEntity(FVector Location, ECrowd_BehaviorState InitialState);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void RemoveEntity(int32 EntityIndex);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityLOD(int32 EntityIndex, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityBehavior(int32 EntityIndex, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void ProcessEntityMovement(int32 EntityIndex, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    TArray<FCrowd_EntityData> GetEntitiesInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mass Entity")
    void InitializeMassSystem();

private:
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    TArray<int32> HighDetailEntities;

    UPROPERTY()
    TArray<int32> MediumDetailEntities;

    UPROPERTY()
    TArray<int32> LowDetailEntities;

    void UpdateLODSystem();
    void OptimizeEntityCount();
    FVector CalculateEntityMovement(const FCrowd_EntityData& Entity, float DeltaTime);
};

#include "Crowd_MassEntityManager.generated.h"