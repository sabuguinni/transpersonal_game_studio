#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// ECore_BiomeType — 6 prehistoric biome types
// RULE: USTRUCT/UENUM at global scope only
// ============================================================
UENUM(BlueprintType)
enum class ECore_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Cretaceous Jungle"),
    Savanna     UMETA(DisplayName = "Open Savanna"),
    Forest      UMETA(DisplayName = "Conifer Forest"),
    Swamp       UMETA(DisplayName = "Primordial Swamp"),
    Mountain    UMETA(DisplayName = "Rocky Highlands"),
    Coastal     UMETA(DisplayName = "Coastal Shore"),
};

// ============================================================
// FCore_BiomeData — runtime data for a single biome
// ============================================================
USTRUCT(BlueprintType)
struct FCore_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    ECore_BiomeType BiomeType = ECore_BiomeType::Jungle;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FText DisplayName;

    /** Ambient temperature in Celsius */
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Environment")
    float BaseTemperature = 28.0f;

    /** 0.0 = arid, 1.0 = saturated */
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Environment")
    float Humidity = 0.5f;

    /** 0.0 = safe, 1.0 = extremely dangerous */
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Survival")
    float DangerRating = 0.5f;

    /** Foliage density multiplier for PCG */
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Foliage")
    float FoliageDensity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Dinosaurs")
    bool bAllowsLargeHerbivores = true;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Dinosaurs")
    bool bAllowsCarnivores = true;

    /** Primary fog tint for this biome */
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Atmosphere")
    FLinearColor PrimaryFogColor = FLinearColor(0.05f, 0.1f, 0.05f, 1.0f);
};

// ============================================================
// UBiomeManager — World Subsystem (auto-created per world)
// Manages biome state, transitions, and survival queries
// ============================================================
UCLASS(BlueprintType, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;

    // --------------------------------------------------------
    // Biome Query
    // --------------------------------------------------------

    /** Returns the current active biome */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    ECore_BiomeType GetCurrentBiome() const;

    /** Returns the biome type at a given world location (height-based) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    ECore_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns full data struct for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FCore_BiomeData GetBiomeData(ECore_BiomeType BiomeType) const;

    // --------------------------------------------------------
    // Biome Transition
    // --------------------------------------------------------

    /** Begin a smooth transition to a new biome over Duration seconds */
    UFUNCTION(BlueprintCallable, Category = "Biome|Transition")
    void TransitionToBiome(ECore_BiomeType NewBiome, float Duration = 5.0f);

    /** Immediately switch biome with no transition */
    UFUNCTION(BlueprintCallable, Category = "Biome|Transition")
    void SetBiomeImmediate(ECore_BiomeType NewBiome);

    // --------------------------------------------------------
    // Survival Queries
    // --------------------------------------------------------

    /** Temperature at location in Celsius (altitude-adjusted) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Humidity 0-1 at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    /** Danger level 0-1 at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetDangerLevelAtLocation(FVector WorldLocation) const;

    /** Returns true if danger < 0.5 */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    bool IsLocationSafeForPlayer(FVector WorldLocation) const;

    // --------------------------------------------------------
    // Dinosaur Spawning Rules
    // --------------------------------------------------------

    /** Returns true if a dinosaur of given type can spawn in this biome */
    UFUNCTION(BlueprintCallable, Category = "Biome|Dinosaurs")
    bool CanSpawnDinosaurInBiome(ECore_BiomeType BiomeType, bool bIsLargeHerbivore, bool bIsCarnivore) const;

    /** Returns valid spawn positions within a biome (line-traced to ground) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Dinosaurs")
    TArray<FVector> GetValidSpawnLocationsInBiome(ECore_BiomeType BiomeType, int32 Count, float SearchRadius = 5000.0f) const;

    // --------------------------------------------------------
    // Delegates
    // --------------------------------------------------------

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBiomeChanged, ECore_BiomeType, NewBiome);

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FOnBiomeChanged OnBiomeChanged;

    // --------------------------------------------------------
    // State (read-only from Blueprint)
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State", meta = (AllowPrivateAccess = "true"))
    ECore_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    ECore_BiomeType TargetBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    float TransitionBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    bool bBiomeTransitionActive;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Environment")
    float TemperatureBase;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Environment")
    float HumidityBase;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Survival")
    float DangerLevel;

private:
    void ApplyBiomeEnvironment(ECore_BiomeType BiomeType);

    float BiomeTransitionDuration;
    float BiomeTransitionElapsed;

    /** Cached biome data (populated on first query) */
    mutable TMap<ECore_BiomeType, FCore_BiomeData> BiomeDataMap;
};
