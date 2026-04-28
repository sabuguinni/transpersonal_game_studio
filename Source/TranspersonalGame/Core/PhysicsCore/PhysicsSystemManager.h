#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;

/**
 * Core physics system manager for Transpersonal Game.
 * Handles physics simulation, collision detection, ragdoll physics, and destruction.
 * Designed for realistic prehistoric survival gameplay with dynamic environments.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics Simulation Control
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsSimulationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool IsPhysicsSimulationEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetGlobalPhysicsScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    float GetGlobalPhysicsScale() const;

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool PerformLineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, 
                         bool bTraceComplex = false, const TArray<AActor*>& ActorsToIgnore = TArray<AActor*>());

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool PerformSphereTrace(const FVector& Start, const FVector& End, float Radius, 
                           FHitResult& OutHit, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool PerformBoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, 
                        const FRotator& Orientation, FHitResult& OutHit, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    TArray<FHitResult> PerformMultiLineTrace(const FVector& Start, const FVector& End, 
                                           bool bTraceComplex = false);

    // Ragdoll Physics
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableRagdollPhysics(USkeletalMeshComponent* SkeletalMesh, bool bBlendPhysics = true);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void DisableRagdollPhysics(USkeletalMeshComponent* SkeletalMesh, bool bBlendBack = true);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetRagdollBlendWeight(USkeletalMeshComponent* SkeletalMesh, float BlendWeight);

    // Dynamic Destruction
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyDestructionForce(UPrimitiveComponent* Component, const FVector& Force, 
                              const FVector& Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void CreateDestructionField(const FVector& Location, float Radius, float Force, 
                               float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void FractureStaticMesh(UStaticMeshComponent* MeshComponent, const FVector& ImpactPoint, 
                           const FVector& ImpactForce);

    // Physics Materials and Properties
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsMaterial(UPrimitiveComponent* Component, class UPhysicalMaterial* PhysMaterial);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetCollisionResponseToChannel(UPrimitiveComponent* Component, 
                                     ECollisionChannel Channel, ECollisionResponse Response);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetMassOverride(UPrimitiveComponent* Component, float Mass, bool bOverrideMass = true);

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyWindForce(const FVector& WindDirection, float WindStrength, float Radius, 
                       const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SimulateEarthquake(const FVector& Epicenter, float Magnitude, float Duration, 
                           float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void CreateWaterBuoyancy(UPrimitiveComponent* Component, float WaterLevel, 
                            float BuoyancyForce = 1000.0f);

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsLOD(UPrimitiveComponent* Component, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    float GetPhysicsFrameTime() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor = true)
    void DebugDrawPhysicsShapes(bool bEnabled, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor = true)
    void DebugDrawCollisionQueries(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void LogPhysicsStats() const;

protected:
    // Internal physics state
    UPROPERTY(BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSimulationEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    float GlobalPhysicsScale;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsOptimizationEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    int32 MaxSimulatedBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    float PhysicsSubstepDeltaTime;

    // Destruction tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<UPrimitiveComponent>> ActiveDestructionComponents;

    UPROPERTY()
    TArray<TWeakObjectPtr<USkeletalMeshComponent>> ActiveRagdolls;

    // Internal helper methods
    void InitializePhysicsSettings();
    void CleanupDestroyedComponents();
    void UpdatePhysicsLOD();
    void ProcessDestructionFields(float DeltaTime);

    // Physics event handlers
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComponent, FVector NormalImpulse, 
                       const FHitResult& Hit);

    UFUNCTION()
    void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                bool bFromSweep, const FHitResult& SweepResult);

private:
    // Timer handles for periodic updates
    FTimerHandle PhysicsUpdateTimer;
    FTimerHandle CleanupTimer;
    FTimerHandle LODUpdateTimer;

    // Performance tracking
    float LastPhysicsFrameTime;
    int32 ActivePhysicsObjectCount;
    
    // Debug state
    bool bDebugPhysicsShapes;
    bool bDebugCollisionQueries;
    float DebugDrawDuration;
};