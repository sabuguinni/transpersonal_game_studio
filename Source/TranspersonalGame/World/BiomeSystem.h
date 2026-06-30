#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest       UMETA(DisplayName = "Dense Forest"),
    Plains       UMETA(DisplayName = "Open Plains"),
    RockyHighlands UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta   UMETA(DisplayName = "River Delta"),
    Swamp        UMETA(DisplayName = "Swamp"),
    Volcanic     UMETA(DisplayName = "Volcanic Region"),
    Count        UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor GroundColor = FLinearColor(0.4f, 0.35f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeWeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningProbability = 0.0f;
};

/**
 * ABiomeSystem — Manages biome zones in the world.
 * Tracks player biome, controls weather per biome, drives dinosaur spawn weights.
 * Agent #5 — Procedural World Generator
 */
UCLASS(ClassGroup = "World", meta = (DisplayName = "Biome System Manager"))
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === BIOME QUERY ===
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeProperties GetBiomeProperties(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetPlayerCurrentBiome() const;

    // === WEATHER ===
    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_BiomeWeatherState GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(const FWorld_BiomeWeatherState& NewWeather);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather")
    void TriggerStorm();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather")
    void ClearWeather();

    // === BIOME REGISTRATION ===
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeProperties& BiomeProps, const FVector& Center);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const;

    // === EVENTS ===
    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnPlayerEnteredBiome(EWorld_BiomeType NewBiome, EWorld_BiomeType PreviousBiome);

    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnWeatherChanged(const FWorld_BiomeWeatherState& NewWeather);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeProperties> RegisteredBiomes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State",
              meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType CurrentPlayerBiome = EWorld_BiomeType::Plains;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|State",
              meta = (AllowPrivateAccess = "true"))
    FWorld_BiomeWeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeTransitionBlendTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float WeatherUpdateInterval = 60.0f;

private:
    TArray<FVector> BiomeCenters;
    float WeatherTimer = 0.0f;
    EWorld_BiomeType PreviousPlayerBiome = EWorld_BiomeType::Plains;

    void UpdatePlayerBiome();
    void UpdateWeatherSimulation(float DeltaTime);
    void InitializeDefaultBiomes();
};
