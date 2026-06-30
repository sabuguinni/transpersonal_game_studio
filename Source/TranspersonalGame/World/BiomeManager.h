#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome type enum — prehistoric survival world biomes
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Swamp       UMETA(DisplayName = "Swamp / Wetlands"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    Coastal     UMETA(DisplayName = "Coastal / Beach"),
    Canyon      UMETA(DisplayName = "Canyon / Badlands"),
    Unknown     UMETA(DisplayName = "Unknown")
};

// Per-biome configuration data
USTRUCT(BlueprintType)
struct FEng_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureBase = 25.0f;          // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityBase = 0.5f;              // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;         // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;       // Relative spawn frequency

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColorTint = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
};

// Zone registration entry
USTRUCT(BlueprintType)
struct FEng_BiomeZoneEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FEng_BiomeConfig Config;
};

/**
 * ABiomeManager — manages biome zones across the prehistoric world.
 * Determines which biome the player is in, drives ambient audio/visual changes,
 * controls dinosaur spawn weights per zone.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // All registered biome zones in this level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FEng_BiomeZoneEntry> BiomeZones;

    // Default biome when player is outside all zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    EEng_BiomeType DefaultBiome = EEng_BiomeType::Plains;

    // Current biome the player occupies
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    EEng_BiomeType CurrentPlayerBiome = EEng_BiomeType::Unknown;

    // Transition blend speed (0.0 = instant, 1.0 = 1 second full blend)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float BiomeTransitionSpeed = 2.0f;

    // How often (seconds) to poll player position for biome update
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float UpdateInterval = 0.5f;

    // Register a new biome zone at runtime
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiomeZone(const FEng_BiomeZoneEntry& Zone);

    // Query which biome a world position falls in
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biomes")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get config for a given biome type
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biomes")
    FEng_BiomeConfig GetBiomeConfig(EEng_BiomeType BiomeType) const;

    // Force-update current player biome (called by tick or external trigger)
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void UpdatePlayerBiome();

    // Called when player enters a new biome
    UFUNCTION(BlueprintImplementableEvent, Category = "Biomes")
    void OnPlayerEnteredBiome(EEng_BiomeType NewBiome, EEng_BiomeType OldBiome);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float TimeSinceLastUpdate = 0.0f;
    EEng_BiomeType LastKnownBiome = EEng_BiomeType::Unknown;

    // Default configs for each biome type
    TMap<EEng_BiomeType, FEng_BiomeConfig> DefaultConfigs;
    void InitDefaultConfigs();
};
