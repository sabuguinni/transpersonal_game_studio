// PCGBiomeSystem.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_006
// Biome system: defines 5 biome types, zone data, and PCG spawning rules.
// Uses World_ prefix for all types per RULE 2.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "PCGBiomeSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    None        UMETA(DisplayName = "None / Transition")
};

UENUM(BlueprintType)
enum class EWorld_VegetationDensity : uint8
{
    Sparse      UMETA(DisplayName = "Sparse"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Dense       UMETA(DisplayName = "Dense"),
    Barren      UMETA(DisplayName = "Barren")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FWorld_BiomeSpawnRule
{
    GENERATED_BODY()

    /** Mesh asset soft reference for this spawn rule */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    /** Minimum scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "0.1", ClampMax = "20.0"))
    float ScaleMin = 0.8f;

    /** Maximum scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "0.1", ClampMax = "20.0"))
    float ScaleMax = 1.5f;

    /** Spawn density: instances per 10000 cm² */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "0", ClampMax = "100"))
    int32 DensityPer10kSqCm = 5;

    /** Maximum slope angle (degrees) for valid spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "0", ClampMax = "90"))
    float MaxSlopeAngle = 35.0f;

    /** Align to surface normal */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    bool bAlignToSurface = true;

    /** Random yaw rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    bool bRandomYaw = true;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZoneData
{
    GENERATED_BODY()

    /** Biome type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    /** Display name for debug/UI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    FString BiomeName = TEXT("Plains");

    /** World-space center of this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    FVector ZoneCenter = FVector::ZeroVector;

    /** Radius of influence (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "100", ClampMax = "100000"))
    float ZoneRadius = 5000.0f;

    /** Vegetation density category */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    EWorld_VegetationDensity VegetationDensity = EWorld_VegetationDensity::Moderate;

    /** Base ground color tint for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    FLinearColor GroundColorTint = FLinearColor(0.3f, 0.25f, 0.1f, 1.0f);

    /** Fog density multiplier for this biome (1.0 = global default) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FogDensityMultiplier = 1.0f;

    /** Ambient temperature in Celsius (affects survival mechanics) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "-50", ClampMax = "80"))
    float AmbientTemperatureCelsius = 22.0f;

    /** Spawn rules for vegetation in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    TArray<FWorld_BiomeSpawnRule> VegetationSpawnRules;

    /** Dinosaur species tags that prefer this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    TArray<FName> PreferredDinoSpecies;

    /** Hazard level 0-1 (0=safe, 1=extreme danger) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HazardLevel = 0.3f;
};

// ─── Actor ───────────────────────────────────────────────────────────────────

/**
 * APCGBiomeZoneActor
 * Placed in the level to define a biome region.
 * The PCGWorldGenerator queries these actors to determine biome at any world position.
 * Supports up to 5 biome zones in MinPlayableMap (Forest/Plains/Rocky/Swamp/Volcanic).
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome Zone"))
class TRANSPERSONALGAME_API APCGBiomeZoneActor : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeZoneActor();

    // ── Zone data ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    FWorld_BiomeZoneData BiomeData;

    // ── Components ────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|PCG",
              meta = (AllowPrivateAccess = "true"))
    UBoxComponent* ZoneBounds;

    // ── Interface ─────────────────────────────────────────────────────────

    /** Returns true if WorldLocation falls within this biome zone */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    bool IsLocationInZone(const FVector& WorldLocation) const;

    /** Returns blend weight 0-1 based on distance from zone center */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetBlendWeightAtLocation(const FVector& WorldLocation) const;

    /** Returns the biome type at this zone */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    EWorld_BiomeType GetBiomeType() const { return BiomeData.BiomeType; }

    /** Returns ambient temperature for survival system */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetAmbientTemperature() const { return BiomeData.AmbientTemperatureCelsius; }

    /** Returns hazard level for AI threat assessment */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetHazardLevel() const { return BiomeData.HazardLevel; }

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

// ─── Manager ─────────────────────────────────────────────────────────────────

/**
 * APCGBiomeManager
 * Singleton-style actor that aggregates all APCGBiomeZoneActor instances
 * and provides world-position → biome queries for other systems.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome Manager"))
class TRANSPERSONALGAME_API APCGBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeManager();

    // ── Registered zones ──────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|PCG")
    TArray<APCGBiomeZoneActor*> RegisteredZones;

    // ── Interface ─────────────────────────────────────────────────────────

    /** Scan the world for all APCGBiomeZoneActor instances and register them */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void RefreshZoneRegistry();

    /** Returns the dominant biome type at WorldLocation */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns the full biome zone data for the dominant biome at WorldLocation */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    FWorld_BiomeZoneData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /** Returns ambient temperature at WorldLocation (for survival system) */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns hazard level at WorldLocation (for AI threat system) */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetHazardLevelAtLocation(const FVector& WorldLocation) const;

    /** Returns number of registered biome zones */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    int32 GetZoneCount() const { return RegisteredZones.Num(); }

protected:
    virtual void BeginPlay() override;

private:
    /** Default biome data returned when no zone matches */
    FWorld_BiomeZoneData DefaultBiomeData;

    void InitializeDefaultBiomeData();
};
