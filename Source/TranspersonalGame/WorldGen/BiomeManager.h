// BiomeManager.h
// Agent #5 — Procedural World Generator
// Manages 5 biome zones: DenseForest, OpenPlains, RockyHighlands, RiverValley, VolcanicBadlands

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    OpenPlains       UMETA(DisplayName = "Open Plains"),
    RockyHighlands   UMETA(DisplayName = "Rocky Highlands"),
    RiverValley      UMETA(DisplayName = "River Valley"),
    VolcanicBadlands UMETA(DisplayName = "Volcanic Badlands"),
    Unknown          UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;
};

UCLASS(ClassGroup = (WorldGen), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Initialize all 5 biome zones
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomes();

    // Get the biome type at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get full biome zone data at location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool GetBiomeZoneAtLocation(const FVector& WorldLocation, FWorld_BiomeZone& OutZone) const;

    // Get all registered biome zones
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const;

    // Get vegetation density at location (0.0 - 1.0)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Get temperature at location (Celsius)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Get humidity at location (0.0 - 1.0)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // Debug: draw biome boundaries
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeBoundaries();

    // Number of registered biomes
    UFUNCTION(BlueprintPure, Category = "Biome")
    int32 GetBiomeCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bAutoInitializeOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawDebugBoundaries = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    float DebugDrawDuration = 5.0f;

private:
    void SetupDefaultBiomeZones();
    float CalculateBlendWeight(const FVector& Location, const FWorld_BiomeZone& Zone) const;
};
