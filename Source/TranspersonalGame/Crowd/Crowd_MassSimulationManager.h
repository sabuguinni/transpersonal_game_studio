#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntityManager.h"
#include "MassProcessingTypes.h"
#include "MassSpawnerTypes.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float UpdateFrequency = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance3 = 15000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    int32 LODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    bool bIsActive = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SimulationSettings SimulationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    TArray<FCrowd_EntityData> ActiveEntities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    int32 CurrentEntityCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    UStaticMesh* EntityMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<UStaticMesh*> LODMeshes;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnEntities(int32 Count, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateEntityLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllEntities();

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const { return CurrentEntityCount; }

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    float GetPerformanceMetric() const;

private:
    void UpdateEntityMovement(float DeltaTime);
    void ProcessLODSystem();
    FVector GetRandomSpawnLocation(FVector Center, float Radius);
    bool IsValidSpawnLocation(FVector Location);

    UPROPERTY()
    class UMassEntityManager* EntityManager;

    float LastUpdateTime = 0.0f;
    bool bSimulationInitialized = false;
};