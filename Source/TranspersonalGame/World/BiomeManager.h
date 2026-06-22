// BiomeManager.h
// Engine Architect #02 — Transpersonal Game Studio
// Biome system header for prehistoric survival world
// P1 Priority: World Generation

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

// ── Biome Types (Cretaceous period biomes) ──
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    CretaceousForest    UMETA(DisplayName = "Cretaceous Forest"),
    VolcanicBadlands    UMETA(DisplayName = "Volcanic Badlands"),
    CoastalWetlands     UMETA(DisplayName = "Coastal Wetlands"),
    OpenSavanna         UMETA(DisplayName = "Open Savanna"),
    MountainHighlands   UMETA(DisplayName = "Mountain Highlands"),
    COUNT               UMETA(Hidden)
};

// ── Biome Definition Data ──
USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::CretaceousForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    // Climate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate", meta = (ClampMin = "-20.0", ClampMax = "80.0"))
    float BaseTemperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseHumidity = 0.5f;

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "0.2"))
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor AmbientColorTint = FLinearColor::White;

    // Gameplay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.5f;

    // Dinosaur species that spawn in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna")
    TArray<FName> DinosaurSpecies;

    // Resource types available
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TArray<FName> AvailableResources;
};

// ── Biome Zone (runtime placement) ──
USTRUCT(BlueprintType)
struct FEng_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FName ZoneID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EEng_BiomeType BiomeType = EEng_BiomeType::CretaceousForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone", meta = (ClampMin = "100.0"))
    float ZoneRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float BlendRadius = 1000.0f;
};

// ── BiomeManager — World Subsystem ──
UCLASS(BlueprintType, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Query API ──

    /** Get the dominant biome type at a world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get full biome definition for a given type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType BiomeType) const;

    /** Get temperature in Celsius at a world location (accounts for altitude) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Get humidity [0..1] at a world location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Get blend weight [0..1] for a specific biome at a location (for transitions) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeBlendWeight(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    /** Get dinosaur species list for a biome */
    UFUNCTION(BlueprintCallable, Category = "Biome|Fauna")
    TArray<FName> GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const;

    // ── Zone Management ──

    /** Register a biome zone (used by PCGWorldGenerator) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Zones")
    void RegisterBiomeZone(const FEng_BiomeZone& Zone);

    /** Unregister a biome zone by ID */
    UFUNCTION(BlueprintCallable, Category = "Biome|Zones")
    void UnregisterBiomeZone(const FName& ZoneID);

    /** Get all active biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome|Zones")
    const TArray<FEng_BiomeZone>& GetActiveBiomeZones() const { return ActiveBiomeZones; }

    // ── State ──

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bBiomeSystemInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeTransitionBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float GlobalTemperatureMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float GlobalHumidityMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType CurrentBiomeType;

private:
    void InitializeBiomeData();

    // All biome definitions keyed by type
    TMap<EEng_BiomeType, FEng_BiomeDefinition> BiomeDefinitions;

    // Runtime active zones (placed by PCGWorldGenerator)
    TArray<FEng_BiomeZone> ActiveBiomeZones;
};
