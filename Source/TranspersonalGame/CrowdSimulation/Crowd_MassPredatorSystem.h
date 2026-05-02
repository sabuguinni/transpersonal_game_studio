#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassPredatorSystem.generated.h"

class UMassEntitySubsystem;
struct FMassExecutionContext;

/**
 * Sistema Mass Entity para comportamento de predadores
 * Gere caça, perseguição e comportamento de pack para carnívoros
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassPredatorSystem : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassPredatorSystem();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    // Query para entidades predadoras
    FMassEntityQuery PredatorEntityQuery;

    /**
     * Processa comportamento individual de predador
     */
    void ProcessPredatorBehavior(FCrowd_PredatorFragment& PredatorFragment,
                               FMassVelocityFragment& VelocityFragment,
                               FTransformFragment& TransformFragment,
                               const FCrowd_DinosaurSpeciesFragment& SpeciesFragment,
                               const FCrowd_PredatorBehaviorParameters& PredatorParams,
                               float DeltaTime, float CurrentTime, int32 EntityIndex);

    /**
     * Actualiza estado de caça do predador
     */
    void UpdateHuntingState(FCrowd_PredatorFragment& PredatorFragment,
                          const FCrowd_DinosaurSpeciesFragment& SpeciesFragment,
                          const FVector& CurrentLocation, float CurrentTime);

    /**
     * Determina estado de caça baseado em condições
     */
    ECrowd_HuntingState DetermineHuntingState(const FCrowd_PredatorFragment& PredatorFragment,
                                            const FCrowd_DinosaurSpeciesFragment& SpeciesFragment,
                                            const FVector& CurrentLocation);

    /**
     * Calcula força de caça (perseguir presa)
     */
    FVector CalculateHuntingForce(const FVector& CurrentLocation,
                                const FCrowd_PredatorFragment& PredatorFragment,
                                const FCrowd_PredatorBehaviorParameters& PredatorParams);

    /**
     * Calcula força de patrulha (explorar território)
     */
    FVector CalculatePatrolForce(const FVector& CurrentLocation,
                               const FCrowd_PredatorFragment& PredatorFragment,
                               const FCrowd_PredatorBehaviorParameters& PredatorParams);

    /**
     * Calcula força de pack (coordenar com outros predadores)
     */
    FVector CalculatePackForce(const FVector& CurrentLocation,
                             const FCrowd_PredatorFragment& PredatorFragment,
                             const FCrowd_PredatorBehaviorParameters& PredatorParams);

    /**
     * Calcula força de emboscada (posicionar para ataque)
     */
    FVector CalculateAmbushForce(const FVector& CurrentLocation,
                               const FCrowd_PredatorFragment& PredatorFragment,
                               const FCrowd_PredatorBehaviorParameters& PredatorParams);

    /**
     * Calcula força territorial (defender território)
     */
    FVector CalculateTerritorialForce(const FVector& CurrentLocation,
                                    const FCrowd_PredatorFragment& PredatorFragment,
                                    const FCrowd_PredatorBehaviorParameters& PredatorParams);

    /**
     * Detecta presas potenciais na área
     */
    bool DetectPrey(const FVector& PredatorLocation,
                   const FCrowd_PredatorFragment& PredatorFragment,
                   const FCrowd_PredatorBehaviorParameters& PredatorParams,
                   FVector& OutPreyLocation);

    /**
     * Verifica se pode atacar a presa
     */
    bool CanAttackPrey(const FVector& PredatorLocation,
                      const FVector& PreyLocation,
                      const FCrowd_PredatorFragment& PredatorFragment,
                      const FCrowd_PredatorBehaviorParameters& PredatorParams);

    /**
     * Obtém multiplicador de velocidade baseado na espécie
     */
    float GetSpeciesHuntingMultiplier(ECrowd_DinosaurSpecies Species);

    /**
     * Obtém raio de detecção baseado na espécie
     */
    float GetSpeciesDetectionRadius(ECrowd_DinosaurSpecies Species);

    /**
     * Obtém força de ataque baseada na espécie
     */
    float GetSpeciesAttackStrength(ECrowd_DinosaurSpecies Species);
};

/**
 * Parâmetros de comportamento para predadores
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PredatorBehaviorParameters
{
    GENERATED_BODY()

    FCrowd_PredatorBehaviorParameters()
        : HuntingWeight(1.0f)
        , PatrolWeight(0.5f)
        , PackWeight(0.8f)
        , AmbushWeight(1.2f)
        , TerritorialWeight(0.6f)
        , MaxForce(500.0f)
        , MaxSpeed(800.0f)
        , DetectionRadius(1500.0f)
        , AttackRange(200.0f)
        , HuntingStrength(100.0f)
        , PatrolStrength(50.0f)
        , PackStrength(80.0f)
        , AmbushStrength(120.0f)
        , TerritorialStrength(60.0f)
        , PackCoordinationRadius(500.0f)
        , TerritoryRadius(2000.0f)
        , StaminaDecayRate(10.0f)
        , StaminaRecoveryRate(5.0f)
    {
    }

    // Pesos das forças de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Weights")
    float HuntingWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Weights")
    float PatrolWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Weights")
    float PackWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Weights")
    float AmbushWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Weights")
    float TerritorialWeight;

    // Limites de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed;

    // Parâmetros de detecção
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float AttackRange;

    // Forças de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces")
    float HuntingStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces")
    float PatrolStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces")
    float PackStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces")
    float AmbushStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces")
    float TerritorialStrength;

    // Parâmetros de coordenação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    float PackCoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    // Parâmetros de stamina
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaRecoveryRate;
};

/**
 * Fragment para dados de predador
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PredatorFragment
{
    GENERATED_BODY()

    FCrowd_PredatorFragment()
        : CurrentState(ECrowd_HuntingState::Patrol)
        , TargetLocation(FVector::ZeroVector)
        , PackCenter(FVector::ZeroVector)
        , TerritoryCenter(FVector::ZeroVector)
        , LastStateUpdateTime(0.0f)
        , StateChangeTime(0.0f)
        , Stamina(100.0f)
        , Hunger(0.0f)
        , Aggression(50.0f)
        , bHasTarget(false)
        , bInPack(false)
        , PackSize(1)
        , LastAttackTime(0.0f)
        , PatrolRadius(1000.0f)
    {
    }

    // Estado actual de caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    ECrowd_HuntingState CurrentState;

    // Localização do alvo actual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FVector TargetLocation;

    // Centro do pack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector PackCenter;

    // Centro do território
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    // Tempos de estado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float LastStateUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float StateChangeTime;

    // Atributos do predador
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    float Aggression;

    // Estado do alvo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    bool bHasTarget;

    // Estado do pack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bInPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackSize;

    // Tempo do último ataque
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastAttackTime;

    // Raio de patrulha
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;
};