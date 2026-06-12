#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Transition  UMETA(DisplayName = "Transition Zone")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm"),
    Blizzard    UMETA(DisplayName = "Blizzard"),
    VolcanicAsh UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
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
    float Temperature = 20.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WindStrength = 0.3f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EWorld_WeatherState> PossibleWeatherStates;

    FWorld_BiomeData()
    {
        PossibleWeatherStates.Add(EWorld_WeatherState::Clear);
        PossibleWeatherStates.Add(EWorld_WeatherState::Overcast);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherDuration = 300.0f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeRemaining = 300.0f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility = 1.0f; // 0-1 range, 1 = clear, 0 = no visibility
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherData CurrentWeatherData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bDynamicWeatherEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 600.0f; // 10 minutes

private:
    float WeatherTimer = 0.0f;
    float TransitionTimer = 0.0f;
    bool bWeatherTransitioning = false;
    EWorld_WeatherState TargetWeatherState = EWorld_WeatherState::Clear;

public:
    // Biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetWindStrengthAtLocation(const FVector& Location) const;

    // Weather functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(EWorld_WeatherState NewWeatherState, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeatherState(EWorld_WeatherState NewWeatherState, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    EWorld_WeatherState GetCurrentWeatherState() const { return CurrentWeatherData.CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetWeatherIntensity() const { return CurrentWeatherData.WeatherIntensity; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetVisibility() const { return CurrentWeatherData.Visibility; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FVector GetWindDirection() const { return CurrentWeatherData.WindDirection; }

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    bool IsWeatherHazardous() const;

protected:
    void UpdateDynamicWeather(float DeltaTime);
    void ProcessWeatherTransition(float DeltaTime);
    EWorld_WeatherState SelectRandomWeatherForBiome(EWorld_BiomeType BiomeType) const;
    void ApplyWeatherEffects();
};