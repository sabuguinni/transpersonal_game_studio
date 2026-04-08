#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "JurassicCrowdSystem.generated.h"

/**
 * Sistema de simulação de multidões para o mundo Jurássico
 * Simula comportamentos emergentes de manadas, bandos e grupos de dinossauros
 * Baseado no Mass Entity Framework do UE5
 */

// Fragmentos específicos para dinossauros
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 1.0f; // Multiplicador de tamanho (0.5 = pequeno, 2.0 = grande)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal = false; // Vive em grupo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 1000.0f; // Raio do território em cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ComfortDistance = 200.0f; // Distância confortável entre indivíduos da mesma espécie
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHerdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1; // ID da manada (-1 = solitário)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionWeight = 1.0f; // Força de coesão com a manada

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationWeight = 2.0f; // Força de separação (evitar colisões)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentWeight = 0.5f; // Força de alinhamento com direção da manada

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdCenter = FVector::ZeroVector; // Centro atual da manada

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdDirection = FVector::ForwardVector; // Direção de movimento da manada
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNaturalRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay = 0.0f; // 0.0 = meia-noite, 0.5 = meio-dia

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WaterSourceLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FeedingGroundLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RestingAreaLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastWaterTime = -1.0f; // Última vez que foi à água

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastFeedTime = -1.0f; // Última vez que se alimentou

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterNeed = 0.0f; // 0.0 = sem sede, 1.0 = sede extrema

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.0f; // 0.0 = saciado, 1.0 = fome extrema
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FThreatAwarenessFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertLevel = 0.0f; // 0.0 = calmo, 1.0 = pânico total

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatDetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastThreatTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFleeingFromThreat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FleeDirection = FVector::ZeroVector;
};

/**
 * Processador principal do sistema de crowd para dinossauros
 * Executa os comportamentos de manada usando Mass Entity
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery HerdQuery;
    
    // Calcula forças de boids (cohesion, separation, alignment)
    FVector CalculateCohesionForce(const FVector& Position, const TArray<FVector>& NearbyPositions, float Weight);
    FVector CalculateSeparationForce(const FVector& Position, const TArray<FVector>& NearbyPositions, float Weight, float MinDistance);
    FVector CalculateAlignmentForce(const FVector& CurrentVelocity, const TArray<FVector>& NearbyVelocities, float Weight);
    
    // Detecta ameaças próximas
    bool DetectThreats(const FVector& Position, float DetectionRadius, FVector& ThreatLocation);
};

/**
 * Processador de rotinas naturais (água, comida, descanso)
 */
UCLASS()
class TRANSPERSONALGAME_API UNaturalRoutineProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UNaturalRoutineProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery RoutineQuery;
    
    // Determina a ação prioritária baseada nas necessidades
    FVector GetPriorityDestination(const FNaturalRoutineFragment& Routine, const FVector& CurrentPosition);
    
    // Atualiza necessidades baseadas no tempo
    void UpdateNeeds(FNaturalRoutineFragment& Routine, float DeltaTime);
};

/**
 * Processador de resposta a ameaças
 */
UCLASS()
class TRANSPERSONALGAME_API UThreatResponseProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UThreatResponseProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery ThreatQuery;
    
    // Calcula direção de fuga otimizada
    FVector CalculateFleeDirection(const FVector& Position, const FVector& ThreatLocation, const FVector& TerrainNormal);
    
    // Propaga alerta para dinossauros próximos
    void PropagateAlert(const FVector& AlertOrigin, float AlertRadius, FMassEntityManager& EntityManager);
};

/**
 * Subsistema principal de crowd simulation para dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawna uma manada de dinossauros numa localização
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnHerd(const FString& SpeciesName, int32 HerdSize, const FVector& SpawnLocation, float SpawnRadius = 500.0f);

    // Remove todos os dinossauros de uma área
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void ClearArea(const FVector& CenterLocation, float Radius);

    // Obtém estatísticas da simulação
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void GetSimulationStats(int32& TotalDinosaurs, int32& ActiveHerds, float& AverageHerdSize);

    // Define pontos de interesse no mundo (água, comida, abrigo)
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void RegisterWaterSource(const FVector& Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void RegisterFeedingGround(const FVector& Location, float Radius = 1500.0f);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void RegisterRestingArea(const FVector& Location, float Radius = 800.0f);

private:
    UPROPERTY()
    TArray<FVector> WaterSources;

    UPROPERTY()
    TArray<FVector> FeedingGrounds;

    UPROPERTY()
    TArray<FVector> RestingAreas;

    int32 NextHerdID = 0;
    
    // Encontra o ponto de interesse mais próximo
    FVector FindNearestPointOfInterest(const FVector& Position, const TArray<FVector>& Points);
};