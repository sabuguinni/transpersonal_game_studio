#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "JurassicCrowdSystem.generated.h"

/**
 * Jurassic Crowd System - Simula ecossistemas vivos de dinossauros
 * Baseado em Mass Entity Framework do UE5
 * Suporta até 50.000 agentes simultâneos com comportamento emergente
 */

// ========== FRAGMENTS ==========

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 SpeciesType = 0; // 0=Herbívoro, 1=Carnívoro, 2=Pterossauro, 3=Aquático, 4=Inseto

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 1.0f; // Multiplicador de tamanho (0.1 = inseto, 10.0 = T-Rex)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.0f; // 0.0 = Pacífico, 1.0 = Altamente agressivo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialTendency = 0.5f; // 0.0 = Solitário, 1.0 = Altamente social
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicFlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockLeaderInfluence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFlockLeader = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentPatrolAngle = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicMigrationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTimer = 0.0f; // Controla quando migrar
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicPredatorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.5f; // 0.0 = Saciado, 1.0 = Faminto

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastHuntTime = 0.0f;
};

// ========== PROCESSORS ==========

UCLASS()
class TRANSPERSONALGAME_API UJurassicFlockingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicFlockingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery FlockingQuery;
    
    // Parâmetros de flocking otimizados para dinossauros
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float SeparationWeight = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float AlignmentWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float CohesionWeight = 1.5f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float MaxFlockingForce = 500.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UJurassicMigrationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicMigrationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery MigrationQuery;
    
    UPROPERTY(EditAnywhere, Category = "Migration")
    float WaypointReachDistance = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Migration")
    float SeasonalCycleDuration = 1200.0f; // 20 minutos de jogo = 1 estação
};

UCLASS()
class TRANSPERSONALGAME_API UJurassicPredatorHuntingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicPredatorHuntingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PredatorQuery;
    FMassEntityQuery PreyQuery;
    
    UPROPERTY(EditAnywhere, Category = "Hunting")
    float HuntingCooldown = 30.0f; // Tempo entre caçadas
    
    UPROPERTY(EditAnywhere, Category = "Hunting")
    float PreyDetectionAngle = 120.0f; // Ângulo de visão do predador
};

UCLASS()
class TRANSPERSONALGAME_API UJurassicTerritorialProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicTerritorialProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery TerritorialQuery;
    
    UPROPERTY(EditAnywhere, Category = "Territory")
    float PatrolVariation = 0.3f; // Variação no patrulhamento
};

// ========== SUBSYSTEM PRINCIPAL ==========

UCLASS()
class TRANSPERSONALGAME_API UJurassicCrowdSubsystem : public UMassEntitySubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawning de diferentes tipos de grupos
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnHerbivoreHerd(FVector Location, int32 Count = 20, float SpreadRadius = 1000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnPredatorPack(FVector Location, int32 Count = 5, float TerritoryRadius = 2000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnPteranodonFlock(FVector Location, int32 Count = 15, float FlightHeight = 1000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnInsectSwarm(FVector Location, int32 Count = 100, float SwarmRadius = 200.0f);

    // Controle de densidade por região
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SetRegionDensity(FVector Center, float Radius, float DensityMultiplier);

    // Debug e estatísticas
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    int32 GetActiveEntityCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void EnableDebugVisualization(bool bEnable);

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

private:
    // Templates de configuração para diferentes tipos
    void SetupHerbivoreEntity(FMassEntityHandle Entity, const FVector& Location);
    void SetupPredatorEntity(FMassEntityHandle Entity, const FVector& Location, float TerritoryRadius);
    void SetupFlyingEntity(FMassEntityHandle Entity, const FVector& Location, float FlightHeight);
    void SetupInsectEntity(FMassEntityHandle Entity, const FVector& Location);
    
    // Gestão de performance
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxEntitiesPerFrame = 1000;
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    float LODDistance1 = 2000.0f; // Distância para LOD alto
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    float LODDistance2 = 5000.0f; // Distância para LOD médio
    
    // Configurações globais
    UPROPERTY(EditAnywhere, Category = "Global Settings")
    bool bEnableSeasonalMigration = true;
    
    UPROPERTY(EditAnywhere, Category = "Global Settings")
    float GlobalActivityMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Global Settings")
    float PlayerInfluenceRadius = 1000.0f; // Raio onde o jogador afeta o comportamento
};