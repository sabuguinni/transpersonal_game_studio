#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ============================================================
// Enums — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Zone"),
    River       UMETA(DisplayName = "River Corridor"),
    WateringHole UMETA(DisplayName = "Watering Hole"),
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Drought     UMETA(DisplayName = "Drought"),
};

// ============================================================
// Structs — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;      // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;
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
    float Width = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 100.0f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    bool bIsNavigable = false;
};

USTRUCT(BlueprintType)
struct FWorld_WateringHole
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WateringHole")
    FVector Location = FVector(8000.0f, 0.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WateringHole")
    float Radius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WateringHole")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WateringHole")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WateringHole")
    TArray<EWorld_BiomeType> ConnectedBiomes;
};

// ============================================================
// ABiomeSystem — main actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Query ----

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // ---- River System ----

    UFUNCTION(BlueprintCallable, Category = "River")
    bool IsNearRiver(const FVector& WorldLocation, float SearchRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "River")
    FVector GetNearestRiverPoint(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "River")
    float GetRiverFlowSpeedAt(const FVector& WorldLocation) const;

    // ---- Watering Hole ----

    UFUNCTION(BlueprintCallable, Category = "WateringHole")
    bool IsNearWateringHole(const FVector& WorldLocation, float SearchRadius = 800.0f) const;

    UFUNCTION(BlueprintCallable, Category = "WateringHole")
    FWorld_WateringHole GetNearestWateringHole(const FVector& WorldLocation) const;

    // ---- Weather ----

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWorld_WeatherState GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWorld_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetRainIntensity() const;

    // ---- PCG / World Setup ----

    UFUNCTION(CallInEditor, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(CallInEditor, Category = "World Generation")
    void RegenerateRiverCourse();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetBiomeCount() const;

    // ---- Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeData> BiomeRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Config")
    TArray<FWorld_RiverSegment> RiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WateringHole|Config")
    TArray<FWorld_WateringHole> WateringHoles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float WeatherTransitionDuration = 120.0f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    float WorldTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    float DayNightCycleSpeed = 1.0f; // 1.0 = real time, 60.0 = 1 min per hour

private:
    float WeatherTimer = 0.0f;
    float NextWeatherChangeTime = 300.0f;

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void ApplyBiomeFogSettings(EWorld_BiomeType Biome);
    void SetupDefaultBiomes();
    void SetupDefaultRiver();
    void SetupDefaultWateringHole();
};
