// BiomeZoneManager.h
// Transpersonal Game Studio — Agent #5 Procedural World Generator
// Manages biome zones: Plains, Forest, Rocky, RiverDelta
// Each zone drives foliage density, ambient audio, weather probability

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeZoneManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Plains      UMETA(DisplayName = "Plains"),
    Forest      UMETA(DisplayName = "Forest"),
    Rocky       UMETA(DisplayName = "Rocky"),
    RiverDelta  UMETA(DisplayName = "River Delta"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Unknown     UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterLevel = -50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RainProbability = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
};

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeZoneManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeZoneManager();

    // All biome zones in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // Query which biome a world location falls into
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // Get the full biome data for a location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeZoneAtLocation(FVector WorldLocation, FWorld_BiomeZone& OutZone) const;

    // Get foliage density multiplier for a location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetFoliageDensityAtLocation(FVector WorldLocation) const;

    // Get rain probability for a location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetRainProbabilityAtLocation(FVector WorldLocation) const;

    // Initialize default biome zones for the MinPlayableMap
    UFUNCTION(CallInEditor, Category = "Biomes")
    void InitializeDefaultBiomes();

    // Returns number of registered biome zones
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    int32 GetBiomeCount() const;

protected:
    virtual void BeginPlay() override;
};
