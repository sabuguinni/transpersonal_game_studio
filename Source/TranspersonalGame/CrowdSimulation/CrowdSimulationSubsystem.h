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
struct TRANSPERSONALGAME_API FDinosaurHerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdSize = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesType = TEXT("Triceratops");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGrazing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertLevel = 0.0f; // 0.0 = calm, 1.0 = panicked
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPredatorPackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PackSize = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesType = TEXT("Velociraptor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.5f; // 0.0 = fed, 1.0 = starving
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAerialFlockData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FlockSize = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlightAltitude = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FlightPath = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlightSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesType = TEXT("Pteranodon");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMigrating = false;
};

/**
 * Subsystem responsible for managing large-scale dinosaur crowd simulation
 * Handles herds, packs, flocks and ecosystem interactions using Mass Entity framework
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Herd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(const FDinosaurHerdData& HerdData, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnPredatorPack(const FPredatorPackData& PackData, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnAerialFlock(const FAerialFlockData& FlockData, const FVector& SpawnLocation);

    // Ecosystem Events
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPredatorAlert(const FVector& AlertLocation, float AlertRadius = 3000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerStampede(const FVector& StampedeOrigin, const FVector& StampedeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalWeatherState(bool bIsStormy);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSimulationLOD(int32 LODLevel); // 0 = Full detail, 3 = Minimal

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCurrentPerformanceMetric() const;

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Active simulation data
    UPROPERTY()
    TArray<FDinosaurHerdData> ActiveHerds;

    UPROPERTY()
    TArray<FPredatorPackData> ActivePacks;

    UPROPERTY()
    TArray<FAerialFlockData> ActiveFlocks;

    // Performance tracking
    UPROPERTY()
    int32 CurrentLODLevel = 1;

    UPROPERTY()
    float LastFrameTime = 0.0f;

    UPROPERTY()
    int32 MaxSimultaneousEntities = 50000;

private:
    void UpdateHerdBehaviors(float DeltaTime);
    void UpdatePackBehaviors(float DeltaTime);
    void UpdateFlockBehaviors(float DeltaTime);
    void UpdatePerformanceLOD(float DeltaTime);
    
    bool bIsInitialized = false;
    float AccumulatedDeltaTime = 0.0f;
};