#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorType BehaviorType = ECrowd_BehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DetectionRadius = 500.0f;
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

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdEntities(int32 EntityCount, const FVector& CenterLocation, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdBehavior(ECrowd_BehaviorType NewBehavior);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ManagerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CrowdDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorType DefaultBehavior = ECrowd_BehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FCrowd_EntitySpawnData> SpawnedEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float UpdateFrequency = 0.1f;

private:
    void InitializeMassEntity();
    void UpdateEntityBehaviors();
    void CleanupInvalidEntities();

    FMassEntityHandle CreateCrowdEntity(const FCrowd_EntitySpawnData& SpawnData);
    void DestroyCrowdEntity(FMassEntityHandle EntityHandle);

    TArray<FMassEntityHandle> ActiveEntities;
    float LastUpdateTime = 0.0f;
    UMassEntitySubsystem* MassEntitySubsystem = nullptr;
};