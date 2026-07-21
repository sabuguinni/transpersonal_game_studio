#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "BiomeSystemManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Wetland     UMETA(DisplayName = "Wetland"),
    Plains      UMETA(DisplayName = "Plains"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Coastal     UMETA(DisplayName = "Coastal")
};

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Snow        UMETA(DisplayName = "Snow"),
    Sandstorm   UMETA(DisplayName = "Sandstorm"),
    Fog         UMETA(DisplayName = "Fog"),
    Blizzard    UMETA(DisplayName = "Blizzard"),
    Drought     UMETA(DisplayName = "Drought")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureRange = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EWorld_WeatherType> PossibleWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* AmbientSound = nullptr;

    FWorld_BiomeData()
    {
        PossibleWeather.Add(EWorld_WeatherType::Clear);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType CurrentWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeRemaining = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeData(EWorld_BiomeType BiomeType, const FWorld_BiomeData& NewData);

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(EWorld_BiomeType BiomeType, EWorld_WeatherType WeatherType, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FWorld_WeatherState GetCurrentWeather(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionWeather(EWorld_BiomeType BiomeType, EWorld_WeatherType NewWeather, float TransitionTime = 30.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateEnvironmentalAudio(const FVector& ListenerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiomeAmbientSound(EWorld_BiomeType BiomeType, class USoundBase* AmbientSound);

    // Habitat Management
    UFUNCTION(BlueprintCallable, Category = "Habitat System")
    TArray<FVector> GetSpawnPointsForHabitat(EWorld_BiomeType BiomeType, int32 Count = 5) const;

    UFUNCTION(BlueprintCallable, Category = "Habitat System")
    bool IsLocationSuitableForSpecies(const FVector& Location, const FString& SpeciesName) const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void VisualizeAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeStatus() const;

protected:
    // Core biome data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EWorld_BiomeType, FWorld_BiomeData> BiomeDataMap;

    // Weather states per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    TMap<EWorld_BiomeType, FWorld_WeatherState> WeatherStates;

    // Audio components for environmental sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TMap<EWorld_BiomeType, class UAudioComponent*> BiomeAudioComponents;

    // Transition zones between biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    float BiomeTransitionRadius = 500.0f;

    // Weather update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    float WeatherUpdateInterval = 10.0f;

    // Last weather update time
    UPROPERTY()
    float LastWeatherUpdate = 0.0f;

    // Audio update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float AudioUpdateInterval = 1.0f;

    // Last audio update time
    UPROPERTY()
    float LastAudioUpdate = 0.0f;

private:
    // Internal helper functions
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeData& BiomeData) const;
    void UpdateBiomeAudio(EWorld_BiomeType BiomeType, float Volume, const FVector& ListenerLocation);
    EWorld_WeatherType SelectRandomWeather(const TArray<EWorld_WeatherType>& PossibleWeather) const;
    void ApplyWeatherEffects(EWorld_BiomeType BiomeType, const FWorld_WeatherState& WeatherState);
};