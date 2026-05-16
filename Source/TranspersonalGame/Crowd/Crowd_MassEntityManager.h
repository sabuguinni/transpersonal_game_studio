#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingTypes.h"
#include "MassEntityConfigAsset.h"
#include "MassSpawnerSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Crowd_MassEntityManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_EntityType : uint8
{
    Civilian,
    Warrior,
    Hunter,
    Child,
    Elder
};

USTRUCT(BlueprintType)
struct FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_EntityType EntityType = ECrowd_EntityType::Civilian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 SpawnCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float WanderRadius = 2000.0f;
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

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdEntities(const FCrowd_SpawnConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdBehavior(ECrowd_EntityType EntityType, float NewSpeed, float NewRadius);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    TArray<FCrowd_SpawnConfig> DefaultSpawnConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float CrowdDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance3 = 15000.0f;

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    TArray<FMassEntityHandle> SpawnedEntities;
    FMassArchetypeHandle CrowdArchetype;
    
    float LastUpdateTime;
    int32 TotalSpawnedEntities;

    void InitializeMassSystem();
    void CreateCrowdArchetype();
    void ProcessEntityLOD();
    void UpdateEntityBehaviors(float DeltaTime);
    FVector GetRandomSpawnLocation(const FCrowd_SpawnConfig& Config) const;
};