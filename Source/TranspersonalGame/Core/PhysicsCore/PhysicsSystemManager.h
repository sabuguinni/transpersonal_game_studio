#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsSystemManager.generated.h"

class UAdvancedCollisionComponent;
class URagdollPhysicsComponent;
class UDestructionComponent;

/**
 * @class UPhysicsSystemManager
 * @brief Central manager for all physics systems in the Transpersonal Game
 * 
 * This component manages the lifecycle and coordination of all physics subsystems:
 * - Advanced collision detection and response
 * - Ragdoll physics for characters and creatures
 * - Environmental destruction systems
 * - Performance optimization for physics calculations
 * 
 * Design Philosophy:
 * - Physics is the emotional signature of the game world
 * - Every physics interaction must feel believable and consistent
 * - Performance is not a feature, it's a requirement
 * 
 * @author Core Systems Programmer #03
 * @date 2024
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * Initialize all physics subsystems
     * Called during BeginPlay to set up the physics environment
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystems();

    /**
     * Shutdown all physics subsystems gracefully
     * Ensures proper cleanup of physics resources
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystems();

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

    /**
     * Get the current physics performance metrics
     * @return Struct containing performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FString GetPhysicsPerformanceMetrics() const;

    /**
     * Enable or disable physics system optimizations
     * @param bEnable Whether to enable optimizations
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsOptimizationsEnabled(bool bEnable);

    /**
     * Get the singleton instance of the physics system manager
     * @param World The world context
     * @return The physics system manager instance
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor)
    static UPhysicsSystemManager* GetPhysicsSystemManager(UWorld* World);

protected:
    /**
     * Update physics performance monitoring
     * @param DeltaTime Time since last update
     */
    void UpdatePerformanceMetrics(float DeltaTime);

    /**
     * Apply physics optimizations based on current performance
     */
    void ApplyPhysicsOptimizations();

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