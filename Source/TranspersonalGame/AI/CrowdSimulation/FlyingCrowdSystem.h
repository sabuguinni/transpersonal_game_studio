#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "FlyingCrowdSystem.generated.h"

// Fragment para criaturas voadoras (pterossauros, insetos)
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFlyingCreatureFragment : public FMassFragment
{
    GENERATED_BODY()

    // Altitude preferida de voo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredAltitude = 1500.0f;

    // Variação permitida na altitude
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AltitudeVariance = 300.0f;

    // Velocidade de voo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlyingSpeed = 1200.0f;

    // Raio de formação de bando
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockRadius = 800.0f;

    // Força de coesão do bando
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockCohesion = 1.0f;

    // Força de separação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockSeparation = 2.0f;

    // Força de alinhamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockAlignment = 1.5f;

    // ID do bando
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FlockID = -1;

    // Estado atual (0=Soaring, 1=Hunting, 2=Roosting, 3=Migrating)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 FlightState = 0;
};

// Fragment para padrões de migração
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMigrationPatternFragment : public FMassFragment
{
    GENERATED_BODY()

    // Pontos de migração (waypoints)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> MigrationWaypoints;

    // Índice do waypoint atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWaypointIndex = 0;

    // Distância mínima para considerar waypoint atingido
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaypointReachDistance = 500.0f;

    // Velocidade de migração
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSpeed = 800.0f;

    // Tempo de descanso em cada waypoint
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RestDuration = 30.0f;

    // Tempo atual de descanso
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentRestTime = 0.0f;
};

// Processor para comportamento de bando voador
UCLASS()
class TRANSPERSONALGAME_API UFlyingFlockProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UFlyingFlockProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Calcula força de coesão do bando
    FVector CalculateFlockCohesion(const FVector& Position, const FVector& FlockCenter, float FlockRadius);
    
    // Calcula força de separação
    FVector CalculateFlockSeparation(const FVector& Position, const TArray<FVector>& NearbyPositions, float SeparationRadius);
    
    // Calcula força de alinhamento
    FVector CalculateFlockAlignment(const FVector& Velocity, const FVector& AverageFlockVelocity);
    
    // Mantém altitude preferida
    FVector CalculateAltitudeCorrection(const FVector& Position, float PreferredAltitude, float AltitudeVariance);
};

// Processor para padrões de migração
UCLASS()
class TRANSPERSONALGAME_API UFlyingMigrationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UFlyingMigrationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};

// Processor para evitar colisões com terreno e obstáculos
UCLASS()
class TRANSPERSONALGAME_API UFlyingObstacleAvoidanceProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UFlyingObstacleAvoidanceProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Distância de detecção de obstáculos
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float ObstacleDetectionRange = 1000.0f;

    // Força de evitamento
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float AvoidanceForce = 2.0f;

    // Altura mínima do solo
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float MinGroundClearance = 200.0f;
};