#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "SharedTypes.h"
#include "World_CretaceousLandscapeExpansion.generated.h"

/**
 * Cretaceous Landscape Expansion Manager
 * Creates and manages the 10km x 10km Cretaceous landscape with 5 distinct biomes
 * Handles biome placement, water bodies, and terrain features
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousLandscapeExpansion : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousLandscapeExpansion();

protected:
    virtual void BeginPlay() override;

    // Landscape management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    class ALandscape* MainLandscape;

    // Biome markers for the 5 Cretaceous biomes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<class APointLight*> BiomeMarkers;

    // Water body actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
    TArray<class AStaticMeshActor*> WaterBodies;

    // Landscape dimensions (in UE5 units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float LandscapeSize = 1000000.0f; // 10km in cm

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FWorld_BiomeZoneData> BiomeZones;

public:
    // Core landscape creation
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    bool CreateMainLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    bool ExpandLandscapeToTarget();

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateBiomeMarkers();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void SetupBiomeZones();

    // Water system
    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateRiverSystem();

    // Terrain features
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void AddTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void CreateVolcanicFeatures();

    // Validation and status
    UFUNCTION(BlueprintCallable, Category = "Status")
    bool ValidateLandscapeSize();

    UFUNCTION(BlueprintCallable, Category = "Status")
    FVector GetLandscapeCenter() const;

    UFUNCTION(BlueprintCallable, Category = "Status")
    float GetCurrentLandscapeSize() const;

    // Asset purchase criteria validation
    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    bool MeetsAssetPurchaseCriteria();

private:
    // Internal landscape creation helpers
    void SetupLandscapeProperties();
    void ConfigureBiomePositions();
    void CreateBiomeTransitions();

    // Biome-specific creation
    void CreateSwampBiome(const FVector& Location);
    void CreateForestBiome(const FVector& Location);
    void CreateSavannaBiome(const FVector& Location);
    void CreateDesertBiome(const FVector& Location);
    void CreateMountainBiome(const FVector& Location);

    // Water creation helpers
    class AStaticMeshActor* CreateWaterPlane(const FVector& Location, const FVector& Scale, const FString& Name);
    void SetupWaterMaterial(class AStaticMeshActor* WaterActor);

    // Status tracking
    bool bLandscapeCreated = false;
    bool bBiomesSetup = false;
    bool bWaterBodiesCreated = false;
};