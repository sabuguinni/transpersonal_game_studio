// BiomeSystem.h
// Agent #5 — Procedural World Generator
// Biome classification, query, and transition system for the prehistoric world

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    River       UMETA(DisplayName = "River Corridor"),
    Unknown     UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Ash         UMETA(DisplayName = "Volcanic Ash")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterXY = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D ExtentXY = FVector2D(2000.f, 2000.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseTemperature = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PrimaryBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType SecondaryBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendAlpha = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Temperature = 25.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float VegetationDensity = 0.5f;
};

// ─── BiomeSystem Actor ────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "WorldGen",
       meta = (DisplayName = "Biome System"))
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Biome Query ──────────────────────────────────────────────────────────

    /** Query biome data at a world XY position */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FWorld_BiomeQueryResult QueryBiomeAtLocation(FVector WorldLocation) const;

    /** Get the dominant biome type at a world XY position */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EWorld_BiomeType GetBiomeTypeAtLocation(FVector WorldLocation) const;

    /** Get temperature at location (biome base + time-of-day modifier) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Get humidity at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    // ── Weather ──────────────────────────────────────────────────────────────

    /** Current global weather state */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    EWorld_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    /** Force a weather transition (for scripted events) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Weather")
    void SetWeatherState(EWorld_WeatherState NewWeather);

    /** Tick weather simulation */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    void TickWeatherSimulation(float DeltaTime);

    // ── Biome Registration ───────────────────────────────────────────────────

    /** Register a biome zone at runtime */
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void RegisterBiomeZone(FWorld_BiomeZone Zone);

    /** Clear all registered biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void ClearBiomeZones();

    /** Get all registered biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const { return BiomeZones; }

    // ── Debug ────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeDebugOverlay(float Duration = 5.f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeBlendRadius = 1500.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WeatherTickInterval = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float DayNightCycleSpeed = 1.0f;

private:
    float WeatherTimer = 0.f;
    float DayPhase = 0.f;   // 0..1 = full day cycle

    void InitializeDefaultBiomes();
    float ComputeDistanceToBiome(const FWorld_BiomeZone& Zone, FVector2D XY) const;
};
