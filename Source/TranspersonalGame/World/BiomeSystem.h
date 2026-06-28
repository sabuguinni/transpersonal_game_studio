#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ============================================================
// BIOME SYSTEM — Agent #05 Procedural World Generator
// Defines 5 prehistoric biomes with distinct properties
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    None        UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    // Temperature in Celsius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float TemperatureMin = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float TemperatureMax = 35.0f;

    // Humidity 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float Humidity = 0.5f;

    // Danger level 0-1 (affects predator spawn rate)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float DangerLevel = 0.5f;

    // Resource density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float ResourceDensity = 0.5f;

    // Fog density for atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    // Ambient light color tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // World space center of this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    FVector WorldCenter = FVector::ZeroVector;

    // Approximate radius of this biome in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    float BiomeRadius = 300000.0f;
};

USTRUCT(BlueprintType)
struct FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString WaterName = TEXT("River");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale = FVector(10.0f, 50.0f, 1.0f);

    // Is this a river (true) or lake/pool (false)?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsRiver = true;

    // Flow direction for rivers (normalized)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector FlowDirection = FVector(0.0f, 1.0f, 0.0f);

    // Flow speed cm/s
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed = 100.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeComponent();

    // Current biome this actor is in
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::None;

    // Blend weight for biome transitions (0=fully in biome, 1=at edge)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome")
    float BiomeBlendWeight = 0.0f;

    // Called when actor enters a new biome
    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnBiomeEntered(EWorld_BiomeType NewBiome, EWorld_BiomeType OldBiome);

    // Get the current biome data
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeData GetCurrentBiomeData() const;

    // Update biome based on world position
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeFromPosition(FVector WorldPosition);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FWorld_BiomeData CachedBiomeData;
    float BiomeUpdateInterval = 2.0f;
    float BiomeUpdateTimer = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

    // All registered biomes in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> RegisteredBiomes;

    // All water bodies in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FWorld_WaterBody> WaterBodies;

    // Get biome at a given world position
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // Get full biome data at a given world position
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeData GetBiomeDataAtLocation(FVector WorldLocation) const;

    // Get nearest water body to a location
    UFUNCTION(BlueprintCallable, Category = "Water")
    bool GetNearestWaterBody(FVector WorldLocation, float SearchRadius, FWorld_WaterBody& OutWaterBody) const;

    // Initialize default prehistoric biomes
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes")
    void InitializeDefaultBiomes();

    // Get danger multiplier at location (for AI spawn rates)
    UFUNCTION(BlueprintCallable, Category = "Biomes|Gameplay")
    float GetDangerMultiplierAtLocation(FVector WorldLocation) const;

    // Get resource density at location (for crafting/gathering)
    UFUNCTION(BlueprintCallable, Category = "Biomes|Gameplay")
    float GetResourceDensityAtLocation(FVector WorldLocation) const;

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    // Blend between two biomes based on distance
    float CalculateBiomeBlend(const FWorld_BiomeData& Biome, FVector Location) const;
};
