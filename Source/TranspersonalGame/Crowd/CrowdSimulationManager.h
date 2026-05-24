#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "../SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    TArray<FMassEntityHandle> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector HerdCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 HerdSize;

    FCrowd_HerdData()
    {
        HerdCenter = FVector::ZeroVector;
        HerdRadius = 1000.0f;
        Species = EDinosaurSpecies::Triceratops;
        HerdSize = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EBiomeType StartBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EBiomeType EndBiome;

    FCrowd_MigrationRoute()
    {
        MigrationSpeed = 200.0f;
        StartBiome = EBiomeType::Savana;
        EndBiome = EBiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_HerdData> ActiveHerds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(EDinosaurSpecies Species, FVector Location, int32 HerdSize);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartMigration(int32 HerdIndex, const FCrowd_MigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateHerdBehavior(int32 HerdIndex, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalCrowdCount() const;

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    // Current simulation state
    float CurrentSeasonTime;
    int32 ActiveEntityCount;
    
    // Performance monitoring
    float LastPerformanceCheck;
    float AverageFrameTime;

public:
    // Public interface for other systems
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(FVector Location, int32 HerdSize, TSubclassOf<class ADinosaur> DinosaurClass);

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnFlock(FVector Location, int32 FlockSize, TSubclassOf<class AFlyingDinosaur> FlyingDinosaurClass);

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnPack(FVector Location, int32 PackSize, TSubclassOf<class APredatorDinosaur> PredatorClass);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerStampede(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMigrationActive(bool bActive);

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const { return ActiveEntityCount; }

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    float GetCurrentPerformanceMetric() const { return AverageFrameTime; }
};