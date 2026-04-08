#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "MassDinosaurSubsystem.generated.h"

class UMassEntitySubsystem;

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros de Manada
    Triceratops,
    Parasaurolophus,
    Brachiosaurus,
    Stegosaurus,
    
    // Carnívoros Solitários
    TyrannosaurusRex,
    Allosaurus,
    
    // Carnívoros de Grupo
    Velociraptor,
    Compsognathus,
    
    // Voadores
    Pteranodon,
    
    // Aquáticos
    Mosasaurus,
    
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Grazing,        // Pastando/comendo
    Migrating,      // Migrando em manada
    Hunting,        // Caçando (carnívoros)
    Fleeing,        // Fugindo de predadores
    Resting,        // Descansando
    Drinking,       // Bebendo água
    Socializing,    // Interação social (herbívoros)
    Patrolling,     // Patrulhando território (carnívoros)
    Investigating,  // Investigando distúrbio
    Panicking       // Pânico generalizado
};

USTRUCT(BlueprintType)
struct FDinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreferredPackSize = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PanicSpeed = 600.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.1f; // 0.0 = passivo, 1.0 = extremamente agressivo
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> PreySpecies;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> PredatorSpecies;
};

USTRUCT()
struct FMassDinosaurFragment : public FMassFragment
{
    GENERATED_BODY()

    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    EDinosaurBehaviorState CurrentState = EDinosaurBehaviorState::Grazing;
    EDinosaurBehaviorState PreviousState = EDinosaurBehaviorState::Grazing;
    
    FMassEntityHandle PackLeader;
    TArray<FMassEntityHandle> PackMembers;
    
    FVector HomeTerritory = FVector::ZeroVector;
    FVector CurrentTarget = FVector::ZeroVector;
    FVector LastKnownThreat = FVector::ZeroVector;
    
    float StateTimer = 0.0f;
    float ThreatLevel = 0.0f;
    float HungerLevel = 0.5f;
    float ThirstLevel = 0.5f;
    float EnergyLevel = 1.0f;
    
    bool bHasPackLeader = false;
    bool bIsPackLeader = false;
    bool bCanSeePlayer = false;
    bool bPlayerIsNearby = false;
    
    int32 UniqueID = 0;
    float VisualVariation = 0.0f; // Para variações físicas únicas
};

USTRUCT()
struct FMassEcosystemFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector RegionCenter = FVector::ZeroVector;
    float RegionRadius = 10000.0f;
    
    // Contadores de população por espécie na região
    TMap<EDinosaurSpecies, int32> SpeciesPopulation;
    
    // Recursos da região
    float VegetationDensity = 1.0f;
    float WaterAvailability = 1.0f;
    float PreyAvailability = 1.0f;
    
    // Estado do ecossistema
    float StressLevel = 0.0f; // Aumenta com presença do jogador
    float MigrationPressure = 0.0f;
    
    bool bPlayerInRegion = false;
    FVector PlayerLastKnownPosition = FVector::ZeroVector;
    float TimeSincePlayerSighting = 0.0f;
};

/**
 * Subsistema responsável pela simulação de multidões de dinossauros
 * Gere até 50.000 dinossauros simultâneos usando Mass Entity
 */
UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Configuração de espécies
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void InitializeSpeciesData();
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    FDinosaurSpeciesData GetSpeciesData(EDinosaurSpecies Species) const;
    
    // Spawning de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnHerd(EDinosaurSpecies Species, FVector Location, int32 Count = 10);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnPredatorPack(EDinosaurSpecies Species, FVector Location, int32 Count = 3);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnSolitaryDinosaur(EDinosaurSpecies Species, FVector Location);
    
    // Gestão de ecossistemas
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void CreateEcosystemRegion(FVector Center, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void UpdateEcosystemStress(FVector PlayerPosition);
    
    // Eventos globais
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void TriggerMigration(FVector FromRegion, FVector ToRegion, float Radius = 5000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void TriggerPanic(FVector EpicenterLocation, float Radius = 3000.0f);
    
    // Debug e monitorização
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    int32 GetTotalDinosaurCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    TMap<EDinosaurSpecies, int32> GetSpeciesPopulation() const;
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void EnableDebugVisualization(bool bEnable);

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;
    
    // Dados das espécies
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species Configuration")
    TMap<EDinosaurSpecies, FDinosaurSpeciesData> SpeciesDatabase;
    
    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation Settings")
    int32 MaxDinosaursPerRegion = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation Settings")
    float EcosystemUpdateInterval = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation Settings")
    float PlayerInfluenceRadius = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnableLODSystem = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float HighDetailRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MediumDetailRadius = 5000.0f;

private:
    // Timers internos
    float EcosystemUpdateTimer = 0.0f;
    
    // Debug
    bool bDebugVisualizationEnabled = false;
    
    // Contadores de performance
    int32 CurrentDinosaurCount = 0;
    int32 ActiveEcosystemRegions = 0;
    
    void UpdateSpeciesPopulation();
    void ProcessEcosystemInteractions();
    void HandlePlayerInfluence();
    
    // Helpers para spawning
    FVector FindSuitableSpawnLocation(EDinosaurSpecies Species, FVector PreferredLocation, float SearchRadius = 2000.0f);
    bool IsLocationSuitableForSpecies(EDinosaurSpecies Species, FVector Location);
    
    // Sistema de IDs únicos para cada dinossauro
    int32 NextUniqueID = 1;
    int32 GenerateUniqueID() { return NextUniqueID++; }
};