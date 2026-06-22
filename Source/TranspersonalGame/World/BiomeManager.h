// BiomeManager.h
// Engine Architect #02 — Transpersonal Game Studio
// Biome system: classification, transitions, dinosaur spawn rules

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Core/SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome Definition Struct (Eng_ prefix — unique across project)
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    FEng_BiomeDefinition()
        : BiomeType(EBiomeType::Forest)
        , BiomeName(TEXT("Unknown Biome"))
        , CenterLocation(FVector2D::ZeroVector)
        , Radius(1000.0f)
        , BaseTemperature(20.0f)
        , Humidity(60.0f)
    {}
};

// ============================================================
// Biome Transition Delegate
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FEng_OnPlayerBiomeChanged,
    EBiomeType, FromBiome,
    EBiomeType, ToBiome
);

// ============================================================
// ABiomeManager — World Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Biome Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FEng_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    EBiomeType CurrentPlayerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    int32 MaxActiveBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDrawBiomes;

    // ---- Events ----

    UPROPERTY(BlueprintAssignable, Category = "Biomes|Events")
    FEng_OnPlayerBiomeChanged OnPlayerBiomeChanged;

    // ---- Classification ----

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType ClassifyLocationBiome(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetBiomeBlendWeight(const FVector& WorldLocation, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    const FEng_BiomeDefinition* GetBiomeDefinition(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<EBiomeType> GetBiomesAtLocation(const FVector& WorldLocation, float SearchRadius = 500.0f) const;

    // ---- Dinosaur Spawning Rules ----

    UFUNCTION(BlueprintCallable, Category = "Biomes|Dinosaurs")
    TArray<EDinosaurSpecies> GetValidDinosaursForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes|Dinosaurs")
    bool CanSpawnDinosaurInBiome(EDinosaurSpecies Species, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes|Dinosaurs")
    float GetDinosaurAggressionModifier(EDinosaurSpecies Species, EBiomeType BiomeType) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void OnBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome);
    void InitializeDefaultBiomes();
    void DebugDrawBiomeBoundaries() const;
};
