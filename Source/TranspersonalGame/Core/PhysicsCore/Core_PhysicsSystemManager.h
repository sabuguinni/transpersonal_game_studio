#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

// Physics material configuration for different terrain types
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMaterialConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    FString MaterialName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    bool bEnableContactModification = false;

    FCore_PhysicsMaterialConfig()
    {
        MaterialName = TEXT("DefaultPhysicsMaterial");
    }
};

// Physics simulation quality levels for performance scaling
UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

// Physics object types for different gameplay elements
UENUM(BlueprintType)
enum class ECore_PhysicsObjectType : uint8
{
    Static      UMETA(DisplayName = "Static Object"),
    Dynamic     UMETA(DisplayName = "Dynamic Object"),
    Kinematic   UMETA(DisplayName = "Kinematic Object"),
    Debris      UMETA(DisplayName = "Debris"),
    Character   UMETA(DisplayName = "Character"),
    Dinosaur    UMETA(DisplayName = "Dinosaur"),
    Projectile  UMETA(DisplayName = "Projectile")
};

// Physics performance metrics for monitoring
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 SleepingRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 CollisionPairs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float MemoryUsageMB = 0.0f;
};

/**
 * Core Physics System Manager
 * Manages all physics simulation, materials, and performance optimization
 * for the prehistoric survival game world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics system initialization and management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool IsPhysicsSystemActive() const { return bIsSystemActive; }

    // Physics quality and performance management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsQuality(ECore_PhysicsQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    ECore_PhysicsQuality GetPhysicsQuality() const { return CurrentPhysicsQuality; }

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void OptimizePhysicsPerformance();

    // Physics material management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UPhysicalMaterial* CreatePhysicsMaterial(const FCore_PhysicsMaterialConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyPhysicsMaterialToActor(AActor* Actor, UPhysicalMaterial* PhysicsMaterial);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupTerrainPhysics();

    // Object physics management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnablePhysicsOnActor(AActor* Actor, ECore_PhysicsObjectType ObjectType);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisablePhysicsOnActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetObjectPhysicsProperties(AActor* Actor, float Mass, float LinearDamping, float AngularDamping);

    // Collision and interaction
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupCollisionChannels();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ConfigureObjectCollision(AActor* Actor, ECore_PhysicsObjectType ObjectType);

    // Destruction and debris system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableDestructionOnActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void CreateDebrisFromActor(AActor* SourceActor, const FVector& ImpactPoint, float ImpactForce);

    // Physics simulation control
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void PausePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ResumePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsTimeScale(float TimeScale);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FCore_PhysicsMetrics GetPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePerformanceMetrics();

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor)
    void DebugDrawPhysicsShapes(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor)
    void LogPhysicsSystemStatus();

protected:
    // Core system state
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    bool bIsSystemActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    ECore_PhysicsQuality CurrentPhysicsQuality = ECore_PhysicsQuality::Medium;

    // Physics materials for different terrain types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TMap<FString, UPhysicalMaterial*> PhysicsMaterials;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval = 1.0f;

    // Physics simulation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxPhysicsStep = 0.033f; // 30 FPS minimum

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float FixedTimeStep = 0.016f; // 60 FPS target

private:
    // Internal system management
    void CreateDefaultPhysicsMaterials();
    void ApplyPhysicsQualitySettings(ECore_PhysicsQuality Quality);
    void CleanupInactivePhysicsObjects();
    
    // Performance optimization
    void OptimizeLODSettings();
    void ManagePhysicsObjectCulling();
    
    // Timer handles
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle PerformanceOptimizationTimer;
};