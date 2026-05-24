#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Handles ragdoll physics, destruction, and enhanced movement for TranspersonalGame
 * Implements realistic physics simulation for prehistoric survival gameplay
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

    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System") 
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool IsPhysicsSystemActive() const { return bPhysicsSystemActive; }

    /**
     * Register a collision component with the physics system
     * @param CollisionComponent The component to register
     */
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterCollisionComponent(UAdvancedCollisionComponent* CollisionComponent);

    /**
     * Unregister a collision component from the physics system
     * @param CollisionComponent The component to unregister
     */
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterCollisionComponent(UAdvancedCollisionComponent* CollisionComponent);

    /**
     * Register a ragdoll component with the physics system
     * @param RagdollComponent The component to register
     */
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterRagdollComponent(URagdollPhysicsComponent* RagdollComponent);

    /**
     * Unregister a ragdoll component from the physics system
     * @param RagdollComponent The component to unregister
     */
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterRagdollComponent(URagdollPhysicsComponent* RagdollComponent);

    /**
     * Register a destruction component with the physics system
     * @param DestructionComponent The component to register
     */
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterDestructionComponent(UDestructionComponent* DestructionComponent);

    /**
     * Unregister a destruction component from the physics system
     * @param DestructionComponent The component to unregister
     */
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterDestructionComponent(UDestructionComponent* DestructionComponent);

    // Collision Management
    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    void SetupCollisionForActor(AActor* Actor, ECore_CollisionType CollisionType);

    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    void EnablePhysicsSimulation(AActor* Actor, bool bEnable);

    /**
     * Get the singleton instance of the physics system manager
     * @param World The world context
     * @return The physics system manager instance
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor)
    static UPhysicsSystemManager* GetPhysicsSystemManager(UWorld* World);

protected:
    // Core System State
    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    bool bPhysicsSystemActive;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    float PhysicsSystemVersion;

    // Ragdoll Management
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Physics")
    TArray<TWeakObjectPtr<ACharacter>> ActiveRagdollCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RagdollActivationForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RagdollDeactivationDelay;

    // Physics Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxActivePhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePhysicsOptimization;

    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float DefaultCollisionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float DefaultMass;

    // Destruction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    float MinDestructionForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    int32 MaxDebrisCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    float DebrisLifetime;

protected:
    /** Array of registered collision components */
// [UHT-FIX]     UPROPERTY()
    TArray<UAdvancedCollisionComponent*> RegisteredCollisionComponents;

    /** Array of registered ragdoll components */
// [UHT-FIX]     UPROPERTY()
    TArray<URagdollPhysicsComponent*> RegisteredRagdollComponents;

    /** Array of registered destruction components */
// [UHT-FIX]     UPROPERTY()
    TArray<UDestructionComponent*> RegisteredDestructionComponents;

    /** Whether physics optimizations are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsOptimizationsEnabled;

    /** Maximum number of physics objects to simulate per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxPhysicsObjectsPerFrame;

    /** Target physics frame rate */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float TargetPhysicsFrameRate;

    /** Current physics frame time */
    float CurrentPhysicsFrameTime;

    /** Physics performance history for averaging */
    TArray<float> PhysicsFrameTimeHistory;

    /** Static instance for singleton pattern */
    static UPhysicsSystemManager* Instance;
};