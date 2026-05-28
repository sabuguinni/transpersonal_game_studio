#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Core_PhysicsSystemManager.generated.h"

class UEngArch_CoreSystemRegistry;

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsSystem, Log, All);

/**
 * Enum for different physics material types in the prehistoric world
 */
UENUM(BlueprintType)
enum class ECore_PhysicsMaterial : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Wood        UMETA(DisplayName = "Wood"),
    Bone        UMETA(DisplayName = "Bone"),
    Flesh       UMETA(DisplayName = "Flesh"),
    Water       UMETA(DisplayName = "Water"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Ice         UMETA(DisplayName = "Ice"),
    Metal       UMETA(DisplayName = "Metal"),
    Vegetation  UMETA(DisplayName = "Vegetation")
};

/**
 * Structure for physics impact events
 */
USTRUCT(BlueprintType)
struct FCore_PhysicsImpact
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* ImpactActor;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactLocation;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactVelocity;

    UPROPERTY(BlueprintReadOnly)
    float ImpactForce;

    UPROPERTY(BlueprintReadOnly)
    ECore_PhysicsMaterial MaterialType;

    FCore_PhysicsImpact()
    {
        ImpactActor = nullptr;
        ImpactLocation = FVector::ZeroVector;
        ImpactVelocity = FVector::ZeroVector;
        ImpactForce = 0.0f;
        MaterialType = ECore_PhysicsMaterial::Rock;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsImpact, const FCore_PhysicsImpact&, ImpactData);

/**
 * Core Physics System Manager
 * 
 * Manages all physics-related functionality for the prehistoric world:
 * - Realistic physics simulation for dinosaurs, objects, and environment
 * - Material-based physics properties (rock, wood, bone, flesh)
 * - Impact and collision event handling
 * - Physics-based destruction and damage
 * - Integration with survival mechanics
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics System Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystem();

    // Material Physics Properties
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void SetPhysicsMaterial(UStaticMeshComponent* MeshComponent, ECore_PhysicsMaterial MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void SetSkeletalPhysicsMaterial(USkeletalMeshComponent* SkeletalMesh, ECore_PhysicsMaterial MaterialType);

    UFUNCTION(BlueprintPure, Category = "Physics Materials")
    float GetMaterialDensity(ECore_PhysicsMaterial MaterialType) const;

    UFUNCTION(BlueprintPure, Category = "Physics Materials")
    float GetMaterialFriction(ECore_PhysicsMaterial MaterialType) const;

    UFUNCTION(BlueprintPure, Category = "Physics Materials")
    float GetMaterialRestitution(ECore_PhysicsMaterial MaterialType) const;

    // Force and Impulse Application
    UFUNCTION(BlueprintCallable, Category = "Physics Forces")
    void ApplyForceToActor(AActor* TargetActor, const FVector& Force, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Physics Forces")
    void ApplyImpulseToActor(AActor* TargetActor, const FVector& Impulse, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Physics Forces")
    void ApplyRadialForce(const FVector& Origin, float Radius, float Strength, bool bImpulse = false);

    // Collision and Impact Handling
    UFUNCTION(BlueprintCallable, Category = "Physics Collision")
    void RegisterForImpactEvents(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Collision")
    void UnregisterFromImpactEvents(AActor* Actor);

    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    // Physics Queries
    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool LineTracePhysics(const FVector& Start, const FVector& End, FHitResult& OutHit, bool bIgnoreActors = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool SphereTracePhysics(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    TArray<AActor*> GetActorsInRadius(const FVector& Center, float Radius);

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Physics Destruction")
    void TriggerDestruction(AActor* TargetActor, const FVector& ImpactPoint, float DestructionForce);

    UFUNCTION(BlueprintCallable, Category = "Physics Destruction")
    bool CanActorBeDestroyed(AActor* TargetActor, float RequiredForce) const;

    // Physics Settings
    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    void SetGlobalPhysicsScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    void SetPhysicsSimulationEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Physics Settings")
    bool IsPhysicsSimulationEnabled() const { return bPhysicsEnabled; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsImpact OnPhysicsImpact;

    // System Integration
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithCoreRegistry();

    UFUNCTION(BlueprintPure, Category = "System Integration")
    bool IsSystemInitialized() const { return bSystemInitialized; }

protected:
    // Internal Physics Management
    void SetupPhysicsMaterials();
    void ConfigurePhysicsSettings();
    void BindCollisionEvents();
    void UnbindCollisionEvents();

    // Material Property Calculations
    void ApplyMaterialProperties(UPrimitiveComponent* Component, ECore_PhysicsMaterial MaterialType);
    float CalculateImpactForce(const FVector& Velocity, float Mass) const;
    ECore_PhysicsMaterial DetermineMaterialType(AActor* Actor) const;

    // Event Processing
    void ProcessImpactEvent(AActor* Actor, const FHitResult& Hit, const FVector& ImpactVelocity);
    void BroadcastImpactEvent(const FCore_PhysicsImpact& ImpactData);

private:
    // System State
    UPROPERTY()
    bool bSystemInitialized;

    UPROPERTY()
    bool bPhysicsEnabled;

    UPROPERTY()
    float GlobalPhysicsScale;

    // Registered Actors for Impact Events
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    // Material Properties Cache
    UPROPERTY()
    TMap<ECore_PhysicsMaterial, float> MaterialDensities;

    UPROPERTY()
    TMap<ECore_PhysicsMaterial, float> MaterialFrictions;

    UPROPERTY()
    TMap<ECore_PhysicsMaterial, float> MaterialRestitutions;

    // System Registry Reference
    UPROPERTY()
    TWeakObjectPtr<UEngArch_CoreSystemRegistry> CoreRegistry;

    // Physics World Cache
    UWorld* CachedWorld;
};