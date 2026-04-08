#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "Engine/World.h"
#include "JurassicCrowdManager.generated.h"

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    SmallHerbivore,     // Compsognathus, Gallimimus - grupos de 20-50
    LargeHerbivore,     // Triceratops, Brachiosaurus - grupos de 5-15
    PackHunter,         // Velociraptors, Deinonychus - grupos de 3-8
    SolitaryPredator,   // T-Rex, Allosaurus - individuais ou pares
    MixedHerd          // Herbívoros de diferentes espécies juntos
};

USTRUCT(BlueprintType)
struct FHerdConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TurnSpeed = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanMigrateSeasonally = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PreferredTerritories;
};

USTRUCT(BlueprintType)
struct FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTriggerTime = 300.0f; // segundos de jogo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType ApplicableHerdType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActiveRoute = false;
};

/**
 * Gestor principal da simulação de multidões de dinossauros
 * Baseado no Mass AI do UE5 mas adaptado para comportamentos pré-históricos
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    AJurassicCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CONFIGURAÇÃO DE MANADAS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<FHerdConfiguration> HerdConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FMigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    int32 MaxSimultaneousEntities = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float SimulationRadius = 50000.0f; // Raio em torno do jogador

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 5000.0f;  // Distância para LOD completo
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 15000.0f; // Distância para LOD médio
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 30000.0f; // Distância para LOD mínimo

    // === SISTEMA MASS AI ===
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    TSubclassOf<class UMassEntityConfigAsset> HerbivoreEntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    TSubclassOf<class UMassEntityConfigAsset> CarnivoreEntityConfig;

    // === ESTADO INTERNO ===
    UPROPERTY()
    TArray<FMassEntityHandle> ActiveHerds;

    UPROPERTY()
    float CurrentGameTime = 0.0f;

    UPROPERTY()
    bool bMigrationInProgress = false;

public:
    // === INTERFACE PÚBLICA ===
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(EDinosaurHerdType HerdType, FVector Location, int32 HerdSize = -1);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMigration(EDinosaurHerdType HerdType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void CauseStampede(FVector EpicenterLocation, float Radius, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetNearbyHerdLocations(FVector Location, float Radius) const;

    // === EVENTOS PARA OUTROS SISTEMAS ===
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHerdSpawned, EDinosaurHerdType, HerdType, FVector, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMigrationStarted, EDinosaurHerdType, HerdType, FVector, Destination);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStampedeTriggered, FVector, Location, float, Radius, int32, AffectedEntities);

    UPROPERTY(BlueprintAssignable)
    FOnHerdSpawned OnHerdSpawned;

    UPROPERTY(BlueprintAssignable)
    FOnMigrationStarted OnMigrationStarted;

    UPROPERTY(BlueprintAssignable)
    FOnStampedeTriggered OnStampedeTriggered;

private:
    // === MÉTODOS INTERNOS ===
    void UpdateSimulation(float DeltaTime);
    void CheckMigrationTriggers();
    void UpdateLODSystem();
    void CleanupDistantEntities();
    
    FHerdConfiguration* GetHerdConfiguration(EDinosaurHerdType HerdType);
    FVector GetOptimalSpawnLocation(EDinosaurHerdType HerdType, FVector PreferredLocation);
    
    // === INTEGRAÇÃO COM OUTROS SISTEMAS ===
    void NotifyNPCBehaviorSystem();
    void NotifyCombatAISystem();
};