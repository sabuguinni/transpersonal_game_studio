// BiomeSystem.h
// Agent #05 — Procedural World Generator
// PROD_CYCLE_AUTO_20260622_009
// Biome classification, PCG data queries, temperature/humidity blending.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BiomeSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// RULE 1: USTRUCT/UENUM at global scope — BEFORE the UCLASS
// RULE 2: Prefix all types with "World_" to avoid name collisions
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverCorridor   UMETA(DisplayName = "River Corridor"),
    Swampland       UMETA(DisplayName = "Swampland"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    EWorld_BiomeType BiomeID = EWorld_BiomeType::OpenPlains;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    FText DisplayName;

    /** World-space center of this biome zone */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    FVector CenterPoint = FVector::ZeroVector;

    /** Influence radius in cm */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    float Radius = 3000.f;

    /** Celsius */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    float BaseTemperature = 20.f;

    /** 0.0 = arid, 1.0 = saturated */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    float Humidity = 0.5f;

    /** 0.0 = barren, 1.0 = dense */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    float VegetationDensity = 0.5f;

    /** 0.0 = safe, 1.0 = extremely dangerous */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    float DangerLevel = 0.3f;

    /** Gameplay tag for ambient audio selection */
    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    FName AmbientAudioTag;
};

// ─────────────────────────────────────────────────────────────────────────────
// UBiomeSystem — World Subsystem (auto-created per world)
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Query API ────────────────────────────────────────────────

    /** Returns the dominant biome type at a world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full data for a given biome type */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    /** 0.0–1.0 blend weight of a biome at a location (for material blending) */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetBlendWeightAtLocation(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    /** Returns all registered biomes */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    TArray<FWorld_BiomeData> GetAllBiomes() const;

    /** Blended temperature at location (°C) */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Blended humidity at location (0–1) */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** True if the location sits in a blend zone between 2+ biomes */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    bool IsInTransitionZone(const FVector& WorldLocation, float TransitionWidth = 500.f) const;

private:
    /** Populates BiomeRegistry with hardcoded world layout */
    void BuildBiomeRegistry();

    /** Map from biome type to its data */
    TMap<EWorld_BiomeType, FWorld_BiomeData> BiomeRegistry;
};
