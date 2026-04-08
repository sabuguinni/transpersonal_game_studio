#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct FPERSONALGAME_API FDinosaurHerdData
{
    GENERATED_BODY()

    // Espécie da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    // Tamanho da manada (min/max)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 50;

    // Densidade por km²
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PopulationDensityPerKm2 = 10.0f;

    // Comportamento da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockingRadius = 2000.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 500.0f; // cm

    // Padrões de migração
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMigratory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationDistance = 50000.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSeasonDuration = 3600.0f; // segundos
};

USTRUCT(BlueprintType)
struct FPERSONALGAME_API FPredatorPackData
{
    GENERATED_BODY()

    // Espécie do grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    // Tamanho do grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinPackSize = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPackSize = 8;

    // Território
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 10000.0f; // cm

    // Comportamento de caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingSpeed = 800.0f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 5000.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinationRadius = 1500.0f; // cm
};

USTRUCT(BlueprintType)
struct FPERSONALGAME_API FBiomePopulationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeName;

    // Configurações de manadas herbívoras
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDinosaurHerdData> HerbivoreHerds;

    // Configurações de grupos predadores
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPredatorPackData> PredatorPacks;

    // Densidade máxima de entidades por área
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxEntitiesPerSquareKm = 200;

    // Distância de spawn do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSpawnDistanceFromPlayer = 5000.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpawnDistanceFromPlayer = 20000.0f; // cm
};

/**
 * Subsistema responsável pela simulação de multidões de dinossauros
 * Gere manadas de herbívoros, grupos de predadores e comportamentos emergentes
 */
UCLASS()
class FPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Configuração do sistema
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetBiomeConfiguration(const FString& BiomeName, const FBiomePopulationConfig& Config);

    // Spawning dinâmico
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerdInArea(const FVector& Location, float Radius, const FDinosaurHerdData& HerdData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnPredatorPackInArea(const FVector& Location, float Radius, const FPredatorPackData& PackData);

    // Gestão populacional
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdatePopulationDensity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCurrentPopulationDensity(const FVector& Location, float Radius) const;

    // Comportamentos emergentes
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPredatorHuntBehavior(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerHerdMigrationBehavior(const FString& SpeciesName);

    // Debug e visualização
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ToggleCrowdDebugVisualization(bool bEnabled);

protected:
    // Referências aos sistemas Mass
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Configurações por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<FString, FBiomePopulationConfig> BiomeConfigurations;

    // Configurações globais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxTotalEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency = 1.0f; // segundos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float CullingDistance = 30000.0f; // cm

    // Estado interno
    UPROPERTY()
    TMap<FString, int32> ActiveHerdCounts;

    UPROPERTY()
    TMap<FString, int32> ActivePackCounts;

    UPROPERTY()
    FVector LastPlayerLocation;

    // Timers
    FTimerHandle PopulationUpdateTimer;
    FTimerHandle MigrationUpdateTimer;

private:
    void UpdatePopulationTick();
    void UpdateMigrationTick();
    void CullDistantEntities(const FVector& PlayerLocation);
    bool CanSpawnInArea(const FVector& Location, float Radius, int32 RequestedCount) const;
    FVector FindSuitableSpawnLocation(const FVector& Center, float Radius) const;
};