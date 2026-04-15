#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

class UCollisionManager;
class URagdollSystem;
class UDestructionSystem;

/**
 * Core physics system manager that coordinates all physics subsystems
 * Handles physics simulation settings, collision detection, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics simulation control
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsSimulationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsPhysicsSimulationEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetGravityScale() const;

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Physics|Performance")
    void SetPhysicsLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics|Performance")
    void OptimizePhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics|Performance")
    void SetMaxPhysicsObjects(int32 MaxObjects);

    // Collision management
    UFUNCTION(BlueprintCallable, Category = "Physics|Collision")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics|Collision")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Physics|Collision")
    void RegisterCollisionObject(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Physics|Collision")
    void UnregisterCollisionObject(UPrimitiveComponent* Component);

    // Ragdoll system integration
    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    void EnableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    void DisableRagdollForActor(AActor* Actor);

    // Destruction system integration
    UFUNCTION(BlueprintCallable, Category = "Physics|Destruction")
    void TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float Force);

    UFUNCTION(BlueprintCallable, Category = "Physics|Destruction")
    void SetDestructionEnabled(bool bEnabled);

    // Physics material management
    UFUNCTION(BlueprintCallable, Category = "Physics|Materials")
    void ApplyPhysicsMaterial(UPrimitiveComponent* Component, class UPhysicalMaterial* Material);

    // Debug and diagnostics
    UFUNCTION(BlueprintCallable, Category = "Physics|Debug", CallInEditor)
    void RunPhysicsSystemDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Physics|Debug")
    void TogglePhysicsDebugDraw(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics|Debug")
    FString GetPhysicsSystemStatus() const;

protected:
    // Physics simulation settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    bool bPhysicsSimulationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float GravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float PhysicsLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    int32 MaxPhysicsObjects;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bOptimizeForPerformance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float PhysicsTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxSimulationSteps;

    // Subsystem references
    UPROPERTY()
    TObjectPtr<UCollisionManager> CollisionManager;

    UPROPERTY()
    TObjectPtr<URagdollSystem> RagdollSystem;

    UPROPERTY()
    TObjectPtr<UDestructionSystem> DestructionSystem;

    // Registered collision objects
    UPROPERTY()
    TArray<TWeakObjectPtr<UPrimitiveComponent>> RegisteredCollisionObjects;

    // Debug settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bDebugDrawEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bShowPhysicsStats;

private:
    void InitializeSubsystems();
    void CleanupSubsystems();
    void UpdatePhysicsSettings();
    void ValidatePhysicsObjects();
    
    // Performance monitoring
    float LastPerformanceCheck;
    int32 CurrentPhysicsObjectCount;
    float AveragePhysicsTickTime;
};