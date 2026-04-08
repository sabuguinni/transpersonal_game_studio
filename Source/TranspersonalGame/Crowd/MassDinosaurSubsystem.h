#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "MassDinosaurSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct FDinosaurHerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdSize = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName = TEXT("Triceratops");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrazingTime = 120.0f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationDistance = 5000.0f;
};

USTRUCT(BlueprintType)
struct FPredatorTerritoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PredatorCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName = TEXT("TRex");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RestTime = 300.0f; // seconds between hunts
};

/**
 * Mass Dinosaur Subsystem
 * Manages large-scale dinosaur simulation using Mass Entity framework
 * Handles herds, territories, migration patterns, and predator-prey relationships
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

    // Herd Management
    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SpawnHerd(const FDinosaurHerdData& HerdData);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SpawnPredatorTerritory(const FPredatorTerritoryData& TerritoryData);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void StartMigration(int32 HerdIndex, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void TriggerPanicResponse(FVector ThreatLocation, float PanicRadius);

    // World State Queries
    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    TArray<FVector> GetActiveHerdLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    int32 GetTotalDinosaurCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    float GetHerdDensityAtLocation(FVector Location, float Radius) const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SetSimulationLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Mass Dinosaur")
    void SetMaxSimulationDistance(float Distance);

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Active herds and territories
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FDinosaurHerdData> ActiveHerds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FPredatorTerritoryData> ActiveTerritories;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxDinosaurCount = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float SimulationRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentLODLevel = 2;

    // Simulation timing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation")
    float HerdUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation")
    float PredatorUpdateInterval = 3.0f;

private:
    void InitializeDefaultHerds();
    void InitializeDefaultTerritories();
    void UpdateHerdBehaviors();
    void UpdatePredatorBehaviors();

    FTimerHandle HerdUpdateTimer;
    FTimerHandle PredatorUpdateTimer;
};