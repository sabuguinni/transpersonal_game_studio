#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "PredatorPackSystem.generated.h"

// Fragment para comportamento de matilha de predadores
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPredatorPackFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID da matilha
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PackID = -1;

    // Papel na matilha
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPackRole Role = EPackRole::Hunter;

    // Líder da matilha (se for seguidor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle LeaderEntity;

    // Alvo atual da caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetPosition = FVector::ZeroVector;

    // ID da entidade alvo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle TargetEntity;

    // Estado atual da matilha
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPackState CurrentState = EPackState::Patrolling;

    // Posição na formação de caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FormationOffset = FVector::ZeroVector;

    // Raio de detecção de presas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 1500.0f;

    // Velocidade de caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingSpeed = 800.0f;

    // Velocidade de patrulha
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolSpeed = 300.0f;

    // Coordenação com outros membros da matilha (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinationLevel = 0.8f;

    // Timer para mudança de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorTimer = 0.0f;
};

// Enums para papéis e estados da matilha
UENUM(BlueprintType)
enum class EPackRole : uint8
{
    Alpha,      // Líder da matilha
    Hunter,     // Caçador principal
    Flanker,    // Flanqueador
    Chaser,     // Perseguidor
    Ambusher    // Emboscador
};

UENUM(BlueprintType)
enum class EPackState : uint8
{
    Patrolling,     // Patrulhando território
    Hunting,        // Caçando presa específica
    Stalking,       // Espreitando presa
    Attacking,      // Atacando
    Feeding,        // Alimentando-se
    Resting,        // Descansando
    Territorial     // Defendendo território
};

// Fragment para estratégia de caça
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHuntingStrategyFragment : public FMassFragment
{
    GENERATED_BODY()

    // Tipo de estratégia de caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHuntingStrategy Strategy = EHuntingStrategy::Encirclement;

    // Posição alvo para a estratégia
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StrategyPosition = FVector::ZeroVector;

    // Progresso da estratégia (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StrategyProgress = 0.0f;

    // Tempo desde início da estratégia
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StrategyDuration = 0.0f;

    // Sucesso da estratégia
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStrategySuccessful = false;
};

UENUM(BlueprintType)
enum class EHuntingStrategy : uint8
{
    Encirclement,   // Cercar a presa
    Ambush,         // Emboscada
    Chase,          // Perseguição direta
    Relay,          // Caça em revezamento
    Pincer          // Movimento de pinça
};

// Processor principal para matilhas de predadores
UCLASS()
class TRANSPERSONALGAME_API UPredatorPackProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UPredatorPackProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PackEntityQuery;
    FMassEntityQuery PreyEntityQuery;

    // Comportamentos da matilha
    void ProcessPatrollingBehavior(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FTransformFragment& Transform);
    void ProcessHuntingBehavior(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FTransformFragment& Transform);
    void ProcessStalkingBehavior(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FTransformFragment& Transform);
    void ProcessAttackingBehavior(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FTransformFragment& Transform);

    // Estratégias de caça
    void ExecuteEncirclementStrategy(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FHuntingStrategyFragment& Strategy, FTransformFragment& Transform);
    void ExecuteAmbushStrategy(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FHuntingStrategyFragment& Strategy, FTransformFragment& Transform);
    void ExecuteChaseStrategy(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FHuntingStrategyFragment& Strategy, FTransformFragment& Transform);

    // Detecção e seleção de presas
    bool DetectPrey(const FVector& Position, float DetectionRadius, FMassEntityHandle& PreyEntity, FVector& PreyPosition);
    bool IsValidPrey(const FMassEntityHandle& Entity);
    float CalculatePreyPriority(const FVector& PredatorPosition, const FVector& PreyPosition, float PreySize);

    // Coordenação da matilha
    void CoordinatePackMovement(FMassExecutionContext& Context, const TArray<FPredatorPackFragment>& PackMembers);
    FVector CalculateFormationPosition(const FVector& LeaderPosition, EPackRole Role, int32 PackSize);

    // Comunicação entre membros da matilha
    void BroadcastPreyDetection(int32 PackID, const FMassEntityHandle& PreyEntity, const FVector& PreyPosition);
    void UpdatePackCoordination(int32 PackID, EPackState NewState);
};

// Processor para territorialidade
UCLASS()
class TRANSPERSONALGAME_API UTerritorialBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UTerritorialBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery TerritorialQuery;

    // Dados de território por matilha
    UPROPERTY()
    TMap<int32, FTerritoryData> PackTerritories;

    void DefendTerritory(FMassExecutionContext& Context, FPredatorPackFragment& PackData, FTransformFragment& Transform);
    bool IsInTerritory(const FVector& Position, int32 PackID);
    void MarkTerritory(int32 PackID, const FVector& Center, float Radius);
};

// Estrutura para dados de território
USTRUCT()
struct FTerritoryData
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Center = FVector::ZeroVector;

    UPROPERTY()
    float Radius = 2000.0f;

    UPROPERTY()
    float LastMarkedTime = 0.0f;

    UPROPERTY()
    TArray<FVector> PatrolPoints;
};