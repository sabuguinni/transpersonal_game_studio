#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "MassDinosaurSubsystem.generated.h"

class UMassEntityConfigAsset;
class UMassSpawnerConfig;

/**
 * Subsystem responsible for managing large-scale dinosaur crowd simulation
 * Handles spawning, lifecycle, and coordination of up to 50,000 dinosaur entities
 */
UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SpawnDinosaurHerd(const FVector& Location, int32 Count, const FString& SpeciesType);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SpawnPredatorPack(const FVector& Location, int32 Count, const FString& SpeciesType);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void DespawnDinosaursInRadius(const FVector& Location, float Radius);

    // Ecosystem Management
    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void TriggerMigration(const FString& SpeciesType, const FVector& FromLocation, const FVector& ToLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SetDayNightCycle(bool bIsDay);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void TriggerWeatherEvent(const FString& WeatherType, float Intensity);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SetSimulationLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    int32 GetActiveDinosaurCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    float GetCurrentPerformanceMetric() const;

protected:
    // Core Mass Framework References
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> EntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> SpawnerSubsystem;

    UPROPERTY()
    TObjectPtr<UMassSimulationSubsystem> SimulationSubsystem;

    // Dinosaur Species Configurations
    UPROPERTY(EditDefaultsOnly, Category = "Dinosaur Species")
    TMap<FString, TObjectPtr<UMassEntityConfigAsset>> HerbivoreConfigs;

    UPROPERTY(EditDefaultsOnly, Category = "Dinosaur Species")
    TMap<FString, TObjectPtr<UMassEntityConfigAsset>> CarnivoreConfigs;

    UPROPERTY(EditDefaultsOnly, Category = "Dinosaur Species")
    TMap<FString, TObjectPtr<UMassEntityConfigAsset>> OmnivoreConfigs;

    // Spawning Configuration
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TObjectPtr<UMassSpawnerConfig> DefaultSpawnerConfig;

    // Performance Tracking
    UPROPERTY()
    int32 CurrentEntityCount;

    UPROPERTY()
    float LastFrameTime;

    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    int32 MaxEntityCount = 50000;

    UPROPERTY(EditDefaultsOnly, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

private:
    // Internal Management
    void UpdatePerformanceMetrics();
    void OptimizeSimulationLOD();
    bool CanSpawnMoreEntities(int32 RequestedCount) const;
    
    // Ecosystem Logic
    void InitializeEcosystemZones();
    void UpdateEcosystemBehavior();
    
    FTimerHandle EcosystemUpdateTimer;
    FTimerHandle PerformanceUpdateTimer;
};