#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurCrowdSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;

/**
 * Fragment que define o comportamento de grupo de dinossauros
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID do grupo/manada
    UPROPERTY()
    int32 HerdID = INDEX_NONE;
    
    // Tipo de comportamento do grupo
    UPROPERTY()
    EDinosaurHerdBehavior HerdBehavior = EDinosaurHerdBehavior::Grazing;
    
    // Posição central do grupo
    UPROPERTY()
    FVector HerdCenter = FVector::ZeroVector;
    
    // Raio de coesão do grupo
    UPROPERTY()
    float CohesionRadius = 1000.0f;
    
    // Força de separação entre indivíduos
    UPROPERTY()
    float SeparationForce = 50.0f;
    
    // Força de alinhamento com o grupo
    UPROPERTY()
    float AlignmentForce = 30.0f;
    
    // Tempo restante no comportamento atual
    UPROPERTY()
    float BehaviorTimeRemaining = 0.0f;
};

/**
 * Fragment que define características únicas de cada dinossauro
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurIdentityFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID único do dinossauro
    UPROPERTY()
    FGuid DinosaurID;
    
    // Espécie do dinossauro
    UPROPERTY()
    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    
    // Variações físicas únicas
    UPROPERTY()
    FDinosaurPhysicalVariations PhysicalVariations;
    
    // Personalidade individual
    UPROPERTY()
    FDinosaurPersonality Personality;
    
    // Nível de energia atual (0-100)
    UPROPERTY()
    float EnergyLevel = 100.0f;
    
    // Nível de fome (0-100)
    UPROPERTY()
    float HungerLevel = 0.0f;
    
    // Nível de sede (0-100)
    UPROPERTY()
    float ThirstLevel = 0.0f;
    
    // Tempo desde a última refeição
    UPROPERTY()
    float TimeSinceLastMeal = 0.0f;
};

/**
 * Fragment que controla o comportamento diário dos dinossauros
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurDailyRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    // Comportamento atual
    UPROPERTY()
    EDinosaurBehavior CurrentBehavior = EDinosaurBehavior::Idle;
    
    // Localização preferida para descanso
    UPROPERTY()
    FVector RestingLocation = FVector::ZeroVector;
    
    // Localização preferida para alimentação
    UPROPERTY()
    FVector FeedingLocation = FVector::ZeroVector;
    
    // Localização preferida para beber água
    UPROPERTY()
    FVector DrinkingLocation = FVector::ZeroVector;
    
    // Horário preferido para cada atividade
    UPROPERTY()
    TMap<EDinosaurBehavior, float> PreferredActivityTimes;
    
    // Duração típica de cada comportamento
    UPROPERTY()
    TMap<EDinosaurBehavior, float> BehaviorDurations;
};

/**
 * Enumerações para tipos de comportamento
 */
UENUM(BlueprintType)
enum class EDinosaurHerdBehavior : uint8
{
    Grazing,        // Pastando em grupo
    Moving,         // Movendo-se como manada
    Resting,        // Descansando
    Drinking,       // Bebendo água
    Fleeing,        // Fugindo de predador
    Territorial     // Comportamento territorial
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle,
    Grazing,
    Walking,
    Running,
    Drinking,
    Sleeping,
    Socializing,
    Alerting,
    Fleeing,
    Hunting,        // Para carnívoros
    Nesting         // Comportamento reprodutivo
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Triceratops,
    Brachiosaurus,
    Stegosaurus,
    Parasaurolophus,
    Ankylosaurus,
    Velociraptor,
    TyrannosaurusRex,
    Allosaurus,
    Compsognathus,
    Gallimimus
};

/**
 * Estruturas para variações físicas e personalidade
 */
USTRUCT(BlueprintType)
struct FDinosaurPhysicalVariations
{
    GENERATED_BODY()

    // Escala geral (0.8 - 1.2)
    UPROPERTY()
    float OverallScale = 1.0f;
    
    // Variação de cor principal (HSV)
    UPROPERTY()
    FLinearColor PrimaryColor = FLinearColor::White;
    
    // Variação de cor secundária
    UPROPERTY()
    FLinearColor SecondaryColor = FLinearColor::White;
    
    // Padrões únicos na pele
    UPROPERTY()
    int32 SkinPatternVariation = 0;
    
