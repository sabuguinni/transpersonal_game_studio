#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "MassDinosaurSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSimulationSubsystem;

/**
 * Subsystem responsible for managing dinosaur crowd simulation
 * Handles spawning, despawning, and lifecycle management of dinosaur herds
 * Supports up to 50,000 simultaneous agents using UE5 Mass Entity framework
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

    // Dinosaur herd management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnHerd(const FVector& Location, int32 HerdSize, TSubclassOf<class ADinosaurBase> DinosaurClass);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void DespawnHerd(int32 HerdID);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void TriggerMigration(const FVector& FromLocation, const FVector& ToLocation, float MigrationRadius = 5000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void TriggerPredatorAlert(const FVector& ThreatLocation, float AlertRadius = 2000.0f);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    int32 GetActiveEntityCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    float GetSimulationPerformance() const;

protected:
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;
    
    UPROPERTY()
    UMassSimulationSubsystem* MassSimulationSubsystem;

    // Herd tracking
    UPROPERTY()
    TMap<int32, FMassEntityHandle> ActiveHerds;
    
    UPROPERTY()
    int32 NextHerdID;

    // Performance metrics
    UPROPERTY()
    float LastFrameTime;
    
    UPROPERTY()
    int32 MaxEntitiesPerFrame;

private:
    void InitializeMassFramework();
    void SetupDinosaurArchetypes();
    void RegisterProcessors();
};