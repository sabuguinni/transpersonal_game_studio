#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Rocky       UMETA(DisplayName = "Rocky"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra")
};

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Misty       UMETA(DisplayName = "Misty"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityModifier = 0.0f;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeColor = FLinearColor::White;
        VegetationDensity = 1.0f;
        TemperatureModifier = 0.0f;
        HumidityModifier = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_WeatherZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsActive = true;

    FWorld_WeatherZone()
    {
        WeatherType = EWorld_WeatherType::Clear;
        Location = FVector::ZeroVector;
        Intensity = 0.5f;
        Duration = 300.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiomeZone(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RemoveBiomeZone(EWorld_BiomeType BiomeType);

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void CreateWeatherZone(const FWorld_WeatherZone& WeatherZone);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    EWorld_WeatherType GetWeatherAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherSystems(float DeltaTime);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsLocationInWater(const FVector& Location) const;

    // Vegetation Management
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationInBiome(EWorld_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearVegetationInRadius(const FVector& Center, float Radius);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherZone> WeatherZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseHumidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WeatherUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxVegetationPerBiome = 100;

private:
    float WeatherUpdateTimer = 0.0f;

    // Helper functions
    float CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& BiomeData) const;
    float CalculateDistanceToWeatherZone(const FVector& Location, const FWorld_WeatherZone& WeatherZone) const;
    void UpdateBiomeInfluence(float DeltaTime);
    void ProcessWeatherTransitions(float DeltaTime);
};

#include "World_BiomeSystem.generated.h"