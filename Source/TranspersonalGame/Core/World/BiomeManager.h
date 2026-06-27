// BiomeManager.h — Biome System for Prehistoric Dinosaur Survival Game
// Agent #05 — Procedural World Generator
// Manages biome zones, transitions, environmental conditions per biome

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ── Biome Type Enum ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    VolcanicHighlands UMETA(DisplayName = "Volcanic Highlands"),
    CoastalSwamp    UMETA(DisplayName = "Coastal Swamp"),
    RockyBadlands   UMETA(DisplayName = "Rocky Badlands"),
};

// ── Biome Environmental Data ───────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    // Temperature range in Celsius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float MinTemperature = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float MaxTemperature = 35.0f;

    // Humidity 0-1 (affects thirst drain rate)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.5f;

    // Danger level 0-1 (affects dinosaur aggression radius)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float DangerLevel = 0.3f;

    // Resource density 0-1 (food, water, crafting materials)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float ResourceDensity = 0.5f;

    // Fog density override for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensityMultiplier = 1.0f;

    // Ambient sound tag for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FName AmbientSoundTag = NAME_None;

    // World bounds of this biome zone (XY center + radius)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    float ZoneRadius = 2000.0f;
};

// ── BiomeManager Actor ─────────────────────────────────────────────────────
UCLASS(ClassGroup = "TranspersonalGame|World", meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Biome Registry ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> RegisteredBiomes;

    // ── Query API ──────────────────────────────────────────────────────────

    /** Returns the biome type at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FWorld_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /** Returns temperature at location (interpolated at biome boundaries) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns danger level at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /** Returns resource density at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetResourceDensityAtLocation(const FVector& WorldLocation) const;

    // ── Initialization ─────────────────────────────────────────────────────

    /** Initializes all default biomes for the MinPlayableMap */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void InitializeDefaultBiomes();

    /** Returns number of registered biomes */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    int32 GetBiomeCount() const { return RegisteredBiomes.Num(); }

private:
    /** Find the closest biome to a world location */
    int32 FindClosestBiomeIndex(const FVector& WorldLocation) const;

    /** Interpolation blend radius between biomes */
    UPROPERTY(EditAnywhere, Category = "Biomes|Transition", meta = (AllowPrivateAccess = "true"))
    float BiomeBlendRadius = 500.0f;

    /** Whether biomes have been initialized */
    UPROPERTY(VisibleAnywhere, Category = "Biomes|State", meta = (AllowPrivateAccess = "true"))
    bool bBiomesInitialized = false;
};
