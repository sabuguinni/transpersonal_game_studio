#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterCoverage = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AtmosphericTint = FLinearColor::White;

    FWorld_BiomeData()
    {
        BiomeType = EBiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        VegetationDensity = 0.5f;
        WaterCoverage = 0.1f;
        RockDensity = 0.2f;
        AtmosphericTint = FLinearColor::White;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float VisibilityRange = 10000.0f;

    FWorld_WeatherState()
    {
        CurrentWeather = EWeatherType::Clear;
        Intensity = 0.5f;
        WindStrength = 0.3f;
        Temperature = 20.0f;
        Humidity = 0.4f;
        VisibilityRange = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> ActiveBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxVegetationPerBiome = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxRocksPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxWaterBodiesPerBiome = 10;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateBiome(EBiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RemoveBiome(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EBiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWeatherType NewWeather, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_WeatherState GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time")
    float GetTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiomeVegetation(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiomeWaterBodies(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiomeRockFormations(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ClearAllBiomes();

protected:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void SpawnVegetationForBiome(const FWorld_BiomeData& BiomeData);
    void SpawnWaterBodiesForBiome(const FWorld_BiomeData& BiomeData);
    void SpawnRockFormationsForBiome(const FWorld_BiomeData& BiomeData);
    FVector GetRandomLocationInBiome(const FWorld_BiomeData& BiomeData) const;
    bool IsLocationInBiome(FVector Location, const FWorld_BiomeData& BiomeData) const;
};