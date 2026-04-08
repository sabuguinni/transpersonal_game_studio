#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "DinosaurCrowdSystem.h"
#include "DinosaurBehaviorProcessor.generated.h"

/**
 * Processador principal que gere o comportamento de todos os dinossauros
 * Executa a cada frame para actualizar estados comportamentais
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    // Query para dinossauros activos
    FMassEntityQuery DinosaurQuery;
    
    // Métodos de comportamento específicos
    void ProcessIdleBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior, 
                           const FDinosaurTypeFragment& Type, const FDinosaurNeedsFragment& Needs,
                           FTransformFragment& Transform);
    
    void ProcessGrazingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                              const FDinosaurTypeFragment& Type, FDinosaurNeedsFragment& Needs,
                              const FTransformFragment& Transform);
    
    void ProcessMovingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                             const FDinosaurTypeFragment& Type, FTransformFragment& Transform,
                             FMassVelocityFragment& Velocity);
    
    void ProcessFleeingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                              const FDinosaurTypeFragment& Type, const FDinosaurPerceptionFragment& Perception,
                              FTransformFragment& Transform, FMassVelocityFragment& Velocity);
    
    void ProcessHuntingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                              const FDinosaurTypeFragment& Type, const FDinosaurPerceptionFragment& Perception,
                              FTransformFragment& Transform, FMassVelocityFragment& Velocity);
    
    void ProcessSleepingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                               FDinosaurNeedsFragment& Needs);
    
    void ProcessDrinkingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                               FDinosaurNeedsFragment& Needs, const FTransformFragment& Transform);
    
    void ProcessSocializingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                                  const FDinosaurTypeFragment& Type, const FDinosaurPerceptionFragment& Perception);
    
    // Métodos auxiliares
    EDinosaurBehavior DetermineNextBehavior(const FDinosaurTypeFragment& Type, 
                                          const FDinosaurNeedsFragment& Needs,
                                          const FDinosaurPerceptionFragment& Perception,
                                          const FDinosaurBehaviorFragment& CurrentBehavior) const;
    
    float CalculateBehaviorDuration(EDinosaurBehavior Behavior, const FDinosaurTypeFragment& Type) const;
    
    FVector GenerateRandomMovementTarget(const FTransformFragment& Transform, float MaxDistance) const;
    
    bool ShouldFleeFromThreat(const FDinosaurTypeFragment& Type, const FDinosaurPerceptionFragment& Perception) const;
    
    bool ShouldHuntPrey(const FDinosaurTypeFragment& Type, const FDinosaurNeedsFragment& Needs,
                       const FDinosaurPerceptionFragment& Perception) const;
};

/**
 * Processador para actualizar necessidades básicas (fome, sede, energia)
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurNeedsProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurNeedsProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery NeedsQuery;
    
    // Configurações de degradação das necessidades
    UPROPERTY(EditAnywhere, Category = "Needs Configuration")
    float HungerDecayRate = 0.1f; // Por segundo
    
    UPROPERTY(EditAnywhere, Category = "Needs Configuration")
    float ThirstDecayRate = 0.15f; // Por segundo
    
    UPROPERTY(EditAnywhere, Category = "Needs Configuration")
    float EnergyDecayRate = 0.05f; // Por segundo
    
    UPROPERTY(EditAnywhere, Category = "Needs Configuration")
    float RestEnergyGain = 0.3f; // Por segundo durante sono
    
    void UpdateHunger(FDinosaurNeedsFragment& Needs, float DeltaTime, const FDinosaurBehaviorFragment& Behavior) const;
    void UpdateThirst(FDinosaurNeedsFragment& Needs, float DeltaTime, const FDinosaurBehaviorFragment& Behavior) const;
    void UpdateEnergy(FDinosaurNeedsFragment& Needs, float DeltaTime, const FDinosaurBehaviorFragment& Behavior) const;
};

/**
 * Processador para detecção de outros dinossauros e ameaças
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurPerceptionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurPerceptionProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PerceptionQuery;
    
    // Configurações de percepção
    UPROPERTY(EditAnywhere, Category = "Perception Configuration")
    float PerceptionUpdateInterval = 0.5f; // Segundos entre actualizações
    
    UPROPERTY(EditAnywhere, Category = "Perception Configuration")
    float MaxPerceptionDistance = 3000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Perception Configuration")
    float PlayerDetectionBonus = 1.5f; // Multiplicador para detectar o jogador
    
    void UpdateNearbyDinosaurs(FDinosaurPerceptionFragment& Perception, const FTransformFragment& Transform,
                              const FDinosaurTypeFragment& Type, UMassEntitySubsystem& EntitySubsystem) const;
    
    void UpdateThreatDetection(FDinosaurPerceptionFragment& Perception, const FTransformFragment& Transform,
                              const FDinosaurTypeFragment& Type, UMassEntitySubsystem& EntitySubsystem) const;
    
    void UpdatePlayerDetection(FDinosaurPerceptionFragment& Perception, const FTransformFragment& Transform,
                              const FDinosaurTypeFragment& Type) const;
    
    bool IsEntityThreat(const FDinosaurTypeFragment& ObserverType, const FDinosaurTypeFragment& TargetType) const;
    bool IsEntityPrey(const FDinosaurTypeFragment& ObserverType, const FDinosaurTypeFragment& TargetType) const;
};

/**
 * Processador para comportamento de grupo (bandos, manadas)
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurGroupProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurGroupProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery GroupQuery;
    
    // Configurações de grupo
    UPROPERTY(EditAnywhere, Category = "Group Configuration")
    float GroupCohesionRadius = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Group Configuration")
    float GroupSeparationRadius = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Group Configuration")
    float LeaderFollowDistance = 500.0f;
    
    void ProcessGroupCohesion(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                            const FDinosaurTypeFragment& Type, FTransformFragment& Transform,
                            FMassVelocityFragment& Velocity, UMassEntitySubsystem& EntitySubsystem) const;
    
    void ProcessLeaderBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior,
                             const FDinosaurTypeFragment& Type, FTransformFragment& Transform,
                             UMassEntitySubsystem& EntitySubsystem) const;
    
    FVector CalculateGroupCenterOfMass(uint32 GroupID, UMassEntitySubsystem& EntitySubsystem) const;
    FVector CalculateSeparationForce(const FTransformFragment& Transform, uint32 GroupID,
                                   UMassEntitySubsystem& EntitySubsystem) const;
};