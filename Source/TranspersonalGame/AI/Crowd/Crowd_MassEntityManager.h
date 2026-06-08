#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassTrafficSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxEntities = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float DensityPerSquareMeter = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    TArray<TSoftObjectPtr<UStaticMesh>> HumanMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    bool bEnablePathfinding = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    bool bEnableLODSystem = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntity();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(int32 Count, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDestination(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestCrowd();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_EntitySpawnConfig SpawnConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 ActiveEntityCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bMassSystemInitialized;

private:
    UMassEntitySubsystem* MassEntitySubsystem;
    UMassSpawnerSubsystem* MassSpawnerSubsystem;
    UMassSimulationSubsystem* MassSimulationSubsystem;

    void InitializeSubsystems();
    void ConfigureMassTraits();
    void SetupLODSystem();
};