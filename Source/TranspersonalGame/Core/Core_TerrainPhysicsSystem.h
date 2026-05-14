#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Eng_ArchitecturalFramework.h"
#include "Core_TerrainPhysicsSystem.generated.h"

// ============================================================================
// CORE TERRAIN PHYSICS SYSTEM - AGENT #3
// Implements realistic terrain physics for prehistoric world interaction
// Integrates with Engine Architectural Framework for performance compliance
// ============================================================================

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Mud,            // Soft, deformable, high friction
    Sand,           // Granular, shifting, medium friction
    Rock,           // Hard, stable, low friction
    Grass,          // Organic, springy, variable friction
    Water,          // Fluid, dynamic, special physics
    Lava,           // Dangerous, hot, damage-dealing
    Ice,            // Slippery, brittle, low friction
    Snow            // Soft, compactable, temperature-sensitive
};

UENUM(BlueprintType)
enum class ECore_TerrainState : uint8
{
    Stable,         // Normal physics behavior
    Deforming,      // Currently being modified
    Unstable,       // Prone to collapse or sliding
    Damaged,        // Cracked or broken
    Wet,            // Affected by water/rain
    Frozen,         // Ice physics active
    Burning         // Fire/lava effects active
};

USTRUCT(BlueprintType)
struct FCore_TerrainProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    ECore_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    ECore_TerrainState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FrictionCoefficient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Deformability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hardness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Moisture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "-40.0", ClampMax = "100.0"))
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bCanDeform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bAffectedByWeather;

    FCore_TerrainProperties()
    {
        TerrainType = ECore_TerrainType::Grass;
        CurrentState = ECore_TerrainState::Stable;
        FrictionCoefficient = 0.7f;
        Deformability = 0.3f;
        Hardness = 50.0f;
        Moisture = 0.2f;
        Temperature = 20.0f;
        bCanDeform = true;
        bAffectedByWeather = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_TerrainDeformation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Force;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    bool bPermanent;

    FCore_TerrainDeformation()
    {
        Location = FVector::ZeroVector;
        Radius = 100.0f;
        Depth = 10.0f;
        Force = 1000.0f;
        Duration = 1.0f;
        bPermanent = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ========================================================================
    // TERRAIN PHYSICS INTERFACE
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainProperties(const FCore_TerrainProperties& Properties);

    UFUNCTION(BlueprintPure, Category = "Terrain Physics")
    FCore_TerrainProperties GetTerrainProperties() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyDeformation(const FCore_TerrainDeformation& Deformation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateFootprint(const FVector& Location, float Weight, float FootSize);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateVehicleTrack(const FVector& StartLocation, const FVector& EndLocation, float TrackWidth, float Weight);

    // ========================================================================
    // ENVIRONMENTAL EFFECTS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyWeatherEffect(float RainIntensity, float Temperature, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateMoisture(float MoistureChange);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTemperature(float TemperatureChange);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ProcessErosion(float DeltaTime);

    // ========================================================================
    // COLLISION AND INTERACTION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateFriction(const FVector& ContactPoint, const FVector& Velocity) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanSupportWeight(float Weight, const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FVector CalculateNormal(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void HandleImpact(const FVector& ImpactLocation, float ImpactForce, AActor* ImpactingActor);

    // ========================================================================
    // PERFORMANCE OPTIMIZATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void OptimizePhysicsLOD(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void EnablePhysicsSimulation(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Terrain Physics")
    bool IsPhysicsActive() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    FCore_TerrainProperties TerrainProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TArray<FCore_TerrainDeformation> ActiveDeformations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float PhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float MaxDeformationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableRealTimeDeformation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableWeatherEffects;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics")
    bool bPhysicsActive;

private:
    void UpdateTerrainState(float DeltaTime);
    void ProcessDeformations(float DeltaTime);
    void UpdatePhysicsMaterial();
    void CalculateStabilityFactor();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_TerrainPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ========================================================================
    // TERRAIN MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void InitializeTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void RegisterTerrainActor(AActor* TerrainActor, ECore_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void UnregisterTerrainActor(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    TArray<AActor*> GetTerrainActorsInRadius(const FVector& Center, float Radius) const;

    // ========================================================================
    // GLOBAL TERRAIN EFFECTS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void ApplyGlobalWeatherEffect(float RainIntensity, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void CreateExplosionDeformation(const FVector& Location, float ExplosionRadius, float ExplosionForce);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void CreateEarthquakeEffect(const FVector& Epicenter, float Magnitude, float Duration);

    // ========================================================================
    // ARCHITECTURAL FRAMEWORK INTEGRATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Management")
    void ValidateArchitecturalCompliance();

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void OptimizePerformance();

    UFUNCTION(BlueprintPure, Category = "Terrain Management")
    EEng_PerformanceTier GetCurrentPerformanceTier() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Management")
    TMap<AActor*, ECore_TerrainType> RegisteredTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Management")
    float GlobalUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Management")
    float MaxSimulationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Management")
    int32 MaxActiveDeformations;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Management")
    UEng_ArchitecturalFramework* ArchitecturalFramework;

private:
    void UpdateGlobalTerrainState(float DeltaTime);
    void ProcessGlobalDeformations(float DeltaTime);
    void MonitorPerformance();
    void EnforcePerformanceLimits();
};

#include "Core_TerrainPhysicsSystem.generated.h"