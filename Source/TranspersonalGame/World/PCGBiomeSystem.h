#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeSystem.generated.h"

// ── Biome type enum ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest    UMETA(DisplayName = "Forest"),
    Plains    UMETA(DisplayName = "Plains"),
    Rocky     UMETA(DisplayName = "Rocky"),
    Swamp     UMETA(DisplayName = "Swamp"),
    Volcanic  UMETA(DisplayName = "Volcanic"),
    River     UMETA(DisplayName = "River"),
    Unknown   UMETA(DisplayName = "Unknown")
};

// ── Per-biome configuration ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    /** World-space center of this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    /** Approximate radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.f;

    /** Foliage density multiplier (0=bare, 1=normal, 2=dense) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FoliageDensity = 1.0f;

    /** Ambient temperature in Celsius — affects player survival stats */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 22.f;

    /** Humidity 0-1 — affects fog density and plant growth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    /** Danger level 0-1 — scales predator spawn frequency */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.3f;

    /** Primary dino species tags that spawn in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> DinoSpeciesTags;

    /** Whether this biome has a water source (affects dino migration paths) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterSource = false;
};

// ── Dino spawn point data ──────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_DinoSpawnPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FName DinoSpeciesTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EWorld_BiomeType Biome = EWorld_BiomeType::Unknown;

    /** Max simultaneous dinos at this point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxCount = 3;

    /** Respawn delay in seconds after last dino dies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float RespawnDelay = 120.f;
};

// ── PCG Biome System Actor ─────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // ── Biome registry ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    // ── Spawn points ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
    TArray<FWorld_DinoSpawnPoint> DinoSpawnPoints;

    // ── World partition tile size (must be >= Lumen trace distance 8000cm) ─
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Streaming")
    float WorldPartitionTileSize = 16000.f;

    // ── Foliage cull distance scale (max 1.0 per PerformanceConfig.ini) ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float FoliageCullDistanceScale = 1.0f;

    // ── Query ─────────────────────────────────────────────────────────────
    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns the full biome config at a given world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeConfig GetBiomeConfigAtLocation(FVector WorldLocation) const;

    /** Returns all spawn points for a given species tag */
    UFUNCTION(BlueprintCallable, Category = "World|Spawning")
    TArray<FWorld_DinoSpawnPoint> GetSpawnPointsForSpecies(FName SpeciesTag) const;

    /** Returns danger level at location (0-1) */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetDangerLevelAtLocation(FVector WorldLocation) const;

    // ── Lifecycle ─────────────────────────────────────────────────────────
    virtual void BeginPlay() override;

#if WITH_EDITOR
    UFUNCTION(CallInEditor, Category = "World|Debug")
    void PopulateDefaultBiomes();
#endif

private:
    void InitDefaultBiomeConfigs();
    void InitDefaultSpawnPoints();
};
