#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ============================================================
// Biome System — Agent #5 Procedural World Generator
// Cycle: PROD_CYCLE_AUTO_20260701_005
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Tropical Jungle"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp Wetlands"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    River       UMETA(DisplayName = "River Delta"),
    Count       UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureCelsius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityPercent = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

USTRUCT(BlueprintType)
struct FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString BodyName = TEXT("River");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale = FVector(10.0f, 60.0f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsRiver = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed = 100.0f;
};

UCLASS(ClassGroup = (WorldGen), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeTransitionBlend = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInWater(const FVector& WorldLocation) const;
};

UCLASS(ClassGroup = (WorldGen), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Biome Data ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FWorld_WaterBody> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float TerrainHeightScale = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowBiomeDebug = false;

    // ── Query API ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    FWorld_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    bool IsInWater(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // ── World Generation ─────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void InitializeDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void GenerateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RegisterBiome(const FWorld_BiomeData& BiomeData);

    // ── Survival Integration ─────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFoodAvailabilityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetWaterAvailabilityAtLocation(const FVector& WorldLocation) const;

private:
    int32 FindNearestBiomeIndex(const FVector& Location) const;
    float ComputeBlendWeight(const FVector& Location, const FWorld_BiomeData& Biome) const;
};
