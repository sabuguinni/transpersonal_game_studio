// BiomeManager.h
// Transpersonal Game Studio — Engine Architect #02
// Biome classification system for the Cretaceous world

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    TropicalForest  UMETA(DisplayName = "Tropical Forest"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Riverbank       UMETA(DisplayName = "Riverbank"),
    Unknown         UMETA(DisplayName = "Unknown")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FEng_BiomeParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;

    /** 0.0 = barren, 1.0 = dense jungle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.5f;

    /** 0.0 = empty, 1.0 = teeming with life */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaunaDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureCelsius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float HumidityPercent = 50.0f;

    /** 0.0 = safe, 1.0 = extremely dangerous */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.5f;
};

USTRUCT(BlueprintType)
struct FEng_BiomeCell
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    int32 GridX = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    int32 GridY = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;
};

// ============================================================
// ABiomeManager — World actor that owns the biome grid
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Configuration ----

    /** Number of cells per axis (total cells = N²) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Grid")
    int32 BiomeGridResolution;

    /** World size in km (square world) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Grid")
    float WorldExtentKm;

    /** Show debug boxes in editor viewport */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugDrawBiomes;

    // ---- Runtime Data ----

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FEng_BiomeCell> BiomeGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TMap<EEng_BiomeType, FEng_BiomeParams> BiomeTable;

    // ---- Query API (callable from Blueprint and other C++ systems) ----

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeParams GetBiomeParams(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void GenerateBiomeGrid();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    EEng_BiomeType ClassifyBiomeAtPosition(const FVector& WorldPos) const;
    void DebugDrawBiomes();
};
