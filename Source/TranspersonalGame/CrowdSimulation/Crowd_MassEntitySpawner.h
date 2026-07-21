#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassEntitySpawner.generated.h"

/**
 * Spawner de entidades Mass para simulação de multidões de dinossauros
 * Gere o spawn massivo de até 50.000 entidades simultâneas usando Mass Entity
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(CrowdSimulation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassEntitySpawner : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração de spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    int32 MaxEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    float SpawnRadius = 100000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    bool bAutoSpawnOnStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    ECrowd_BiomeType TargetBiome = ECrowd_BiomeType::Savana;

    // Configuração por tipo de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Types")
    TMap<ECrowd_DinosaurType, int32> DinosaurCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Types")
    TMap<ECrowd_DinosaurType, float> SpawnProbabilities;

    // Performance e LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 50000.0f;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnMassEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnDinosaurType(ECrowd_DinosaurType DinosaurType, int32 Count, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    int32 GetCurrentEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODSettings(float NewLOD1, float NewLOD2, float NewCulling);

    // Eventos
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnMassSpawnCompleted(int32 SpawnedCount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEntityCountChanged(int32 NewCount);

private:
    // Referências internas
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Estado interno
    int32 CurrentEntityCount = 0;
    float LastSpawnTime = 0.0f;
    bool bIsSpawning = false;

    // Métodos internos
    void InitializeMassSubsystems();
    FVector GetBiomeSpawnLocation(ECrowd_BiomeType Biome) const;
    void SpawnEntitiesInBatch(int32 BatchSize);
    void UpdateEntityLOD();
    void CleanupDistantEntities();
    
    // Configuração de entidades Mass
    void SetupMassEntityConfig();
    void ConfigureDinosaurFragments(ECrowd_DinosaurType DinosaurType);
};