#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Core_PhysicsIntegrator.generated.h"

class UCore_PhysicsManager;
class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

/**
 * Core Physics Integrator - Coordinates all physics subsystems
 * Provides unified interface for physics operations across the game
 * Manages physics simulation quality and performance scaling
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics System References
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    UCore_PhysicsManager* PhysicsManager;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    UCore_CollisionSystem* CollisionSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    UCore_RagdollSystem* RagdollSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    UCore_DestructionSystem* DestructionSystem;

    // Physics Quality Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Quality", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float PhysicsTimeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Quality", meta = (ClampMin = "30", ClampMax = "120"))
    int32 PhysicsSubsteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Quality")
    bool bEnableHighQualityPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Quality")
    bool bEnablePhysicsLOD;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentPhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveConstraints;

    // Physics Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsQuality(float Quality);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void EnablePhysicsLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsPerformance();

    // Character Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void IntegrateCharacterPhysics(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void ApplyPhysicsImpulse(AActor* Actor, FVector Impulse, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void EnableRagdoll(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void DisableRagdoll(ACharacter* Character);

    // Destruction Integration
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(AActor* Actor, FVector ImpactLocation, float Force);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateDebris(FVector Location, int32 DebrisCount);

    // Collision Integration
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void UnregisterCollisionActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool CheckCollisionAtLocation(FVector Location, float Radius);

    // Fluid Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void ApplyFluidForces(AActor* Actor, FVector FluidVelocity, float Density);

    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void CreateWaterSplash(FVector Location, float Intensity);

    // Vehicle Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyVehiclePhysics(AActor* Vehicle, float Throttle, float Steering);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateVehicleSuspension(AActor* Vehicle);

    // Terrain Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysics(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainStability(FVector Location);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawPhysicsInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TogglePhysicsDebugDraw();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsStats();

private:
    // Internal state tracking
    bool bSystemsInitialized;
    float LastPerformanceUpdate;
    TArray<AActor*> RegisteredActors;
    
    // Performance thresholds
    static constexpr float MAX_PHYSICS_FRAME_TIME = 16.67f; // 60fps
    static constexpr int32 MAX_PHYSICS_BODIES = 1000;
    static constexpr int32 MAX_CONSTRAINTS = 500;

    // Internal helper functions
    void UpdatePhysicsQuality();
    void MonitorPerformance();
    void OptimizePhysicsSettings();
    bool ShouldUsePhysicsLOD(AActor* Actor);
    float CalculateDistanceToPlayer(AActor* Actor);
};