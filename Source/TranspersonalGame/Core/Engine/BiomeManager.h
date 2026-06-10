#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "../../SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * BiomeManager - Core biome system architecture for prehistoric world
 * Manages biome transitions, environmental parameters, and ecosystem rules
 * Engine Architect responsibility - defines biome structure for all systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // CORE BIOME COMPONENTS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Core")
    UStaticMeshComponent* BiomeVisualizationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Core")
    UBoxComponent* BiomeBounds;

    // BIOME CONFIGURATION
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EEng_BiomeType CurrentBiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float TemperatureBase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float HumidityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float FoodAvailability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float WaterAvailability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float DangerLevel;

    // ECOSYSTEM PARAMETERS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<EEng_DinosaurSpecies> NativeDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<EEng_VegetationType> NativeVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 MaxDinosaurPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float VegetationDensity;

public:
    // BIOME QUERY INTERFACE
    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    EEng_BiomeType GetBiomeTypeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    float GetTemperatureAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    float GetHumidityAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    bool IsLocationInBiome(FVector WorldLocation);

    // BIOME MANAGEMENT
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiome(EEng_BiomeType BiomeType, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UpdateBiomeParameters(float Temperature, float Humidity, float Food, float Water);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RegisterDinosaurInBiome(class ADinosaurBase* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UnregisterDinosaurFromBiome(class ADinosaurBase* Dinosaur);

    // ECOSYSTEM VALIDATION
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    bool CanSupportDinosaurSpecies(EEng_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    int32 GetCurrentDinosaurCount();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    float GetEcosystemHealthScore();

private:
    // INTERNAL STATE
    TArray<class ADinosaurBase*> RegisteredDinosaurs;
    float LastUpdateTime;
    bool bBiomeInitialized;

    // INTERNAL METHODS
    void UpdateEcosystemBalance();
    void ValidateBiomeIntegrity();
    FVector GetBiomeCenterLocation();
};