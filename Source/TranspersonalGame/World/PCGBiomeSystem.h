#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeSystem.generated.h"

/**
 * Biome types for the Cretaceous world.
 * Each biome drives foliage density, ambient sound, survival modifiers.
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp Lowlands"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    River       UMETA(DisplayName = "River Corridor"),
    Unknown     UMETA(DisplayName = "Unknown")
};

/**
 * Per-biome configuration: survival modifiers, foliage density, ambient colour.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    /** World-space centre of this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Centre = FVector::ZeroVector;

    /** Radius in cm where this biome is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 250000.f;

    /** Temperature modifier applied to player survival stats (°C delta) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float TemperatureDelta = 0.f;

    /** Humidity modifier — affects thirst drain rate (multiplier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HumidityMultiplier = 1.0f;

    /** Foliage instances per 100m² cell — cap at 500 for 60fps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage", meta = (ClampMin = "0", ClampMax = "500"))
    int32 FoliageDensity = 200;

    /** Ambient fog colour for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor FogTint = FLinearColor(0.5f, 0.6f, 0.5f, 1.f);

    /** Dinosaur spawn weight multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float DinoSpawnMultiplier = 1.0f;
};

/**
 * APCGBiomeSystem — manages biome zones, queries player biome,
 * and broadcasts biome-change events for audio/survival systems.
 *
 * Placed once in MinPlayableMap. World Partition cell size: 128m (matches shadow cascades).
 */
UCLASS(ClassGroup = "WorldGen", meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** All biome zones in this world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    /** Query which biome a world-space location falls into */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get the full config for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeConfig& OutConfig) const;

    /** Apply biome survival modifiers to the player (called on biome enter) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void ApplyBiomeModifiers(EWorld_BiomeType NewBiome, AActor* PlayerActor);

    /** Current biome the player is in */
    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    EWorld_BiomeType CurrentPlayerBiome = EWorld_BiomeType::Unknown;

    /** How often (seconds) to re-check player biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float BiomeCheckInterval = 1.0f;

private:
    float TimeSinceLastBiomeCheck = 0.f;

    void InitDefaultBiomes();
    void CheckPlayerBiome();
};
