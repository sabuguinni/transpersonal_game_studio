#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../SharedTypes.h"
#include "Core_TerrainPhysicsSystem.generated.h"

// Forward declarations
class ALandscape;
class UPhysicalMaterial;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float SlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Restitution = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bIsWaterlogged = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Density = 1.0f;

    FCore_TerrainPhysicsData()
    {
        SlopeAngle = 0.0f;
        Friction = 0.7f;
        Restitution = 0.1f;
        BiomeType = EEng_BiomeType::Forest;
        bIsWaterlogged = false;
        Density = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainMaterialMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Mapping")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Mapping")
    TSoftObjectPtr<UPhysicalMaterial> PhysicalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Mapping")
    float FrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Mapping")
    float RestitutionMultiplier = 1.0f;

    FCore_TerrainMaterialMapping()
    {
        BiomeType = EEng_BiomeType::Forest;
        FrictionMultiplier = 1.0f;
        RestitutionMultiplier = 1.0f;
    }
};

/**
 * Core Terrain Physics System
 * Manages realistic physics interactions between characters, objects, and terrain
 * Integrates with UE5 Landscape system and provides biome-specific physics materials
 * Handles slope-based movement, terrain deformation, and environmental physics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core terrain physics functionality
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsData GetTerrainPhysicsAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainPhysicsToActor(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateSlopeAngle(const FVector& Location, float SampleRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationOnSteepSlope(const FVector& Location, float MaxSlopeAngle = 45.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainMaterialMapping(EEng_BiomeType BiomeType, UPhysicalMaterial* PhysMaterial);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    UPhysicalMaterial* GetPhysicalMaterialForBiome(EEng_BiomeType BiomeType);

    // Terrain deformation and interaction
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainDeformation(const FVector& Location, float Radius, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanDeformTerrain(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateFootprint(const FVector& Location, float FootSize, float Depth);

    // Environmental physics
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyEnvironmentalForces(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainStability(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationUnderwater(const FVector& Location);

    // Integration with architectural framework
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithArchitecturalFramework();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugTerrainPhysics();

protected:
    // Core terrain physics properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_TerrainMaterialMapping> BiomeMaterialMappings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float DefaultFriction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float DefaultRestitution = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float MaxSlopeAngle = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float TerrainSampleRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    bool bEnableTerrainDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float DeformationIntensityMultiplier = 1.0f;

    // Cached references
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<ALandscape> CachedLandscape;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<UWorld> CachedWorld;

private:
    // Internal helper functions
    void InitializeTerrainPhysics();
    void CacheLandscapeReference();
    void SetupDefaultMaterialMappings();
    FVector GetTerrainNormal(const FVector& Location);
    float GetTerrainHeight(const FVector& Location);
    EEng_BiomeType DetermineBiomeAtLocation(const FVector& Location);
    void ApplyPhysicalMaterialToLocation(const FVector& Location, UPhysicalMaterial* Material);

    // Performance optimization
    float LastUpdateTime = 0.0f;
    float UpdateFrequency = 0.1f; // Update every 100ms
    TMap<FVector, FCore_TerrainPhysicsData> CachedPhysicsData;
    void ClearOldCacheEntries();
};