#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntityConfigAsset.h"
#include "GameFramework/Actor.h"
#include "DinosaurMassSpawner.generated.h"

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    Herbivore_Small     UMETA(DisplayName = "Small Herbivores (Compsognathus, etc)"),
    Herbivore_Medium    UMETA(DisplayName = "Medium Herbivores (Triceratops, etc)"),
    Herbivore_Large     UMETA(DisplayName = "Large Herbivores (Brontosaurus, etc)"),
    Carnivore_Pack      UMETA(DisplayName = "Pack Hunters (Velociraptors, etc)"),
    Carnivore_Solo      UMETA(DisplayName = "Solo Predators (T-Rex, etc)"),
    Scavenger           UMETA(DisplayName = "Scavengers (Pteranodons, etc)"),
    Aquatic             UMETA(DisplayName = "Aquatic Species (Plesiosaurs, etc)")
};

USTRUCT(BlueprintType)
struct FDinosaurHerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    EDinosaurHerdType HerdType = EDinosaurHerdType::Herbivore_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    int32 MinHerdSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    int32 MaxHerdSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    bool bMigratory = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float AggressionLevel = 0.2f; // 0.0 = Passive, 1.0 = Highly Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<EDinosaurHerdType> PreyTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<EDinosaurHerdType> PredatorTypes;
};

/**
 * Spawner for dinosaur herds using Mass Entity Framework
 * Handles large-scale simulation of dinosaur populations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurMassSpawner : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurMassSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    TArray<FDinosaurHerdConfig> HerdConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    float SimulationRadius = 10000.0f; // Distance from player where simulation is active

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    float DespawnDistance = 15000.0f; // Distance where entities are despawned

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    class UMassEntityConfigAsset* DinosaurEntityConfig;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 EntitiesPerFrame = 100; // Max entities to process per frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 2000.0f; // Full simulation distance
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 5000.0f; // Reduced simulation distance
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 10000.0f; // Minimal simulation distance

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Spawning")
    void SpawnHerd(const FDinosaurHerdConfig& Config, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Spawning")
    void DespawnEntitiesInRadius(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Spawning")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Spawning")
    void SetSimulationActive(bool bActive);

private:
    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    bool bSimulationActive = true;
    float LastSpawnTime = 0.0f;
    
    void UpdateLODLevels();
    void ProcessEntityCulling();
    FVector GetPlayerLocation() const;
};