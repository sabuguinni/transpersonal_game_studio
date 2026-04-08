#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowdType : uint8
{
    HerbivoreHerd,      // Manadas de herbívoros (Triceratops, Parasaurolophus)
    PredatorPack,       // Grupos de predadores (Velociraptors)
    ScavengerFlock,     // Bandos de carniceiros (Compsognathus)
    FlyingSwarm,        // Enxames voadores (Pteranodon)
    AquaticSchool       // Cardumes aquáticos (Plesiosaur)
};

UENUM(BlueprintType)
enum class ECrowdBehaviorState : uint8
{
    Grazing,           // Pastando calmamente
    Migrating,         // Em migração
    Fleeing,           // Fugindo de predador
    Drinking,          // Bebendo água
    Resting,           // Descansando
    Hunting,           // Caçando (predadores)
    Investigating      // Investigando distúrbio
};

USTRUCT(BlueprintType)
struct FCrowdSpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdType CrowdType = ECrowdType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinGroupSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGroupSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 300.0f;
};

UCLASS()
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Mass Entity System
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    // Crowd spawn parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowdSpawnParameters> CrowdSpawnConfigs;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 1000.0f; // Full simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 3000.0f; // Simplified simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 8000.0f; // Visual only

    // Environmental factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<AActor*> WaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<AActor*> FeedingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<AActor*> RestingAreas;

public:
    // Spawn crowd functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowd(const FCrowdSpawnParameters& SpawnParams, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMassFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartSeasonalMigration(ECrowdType CrowdType, const FVector& TargetLocation);

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetNearestCrowdPositions(const FVector& QueryLocation, float Radius) const;

private:
    void InitializeMassEntitySystem();
    void UpdateCrowdBehaviors(float DeltaTime);
    void ProcessEnvironmentalInfluences();
    void ManageLODSystem();

    // Internal crowd tracking
    TArray<FMassEntityHandle> ActiveCrowdEntities;
    float LastBehaviorUpdateTime = 0.0f;
    float BehaviorUpdateInterval = 0.1f; // 10 FPS for crowd logic
};