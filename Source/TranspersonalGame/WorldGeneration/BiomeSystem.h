#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "BiomeSystem.generated.h"

// Biome type enum — unique prefix World_ to avoid conflicts
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highland"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    None        UMETA(DisplayName = "None")
};

// Biome zone data struct
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;
};

// Weather state for a biome
USTRUCT(BlueprintType)
struct FWorld_BiomeWeather
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningChance = 0.0f;
};

/**
 * ABiomeSystem — Manages 5 distinct biome zones for the prehistoric world.
 * Handles biome detection, weather per biome, and environmental audio cues.
 * Agent #5 — Procedural World Generator
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    // --- Biome Zone Data ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // --- Current player biome ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    EWorld_BiomeType CurrentPlayerBiome;

    // --- Weather per biome ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TMap<EWorld_BiomeType, FWorld_BiomeWeather> BiomeWeatherMap;

    // --- Day/Night cycle time (0-24h) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayDurationSeconds;

    // --- Methods ---
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_BiomeWeather GetWeatherForBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void AdvanceTimeOfDay(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    UFUNCTION(CallInEditor, Category = "Debug")
    void InitializeDefaultBiomes();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void SetupDefaultWeather();
};
