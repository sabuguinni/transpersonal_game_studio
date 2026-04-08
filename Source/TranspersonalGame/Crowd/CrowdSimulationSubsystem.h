#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntityConfigAsset;
class AMassSpawner;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ADinosaurPawn> DinosaurClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnDensityPerKm2 = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 5000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDinosaurHerdConfig> HerbivoreConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDinosaurHerdConfig> CarnivoreConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BiomeType = 0.0f; // 0=Forest, 1=Plains, 2=Swamp, 3=Mountains
};

/**
 * Subsystem responsável pela simulação de massas de dinossauros
 * Utiliza Mass Entity Framework para performance de até 50.000 agentes
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

    // Crowd Simulation Interface
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerdInZone(const FEcosystemZone& Zone, const FDinosaurHerdConfig& HerdConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateEcosystemDynamics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveDinosaurCount() const { return ActiveDinosaurCount; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSimulationEnabled(bool bEnabled);

protected:
    // Mass Entity Integration
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Ecosystem Configuration
    UPROPERTY(EditAnywhere, Category = "Ecosystem")
    TArray<FEcosystemZone> EcosystemZones;

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxSimultaneousAgents = 50000;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float UpdateFrequency = 0.1f; // 10 updates per second

    // Runtime State
    UPROPERTY()
    int32 ActiveDinosaurCount = 0;

    UPROPERTY()
    bool bSimulationEnabled = true;

    FTimerHandle EcosystemUpdateTimer;

private:
    void SetupMassEntityConfiguration();
    void CreateEcosystemZones();
    void SpawnInitialPopulation();
    void ProcessPredatorPreyInteractions();
    void UpdateHerdBehaviors();
    void ManagePopulationDensity();
};