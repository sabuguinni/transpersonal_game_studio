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
 * Fragment que define o tipo de dinossauro e suas características comportamentais
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurTypeFragment : public FMassFragment
{
    GENERATED_BODY()

    // Tipo de dinossauro (Herbívoro, Carnívoro, Omnívoro)
    UPROPERTY()
    uint8 DinosaurType = 0;
    
    // Tamanho relativo (0=muito pequeno, 255=gigante)
    UPROPERTY()
    uint8 SizeCategory = 128;
    
    // Agressividade (0=pacífico, 255=extremamente agressivo)
    UPROPERTY()
    uint8 AggressionLevel = 64;
    
    // Velocidade base de movimento
    UPROPERTY()
    float BaseSpeed = 300.0f;
    
    // Raio de detecção de outros dinossauros
    UPROPERTY()
    float DetectionRadius = 1000.0f;
    
    // Distância mínima que mantém de predadores
    UPROPERTY()
    float FleeDistance = 1500.0f;
};

/**
 * Fragment que define o estado comportamental atual do dinossauro
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Estado atual (0=Idle, 1=Foraging, 2=Hunting, 3=Fleeing, 4=Drinking, 5=Resting)
    UPROPERTY()
    uint8 CurrentState = 0;
    
    // Tempo no estado atual
    UPROPERTY()
    float StateTime = 0.0f;
    
    // Duração mínima do estado atual
    UPROPERTY()
    float MinStateDuration = 5.0f;
    
    // Energia atual (0-100)
    UPROPERTY()
    float Energy = 100.0f;
    
    // Fome atual (0-100, onde 100 = muito faminto)
    UPROPERTY()
    float Hunger = 0.0f;
    
    // Sede atual (0-100, onde 100 = muito sedento)
    UPROPERTY()
    float Thirst = 0.0f;
    
    // Target entity (para caça ou fuga)
    UPROPERTY()
    FMassEntityHandle TargetEntity;
    
    // Posição de destino atual
    UPROPERTY()
    FVector TargetLocation = FVector::ZeroVector;
};

/**
 * Fragment que define variações físicas únicas de cada dinossauro
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurVariationFragment : public FMassFragment
{
    GENERATED_BODY()

    // Escala geral do modelo (0.8 a 1.2)
    UPROPERTY()
    float OverallScale = 1.0f;
    
    // Variação de cor primária (0-255)
    UPROPERTY()
    uint8 ColorVariation = 128;
    
    // Padrão de manchas/listras (0-15)
    UPROPERTY()
    uint8 PatternType = 0;
    
    // Características físicas únicas (bitmask)
    // Bit 0: Corno maior, Bit 1: Cauda mais longa, Bit 2: Postura diferente, etc.
    UPROPERTY()
    uint16 PhysicalTraits = 0;
    
    // ID único para reconhecimento pelo jogador
    UPROPERTY()
    uint32 UniqueID = 0;
};

/**
 * Fragment para controlo de grupos/manadas
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID do grupo/manada (0 = solitário)
    UPROPERTY()
    uint32 HerdID = 0;
    
    // Posição dentro da hierarquia da manada (0=líder, 255=seguidor)
    UPROPERTY()
    uint8 HerdRank = 128;
    
    // Distância preferida do centro da manada
    UPROPERTY()
    float PreferredHerdDistance = 500.0f;
    
    // Força de coesão com a manada
    UPROPERTY()
    float HerdCohesion = 1.0f;
};

/**
 * Processor principal para simulação de comportamento de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Funções de comportamento
    void ProcessIdleBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior, 
                           const FDinosaurTypeFragment& Type, const FTransformFragment& Transform);
    
    void ProcessForagingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                               const FDinosaurTypeFragment& Type, FMassVelocityFragment& Velocity);
    
    void ProcessHuntingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                              const FDinosaurTypeFragment& Type, FMassVelocityFragment& Velocity);
    
    void ProcessFleeingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                              const FDinosaurTypeFragment& Type, FMassVelocityFragment& Velocity);
    
    void UpdateNeeds(FDinosaurBehaviorFragment& Behavior, float DeltaTime);
    void FindNearbyTargets(FMassExecutionContext& Context, const FTransformFragment& Transform,
                          const FDinosaurTypeFragment& Type, FDinosaurBehaviorFragment& Behavior);
};

/**
 * Processor para movimento e navegação de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurMovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    void ApplyMovement(FMassExecutionContext& Context, FTransformFragment& Transform,
                      FMassVelocityFragment& Velocity, const FDinosaurBehaviorFragment& Behavior,
                      const FDinosaurTypeFragment& Type, float DeltaTime);
    
    void ApplyAvoidance(FMassVelocityFragment& Velocity, const FTransformFragment& Transform,
                       const FDinosaurTypeFragment& Type, FMassExecutionContext& Context);
};

/**
 * Processor para comportamento de manadas
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
    FMassEntityQuery EntityQuery;
    
    void ProcessHerdBehavior(FMassExecutionContext& Context, const FDinosaurHerdFragment& Herd,
                           FMassVelocityFragment& Velocity, const FTransformFragment& Transform);
    
    void CalculateHerdCenter(FMassExecutionContext& Context, uint32 HerdID, FVector& OutCenter, int32& OutCount);
};

/**
 * Subsystem principal para gestão da simulação de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawning de dinossauros
    UFUNCTION(BlueprintCallable)
    void SpawnDinosaurGroup(const FVector& Location, int32 Count, uint8 DinosaurType, 
                           float SpawnRadius = 1000.0f);
    
    UFUNCTION(BlueprintCallable)
    void SpawnSolitaryDinosaur(const FVector& Location, uint8 DinosaurType);
    
    // Gestão de população
    UFUNCTION(BlueprintCallable)
    int32 GetDinosaurCount() const;
    
    UFUNCTION(BlueprintCallable)
    int32 GetDinosaurCountInRadius(const FVector& Location, float Radius) const;
    
    // Configuração de densidade por zona
    UFUNCTION(BlueprintCallable)
    void SetZoneDensity(const FString& ZoneName, float DensityMultiplier);

private:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;
    
    // Configurações de spawn
    TMap<FString, float> ZoneDensityMap;
    uint32 NextUniqueID = 1;
    uint32 NextHerdID = 1;
    
    // Funções auxiliares
    FDinosaurVariationFragment GenerateUniqueVariation();
    void InitializeDinosaurEntity(FMassEntityHandle Entity, uint8 DinosaurType, 
                                const FVector& Location, uint32 HerdID = 0);
};