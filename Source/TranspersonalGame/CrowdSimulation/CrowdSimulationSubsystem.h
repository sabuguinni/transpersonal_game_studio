#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCrowdSettings
{
    GENERATED_BODY()

    // Maximum number of dinosaurs per species in simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxHerbivores = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCarnivores = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxFlyers = 100;

    // Simulation radius around player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SimulationRadius = 5000.0f;

    // LOD distances for crowd detail
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FullDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 5000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdData
{
    GENERATED_BODY()

    // Herd identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = 0;

    // Species type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    // Herd size range
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 15;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 300.0f;

    // Danger response
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeSpeed = 1200.0f;
};

/**
 * Subsystem responsible for managing large-scale dinosaur crowd simulation
 * Uses Mass Entity framework for performance with thousands of agents
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd management functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(const FDinosaurHerdData& HerdData, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPredatorAlert(const FVector& PredatorLocation, float AlertRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD();

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FDinosaurCrowdSettings CrowdSettings;

    // Active herds tracking
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<FDinosaurHerdData> ActiveHerds;

protected:
    // Mass Entity references
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Current player position for LOD calculations
    FVector CurrentPlayerLocation;

    // Internal functions
    void SetupMassEntityProcessors();
    void CreateHerdBehaviorProcessors();
    void UpdateSimulationBounds();

    // Herd management
    int32 NextHerdID = 1;
    TMap<int32, FVector> HerdCenters;
    TMap<int32, float> HerdLastUpdateTime;

private:
    bool bIsInitialized = false;
    float LastLODUpdateTime = 0.0f;
    const float LODUpdateInterval = 1.0f; // Update LOD every second
};