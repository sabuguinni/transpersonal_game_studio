#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core/SharedTypes.h"
#include "Core_TerrainPhysicsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float SlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Roughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Hardness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    EBiomeType TerrainBiome = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bIsWalkable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bCanBuildOn = true;

    FCore_TerrainPhysicsData()
    {
        SlopeAngle = 0.0f;
        Roughness = 0.5f;
        Friction = 0.7f;
        Hardness = 1.0f;
        TerrainBiome = EBiomeType::Grassland;
        bIsWalkable = true;
        bCanBuildOn = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMaterialSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float StaticFriction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float DynamicFriction = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float SleepThresholdMultiplier = 1.0f;

    FCore_PhysicsMaterialSettings()
    {
        StaticFriction = 0.7f;
        DynamicFriction = 0.6f;
        Restitution = 0.3f;
        Density = 1.0f;
        SleepThresholdMultiplier = 1.0f;
    }
};

/**
 * Core Terrain Physics Manager - Handles terrain-based physics interactions
 * Manages surface properties, slope calculations, and physics material assignment
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core terrain physics methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsData GetTerrainDataAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateSlopeAngle(const FVector& Location, float SampleRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationWalkable(const FVector& Location, float MaxSlopeAngle = 45.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanBuildAtLocation(const FVector& Location, float MaxSlopeAngle = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdatePhysicsMaterialAtLocation(const FVector& Location, const FCore_PhysicsMaterialSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FVector GetSurfaceNormal(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainHardness(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainEffectsToActor(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterTerrainPhysicsData(const FVector& Location, const FCore_TerrainPhysicsData& Data);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ClearTerrainPhysicsData();

    // Physics material management
    UFUNCTION(BlueprintCallable, Category = "Physics Material")
    void CreatePhysicsMaterialForBiome(EBiomeType BiomeType, const FCore_PhysicsMaterialSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Physics Material")
    class UPhysicalMaterial* GetPhysicsMaterialForBiome(EBiomeType BiomeType);

    // Terrain analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    TArray<FVector> GetNearbyWalkablePositions(const FVector& CenterLocation, float Radius = 500.0f, int32 NumSamples = 16);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    bool IsLocationUnderwater(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetWaterDepthAtLocation(const FVector& Location);

protected:
    // Terrain physics data storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Physics")
    TMap<FVector, FCore_TerrainPhysicsData> TerrainDataMap;

    // Physics materials for different biomes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Materials")
    TMap<EBiomeType, class UPhysicalMaterial*> BiomePhysicsMaterials;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TerrainSampleRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxWalkableSlope = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxBuildableSlope = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableTerrainPhysicsDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PhysicsUpdateInterval = 0.1f;

    // Internal state
    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    class ALandscape* CachedLandscape = nullptr;

    // Helper methods
    void InitializeDefaultPhysicsMaterials();
    void CacheLandscapeReference();
    FVector PerformLineTrace(const FVector& Start, const FVector& End);
    void DebugDrawTerrainData(const FVector& Location, const FCore_TerrainPhysicsData& Data);
};