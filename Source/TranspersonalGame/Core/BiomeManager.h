// BiomeManager.h
// Engine Architect #02 — PROD_CYCLE_AUTO_20260620_001
// P1 Priority: World Generation — Biome system header
// RULE: Every USTRUCT/UENUM at global scope. Unique Eng_ prefix. .generated.h last.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BiomeManager.generated.h"

// ============================================================
// Enums — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Grassland   UMETA(DisplayName = "Grassland Plains"),
    Forest      UMETA(DisplayName = "Dense Forest"),
    Desert      UMETA(DisplayName = "Arid Desert"),
    Jungle      UMETA(DisplayName = "Tropical Jungle"),
    Swamp       UMETA(DisplayName = "Prehistoric Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Badlands"),
    Coastal     UMETA(DisplayName = "Coastal Shore"),
    Alpine      UMETA(DisplayName = "Alpine Highland"),
};

// ============================================================
// Structs — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    /** Ambient temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseTemperature = 22.0f;

    /** 0.0 = arid, 1.0 = saturated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseHumidity = 0.5f;

    /** Multiplier for dinosaur spawn density in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float DinosaurDensity = 1.0f;

    /** Multiplier for vegetation spawn density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float VegetationDensity = 1.0f;

    /** 1=safe, 5=extremely dangerous */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "1", ClampMax = "5"))
    int32 DangerLevel = 1;

    /** Species that naturally inhabit this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    TArray<FName> NativeDinosaurSpecies;
};

// ============================================================
// Delegate
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEng_OnBiomeChanged,
    EEng_BiomeType, OldBiome,
    EEng_BiomeType, NewBiome);

// ============================================================
// BiomeManager — GameInstance subsystem
// ============================================================

UCLASS(BlueprintType, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API UBiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // UGameInstanceSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --------------------------------------------------------
    // Biome queries
    // --------------------------------------------------------

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full data struct for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    /** Returns temperature (°C) at a world location, accounting for altitude */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns hazard level 0.0-1.0 at a world location */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHazardLevelAtLocation(const FVector& WorldLocation) const;

    /** Returns list of dinosaur species native to the given biome */
    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    TArray<FName> GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const;

    // --------------------------------------------------------
    // Biome transition
    // --------------------------------------------------------

    /** Transition to a new biome (called by world generator as player moves) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetCurrentBiome(EEng_BiomeType NewBiome, float TransitionDuration = 3.0f);

    /** Tick the blend alpha for smooth biome transitions */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeTransition(float DeltaTime, float TransitionDuration = 3.0f);

    /** Draw debug spheres showing biome regions in editor */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DrawBiomeDebug(UObject* WorldContext, const FVector& Center, float Radius = 5000.0f);

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType PreviousBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float TransitionBlendAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bBiomeTransitionActive;

    /** Global world temperature offset (set by weather system) */
    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    float WorldTemperature;

    /** Global world humidity (0-1) */
    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    float WorldHumidity;

    /** Player altitude (set by character each tick) */
    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    float WorldAltitude;

    // --------------------------------------------------------
    // Events
    // --------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnBiomeChanged OnBiomeChanged;

private:
    /** Internal biome data registry — populated in Initialize() */
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataTable;

    void InitializeDefaultBiomeData();
    FColor GetBiomeDebugColor(EEng_BiomeType BiomeType) const;
};
