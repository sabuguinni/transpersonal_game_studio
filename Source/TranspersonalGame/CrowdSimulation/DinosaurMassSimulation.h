#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "DinosaurMassSimulation.generated.h"

// Forward declarations
class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros pequenos (domesticáveis)
    Compsognathus,
    Hypsilophodon,
    Dryosaurus,
    
    // Herbívoros médios (manadas)
    Parasaurolophus,
    Triceratops,
    Stegosaurus,
    
    // Herbívoros grandes (grupos pequenos)
    Brontosaurus,
    Diplodocus,
    Brachiosaurus,
    
    // Carnívoros pequenos (grupos de caça)
    Velociraptor,
    Deinonychus,
    Utahraptor,
    
    // Carnívoros médios (solitários/pares)
    Allosaurus,
    Carnotaurus,
    Ceratosaurus,
    
    // Carnívoros grandes (solitários)
    TyrannosaurusRex,
    Spinosaurus,
    Giganotosaurus
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Grazing,        // Herbívoros a pastar
    Hunting,        // Carnívoros a caçar
    Fleeing,        // Fugindo de predador
    Socializing,    // Interação social (manada)
    Resting,        // Descanso
    Drinking,       // Beber água
    Nesting,        // Comportamento de ninho
    Territorial,    // Defesa territorial
    Migrating,      // Migração sazonal
    Investigating   // Investigar distúrbio (jogador)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdTendency = 0.8f; // 0 = solitário, 1 = sempre em grupo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinGroupSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGroupSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> PreySpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> PredatorSpecies;
};

// Mass Entity Fragments para dinossauros
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;
    FDinosaurSpeciesData SpeciesData;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    EDinosaurBehaviorState CurrentState = EDinosaurBehaviorState::Grazing;
    EDinosaurBehaviorState PreviousState = EDinosaurBehaviorState::Grazing;
    float StateTimer = 0.0f;
    float StateDuration = 10.0f;
    FVector TargetLocation = FVector::ZeroVector;
    bool bHasTarget = false;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurGroupFragment : public FMassFragment
{
    GENERATED_BODY()

    int32 GroupID = -1;
    FVector GroupCenterLocation = FVector::ZeroVector;
    int32 GroupSize = 1;
    bool bIsGroupLeader = false;
    float GroupCohesionRadius = 500.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurMemoryFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    float TimeSincePlayerSeen = 999.0f;
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex = 0;
    FVector HomeLocation = FVector::ZeroVector;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurPhysicalTraitsFragment : public FMassFragment
{
    GENERATED_BODY()

    // Variações físicas únicas para cada indivíduo
    float SizeVariation = 1.0f; // 0.8 - 1.2
    FLinearColor ColorVariation = FLinearColor::White;
    float HornSizeVariation = 1.0f;
    float TailLengthVariation = 1.0f;
    int32 UniqueID = 0; // Para identificação individual
    FString NickName; // Nome gerado proceduralmente
};

/**
 * Sistema principal de simulação de dinossauros usando Mass Entity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurMassSimulation : public UObject
{
    GENERATED_BODY()

public:
    UDinosaurMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void InitializeSimulation(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnDinosaurHerd(EDinosaurSpecies Species, FVector Location, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnSolitaryDinosaur(EDinosaurSpecies Species, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SetPlayerLocation(FVector PlayerLoc);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    TArray<FVector> GetNearbyDinosaurLocations(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void TriggerMigrationEvent(FVector FromArea, FVector ToArea);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Database")
    TMap<EDinosaurSpecies, FDinosaurSpeciesData> SpeciesDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    int32 MaxSimultaneousDinosaurs = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float SimulationRadius = 10000.0f; // Raio em torno do jogador

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float LODDistance1 = 2000.0f; // Distância para LOD completo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float LODDistance2 = 5000.0f; // Distância para LOD médio

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
    float LODDistance3 = 10000.0f; // Distância para LOD mínimo

private:
    UPROPERTY()
    UWorld* CachedWorld;

    FVector PlayerLocation;
    int32 NextUniqueID;
    int32 NextGroupID;

    void InitializeSpeciesDatabase();
    FDinosaurSpeciesData GetSpeciesData(EDinosaurSpecies Species);
    int32 GenerateUniqueID();
    int32 GenerateGroupID();
    FString GenerateDinosaurNickname(EDinosaurSpecies Species);
};