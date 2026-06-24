// PCGBiomeSystem.h
// Agent #05 — Procedural World Generator
// Biome classification and environmental data system for Cretaceous world

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "PCGBiomeSystem.generated.h"

// ─── Biome Types ────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Savanna     UMETA(DisplayName = "Open Savanna"),
    Swamp       UMETA(DisplayName = "Coastal Swamp"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Volcanic    UMETA(DisplayName = "Volcanic Badlands"),
    River       UMETA(DisplayName = "River Delta"),
    Plains      UMETA(DisplayName = "Open Plains"),
    COUNT       UMETA(Hidden)
};

// ─── Biome Environmental Data ────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FWorld_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");

    // Temperature range (Celsius)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxTemperature = 35.0f;

    // Humidity 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 0.6f;

    // Vegetation density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float VegetationDensity = 0.7f;

    // Fog density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.2f;

    // Fog color tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);

    // Ambient light color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AmbientColor = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);

    // Danger level 0-1 (affects dinosaur aggression)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float DangerLevel = 0.5f;

    // Resource richness 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float ResourceRichness = 0.5f;

    // World-space center of this biome zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FVector ZoneCenter = FVector::ZeroVector;

    // Radius of biome influence (Unreal units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float ZoneRadius = 5000.0f;
};

// ─── Biome Query Result ──────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FWorld_BiomeQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PrimaryBiome = EWorld_BiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType SecondaryBiome = EWorld_BiomeType::Plains;

    // Blend factor 0=primary only, 1=secondary only
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendFactor = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Temperature = 28.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float DangerLevel = 0.3f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bIsValid = false;
};

// ─── PCGBiomeSystem Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "World")
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // ── Biome Zones ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TArray<FWorld_BiomeData> BiomeZones;

    // ── Query API ────────────────────────────────────────────────────────────

    /** Query the biome at a world-space location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeQueryResult QueryBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get the dominant biome type at a location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    /** Get temperature at location (blended between biomes) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Get danger level at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /** Get vegetation density at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // ── Initialization ───────────────────────────────────────────────────────

    /** Initialize default Cretaceous biome zones */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void InitializeDefaultBiomes();

    /** Find nearest biome zone to a location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;

protected:
    virtual void BeginPlay() override;

private:
    /** Compute blend weight for a biome zone at a given distance */
    float ComputeBlendWeight(float Distance, float ZoneRadius) const;

    /** Find the two closest biome zones for blending */
    void FindTwoClosestBiomes(const FVector& Location, int32& OutPrimary, int32& OutSecondary, float& OutBlend) const;
};
