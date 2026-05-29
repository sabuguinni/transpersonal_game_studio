#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_EcosystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_EcosystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Temperate_Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float BiodiversityIndex = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float CarryingCapacity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float PredatorPreyRatio = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float VegetationDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float WaterAvailability = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<FString> DominantSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float SeasonalVariation = 0.5f;

    FWorld_EcosystemData()
    {
        DominantSpecies.Add("Ferns");
        DominantSpecies.Add("Conifers");
        DominantSpecies.Add("Cycads");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_FoodChainData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food Chain")
    TArray<FString> Producers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food Chain")
    TArray<FString> PrimaryConsumers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food Chain")
    TArray<FString> SecondaryConsumers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food Chain")
    TArray<FString> ApexPredators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food Chain")
    float EnergyTransferEfficiency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food Chain")
    float BiomassDistribution = 1.0f;

    FWorld_FoodChainData()
    {
        Producers.Add("Ferns");
        Producers.Add("Mosses");
        PrimaryConsumers.Add("Triceratops");
        PrimaryConsumers.Add("Hadrosaurs");
        SecondaryConsumers.Add("Dromaeosaurs");
        ApexPredators.Add("Tyrannosaurus");
        ApexPredators.Add("Giganotosaurus");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_EcosystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_EcosystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EcosystemVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FWorld_EcosystemData EcosystemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FWorld_FoodChainData FoodChainData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float EcosystemRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float PopulationUpdateInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableDynamicPopulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableSeasonalChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableMigrationPatterns = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float CurrentSeason = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<AActor*> EcosystemActors;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdatePopulations();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void ProcessSeasonalChanges();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void CalculateFoodChainBalance();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SpawnEcosystemActors();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateBiodiversity();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void ProcessMigration();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    float GetPopulationDensity() const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    float GetEcosystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SetBiomeType(EWorld_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    TArray<FString> GetActiveSpecies() const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void AddSpeciesToEcosystem(const FString& SpeciesName, int32 InitialPopulation);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RemoveSpeciesFromEcosystem(const FString& SpeciesName);

private:
    FTimerHandle PopulationUpdateTimer;
    FTimerHandle SeasonalChangeTimer;
    
    float LastUpdateTime = 0.0f;
    float SeasonalCycleLength = 120.0f; // 2 minutes per season for testing
    
    void UpdateEcosystemVisualization();
    void CalculateCarryingCapacity();
    void ProcessPredatorPreyInteractions();
    void UpdateVegetationGrowth();
    void ProcessEnvironmentalPressures();
};