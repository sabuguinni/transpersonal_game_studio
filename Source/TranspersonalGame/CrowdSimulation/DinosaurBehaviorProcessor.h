#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "DinosaurCrowdManager.h"
#include "DinosaurBehaviorProcessor.generated.h"

/**
 * Processor principal para comportamento de dinossauros usando Mass Entity
 * Implementa state machine para cada dinossauro baseado em necessidades biológicas
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
    // Query para dinossauros ativos
    FMassEntityQuery DinosaurQuery;

    // Sistemas de comportamento
    void ProcessWanderingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FTransformFragment& Transform, FDinosaurSpeciesFragment& Species);
    void ProcessFeedingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FTransformFragment& Transform, FDinosaurSpeciesFragment& Species);
    void ProcessHuntingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FTransformFragment& Transform, FDinosaurSpeciesFragment& Species);
    void ProcessFleeingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FTransformFragment& Transform, FDinosaurSpeciesFragment& Species);
    void ProcessSocializingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FDinosaurSocialFragment& Social);

    // Transições de estado
    void EvaluateStateTransitions(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FDinosaurSpeciesFragment& Species, FTransformFragment& Transform);

    // Utilitários
    bool FindNearestFood(FVector CurrentLocation, EDinosaurType SpeciesType, float SearchRadius, FVector& OutFoodLocation);
    bool FindNearestWater(FVector CurrentLocation, float SearchRadius, FVector& OutWaterLocation);
    bool DetectThreats(FVector CurrentLocation, float DetectionRadius, EDinosaurSize MySize, FVector& OutThreatLocation);
    bool FindSafeLocation(FVector CurrentLocation, FVector ThreatLocation, float MinDistance, FVector& OutSafeLocation);

    // Configurações
    UPROPERTY(EditAnywhere, Category = "Behavior Settings")
    float StateChangeInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior Settings")
    float FoodSearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior Settings")
    float WaterSearchRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior Settings")
    float ThreatDetectionRadius = 1500.0f;
};

/**
 * Processor para movimento de dinossauros com avoidance
 * Integra com o sistema Mass Avoidance para movimentos naturais
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
    FMassEntityQuery MovementQuery;

    // Sistemas de movimento
    void ProcessWanderingMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FTransformFragment& Transform, FMassVelocityFragment& Velocity, FDinosaurBehaviorFragment& Behavior, FDinosaurSpeciesFragment& Species);
    void ProcessTargetedMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FTransformFragment& Transform, FMassVelocityFragment& Velocity, FDinosaurBehaviorFragment& Behavior, FDinosaurSpeciesFragment& Species);
    void ProcessFleeMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FTransformFragment& Transform, FMassVelocityFragment& Velocity, FDinosaurBehaviorFragment& Behavior, FDinosaurSpeciesFragment& Species);

    // Movimento de bando
    void ApplyPackCohesion(FMassEntityManager& EntityManager, FMassEntityHandle Entity, FTransformFragment& Transform, FMassVelocityFragment& Velocity, FDinosaurSocialFragment& Social);
    void ApplyPackSeparation(FMassEntityManager& EntityManager, FMassEntityHandle Entity, FTransformFragment& Transform, FMassVelocityFragment& Velocity, FDinosaurSocialFragment& Social);
    void ApplyPackAlignment(FMassEntityManager& EntityManager, FMassEntityHandle Entity, FTransformFragment& Transform, FMassVelocityFragment& Velocity, FDinosaurSocialFragment& Social);

    // Configurações de movimento
    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float MaxWanderSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float MaxFleeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float PackCohesionStrength = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float PackSeparationStrength = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float PackAlignmentStrength = 0.2f;
};

/**
 * Processor para interações sociais entre dinossauros
 * Gere formação de bandos, hierarquias e comunicação
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurSocialProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurSocialProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery SocialQuery;

    // Sistemas sociais
    void ProcessPackFormation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPackMaintenance(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurSocialFragment& Social, FTransformFragment& Transform);
    void ProcessTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FTransformFragment& Transform, FDinosaurSpeciesFragment& Species);

    // Comunicação entre dinossauros
    void ProcessDinosaurCommunication(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurSocialFragment& Social, FTransformFragment& Transform);

    // Gestão de bandos
    void CreateNewPack(FMassEntityManager& EntityManager, TArray<FMassEntityHandle>& Entities, EDinosaurType SpeciesType);
    void DisbandPack(FMassEntityManager& EntityManager, int32 PackID);
    bool ShouldFormPack(FMassEntityManager& EntityManager, FMassEntityHandle Entity, FDinosaurSpeciesFragment& Species, FDinosaurSocialFragment& Social);

    // Configurações sociais
    UPROPERTY(EditAnywhere, Category = "Social Settings")
    float PackFormationRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Social Settings")
    float PackDisbandDistance = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Social Settings")
    int32 MaxPacksPerSpecies = 20;

    // Tracking de bandos ativos
    TMap<int32, TArray<FMassEntityHandle>> ActivePacks;
    int32 NextPackID = 0;
};

/**
 * Processor para necessidades biológicas dos dinossauros
 * Gere fome, sede, cansaço e reprodução
 */
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
    FMassEntityQuery NeedsQuery;

    // Sistemas de necessidades
    void UpdateHunger(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, FDinosaurSpeciesFragment& Species, float DeltaTime);
    void UpdateThirst(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, float DeltaTime);
    void UpdateFatigue(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassEntityHandle Entity, FDinosaurBehaviorFragment& Behavior, float DeltaTime);

    // Configurações de necessidades
    UPROPERTY(EditAnywhere, Category = "Needs Settings")
    float HungerIncreaseRate = 0.1f; // Por minuto

    UPROPERTY(EditAnywhere, Category = "Needs Settings")
    float ThirstIncreaseRate = 0.15f; // Por minuto

    UPROPERTY(EditAnywhere, Category = "Needs Settings")
    float FatigueIncreaseRate = 0.05f; // Por minuto

    UPROPERTY(EditAnywhere, Category = "Needs Settings")
    float CriticalNeedThreshold = 0.8f; // Quando as necessidades se tornam críticas
};