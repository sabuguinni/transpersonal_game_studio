// BiomeManager.h
// Agent #5 — Procedural World Generator
// Manages biome zones: Forest, Savanna, Rocky, River, Lake
// Each zone has environmental multipliers for survival stats

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Rocky       UMETA(DisplayName = "Rocky"),
    River       UMETA(DisplayName = "River"),
    Lake        UMETA(DisplayName = "Lake"),
    Unknown     UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Centre = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    // Survival stat multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float ThirstDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float HungerDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float TemperatureOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float FearMultiplier = 1.0f;

    // Visual identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogTint = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.03f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // Returns the biome type at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Returns the full biome zone data at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    // Returns thirst drain multiplier for a location (used by SurvivalComponent)
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetThirstMultiplierAt(const FVector& WorldLocation) const;

    // Returns hunger drain multiplier for a location
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetHungerMultiplierAt(const FVector& WorldLocation) const;

    // Returns fear multiplier for a location (proximity to predator zones)
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetFearMultiplierAt(const FVector& WorldLocation) const;

    // Registered biome zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    // Default zone returned when no biome matches
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeZone DefaultZone;

protected:
    virtual void BeginPlay() override;

private:
    // Initialise default biome zones matching MinPlayableMap layout
    void InitialiseDefaultZones();
};
