#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_CretaceousEcosystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_EcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float WaterCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<AActor*> ZoneActors;

    FWorld_EcosystemZone()
    {
        BiomeType = EWorld_BiomeType::Temperate;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        VegetationDensity = 0.5f;
        WaterCoverage = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterSystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FVector> RiverPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FVector> LakeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float RiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bHasWaterfalls;

    FWorld_WaterSystem()
    {
        RiverWidth = 500.0f;
        WaterLevel = 0.0f;
        bHasWaterfalls = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_CretaceousEcosystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_CretaceousEcosystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Management")
    TArray<FWorld_EcosystemZone> EcosystemZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Systems")
    FWorld_WaterSystem WaterSystemConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Management")
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Management")
    float EcosystemUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Management")
    bool bAutoManageEcosystem;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void InitializeEcosystemZones();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void CreateBiomeZone(EWorld_BiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Water Systems")
    void GenerateRiverNetwork();

    UFUNCTION(BlueprintCallable, Category = "Water Systems")
    void CreateLakeSystem();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void PopulateVegetation(const FWorld_EcosystemZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void UpdateEcosystemDynamics();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    EWorld_BiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    float GetVegetationDensityAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    bool IsLocationNearWater(FVector Location, float SearchRadius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void CreateWaterfall(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void SpawnBiomeSpecificVegetation(EWorld_BiomeType BiomeType, FVector Location);

private:
    float LastEcosystemUpdate;
    
    void CreateForestVegetation(FVector Location);
    void CreateSavannaVegetation(FVector Location);
    void CreateSwampVegetation(FVector Location);
    void CreateDesertVegetation(FVector Location);
    void CreateMountainVegetation(FVector Location);
    
    AActor* SpawnVegetationActor(FVector Location, float Scale = 1.0f);
    AActor* SpawnWaterActor(FVector Location, FVector Scale);
    
    bool IsValidSpawnLocation(FVector Location) const;
    FVector GetRandomLocationInZone(const FWorld_EcosystemZone& Zone) const;
};