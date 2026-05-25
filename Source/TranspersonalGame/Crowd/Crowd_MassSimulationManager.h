#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 1000.0f;

    FCrowd_EntityConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        MovementSpeed = 200.0f;
        WanderRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomePopulation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 PopulationDensity = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FMassEntityHandle> ActiveEntities;

    FCrowd_BiomePopulation()
    {
        BiomeType = EBiomeType::Savanna;
        BiomeCenter = FVector::ZeroVector;
        PopulationDensity = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core simulation methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FVector& Location, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDestination(const FVector& Destination);

    // Biome population management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PopulateBiome(EBiomeType BiomeType, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateBiomePopulations();

    // LOD and performance management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void CullDistantEntities();

    // Query and debug methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetEntityPositions() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestCrowd();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    FCrowd_EntityConfig EntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    TArray<FCrowd_BiomePopulation> BiomePopulations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float CullDistance = 10000.0f;

    // Runtime state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd State")
    int32 TotalActiveEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd State")
    bool bSimulationActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd State")
    float LastUpdateTime = 0.0f;

    // Mass Entity subsystem reference
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem = nullptr;

private:
    // Internal methods
    void InitializeBiomePopulations();
    void UpdateEntityMovement(float DeltaTime);
    void ProcessLODUpdates();
    FVector GetRandomPositionInBiome(EBiomeType BiomeType) const;
    bool IsEntityInCullRange(const FVector& EntityPosition) const;
};