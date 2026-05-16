#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntitySpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float MinSpawnDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float LODDistance2 = 3000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FlockingStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AvoidanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bEnableObstacleAvoidance = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntitySpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntitySpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    FCrowd_SpawnParameters SpawnParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    FCrowd_EntityBehavior EntityBehavior;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnMassEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void UpdateLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SetSpawnCenter(const FVector& NewCenter);

protected:
    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    void InitializeMassSystem();
    void CreateEntityArchetype();
    FVector GetRandomSpawnLocation() const;
    bool IsValidSpawnLocation(const FVector& Location) const;
    void UpdateEntityBehaviors();

private:
    FMassArchetypeHandle EntityArchetype;
    bool bIsInitialized = false;
    float LastUpdateTime = 0.0f;
    const float UpdateInterval = 0.1f;
};