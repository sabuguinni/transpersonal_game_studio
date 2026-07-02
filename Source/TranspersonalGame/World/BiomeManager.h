// BiomeManager.h — Transpersonal Game Studio
// Engine Architect #02 — PROD_CYCLE_AUTO_20260702_004
// Biome system for Cretaceous world — 6 biome types with temperature, humidity, foliage density

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Math/RandomStream.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    OpenPlain    UMETA(DisplayName = "Open Plain / Savanna"),
    Jungle       UMETA(DisplayName = "Dense Jungle"),
    Swamp        UMETA(DisplayName = "Primordial Swamp"),
    RiverDelta   UMETA(DisplayName = "River Delta"),
    Coastal      UMETA(DisplayName = "Coastal Shore"),
    Volcanic     UMETA(DisplayName = "Volcanic Badlands"),
    COUNT        UMETA(Hidden)
};

// ============================================================
// STRUCTS — must be at global scope (UE5 compilation rule)
// ============================================================

USTRUCT(BlueprintType)
struct FEng_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::OpenPlain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName DisplayName = FName("Unknown Biome");

    // Climate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate", meta = (ClampMin = "-20.0", ClampMax = "70.0"))
    float BaseTemperature = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    // Ecology
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PredatorSpawnWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HerbivoreSpawnWeight = 0.5f;

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "0.2"))
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float AmbientLightIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FEng_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 ZoneID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EEng_BiomeType BiomeType = EEng_BiomeType::OpenPlain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector2D Center = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FEng_BiomeConfig Config;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API UBiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // UGameInstanceSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Query API ----

    /** Returns the dominant biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns the full config for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FEng_BiomeConfig GetBiomeConfig(EEng_BiomeType BiomeType) const;

    /** Returns 0-1 blend weight of a biome at a location (for smooth transitions) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetBlendWeightAtLocation(FVector WorldLocation, EEng_BiomeType BiomeType) const;

    /** Returns true if location is within the specified biome zone */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    bool IsLocationInBiome(FVector WorldLocation, EEng_BiomeType BiomeType) const;

    /** Returns all biome types within SearchRadius of WorldLocation */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    TArray<EEng_BiomeType> GetNeighboringBiomes(FVector WorldLocation, float SearchRadius = 1000.0f) const;

    /** Returns temperature at location (°C), adjusted for altitude */
    UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Returns humidity at location (0-1) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    // ---- Configuration ----

    /** Re-seeds the world and regenerates all biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome|Config")
    void SetWorldSeed(int32 NewSeed);

    /** Returns display name for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome|Config")
    FName GetBiomeDisplayName(EEng_BiomeType BiomeType) const;

    /** Returns number of active biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome|Config")
    int32 GetActiveBiomeZoneCount() const;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    EEng_BiomeType CurrentBiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 WorldSeedValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float BiomeBlendRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    bool bBiomesInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Zones")
    TArray<FEng_BiomeZone> ActiveBiomeZones;

private:
    void InitializeBiomes();
    void GenerateBiomeZones();

    TMap<EEng_BiomeType, FEng_BiomeConfig> BiomeConfigs;
    FRandomStream BiomeRandom;
};
