#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeSystem.generated.h"

// Prehistoric biome types — all 6 biomes created in MinPlayableMap
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Prehistoric Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    VolcanicZone    UMETA(DisplayName = "Volcanic Zone"),
    Swampland       UMETA(DisplayName = "Swampland")
};

// Per-biome data struct
USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;      // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

// Biome query result
USTRUCT(BlueprintType)
struct FWorld_BiomeQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendWeight = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bIsValid = false;
};

/**
 * ABiomeSystem — manages all prehistoric biomes in the world.
 * Placed in MinPlayableMap. Provides biome queries for AI, weather, audio.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    // All registered biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeData> Biomes;

    // Query which biome a world location belongs to
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeQueryResult GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get biome data by type
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    bool GetBiomeData(EWorld_BiomeType BiomeType, FWorld_BiomeData& OutData) const;

    // Initialize default prehistoric biomes matching MinPlayableMap layout
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void InitializeBiomes();

    // Get ambient temperature at location (affects survival stats)
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Get humidity at location (affects thirst drain rate)
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // Get fog color for current biome (used by weather system)
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FLinearColor GetFogColorAtLocation(const FVector& WorldLocation) const;

    // Debug: draw biome boundaries in viewport
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes|Debug")
    void DrawBiomeBoundaries();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Find nearest biome to location, returns index or -1
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;
};
