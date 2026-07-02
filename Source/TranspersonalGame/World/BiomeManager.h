// BiomeManager.h — Agent #05 Procedural World Generator — PROD_CYCLE_AUTO_20260702_002
// Manages 5 prehistoric biome zones: Forest, Plains, Rocky, Swamp, Volcanic
// Follows SharedTypes.h conventions — prefix World_ for all types

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ─── Biome Type Enum ─────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest    UMETA(DisplayName = "Forest"),
    Plains    UMETA(DisplayName = "Plains"),
    Rocky     UMETA(DisplayName = "Rocky"),
    Swamp     UMETA(DisplayName = "Swamp"),
    Volcanic  UMETA(DisplayName = "Volcanic"),
    Unknown   UMETA(DisplayName = "Unknown")
};

// ─── Biome Zone Data ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FLinearColor DebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float WaterPresence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float DangerLevel = 0.5f;

    FWorld_BiomeZone() {}
    FWorld_BiomeZone(EWorld_BiomeType InType, FVector InCenter, float InRadius)
        : BiomeType(InType), CenterLocation(InCenter), Radius(InRadius) {}
};

// ─── Biome Manager Actor ──────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, ClassGroup = "World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    // ── Biome Registry ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    bool bAutoInitializeBiomes = true;

    // ── Query API ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    bool IsLocationInWater(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const { return BiomeZones; }

    // ── Initialization ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void InitializeDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    // ── Debug ───────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Debug")
    void DrawBiomeDebugSpheres(float Duration = 5.0f);

private:
    // Internal lookup helper
    int32 FindClosestBiomeIndex(const FVector& WorldLocation) const;
};
