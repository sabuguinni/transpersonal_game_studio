// BiomeManager.h
// Agent #5 — Procedural World Generator
// Biome zone management, PCG terrain variation, river system

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — Global scope (RULE 1: USTRUCT/UENUM at global scope)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    CoastalSwamp    UMETA(DisplayName = "Coastal Swamp"),
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Ash         UMETA(DisplayName = "Volcanic Ash"),
};

// ============================================================
// STRUCTS — Global scope
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RadiusMeters = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseTemperatureCelsius = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityPercent = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor GroundColor = FLinearColor(0.1f, 0.08f, 0.05f, 1.f);
};

USTRUCT(BlueprintType)
struct FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint = FVector(0.f, 1000.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float WidthMeters = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float DepthMeters = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    bool bIsCrossable = true;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(ClassGroup = "WorldGen", meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Biome Queries ----

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetDinosaurSpawnWeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    const FWorld_BiomeZone* GetBiomeZoneByType(EWorld_BiomeType BiomeType) const;

    // ---- River Queries ----

    UFUNCTION(BlueprintCallable, Category = "World|River")
    bool IsLocationInRiver(const FVector& WorldLocation, float Tolerance = 50.f) const;

    UFUNCTION(BlueprintCallable, Category = "World|River")
    TArray<FWorld_RiverSegment> GetRiverSegmentsNearLocation(const FVector& WorldLocation, float SearchRadius = 5000.f) const;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    int32 ActiveBiomeCount;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    bool bBiomesInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Climate")
    float GlobalWetnessScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Climate")
    float GlobalTemperatureScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|River")
    float RiverFlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Climate")
    bool bEnableWeatherTransitions;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(BlueprintReadOnly, Category = "World|River")
    TArray<FWorld_RiverSegment> RiverSegments;

private:
    void InitializeBiomeZones();
    void InitializeRiverSystem();
};
