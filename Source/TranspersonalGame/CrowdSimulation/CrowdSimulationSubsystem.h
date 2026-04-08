#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntityConfigAsset;
class AMassSpawner;

UENUM(BlueprintType)
enum class ECrowdType : uint8
{
    DinosaurHerd,
    PterosaurFlock,
    FishSchool,
    InsectSwarm,
    MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FPREHISTORICCROWDCONFIG
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdType CrowdType = ECrowdType::DinosaurHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AvoidanceRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance1 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance3 = 10000.0f;
};

/**
 * Subsystem responsável pela simulação de multidões de criaturas pré-históricas
 * Usa Mass AI para simular até 50.000 agentes simultâneos
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowd(const FPREHISTORICCROWDCONFIG& Config, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowds();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalCrowdDensity(float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalAgentCount() const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPerformanceMode(bool bHighPerformance);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODDistances(float Distance1, float Distance2, float Distance3);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPanicBehavior(const FVector& ThreatLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerFeedingBehavior(const FVector& FoodLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetWeatherInfluence(float WindStrength, const FVector& WindDirection);

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    UPROPERTY()
    TArray<TObjectPtr<AMassSpawner>> ActiveSpawners;

    UPROPERTY()
    TMap<ECrowdType, TObjectPtr<UMassEntityConfigAsset>> CrowdConfigs;

    UPROPERTY()
    float GlobalDensityMultiplier = 1.0f;

    UPROPERTY()
    bool bHighPerformanceMode = false;

    UPROPERTY()
    int32 MaxTotalAgents = 50000;

    UPROPERTY()
    int32 CurrentAgentCount = 0;

private:
    void InitializeCrowdConfigs();
    void SetupMassProcessors();
    AMassSpawner* CreateSpawnerForCrowdType(ECrowdType CrowdType, const FVector& Location);
    void OptimizePerformance();
    void UpdateAgentCounts();
};