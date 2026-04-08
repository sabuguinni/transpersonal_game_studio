#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "DinosaurHerdSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Fragment que define as características de uma espécie de dinossauro
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    // Tipo de dinossauro (Herbívoro, Carnívoro, Omnívoro)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 DietType = 0; // 0=Herbívoro, 1=Carnívoro, 2=Omnívoro

    // Tamanho da espécie (0=Pequeno, 1=Médio, 2=Grande, 3=Gigante)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 SizeCategory = 0;

    // Nível de agressividade (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 AggressionLevel = 50;

    // Tendência para formar grupos (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HerdTendency = 128;

    // Velocidade máxima da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 500.0f;

    // Raio de detecção de outros dinossauros
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 1000.0f;
};

/**
 * Fragment que define as características únicas de um dinossauro individual
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurIndividualFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID único do dinossauro para tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 UniqueID = 0;

    // Variações físicas (0-255 para cada característica)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 SizeVariation = 128; // Variação de tamanho

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 ColorVariation = 128; // Variação de cor

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 FeatureVariation = 128; // Variação de características (cornos, cristas, etc.)

    // Estado de saúde (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HealthLevel = 255;

    // Nível de fome (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HungerLevel = 128;

    // Nível de sede (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 ThirstLevel = 128;

    // Nível de cansaço (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 FatigueLevel = 0;

    // Tempo desde a última refeição (em segundos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastMeal = 0.0f;

    // Personalidade individual (afeta comportamentos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 Boldness = 128; // Quão corajoso é

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 Curiosity = 128; // Quão curioso é

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 Sociability = 128; // Quão social é
};

/**
 * Fragment que define o estado comportamental atual do dinossauro
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Estado comportamental atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 CurrentBehavior = 0; // 0=Pastando, 1=Movendo, 2=Descansando, 3=Alerta, 4=Fugindo, 5=Caçando, 6=Bebendo, 7=Socializando

    // Tempo no estado atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInCurrentBehavior = 0.0f;

    // Duração planejada para o estado atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlannedBehaviorDuration = 0.0f;

    // Prioridade do comportamento atual (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 BehaviorPriority = 128;

    // Target para comportamentos direcionais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    // Handle da entidade que está sendo seguida/evitada (se aplicável)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle TargetEntity;
};

/**
 * Fragment que define a afiliação a uma manada
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID da manada (0 = sem manada)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 HerdID = 0;

    // Posição hierárquica na manada (0=Líder, 255=Subordinado)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HerdRank = 128;

    // Distância preferida do centro da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredHerdDistance = 500.0f;

    // Tempo desde a última interação social
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastSocialInteraction = 0.0f;
};

/**
 * Processor principal para simulação de manadas de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para dinossauros em manadas
    FMassEntityQuery HerdDinosaurQuery;
    
    // Query para dinossauros solitários
    FMassEntityQuery SolitaryDinosaurQuery;

    // Funções de processamento
    void ProcessHerdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessSolitaryBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateIndividualNeeds(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void HandleHerdFormation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void HandleThreatResponse(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Parâmetros de simulação
    UPROPERTY(EditAnywhere, Category = "Herd Simulation")
    float HerdFormationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Simulation")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Simulation")
    float MaxHerdSize = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Simulation")
    float HerdCohesionStrength = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Simulation")
    float HerdSeparationStrength = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Simulation")
    float HerdAlignmentStrength = 0.5f;
};

/**
 * Processor para comportamentos de predação
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurPredationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurPredationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PredatorQuery;
    FMassEntityQuery PreyQuery;

    void ProcessHunting(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessFleeingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UPROPERTY(EditAnywhere, Category = "Predation")
    float HuntingRange = 3000.0f;

    UPROPERTY(EditAnywhere, Category = "Predation")
    float FleeDistance = 2000.0f;
};

/**
 * Subsystem para gerenciar a simulação de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawnar uma manada de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnDinosaurHerd(const FVector& Location, int32 HerdSize, uint8 SpeciesType);

    // Spawnar um predador solitário
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnSolitaryPredator(const FVector& Location, uint8 SpeciesType);

    // Obter estatísticas da simulação
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    int32 GetTotalDinosaurCount() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    int32 GetActiveHerdCount() const;

private:
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    // Contador global de IDs únicos
    uint32 NextUniqueID = 1;
    uint32 NextHerdID = 1;

    // Mapa de manadas ativas
    TMap<uint32, TArray<FMassEntityHandle>> ActiveHerds;

    // Geração procedural de características
    void GenerateUniqueCharacteristics(FDinosaurIndividualFragment& Individual);
    uint32 GetNextUniqueID() { return NextUniqueID++; }
    uint32 GetNextHerdID() { return NextHerdID++; }
};