#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassExecutionContext.h"
#include "SharedTypes.h"
#include "Crowd_MassivePrehistoricCrowdSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_PrehistoricSpeciesType : uint8
{
    Herbivore_Small     UMETA(DisplayName = "Small Herbivore"),
    Herbivore_Large     UMETA(DisplayName = "Large Herbivore"),
    Carnivore_Pack      UMETA(DisplayName = "Pack Carnivore"),
    Carnivore_Apex      UMETA(DisplayName = "Apex Predator"),
    Omnivore_Scavenger  UMETA(DisplayName = "Scavenger"),
    Flying_Pterosaur    UMETA(DisplayName = "Flying Pterosaur"),
    Aquatic_Marine      UMETA(DisplayName = "Marine Reptile")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PrehistoricHerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    ECrowd_PrehistoricSpeciesType SpeciesType = ECrowd_PrehistoricSpeciesType::Herbivore_Small;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    int32 HerdSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float CohesionStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float SeparationStrength = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float AlignmentStrength = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    bool bIsMigratory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float MigrationSpeed = 100.0f;

    FCrowd_PrehistoricHerdData()
    {
        SpeciesType = ECrowd_PrehistoricSpeciesType::Herbivore_Small;
        HerdSize = 10;
        HerdRadius = 500.0f;
        CohesionStrength = 1.0f;
        SeparationStrength = 2.0f;
        AlignmentStrength = 1.5f;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 2000.0f;
        bIsMigratory = false;
        MigrationSpeed = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveEcosystemConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 MaxEntitiesPerSpecies = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float EcosystemRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<FCrowd_PrehistoricHerdData> SpeciesConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float PredatorPreyRatio = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float FoodChainBalance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableMassiveBattles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float BattleIntensityMultiplier = 1.5f;

    FCrowd_MassiveEcosystemConfig()
    {
        MaxEntitiesPerSpecies = 1000;
        EcosystemRadius = 10000.0f;
        PredatorPreyRatio = 0.1f;
        FoodChainBalance = 1.0f;
        bEnableMassiveBattles = true;
        BattleIntensityMultiplier = 1.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassivePrehistoricCrowdSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassivePrehistoricCrowdSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EcosystemVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Crowd System")
    FCrowd_MassiveEcosystemConfig EcosystemConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Crowd System")
    int32 TotalActiveEntities = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Crowd System")
    float PerformanceBudget = 16.67f; // Target 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Crowd System")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Crowd System")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Crowd System")
    float LODDistanceFar = 5000.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void InitializeEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void SpawnSpeciesHerd(ECrowd_PrehistoricSpeciesType SpeciesType, int32 Count, FVector SpawnCenter);

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void TriggerMassiveBattle(FVector BattleCenter, float BattleRadius);

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    int32 GetActiveEntityCount() const { return TotalActiveEntities; }

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    float GetCurrentPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void SetEcosystemRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void AddSpeciesConfiguration(const FCrowd_PrehistoricHerdData& SpeciesConfig);

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void StartMigrationEvent();

    UFUNCTION(BlueprintCallable, Category = "Massive Crowd System")
    void HandlePredatorEncounter(FVector EncounterLocation, ECrowd_PrehistoricSpeciesType PredatorType);

private:
    void UpdateEcosystemBalance();
    void ProcessMassEntities();
    void HandleTerritorialBehavior();
    void UpdateMigrationPatterns();
    void ProcessPredatorPreyInteractions();
    void OptimizeCrowdDensity();

    TArray<FMassEntityHandle> ActiveEntities;
    float LastPerformanceCheck = 0.0f;
    bool bEcosystemInitialized = false;
};