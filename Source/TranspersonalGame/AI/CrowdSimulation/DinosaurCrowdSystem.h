#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurCrowdSystem.generated.h"

// Fragment que define o comportamento de manada
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID da manada a que pertence
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1;

    // Posição do líder da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdLeaderPosition = FVector::ZeroVector;

    // Raio de coesão da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 2000.0f;

    // Força de separação entre indivíduos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationForce = 1.5f;

    // Força de alinhamento com a manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentForce = 1.0f;

    // Estado atual da manada (pastando, migrando, fugindo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HerdState = 0; // 0=Grazing, 1=Migrating, 2=Fleeing, 3=Drinking
};

// Fragment para comportamento de espécie
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    // Tipo de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 SpeciesType = 0; // 0=Triceratops, 1=Parasaurolophus, 2=Brachiosaurus, etc.

    // Tamanho do dinossauro (afeta comportamento)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BodySize = 1.0f;

    // Velocidade máxima da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 800.0f;

    // Nível de agressividade (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.1f;

    // Distância de detecção de predadores
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredatorDetectionRange = 3000.0f;
};

// Fragment para necessidades biológicas
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    // Nível de fome (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.3f;

    // Nível de sede (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThirstLevel = 0.2f;

    // Nível de cansaço (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FatigueLevel = 0.1f;

    // Tempo desde a última refeição
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastMeal = 0.0f;

    // Tempo desde a última bebida
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastDrink = 0.0f;
};

// Processor principal para comportamento de manada
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
    FMassEntityQuery EntityQuery;

    // Calcula força de coesão da manada
    FVector CalculateCohesionForce(const FVector& Position, const FVector& HerdCenter, float CohesionRadius);
    
    // Calcula força de separação entre indivíduos
    FVector CalculateSeparationForce(const FVector& Position, const TArray<FVector>& NearbyPositions, float SeparationRadius);
    
    // Calcula força de alinhamento com a manada
    FVector CalculateAlignmentForce(const FVector& Velocity, const FVector& AverageHerdVelocity);
};

// Processor para necessidades biológicas
UCLASS()
class TRANSPERSONALGAME_API UDinosaurNeedsProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurNeedsProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Taxa de aumento da fome por segundo
    UPROPERTY(EditAnywhere, Category = "Needs")
    float HungerIncreaseRate = 0.01f;

    // Taxa de aumento da sede por segundo
    UPROPERTY(EditAnywhere, Category = "Needs")
    float ThirstIncreaseRate = 0.015f;

    // Taxa de aumento do cansaço por segundo
    UPROPERTY(EditAnywhere, Category = "Needs")
    float FatigueIncreaseRate = 0.005f;
};

// Processor para detecção de predadores e resposta de fuga
UCLASS()
class TRANSPERSONALGAME_API UDinosaurPredatorAvoidanceProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurPredatorAvoidanceProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PreyQuery;
    FMassEntityQuery PredatorQuery;

    // Distância máxima para detecção de predadores
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float MaxPredatorDetectionRange = 5000.0f;

    // Multiplicador de velocidade durante fuga
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float FleeSpeedMultiplier = 2.0f;
};