#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    FWorld_BiomeData()
    {
        VegetationTypes.Add("Tree");
        VegetationTypes.Add("Bush");
        VegetationTypes.Add("Fern");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherType = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.5f;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiome(EBiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWeatherType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeather(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWeatherType GetCurrentWeather() const { return CurrentWeather.WeatherType; }

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyBiomeEffects(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SpawnBiomeVegetation(EBiomeType BiomeType, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void CreateWaterFeatures(EBiomeType BiomeType, FVector Location);

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void GenerateTestBiomes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearAllBiomes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> ActiveBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherData CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherData TargetWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BiomeTransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WeatherUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableDynamicWeather = true;

private:
    float WeatherUpdateTimer = 0.0f;
    
    void UpdateFogEffects();
    void UpdateSkyEffects();
    void SpawnVegetationActor(FVector Location, FString VegetationType, float Scale = 1.0f);
    AStaticMeshActor* CreateWaterPlane(FVector Location, FVector Scale);
};