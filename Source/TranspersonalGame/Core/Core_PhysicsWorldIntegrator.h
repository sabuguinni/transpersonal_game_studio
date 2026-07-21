#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Subsystems/WorldSubsystem.h"
#include "Physics/PhysicsFiltering.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/CollisionProfile.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Core_PhysicsWorldIntegrator.generated.h"

class ALandscape;
class UStaticMeshComponent;
class UPrimitiveComponent;
class UPhysicalMaterial;

UENUM(BlueprintType)
enum class ECore_PhysicsIntegrationType : uint8
{
    None            UMETA(DisplayName = "None"),
    Basic           UMETA(DisplayName = "Basic Physics"),
    Advanced        UMETA(DisplayName = "Advanced Physics"),
    Realistic       UMETA(DisplayName = "Realistic Physics"),
    Performance     UMETA(DisplayName = "Performance Optimized")
};

UENUM(BlueprintType)
enum class ECore_TerrainPhysicsMode : uint8
{
    Static          UMETA(DisplayName = "Static Terrain"),
    Dynamic         UMETA(DisplayName = "Dynamic Terrain"),
    Deformable      UMETA(DisplayName = "Deformable Terrain"),
    Destructible    UMETA(DisplayName = "Destructible Terrain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsWorldSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    ECore_PhysicsIntegrationType IntegrationType = ECore_PhysicsIntegrationType::Advanced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    ECore_TerrainPhysicsMode TerrainMode = ECore_TerrainPhysicsMode::Dynamic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float WorldBoundsRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnableRealTimePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnableTerrainDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnableFluidDynamics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnableDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    int32 MaxPhysicsActors = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float PhysicsTickRate = 60.0f;

    FCore_PhysicsWorldSettings()
    {
        IntegrationType = ECore_PhysicsIntegrationType::Advanced;
        TerrainMode = ECore_TerrainPhysicsMode::Dynamic;
        GravityScale = 1.0f;
        WorldBoundsRadius = 50000.0f;
        bEnableRealTimePhysics = true;
        bEnableTerrainDeformation = true;
        bEnableFluidDynamics = false;
        bEnableDestruction = true;
        MaxPhysicsActors = 5000;
        PhysicsTickRate = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionQueryTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionQueries = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceOptimal = true;

    FCore_PhysicsPerformanceMetrics()
    {
        ActivePhysicsActors = 0;
        PhysicsFrameTime = 0.0f;
        CollisionQueryTime = 0.0f;
        CollisionQueries = 0;
        MemoryUsageMB = 0.0f;
        bIsPerformanceOptimal = true;
    }
};

/**
 * Core Physics World Integrator - Master physics system for the prehistoric world
 * Manages all physics interactions between terrain, characters, dinosaurs, and environment
 * Provides realistic physics simulation for survival gameplay
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsWorldIntegrator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World integration
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void InitializePhysicsWorld();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ShutdownPhysicsWorld();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsWorld(float DeltaTime);

    // Settings management
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsWorldSettings(const FCore_PhysicsWorldSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    FCore_PhysicsWorldSettings GetPhysicsWorldSettings() const;

    // Terrain physics integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterTerrainActor(ALandscape* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UnregisterTerrainActor(ALandscape* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void EnableTerrainPhysics(ALandscape* TerrainActor, bool bEnable);

    // Actor physics management
    UFUNCTION(BlueprintCallable, Category = "Actor Physics")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Actor Physics")
    void UnregisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Actor Physics")
    void SetActorPhysicsEnabled(AActor* Actor, bool bEnabled);

    // Collision and interaction
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformPhysicsTrace(const FVector& Start, const FVector& End, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    TArray<FHitResult> PerformPhysicsSweep(const FVector& Start, const FVector& End, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionProfile(UPrimitiveComponent* Component, const FName& ProfileName);

    // Material physics
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyPhysicalMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* Material);

    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    UPhysicalMaterial* GetSurfacePhysicalMaterial(const FVector& Location);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PhysicsPerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(bool bHighPerformance);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnablePhysicsDebugDraw(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawPhysicsDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidatePhysicsIntegration();

protected:
    // Core settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FCore_PhysicsWorldSettings WorldSettings;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsPerformanceMetrics PerformanceMetrics;

    // Registered actors
    UPROPERTY()
    TArray<TWeakObjectPtr<ALandscape>> RegisteredTerrain;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredPhysicsActors;

    // System state
    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    bool bIsPhysicsEnabled = true;

    UPROPERTY()
    bool bDebugDrawEnabled = false;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    int32 FrameCounter = 0;

private:
    // Internal methods
    void InitializePhysicsSettings();
    void UpdatePerformanceMetrics();
    void CleanupInvalidActors();
    void ProcessPhysicsUpdates(float DeltaTime);
    void HandleTerrainPhysics(float DeltaTime);
    void HandleActorPhysics(float DeltaTime);
    void OptimizePhysicsLOD();
    void ValidateSystemIntegrity();

    // Performance tracking
    double LastFrameTime = 0.0;
    int32 PhysicsUpdateCounter = 0;
    float AccumulatedFrameTime = 0.0f;
};