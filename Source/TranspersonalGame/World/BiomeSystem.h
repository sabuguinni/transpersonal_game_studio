#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ── Biome Zone Descriptor ────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterXY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeTint = FLinearColor(0.2f, 0.6f, 0.1f, 1.0f);
};

// ── Biome Type Enum ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highland"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp")
};

// ── Biome System Actor ───────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome System Manager"))
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    // ── Registered Biome Zones ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zones")
    TArray<FWorld_BiomeZone> BiomeZones;

    // ── Query: get biome type at world location ──────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // ── Query: get biome zone data at location ───────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    bool GetBiomeZoneAtLocation(FVector WorldLocation, FWorld_BiomeZone& OutZone) const;

    // ── Query: vegetation density multiplier at location ─────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetVegetationDensityAt(FVector WorldLocation) const;

    // ── Query: is location near water ────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    bool IsNearWater(FVector WorldLocation, float SearchRadius = 500.0f) const;

    // ── Setup: register default Cretaceous biomes ────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void RegisterDefaultBiomes();

    // ── Debug: draw biome zones in viewport ──────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeDebug(float Duration = 5.0f) const;

protected:
    virtual void BeginPlay() override;

private:
    // Cached biome lookup for performance
    UPROPERTY()
    TMap<int32, EWorld_BiomeType> BiomeTypeCache;
};
