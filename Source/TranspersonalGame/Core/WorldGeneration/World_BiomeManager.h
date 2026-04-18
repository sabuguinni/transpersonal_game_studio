#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../../SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString AudioZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    FWorld_BiomeData()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        TreeDensity = 0.5f;
        RockDensity = 0.3f;
        AudioZone = TEXT("default");
        BiomeType = EBiomeType::Forest;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    FWorld_WeatherZone()
    {
        ZoneName = TEXT("Clear");
        Location = FVector::ZeroVector;
        WeatherType = EWeatherType::Clear;
        Visibility = 10000.0f;
        Intensity = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateBiomeVegetation(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateBiomeRocks(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void InitializeWeatherZones();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWeatherType GetWeatherAtLocation(const FVector& Location) const;

    // Audio Zones
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CreateAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString GetAudioZoneAtLocation(const FVector& Location) const;

    // Terrain Features
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void CreateTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHeightModifierAtLocation(const FVector& Location) const;

    // Water Features
    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateWaterFeatures();

    UFUNCTION(BlueprintCallable, Category = "Water")
    bool IsLocationNearWater(const FVector& Location, float Radius = 500.0f) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeDataArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherZone> WeatherZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 VegetationSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RockSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float WorldScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxRocksPerBiome;

private:
    void SetupDefaultBiomes();
    void SetupDefaultWeatherZones();
    FVector GetRandomPointInRadius(const FVector& Center, float Radius) const;
    float GetNoiseValue(const FVector& Location, float Scale) const;
};