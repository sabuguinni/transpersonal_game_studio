// BiomeManager.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260623_007
// Biome system for the prehistoric survival game.
// Defines 6 biome types: Grassland, Forest, Desert, Swamp, Volcanic, Tundra.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// UENUM — Biome types (global scope, Eng_ prefix)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Grassland   UMETA(DisplayName = "Grassland Plains"),
    Forest      UMETA(DisplayName = "Dense Forest"),
    Desert      UMETA(DisplayName = "Arid Desert"),
    Swamp       UMETA(DisplayName = "Primordial Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Badlands"),
    Tundra      UMETA(DisplayName = "Frozen Tundra")
};

// ─────────────────────────────────────────────────────────────────────────────
// USTRUCT — Biome transition data (global scope)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType FromBiome = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType ToBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// USTRUCT — Biome runtime data (global scope)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FEng_BiomeRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D TemperatureRange = FVector2D(15.0f, 30.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D HumidityRange = FVector2D(0.3f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float PredatorSpawnWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HerbivoreSpawnWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegate — fired when a biome transition completes
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FEng_OnBiomeTransitionComplete,
    EEng_BiomeType, FromBiome,
    EEng_BiomeType, ToB iome
);

// ─────────────────────────────────────────────────────────────────────────────
// ABiomeManager — placed once in the level, manages all biome logic
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Lifecycle ─────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Core API (Blueprint-callable) ─────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TMap<EEng_BiomeType, float> GetBiomeBlendWeights(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void TriggerBiomeTransition(EEng_BiomeType FromBiome, EEng_BiomeType ToBiome, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeRuntimeData GetBiomeRuntimeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetCurrentDominantBiome() const { return CurrentDominantBiome; }

    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetActiveBiomeCount() const { return BiomeRuntimeData.Num(); }

    // ── Delegate ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnBiomeTransitionComplete OnBiomeTransitionComplete;

    // ── Config ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float TransitionBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugBiomeOverlay;

private:
    // Runtime state
    TMap<EEng_BiomeType, FEng_BiomeRuntimeData> BiomeRuntimeData;
    TArray<FEng_BiomeTransition> ActiveTransitions;
    EEng_BiomeType CurrentDominantBiome;
    bool bBiomesInitialized;
    FTimerHandle BiomeUpdateTimerHandle;

    // Internal methods
    void InitializeBiomes();
    void UpdateBiomeStates();

#if WITH_EDITOR
    void DrawDebugBiomeOverlay();
#endif
};
