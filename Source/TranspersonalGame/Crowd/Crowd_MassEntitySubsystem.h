#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityConfigAsset.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxEntities = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float SpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance1 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODDistance3 = 10000.0f;

    FCrowd_EntityConfig()
    {
        MaxEntities = 5000;
        SpawnRadius = 10000.0f;
        MovementSpeed = 300.0f;
        LODDistance1 = 2000.0f;
        LODDistance2 = 5000.0f;
        LODDistance3 = 10000.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FVector& Center, int32 Count, ECrowd_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdConfig(const FCrowd_EntityConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistances(float LOD1, float LOD2, float LOD3);

protected:
    UPROPERTY()
    FCrowd_EntityConfig CrowdConfig;

    UPROPERTY()
    TArray<FMassEntityHandle> ActiveEntities;

    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    void InitializeMassFramework();
    void CreateEntityArchetype();
    void UpdateEntityLOD(const FMassEntityHandle& Entity, float DistanceToPlayer);
    FVector GetRandomMovementTarget(const FVector& CurrentPosition);

private:
    bool bIsInitialized;
    FMassArchetypeHandle CrowdArchetype;
    float LastUpdateTime;
    int32 CurrentEntityCount;
};