#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "EngArch_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Fertility = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    FEng_BiomeData()
    {
        NativeDinosaurs.Empty();
        VegetationTypes.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeOfDay = 12.0f;

    FEng_WeatherData() {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FEng_BiomeData> GetAllBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeather(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FEng_WeatherData GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherOverride(const FEng_WeatherData& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ClearWeatherOverride();

    // Day/Night Cycle
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetDayProgress() const;

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool CanDinosaurSurviveInBiome(EDinosaurSpecies Species, EBiomeType BiomeType) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FEng_WeatherData CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bWeatherOverrideActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeScale = 1.0f;

private:
    void SetupDefaultBiomes();
    void UpdateEnvironmentalEffects();
    float CalculateDistanceToBiome(const FVector& Location, EBiomeType BiomeType) const;
};