    // Tamanho relativo de características específicas
    UPROPERTY()
    float HornSize = 1.0f;          // Para espécies com chifres
    
    UPROPERTY()
    float TailLength = 1.0f;
    
    UPROPERTY()
    float NeckLength = 1.0f;
    
    // Cicatrizes ou marcas distintivas
    UPROPERTY()
    TArray<FVector2D> DistinctiveMarks;
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    // Agressividade (0-100)
    UPROPERTY()
    float Aggressiveness = 50.0f;
    
    // Curiosidade (0-100)
    UPROPERTY()
    float Curiosity = 50.0f;
    
    // Sociabilidade (0-100)
    UPROPERTY()
    float Sociability = 50.0f;
    
    // Cautela (0-100)
    UPROPERTY()
    float Caution = 50.0f;
    
    // Tendência a liderar o grupo
    UPROPERTY()
    float Leadership = 50.0f;
    
    // Velocidade de reação a ameaças
    UPROPERTY()
    float ReactionSpeed = 50.0f;
};

/**
 * Processor principal para simulação de multidões de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurCrowdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurCrowdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para dinossauros em grupos
    FMassEntityQuery HerdQuery;
    
    // Query para dinossauros individuais
    FMassEntityQuery IndividualQuery;
    
    // Funções de comportamento
    void ProcessHerdBehavior(FMassExecutionContext& Context);
    void ProcessIndividualBehavior(FMassExecutionContext& Context);
    void UpdateDailyRoutines(FMassExecutionContext& Context);
    
    // Funções de movimento
    FVector CalculateHerdCohesion(const FDinosaurHerdFragment& HerdData, const FTransformFragment& Transform, const TArray<FVector>& NearbyPositions);
    FVector CalculateSeparation(const FTransformFragment& Transform, const TArray<FVector>& NearbyPositions, float SeparationRadius);
    FVector CalculateAlignment(const FMassVelocityFragment& Velocity, const TArray<FVector>& NearbyVelocities);
    
    // Sistema de necessidades
    void UpdateNeeds(FDinosaurIdentityFragment& Identity, float DeltaTime);
    EDinosaurBehavior DetermineBehaviorFromNeeds(const FDinosaurIdentityFragment& Identity, const FDinosaurPersonality& Personality);
};

/**
 * Processor para spawning procedural de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurSpawningProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurSpawningProcessor();
    
    // Spawna uma manada de dinossauros em uma localização
    UFUNCTION(BlueprintCallable)
    void SpawnHerd(EDinosaurSpecies Species, const FVector& Location, int32 HerdSize, float SpreadRadius);
    
    // Spawna dinossauros solitários
    UFUNCTION(BlueprintCallable)
    void SpawnSolitaryDinosaur(EDinosaurSpecies Species, const FVector& Location);

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Gera variações físicas únicas
    FDinosaurPhysicalVariations GeneratePhysicalVariations(EDinosaurSpecies Species);
    
    // Gera personalidade única
    FDinosaurPersonality GeneratePersonality(EDinosaurSpecies Species);
    
    // Configurações de spawn por espécie
    UPROPERTY()
    TMap<EDinosaurSpecies, FDinosaurSpeciesConfig> SpeciesConfigs;
};

/**
 * Configuração por espécie
 */
USTRUCT(BlueprintType)
struct FDinosaurSpeciesConfig
{
    GENERATED_BODY()

    // Tamanho típico de manada
    UPROPERTY()
    FIntPoint HerdSizeRange = FIntPoint(3, 8);
    
    // Comportamento social
    UPROPERTY()
    bool IsSocialSpecies = true;
    
    // Comportamentos permitidos
    UPROPERTY()
    TArray<EDinosaurBehavior> AllowedBehaviors;
    
    // Preferências de habitat
    UPROPERTY()
    TArray<EBiomeType> PreferredBiomes;
    
    // Dieta
    UPROPERTY()
    EDinosaurDiet Diet = EDinosaurDiet::Herbivore;
    
    // Nível de agressividade da espécie
    UPROPERTY()
    float SpeciesAggressiveness = 50.0f;
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Herbivore,
    Carnivore,
    Omnivore
};

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest,
    Plains,
    Swamp,
    Mountains,
    Desert,
    Coastal
};