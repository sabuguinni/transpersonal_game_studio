#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    FWorld_BiomeZone()
    {
        BiomeName = TEXT("Unknown");
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeType = EBiomeType::Forest;
        Temperature = 20.0f;
        Humidity = 50.0f;
        Elevation = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 TreeCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 BushCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MinTreeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MaxTreeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity;

    FWorld_VegetationConfig()
    {
        TreeCount = 10;
        BushCount = 5;
        MinTreeScale = 1.0f;
        MaxTreeScale = 2.0f;
        VegetationDensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FString WeatherName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration;

    FWorld_WeatherZone()
    {
        WeatherName = TEXT("Clear");
        Location = FVector::ZeroVector;
        WeatherType = EWeatherType::Clear;
        Intensity = 1.0f;
        Duration = 300.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FWorld_VegetationConfig> VegetationConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherZone> WeatherZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float WorldScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnStart;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GenerateVegetation();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeZone GetNearestBiome(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherSystems();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWeatherType GetWeatherAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Generation", CallInEditor)
    void RegenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "Generation", CallInEditor)
    void ClearGeneratedContent();

private:
    void SetupDefaultBiomes();
    void SetupVegetationConfigs();
    void SpawnVegetationInZone(const FWorld_BiomeZone& Zone, const FWorld_VegetationConfig& Config);
    FVector GetRandomPointInZone(const FWorld_BiomeZone& Zone) const;
    float CalculateDistanceToZone(const FVector& Location, const FWorld_BiomeZone& Zone) const;
};