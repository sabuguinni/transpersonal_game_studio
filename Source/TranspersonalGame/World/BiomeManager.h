#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome type enumeration — prehistoric world zones
// ============================================================
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    None        UMETA(DisplayName = "None"),
    Jungle      UMETA(DisplayName = "Jungle"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    River       UMETA(DisplayName = "River Delta"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Cave        UMETA(DisplayName = "Cave System"),
    Coastal     UMETA(DisplayName = "Coastal"),
};

// ============================================================
// Per-biome configuration data
// ============================================================
USTRUCT(BlueprintType)
struct FEng_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString DisplayName = TEXT("Unknown Biome");

    /** World-space center of this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    /** Radius in cm for biome influence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 200000.0f;

    /** Ambient temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float AmbientTemperature = 28.0f;

    /** Water availability 0-1 (affects thirst drain rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float WaterAvailability = 0.5f;

    /** Food availability 0-1 (affects hunger drain rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float FoodAvailability = 0.5f;

    /** Danger level 0-1 (affects dinosaur aggression) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float DangerLevel = 0.3f;

    /** Fog density override for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    /** Sky color tint for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor SkyTint = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
};

// ============================================================
// BiomeManager Actor — manages biome zones in the world
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --------------------------------------------------------
    // Biome Query API
    // --------------------------------------------------------

    /** Returns the dominant biome type at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns the full config for the biome at a given location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeConfig GetBiomeConfigAtLocation(const FVector& WorldLocation) const;

    /** Returns temperature at location (blends between biomes) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns water availability 0-1 at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetWaterAvailabilityAtLocation(const FVector& WorldLocation) const;

    /** Returns danger level 0-1 at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Danger")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /** Returns all biome configs */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FEng_BiomeConfig> GetAllBiomes() const { return BiomeConfigs; }

    /** Register a new biome zone at runtime */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiome(const FEng_BiomeConfig& Config);

    /** Initialize default prehistoric biomes */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void InitializeDefaultBiomes();

    // --------------------------------------------------------
    // Properties
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FEng_BiomeConfig> BiomeConfigs;

    /** How often (seconds) to update player biome state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float UpdateInterval = 2.0f;

    /** Enable biome visual transitions (fog/sky changes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    bool bEnableVisualTransitions = true;

    /** Transition blend speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float TransitionSpeed = 1.0f;

private:
    float TimeSinceLastUpdate = 0.0f;
    EEng_BiomeType CurrentPlayerBiome = EEng_BiomeType::None;

    /** Find the nearest biome config to a world location */
    const FEng_BiomeConfig* FindNearestBiome(const FVector& WorldLocation) const;

    /** Apply visual settings for current biome */
    void ApplyBiomeVisuals(const FEng_BiomeConfig& Config);
};
