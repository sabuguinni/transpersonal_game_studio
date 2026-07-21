#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassLODFragments.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveCrowdSystem.generated.h"

// Fragmento para dados de multidão massiva
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveCrowdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID do grupo de multidão
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = 0;

    // Tipo de bioma onde a entidade está localizada
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savana;

    // Estado atual da entidade na multidão
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_EntityState CurrentState = ECrowd_EntityState::Idle;

    // Velocidade de movimento da entidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 300.0f;

    // Raio de detecção para outros membros da multidão
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DetectionRadius = 500.0f;

    // Força de coesão do grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CohesionForce = 1.0f;

    // Força de separação entre entidades
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SeparationForce = 2.0f;

    // Força de alinhamento com o grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlignmentForce = 1.5f;

    // Nível de LOD atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 CurrentLODLevel = 0;

    // Distância do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DistanceToPlayer = 0.0f;

    // Flag para culling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsCulled = false;

    // Tempo desde a última atualização
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float LastUpdateTime = 0.0f;
};

// Fragmento para dados de LOD da multidão
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveLODFragment : public FMassFragment
{
    GENERATED_BODY()

    // Nível de LOD (0 = máxima qualidade, 4 = culled)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 LODLevel = 0;

    // Distâncias para cada nível de LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODDistances = {5000.0f, 15000.0f, 30000.0f, 50000.0f};

    // Escalas para cada nível de LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODScales = {1.0f, 0.8f, 0.6f, 0.4f, 0.1f};

    // Flag de visibilidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bIsVisible = true;

    // Tempo desde a última verificação de LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LastLODCheckTime = 0.0f;

    // Intervalo de verificação de LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODCheckInterval = 0.5f;
};

// Fragmento para movimento de multidão
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveMovementFragment : public FMassFragment
{
    GENERATED_BODY()

    // Velocidade atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector CurrentVelocity = FVector::ZeroVector;

    // Direção desejada
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector DesiredDirection = FVector::ForwardVector;

    // Posição alvo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector TargetLocation = FVector::ZeroVector;

    // Waypoint atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    int32 CurrentWaypointIndex = 0;

    // Lista de waypoints para o grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    TArray<FVector> GroupWaypoints;

    // Velocidade máxima
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed = 500.0f;

    // Aceleração máxima
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxAcceleration = 200.0f;

    // Raio de chegada ao waypoint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WaypointRadius = 100.0f;
};

// Processador para sistema de multidão massiva
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassiveCrowdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassiveCrowdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para entidades de multidão
    FMassEntityQuery CrowdEntityQuery;

    // Configurações de multidão
    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    int32 MaxEntitiesPerBatch = 50;

    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float UpdateFrequency = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float CullingDistance = 75000.0f;

    // Processar comportamento de flocking
    void ProcessFlockingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Processar LOD baseado na distância
    void ProcessLODSystem(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Processar movimento de waypoints
    void ProcessWaypointMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Calcular forças de flocking
    FVector CalculateFlockingForces(const FCrowd_MassiveCrowdFragment& CrowdData, 
                                   const FTransformFragment& Transform,
                                   const TArray<FVector>& NearbyPositions);
};

// Processador de LOD para multidões massivas
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassiveLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassiveLODProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para LOD
    FMassEntityQuery LODQuery;

    // Posição do jogador para cálculo de distância
    FVector PlayerLocation = FVector::ZeroVector;

    // Atualizar posição do jogador
    void UpdatePlayerLocation(const UWorld* World);

    // Calcular nível de LOD baseado na distância
    int32 CalculateLODLevel(float DistanceToPlayer, const TArray<float>& LODDistances);
};

// Spawner para multidões massivas
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ACrowd_MassiveCrowdSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassiveCrowdSpawner();

    // Spawnar multidão em um bioma específico
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdInBiome(ECrowd_BiomeType BiomeType, int32 GroupCount = 3, int32 EntitiesPerGroup = 20);

    // Spawnar multidão em uma posição específica
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdAtLocation(const FVector& Location, int32 EntityCount = 50, float SpreadRadius = 1000.0f);

    // Limpar todas as multidões
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllCrowds();

protected:
    virtual void BeginPlay() override;

    // Configurações de spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 TotalCrowdEntities = 300;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 GroupsPerBiome = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 EntitiesPerGroup = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float GroupSpreadRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float EntitySpreadRadius = 1000.0f;

    // Centros dos biomas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TArray<FVector> BiomeCenters = {
        FVector(-50000, -45000, 100),  // Pantano
        FVector(-45000, 40000, 100),   // Floresta
        FVector(0, 0, 100),            // Savana
        FVector(55000, 0, 100),        // Deserto
        FVector(40000, 50000, 600)     // Montanha
    };

private:
    // Subsistema de entidades Mass
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    // Configuração de spawn
    FMassEntityConfig CrowdEntityConfig;

    // Criar configuração de entidade
    void SetupEntityConfig();

    // Spawnar entidades em uma posição
    void SpawnEntitiesAtLocation(const FVector& Center, int32 Count, int32 GroupID, ECrowd_BiomeType BiomeType);

    // Gerar waypoints para um grupo
    TArray<FVector> GenerateWaypointsForGroup(const FVector& GroupCenter, float Radius = 5000.0f);
};

// Manager para sistema de multidão massiva
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassiveCrowdManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassiveCrowdManager();

    // Inicializar sistema de multidão
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    // Atualizar sistema de multidão
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdSystem(float DeltaTime);

    // Obter estatísticas da multidão
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    FCrowd_CrowdStats GetCrowdStatistics() const;

    // Definir densidade da multidão
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDensity(float NewDensity);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurações do manager
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CrowdDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableBatching = true;

private:
    // Spawner de multidão
    UPROPERTY()
    ACrowd_MassiveCrowdSpawner* CrowdSpawner;

    // Estatísticas atuais
    FCrowd_CrowdStats CurrentStats;

    // Tempo da última atualização
    float LastUpdateTime = 0.0f;

    // Atualizar estatísticas
    void UpdateStatistics();

    // Otimizar performance
    void OptimizePerformance();
};