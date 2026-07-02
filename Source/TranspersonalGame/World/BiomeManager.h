#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest    UMETA(DisplayName = "Forest"),
    Plains    UMETA(DisplayName = "Plains"),
    Rocky     UMETA(DisplayName = "Rocky"),
    Swamp     UMETA(DisplayName = "Swamp"),
    Volcanic  UMETA(DisplayName = "Volcanic"),
    River     UMETA(DisplayName = "River"),
    Unknown   UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsStorming = false;
};

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Query biome at world position
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get full biome zone data at location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    // Get current weather state
    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_WeatherState GetCurrentWeather() const;

    // Get danger level at location (0-1)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // Get temperature at location (Celsius)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Get humidity at location (0-1)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // Register a biome zone
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    // Update weather simulation
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void AdvanceWeather(float DeltaSeconds);

    // Get biome display name
    UFUNCTION(BlueprintPure, Category = "Biome")
    static FString GetBiomeName(EWorld_BiomeType BiomeType);

    // Biome zones in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    // Current global weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherState CurrentWeather;

    // Weather change rate (0-1 per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeRate = 0.01f;

    // Enable weather simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bSimulateWeather = true;

    // Day/night cycle time (0-1, 0=midnight, 0.5=noon)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float TimeOfDay = 0.5f;

    // Day length in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayLengthSeconds = 600.0f;

    // Enable day/night cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bSimulateDayNight = true;

private:
    void InitializeDefaultBiomes();
    void UpdateDayNightCycle(float DeltaSeconds);
    float WeatherTimer = 0.0f;
    float WeatherTargetRain = 0.0f;
    float WeatherTargetFog = 0.02f;
};
