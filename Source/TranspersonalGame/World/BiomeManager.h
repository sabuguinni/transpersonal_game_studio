// BiomeManager.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260622_002
// Manages biome zones, terrain features, and environmental audio triggers
// for the MinPlayableMap. Four biomes: Forest, Plains, RockyRidge, RiverValley.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome type enum — unique prefix World_ to avoid conflicts
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    RockyRidge  UMETA(DisplayName = "Rocky Ridge"),
    RiverValley UMETA(DisplayName = "River Valley"),
    Unknown     UMETA(DisplayName = "Unknown")
};

// Per-biome configuration data
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    // World-space center of this biome zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    // Radius in Unreal Units within which this biome is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius = 2000.0f;

    // Ambient temperature in Celsius — affects player survival stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float AmbientTemperatureCelsius = 28.0f;

    // Humidity 0-1 — affects thirst drain rate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float Humidity = 0.5f;

    // Danger level 0-1 — used by AI spawn system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|AI")
    float DangerLevel = 0.5f;

    // Max dinosaur population in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|AI")
    int32 MaxDinoPopulation = 5;

    // Fog density multiplier for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensityMultiplier = 1.0f;

    FWorld_BiomeConfig() {}
};

// Biome transition event delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWorld_OnBiomeChanged,
    EWorld_BiomeType, OldBiome,
    EWorld_BiomeType, NewBiome);

UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Biome Configurations ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    // Current biome the player is in
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    EWorld_BiomeType CurrentPlayerBiome = EWorld_BiomeType::Unknown;

    // How often (seconds) to check player biome position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeCheckInterval = 2.0f;

    // Broadcast when player crosses biome boundary
    UPROPERTY(BlueprintAssignable, Category = "Biomes")
    FWorld_OnBiomeChanged OnBiomeChanged;

    // --- API ---

    // Returns the biome type at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Returns config for a given biome type (nullptr if not found)
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeConfig& OutConfig) const;

    // Returns danger level at a given location (0-1)
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // Returns ambient temperature at a given location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Returns humidity at a given location (affects thirst drain)
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // Force-initialize the four default MinPlayableMap biomes
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes")
    void InitializeDefaultBiomes();

private:
    float BiomeCheckTimer = 0.0f;

    // Cached player pawn location for biome checks
    FVector LastCheckedPlayerLocation = FVector::ZeroVector;

    void CheckPlayerBiome();
    void SetupForestBiome();
    void SetupPlainsBiome();
    void SetupRockyRidgeBiome();
    void SetupRiverValleyBiome();
};
