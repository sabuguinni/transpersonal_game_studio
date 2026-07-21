#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "Core_AdvancedPhysicsIntegrator.generated.h"

// Forward declarations
class UPhysicalMaterial;
class ALandscape;
class ATriggerVolume;

UENUM(BlueprintType)
enum class ECore_BiomePhysicsType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp Physics"),
    Forest      UMETA(DisplayName = "Forest Physics"),
    Savanna     UMETA(DisplayName = "Savanna Physics"),
    Desert      UMETA(DisplayName = "Desert Physics"),
    Mountain    UMETA(DisplayName = "Mountain Physics")
};

USTRUCT(BlueprintType)
struct FCore_BiomePhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    FCore_BiomePhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        bEnableGravity = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_TerrainPhysicsZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    ECore_BiomePhysicsType BiomeType = ECore_BiomePhysicsType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneExtent = FVector(5000.0f, 5000.0f, 1000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FCore_BiomePhysicsProperties PhysicsProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TSoftObjectPtr<UPhysicalMaterial> PhysicalMaterial;

    FCore_TerrainPhysicsZone()
    {
        BiomeType = ECore_BiomePhysicsType::Savanna;
        ZoneCenter = FVector::ZeroVector;
        ZoneExtent = FVector(5000.0f, 5000.0f, 1000.0f);
    }
};

/**
 * Advanced Physics Integrator for Transpersonal Game
 * Manages complex physics interactions between terrain, objects, and biomes
 * Provides realistic physics simulation for prehistoric survival gameplay
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_AdvancedPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_AdvancedPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Physics Integration Methods
    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    void InitializePhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    void UpdateTerrainPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    void ApplyBiomePhysics(AActor* Actor, ECore_BiomePhysicsType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    ECore_BiomePhysicsType GetBiomeTypeAtLocation(const FVector& Location) const;

    // Terrain Physics Zone Management
    UFUNCTION(BlueprintCallable, Category = "Physics Zones")
    void CreatePhysicsZone(const FCore_TerrainPhysicsZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Physics Zones")
    void RemovePhysicsZone(ECore_BiomePhysicsType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Physics Zones")
    TArray<FCore_TerrainPhysicsZone> GetAllPhysicsZones() const;

    // Advanced Physics Simulation
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void SimulateTerrainDeformation(const FVector& ImpactLocation, float Force, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ApplyEnvironmentalForces(AActor* Actor, const FVector& WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ProcessObjectTerrainInteraction(AActor* Object, const FVector& ContactPoint);

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    UPhysicalMaterial* GetPhysicalMaterialForBiome(ECore_BiomePhysicsType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void UpdatePhysicalMaterialProperties(ECore_BiomePhysicsType BiomeType, const FCore_BiomePhysicsProperties& Properties);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidatePhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawPhysicsZoneDebug(bool bShowZones = true);

protected:
    // Physics Zone Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zones", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_TerrainPhysicsZone> PhysicsZones;

    // Physics Integration Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableAdvancedPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings", meta = (AllowPrivateAccess = "true"))
    float TerrainDeformationThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings", meta = (AllowPrivateAccess = "true"))
    float EnvironmentalForceMultiplier = 1.0f;

    // Cached References
    UPROPERTY(BlueprintReadOnly, Category = "Cached References", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<UWorld> CachedWorld;

    UPROPERTY(BlueprintReadOnly, Category = "Cached References", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<ALandscape>> LandscapeActors;

    UPROPERTY(BlueprintReadOnly, Category = "Cached References", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<ATriggerVolume>> PhysicsZoneTriggers;

private:
    // Internal Physics Processing
    void ProcessPhysicsZoneOverlaps();
    void UpdateObjectPhysicsProperties(AActor* Actor, const FCore_BiomePhysicsProperties& Properties);
    void CacheWorldReferences();
    void InitializeBiomePhysicsZones();

    // Performance Optimization
    float LastPhysicsUpdateTime = 0.0f;
    int32 PhysicsFrameCounter = 0;
};