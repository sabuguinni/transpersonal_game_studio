#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core/SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPhysicsImpact, AActor*, HitActor, FVector, ImpactPoint, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsBreak, AActor*, BrokenActor, FVector, BreakLocation);

/**
 * Core Physics System Manager - Manages all physics interactions in the prehistoric survival world
 * Handles realistic physics responses for dinosaur combat, environmental destruction, and survival mechanics
 * Integrates with UE5's Chaos Physics for authentic prehistoric world simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics impact events
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsImpact OnPhysicsImpact;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsBreak OnPhysicsBreak;

    // Core physics management
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void RegisterPhysicsActor(AActor* Actor, ECore_PhysicsType PhysicsType = ECore_PhysicsType::Dynamic);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void UnregisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void SetGlobalPhysicsSettings(float Gravity = -980.0f, float LinearDamping = 0.01f, float AngularDamping = 0.0f);

    // Impact and force application
    UFUNCTION(BlueprintCallable, Category = "Physics Forces")
    void ApplyImpactForce(AActor* TargetActor, FVector ImpactLocation, FVector ForceDirection, float ForceMagnitude);

    UFUNCTION(BlueprintCallable, Category = "Physics Forces")
    void ApplyRadialForce(FVector Origin, float Radius, float Strength, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Forces")
    void ApplyExplosionForce(FVector ExplosionOrigin, float InnerRadius, float OuterRadius, float Strength, bool bLinearFalloff = true);

    // Collision and physics queries
    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool LineTracePhysics(FVector Start, FVector End, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    TArray<FHitResult> SphereTracePhysics(FVector Center, float Radius, TArray<AActor*> IgnoreActors);

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    float CalculateImpactDamage(float ImpactVelocity, float ActorMass, ECore_PhysicsType PhysicsType);

    // Environmental physics
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void SimulateRockfall(FVector Origin, int32 RockCount = 10, float SpreadRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void SimulateTreeFall(AActor* TreeActor, FVector ImpactDirection, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void CreateDebrisField(FVector Origin, float Radius, int32 DebrisCount = 20);

    // Physics material management
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void SetActorPhysicsMaterial(AActor* Actor, ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    ECore_SurfaceType GetSurfaceTypeFromHit(const FHitResult& HitResult);

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsForPerformance(float MaxPhysicsDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLOD(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DebugDrawPhysicsInfo(bool bEnabled = true, float Duration = 5.0f);

protected:
    // Internal physics tracking
    UPROPERTY()
    TMap<AActor*, ECore_PhysicsType> RegisteredPhysicsActors;

    UPROPERTY()
    TArray<AActor*> ActivePhysicsActors;

    // Physics settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float GlobalGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float MaxPhysicsSimulationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    bool bEnablePhysicsOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    int32 MaxActivePhysicsActors;

    // Internal methods
    void UpdatePhysicsActors(float DeltaTime);
    void ProcessPhysicsEvents();
    void CleanupInactivePhysicsActors();
    
    // Timer handles
    FTimerHandle PhysicsUpdateTimer;
    FTimerHandle PhysicsCleanupTimer;
};