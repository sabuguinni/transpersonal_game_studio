#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterAvailability = 50.0f;

    FEng_BiomeData()
    {
        VegetationTypes = {"Grass", "Ferns"};
        DinosaurSpecies = {"Triceratops", "Parasaurolophus"};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeAtLocation(const FVector& WorldLocation, EBiomeType NewBiome, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EBiomeType> GetNearbyBiomes(const FVector& WorldLocation, float SearchRadius = 5000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeInfluenceAtLocation(const FVector& WorldLocation, EBiomeType BiomeType) const;

    // Biome transition functions
    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    void AddBiomeTransition(const FEng_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    FEng_BiomeTransition GetBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const;

    // World generation integration
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeMap(int32 WorldSizeKm = 16);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ApplyBiomeToTerrain(const FVector& Location, float Radius = 2000.0f);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsLocationSuitableForSpecies(const FVector& WorldLocation, const FString& SpeciesName) const;

    // Debug and editor functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawBiomes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeInfoAtLocation(const FVector& WorldLocation) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Data")
    TMap<EBiomeType, FEng_BiomeData> BiomeDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Transitions")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Settings")
    int32 BiomeMapResolution = 512;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Settings")
    float WorldSizeMeters = 16000000.0f; // 16km x 16km

    // Internal biome map storage
    TArray<TArray<EBiomeType>> BiomeMap;
    TArray<TArray<float>> TemperatureMap;
    TArray<TArray<float>> HumidityMap;
    TArray<TArray<float>> ElevationMap;

private:
    void InitializeBiomeData();
    void InitializeDefaultTransitions();
    EBiomeType CalculateBiomeFromEnvironment(float Temperature, float Humidity, float Elevation) const;
    FVector2D WorldToMapCoordinates(const FVector& WorldLocation) const;
    bool IsValidMapCoordinate(const FVector2D& MapCoord) const;
};