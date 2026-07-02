#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome system enums and structs — Agent #5 World Generator
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Jungle Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Highland    UMETA(DisplayName = "Rocky Highland"),
    Volcanic    UMETA(DisplayName = "Volcanic Zone"),
    Wetland     UMETA(DisplayName = "Coastal Wetland"),
    River       UMETA(DisplayName = "River Corridor"),
    Count       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    AshFall     UMETA(DisplayName = "Volcanic Ash Fall")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseTemperatureCelsius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityPercent = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogTint = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EWorld_WeatherState> AllowedWeather;
};

USTRUCT(BlueprintType)
struct FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint = FVector(1000.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 150.0f;
};

// ============================================================
// ABiomeManager — manages biome zones, weather, and river data
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Query ----

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    const FWorld_BiomeData* GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // ---- Weather ----

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    EWorld_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void SetWeather(EWorld_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void AdvanceWeatherCycle(float DeltaSeconds);

    // ---- River ----

    UFUNCTION(BlueprintCallable, Category = "World|River")
    bool IsLocationNearRiver(const FVector& WorldLocation, float CheckRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "World|River")
    float GetRiverDistanceAtLocation(const FVector& WorldLocation) const;

    // ---- Day/Night ----

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    float GetTimeOfDay() const { return TimeOfDayHours; }

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    bool IsDaytime() const { return TimeOfDayHours >= 6.0f && TimeOfDayHours < 20.0f; }

    // ---- Initialization ----

    UFUNCTION(CallInEditor, Category = "World|Setup")
    void InitializeBiomes();

    UFUNCTION(CallInEditor, Category = "World|Setup")
    void InitializeRiverPath();

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<FWorld_RiverSegment> RiverPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionDuration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float TimeOfDayHours = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayDurationSeconds = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bEnableDayNightCycle = true;

private:
    float WeatherTimer = 0.0f;
    float NextWeatherChangeTime = 300.0f;

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeather(float DeltaTime);
    void ApplyBiomeFogSettings(EWorld_BiomeType BiomeType);
};
