#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingTypes.h"
#include "MassEntityConfigAsset.h"
#include "MassSpawnerSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 EntityCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bIsActive = true;
};

UENUM(BlueprintType)
enum class ECrowd_EntityBehavior : uint8
{
    Wandering,
    Following,
    Fleeing,
    Gathering,
    Hunting,
    Resting
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UObject
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeMassSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnEntityCluster(const FCrowd_EntitySpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityBehavior(ECrowd_EntityBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetEntityCount(int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void DestroyAllEntities();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mass Entity")
    void TestSpawnEntities();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    TArray<FCrowd_EntitySpawnData> SpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntityCount = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    ECrowd_EntityBehavior CurrentBehavior = ECrowd_EntityBehavior::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float EntitySpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float EntityDetectionRadius = 500.0f;

private:
    UPROPERTY()
    TWeakObjectPtr<UWorld> CachedWorld;

    UPROPERTY()
    TWeakObjectPtr<UMassEntitySubsystem> MassSubsystem;

    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    void ProcessEntityMovement();
    void UpdateEntityLOD();
    void HandleEntityCollisions();
};