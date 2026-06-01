#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Core_TerrainPhysics.generated.h"

/**
 * Core Terrain Physics System
 * Handles terrain-specific physics: ground detection, slope calculations, surface materials
 * Manages interaction between characters/objects and terrain surfaces
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UObject
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

    // Terrain Detection
    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics")
    bool GetGroundInfo(FVector Location, FHitResult& GroundHit, float TraceDistance = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics")
    float GetSlopeAngle(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics")
    FVector GetSurfaceNormal(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics")
    bool IsLocationOnSteepSlope(FVector Location, float MaxSlopeAngle = 45.0f);

    // Surface Material Detection
    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Materials")
    class UPhysicalMaterial* GetSurfaceMaterial(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Materials")
    float GetSurfaceFriction(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Materials")
    bool IsSurfaceWalkable(FVector Location);

    // Terrain Modification
    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Modification")
    void CreateCrater(FVector Location, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Modification")
    void FlattenTerrain(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Modification")
    void RaiseTerrain(FVector Location, float Radius, float Height);

    // Character Terrain Interaction
    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Character")
    void ApplyTerrainEffectsToCharacter(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Character")
    float GetMovementSpeedModifier(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Character")
    bool ShouldCharacterSlide(class ACharacter* Character);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Environment")
    void SimulateErosion(FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Environment")
    void ApplyWeatherEffects(FVector Location, float WetnessFactor);

    UFUNCTION(BlueprintCallable, Category = "Core Terrain Physics|Environment")
    void UpdateTerrainStability();

protected:
    // Terrain Detection Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    float GroundTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    float SlopeDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    TArray<TEnumAsByte<EObjectTypeQuery>> TerrainObjectTypes;

    // Surface Material Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials")
    TMap<class UPhysicalMaterial*, float> MaterialFrictionMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials")
    TMap<class UPhysicalMaterial*, float> MaterialSpeedModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials")
    float DefaultFriction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Materials")
    float DefaultSpeedModifier;

    // Slope Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float MaxWalkableSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float SlideThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float SlideForceMultiplier;

    // Terrain Modification Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Modification")
    bool bAllowTerrainModification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Modification")
    float MaxModificationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Modification")
    float MaxModificationDepth;

    // Environmental Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    bool bEnableErosion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float ErosionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    bool bEnableWeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float WeatherEffectStrength;

private:
    // Cached terrain references
    TWeakObjectPtr<class ALandscape> CachedLandscape;
    TArray<TWeakObjectPtr<AActor>> TerrainActors;

    // Helper functions
    bool FindLandscapeInWorld();
    void CacheTerrainActors();
    FVector CalculateSlideDirection(FVector Location, FVector SurfaceNormal);
    float CalculateDistanceToTerrain(FVector Location);
    bool IsLocationInWater(FVector Location);
    void UpdateMaterialCache();
};