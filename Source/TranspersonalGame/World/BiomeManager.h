// BiomeManager.h
// Procedural World Generator — Agent #5
// Manages biome zones, atmosphere, and environmental features for the prehistoric world.
// Biomes: JungleForest, VolcanicPlains, RockyHighlands, RiverDelta, OpenSavanna

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"
#include "Components/StaticMeshComponent.h"
#include "BiomeManager.generated.h"

// ── ENUMS ─────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    JungleForest    UMETA(DisplayName = "Jungle Forest"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
};

// ── STRUCTS ───────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AtmosphereColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity = 50;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor LightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LightIntensity = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AttenuationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bCastShadows = false;
};

// ── BIOME MANAGER ACTOR ───────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "World Biome Manager"))
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

    // ── BIOME ZONES ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    float BiomeTransitionBlendRadius = 500.0f;

    // ── ATMOSPHERE CONFIGS ───────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Atmosphere")
    FWorld_BiomeAtmosphere JungleAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Atmosphere")
    FWorld_BiomeAtmosphere VolcanicAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Atmosphere")
    FWorld_BiomeAtmosphere HighlandAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Atmosphere")
    FWorld_BiomeAtmosphere RiverAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Atmosphere")
    FWorld_BiomeAtmosphere SavannaAtmosphere;

    // ── RUNTIME STATE ────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Runtime",
        meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType CurrentPlayerBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Runtime",
        meta = (AllowPrivateAccess = "true"))
    float CurrentBiomeBlend;

    // ── FUNCTIONS ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetBiomeBlendWeight(FVector WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeZone GetBiomeZone(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    void UpdatePlayerBiome(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    bool IsLocationInWater(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FLinearColor GetAtmosphereColorAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Debug")
    void PrintBiomeDebugInfo() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultAtmospheres();

    UPROPERTY()
    APawn* TrackedPlayer;
};
