#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Core_TerrainPhysicsIntegrator.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"), 
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    RiverBed        UMETA(DisplayName = "River Bed"),
    RockyOutcrop    UMETA(DisplayName = "Rocky Outcrop")
};

USTRUCT(BlueprintType)
struct FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableComplexCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SurfaceStability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MovementSpeedMultiplier = 1.0f;

    FCore_TerrainPhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 2.5f;
        bEnableComplexCollision = true;
        SurfaceStability = 1.0f;
        MovementSpeedMultiplier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_BiomePhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ECore_TerrainType TerrainType = ECore_TerrainType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FCore_TerrainPhysicsProperties PhysicsProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    FCore_BiomePhysicsConfig()
    {
        TerrainType = ECore_TerrainType::Savanna;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 50000.0f;
    }
};

/**
 * Core Systems Programmer implementation for terrain physics integration
 * Manages physical properties of different terrain types and biomes
 * Integrates with UE5 landscape and physics systems for realistic ground interaction
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TArray<FCore_BiomePhysicsConfig> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableDynamicFriction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float PhysicsUpdateInterval = 0.1f;

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysicsForBiome(ECore_TerrainType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties GetTerrainPropertiesAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyPhysicsPropertiesToActor(AActor* TargetActor, const FCore_TerrainPhysicsProperties& Properties);

    // Landscape Integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ConfigureLandscapePhysics(ALandscape* LandscapeActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetupBiomePhysicsMaterials();

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics", CallInEditor)
    void ValidateTerrainPhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DebugDrawBiomeBoundaries();

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void OptimizeTerrainPhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    int32 GetActivePhysicsActorCount() const;

protected:
    // Internal state
    UPROPERTY()
    TMap<ECore_TerrainType, class UPhysicalMaterial*> BiomePhysicsMaterials;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;

    UPROPERTY()
    float LastPhysicsUpdateTime = 0.0f;

    // Internal methods
    void CreatePhysicsMaterialForBiome(ECore_TerrainType BiomeType, const FCore_TerrainPhysicsProperties& Properties);
    void UpdateActorPhysicsProperties(AActor* Actor);
    bool IsLocationInBiome(const FVector& Location, const FCore_BiomePhysicsConfig& BiomeConfig) const;
    void InitializeDefaultBiomeConfigurations();
};