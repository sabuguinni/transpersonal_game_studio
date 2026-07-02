#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Badlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    River       UMETA(DisplayName = "River"),
    Unknown     UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogTint = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
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
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TemperatureModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsStorming = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Biome Query ---

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInWater(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // --- Weather ---

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_WeatherState GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(const FWorld_WeatherState& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerStorm(float Duration);

    // --- Day/Night ---

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    float GetTimeOfDay() const { return TimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    bool IsDaytime() const { return TimeOfDay >= 6.0f && TimeOfDay < 20.0f; }

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    FString GetTimeOfDayString() const;

    // --- PCG Biome Registration ---

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ClearAllBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const { return BiomeZones.Num(); }

    // --- Editor Utility ---

    UFUNCTION(CallInEditor, Category = "Biome")
    void SetupDefaultBiomes();

    UFUNCTION(CallInEditor, Category = "Biome")
    void PrintBiomeReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    FWorld_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float WeatherTransitionSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float StormTimeRemaining = 0.0f;

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeather(float DeltaTime);
    void ApplyBiomeFogTint(const FWorld_BiomeZone& Zone);
};
