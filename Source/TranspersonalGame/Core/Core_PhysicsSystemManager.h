#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Core_PhysicsSystemManager.generated.h"

// Forward declarations
class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsImpact, AActor*, ImpactedActor, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDestroyed, AActor*, DestroyedActor);

/**
 * Core Physics System Manager
 * Centralized management for all physics-related systems in the game
 * Handles collision detection, ragdoll physics, and destruction mechanics
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

    // Physics System Management
    UFUNCTION(BlueprintCallable, Category = "Physics Systems")
    void InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Systems")
    void ShutdownPhysicsSystems();

    // Collision System Access
    UFUNCTION(BlueprintCallable, Category = "Physics Systems")
    UCore_CollisionSystem* GetCollisionSystem() const { return CollisionSystem; }

    // Ragdoll System Access
    UFUNCTION(BlueprintCallable, Category = "Physics Systems")
    UCore_RagdollSystem* GetRagdollSystem() const { return RagdollSystem; }

    // Destruction System Access
    UFUNCTION(BlueprintCallable, Category = "Physics Systems")
    UCore_DestructionSystem* GetDestructionSystem() const { return DestructionSystem; }

    // Global Physics Settings
    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    void SetGlobalPhysicsMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Physics Settings")
    float GetGlobalPhysicsMultiplier() const { return GlobalPhysicsMultiplier; }

    // Actor Physics Management
    UFUNCTION(BlueprintCallable, Category = "Actor Physics")
    void EnablePhysicsForActor(AActor* Actor, bool bEnableRagdoll = false);

    UFUNCTION(BlueprintCallable, Category = "Actor Physics")
    void DisablePhysicsForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Actor Physics")
    void ApplyImpulseToActor(AActor* Actor, FVector Impulse, FVector Location);

    // Dinosaur-Specific Physics
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void SetupDinosaurPhysics(AActor* DinosaurActor, float Mass = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void TriggerDinosaurRagdoll(AActor* DinosaurActor, FVector ImpactForce);

    // Event Delegates
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsImpact OnPhysicsImpact;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnActorDestroyed OnActorDestroyed;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Debug", CallInEditor = true)
    void DebugShowPhysicsInfo();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void TogglePhysicsDebugDraw(bool bEnabled);

protected:
    // Core Physics Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UCore_CollisionSystem* CollisionSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UCore_RagdollSystem* RagdollSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems")
    UCore_DestructionSystem* DestructionSystem;

    // Global Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float GlobalPhysicsMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsDebug;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bAutoSetupDinosaurPhysics;

    // Internal Functions
    void RegisterPhysicsEvents();
    void UnregisterPhysicsEvents();

    UFUNCTION()
    void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

private:
    bool bSystemsInitialized;
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
};