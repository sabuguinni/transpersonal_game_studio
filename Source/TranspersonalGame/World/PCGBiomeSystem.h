// PCGBiomeSystem.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_006
// Biome data system: defines biome zones, river paths, terrain variation
// Integrates with UE5 PCG framework for procedural world population

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "PCGBiomeSystem.generated.h"

// ── Biome type enum ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    RockyHighland   UMETA(DisplayName = "Rocky Highland"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    CoastalShore    UMETA(DisplayName = "Coastal Shore"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
};

// ── Biome zone definition ────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterPresence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeDebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");
};

// ── River segment definition ─────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint = FVector(0, 1000, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 100.0f;
};

// ── PCG Biome System Actor ───────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Biome zones ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // ── River network ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Rivers")
    TArray<FWorld_RiverSegment> RiverSegments;

    // ── PCG settings ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    float GlobalVegetationScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 RandomSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    bool bAutoGenerateOnBeginPlay = false;

    // ── Debug ─────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Debug")
    bool bDrawBiomeDebugSpheres = false;

    // ── Blueprint-callable functions ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeZone GetBiomeZoneAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetVegetationDensityAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Rivers")
    bool IsLocationNearRiver(FVector WorldLocation, float SearchRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    void RegisterDefaultBiomes();

    UFUNCTION(CallInEditor, Category = "World|PCG")
    void GenerateWorld();

    UFUNCTION(CallInEditor, Category = "World|PCG")
    void ClearGeneratedActors();

private:
    // ── Debug light component ─────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UPointLightComponent* DebugLight;

    // ── Internal helpers ──────────────────────────────────────────────────────
    float CalculateDistanceToBiomeCenter(FVector Location, const FWorld_BiomeZone& Zone) const;
    void InitializeDefaultRiverPath();
};
