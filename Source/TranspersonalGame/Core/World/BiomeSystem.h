#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// Biome type enum — unique prefix World_ to avoid conflicts
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Prehistoric Forest"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Volcanic        UMETA(DisplayName = "Volcanic Zone"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Coastal         UMETA(DisplayName = "Coastal"),
    COUNT           UMETA(Hidden)
};

// Biome data struct — defines environmental properties per biome
USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Forest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float AmbientTemperature = 22.0f;   // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float Humidity = 0.7f;              // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float VegetationDensity = 0.8f;     // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float DinosaurSpawnRate = 1.0f;     // Multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FName AmbientSoundCue = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float PlayerStaminaDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float PlayerHungerDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float PlayerThirstDrainMultiplier = 1.0f;
};

// Weather state for a biome
UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    COUNT       UMETA(Hidden)
};

/**
 * ABiomeSystem — manages biome zones, transitions, and environmental state.
 * Agent #5 — Procedural World Generator
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Biome System Manager"))
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Biome Query ---

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data for a given biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    /** Returns biome data at a specific world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    // --- Weather ---

    /** Returns current weather state for a biome */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    EWorld_WeatherState GetCurrentWeather(EWorld_BiomeType BiomeType) const;

    /** Forces a weather transition (for scripted events) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    void SetWeather(EWorld_BiomeType BiomeType, EWorld_WeatherState NewWeather);

    /** Advances weather simulation by DeltaTime */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    void TickWeather(float DeltaTime);

    // --- Biome Registration ---

    /** Registers a biome zone (called by BiomeVolume actors) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(EWorld_BiomeType BiomeType, const FBox& ZoneBounds);

    // --- Debug ---

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void PrintBiomeDebugInfo() const;

protected:
    /** Default biome data table — indexed by EWorld_BiomeType */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeData> BiomeDataTable;

    /** Current weather per biome */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Weather")
    TMap<uint8, uint8> CurrentWeatherMap;

    /** Registered biome zones: BiomeType → bounding box */
    TMap<EWorld_BiomeType, FBox> BiomeZones;

    /** Weather transition timer per biome */
    TMap<EWorld_BiomeType, float> WeatherTimers;

    /** Minimum seconds before weather can change */
    UPROPERTY(EditAnywhere, Category = "Biome|Weather")
    float MinWeatherDuration = 120.0f;

    /** Maximum seconds before weather changes */
    UPROPERTY(EditAnywhere, Category = "Biome|Weather")
    float MaxWeatherDuration = 600.0f;

private:
    void InitializeBiomeDefaults();
    EWorld_WeatherState PickNextWeather(EWorld_BiomeType BiomeType) const;
};
