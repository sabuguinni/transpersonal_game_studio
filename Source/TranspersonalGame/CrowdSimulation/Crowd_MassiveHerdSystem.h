#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveHerdSystem.generated.h"

// Fragment para dados de rebanho massivo
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID do rebanho (múltiplas entidades podem pertencer ao mesmo rebanho)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 HerdID = 0;

    // Posição do líder do rebanho
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector LeaderPosition = FVector::ZeroVector;

    // Distância máxima do líder
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float MaxDistanceFromLeader = 2000.0f;

    // Velocidade de movimento do rebanho
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdSpeed = 300.0f;

    // Estado atual do rebanho
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    ECrowd_HerdBehaviorState BehaviorState = ECrowd_HerdBehaviorState::Grazing;

    // Tempo desde a última mudança de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float TimeSinceLastBehaviorChange = 0.0f;

    // Nível de alerta do rebanho (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float AlertLevel = 0.0f;
};

// Fragment para LOD de multidões massivas
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveLODFragment : public FMassFragment
{
    GENERATED_BODY()

    // Nível de LOD atual (0=máximo detalhe, 3=mínimo detalhe)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 CurrentLODLevel = 0;

    // Distância do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DistanceToPlayer = 0.0f;

    // Se deve ser renderizado neste frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bShouldRender = true;

    // Se deve processar IA neste frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bShouldProcessAI = true;

    // Frequência de atualização (frames entre atualizações)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 UpdateFrequency = 1;
};

// Processor para sistema de rebanhos massivos
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassiveHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassiveHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para entidades de rebanho
    FMassEntityQuery HerdEntityQuery;

    // Configurações do sistema
    UPROPERTY(EditAnywhere, Category = "Herd Settings")
    float MaxHerdSize = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Settings")
    float HerdCohesionStrength = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Settings")
    float HerdSeparationStrength = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Settings")
    float HerdAlignmentStrength = 0.5f;

    // Métodos auxiliares
    void ProcessHerdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateHerdLeaderPosition(int32 HerdID, const FVector& NewPosition);
    void HandlePredatorThreat(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Cache de posições dos líderes dos rebanhos
    TMap<int32, FVector> HerdLeaderPositions;
    TMap<int32, float> HerdAlertLevels;
};

// Processor para LOD de multidões massivas
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
    // Query para entidades com LOD
    FMassEntityQuery LODEntityQuery;

    // Configurações de LOD
    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD0Distance = 1000.0f;  // Máximo detalhe

    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD1Distance = 3000.0f;  // Alto detalhe

    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD2Distance = 8000.0f;  // Médio detalhe

    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD3Distance = 15000.0f; // Baixo detalhe

    // Métodos auxiliares
    void UpdateLODLevels(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    int32 CalculateLODLevel(float Distance) const;
    bool ShouldProcessEntity(int32 LODLevel, int32 FrameNumber) const;
    
    // Cache da posição do jogador
    FVector CachedPlayerPosition = FVector::ZeroVector;
    float PlayerPositionCacheTime = 0.0f;
};

// Sistema principal de multidões massivas
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassiveHerdSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassiveHerdSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Herd Settings")
    int32 MaxEntitiesPerHerd = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Herd Settings")
    int32 MaxTotalHerds = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Herd Settings")
    float HerdSpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Herd Settings")
    TArray<ECrowd_DinosaurSpecies> AllowedSpecies;

    // Componentes do sistema
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Massive Herd System")
    void SpawnMassiveHerd(ECrowd_DinosaurSpecies Species, const FVector& Location, int32 HerdSize = 30);

    UFUNCTION(BlueprintCallable, Category = "Massive Herd System")
    void DespawnAllHerds();

    UFUNCTION(BlueprintCallable, Category = "Massive Herd System")
    int32 GetTotalActiveEntities() const;

    UFUNCTION(BlueprintCallable, Category = "Massive Herd System")
    void SetLODDistances(float LOD0, float LOD1, float LOD2, float LOD3);

private:
    // Referência ao subsistema Mass Entity
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    // Processadores registrados
    UPROPERTY()
    UCrowd_MassiveHerdProcessor* HerdProcessor;

    UPROPERTY()
    UCrowd_MassiveLODProcessor* LODProcessor;

    // Estado interno
    TArray<FMassArchetypeHandle> ActiveArchetypes;
    TMap<int32, TArray<FMassEntityHandle>> HerdEntities;
    int32 NextHerdID = 1;

    // Métodos privados
    void InitializeMassSystem();
    void CreateHerdArchetype();
    void RegisterProcessors();
    void UpdateHerdStatistics();
    
    // Estatísticas de performance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics", meta = (AllowPrivateAccess = "true"))
    int32 TotalActiveEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics", meta = (AllowPrivateAccess = "true"))
    int32 TotalActiveHerds = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics", meta = (AllowPrivateAccess = "true"))
    float AverageFrameTime = 0.0f;
};