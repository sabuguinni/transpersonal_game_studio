#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsSystemManager.generated.h"

class UPhysicsCollisionManager;
class URagdollSystemManager;
class UDestructionSystemManager;
class UVehiclePhysicsManager;

/**
 * @brief Central physics system manager for Transpersonal Game
 * 
 * Orchestrates all physics subsystems including collision, ragdoll, destruction, and vehicle physics.
 * Implements performance monitoring and LOD systems for 60fps PC / 30fps console targets.
 * 
 * @author Core Systems Programmer #03
 * @version 1.0
 * @date 2024
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End UActorComponent Interface

    /**
     * Initialize all physics subsystems
     * @param InWorld The world context for physics simulation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystems(UWorld* InWorld);

    /**
     * Shutdown all physics subsystems gracefully
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystems();

    /**
     * Update physics LOD based on performance metrics
     * @param CurrentFPS Current frame rate
     * @param TargetFPS Target frame rate (60 PC, 30 console)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsLOD(float CurrentFPS, float TargetFPS);

    /**
     * Get collision manager subsystem
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    UPhysicsCollisionManager* GetCollisionManager() const { return CollisionManager; }

    /**
     * Get ragdoll system manager
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    URagdollSystemManager* GetRagdollManager() const { return RagdollManager; }

    /**
     * Get destruction system manager
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    UDestructionSystemManager* GetDestructionManager() const { return DestructionManager; }

    /**
     * Get vehicle physics manager
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    UVehiclePhysicsManager* GetVehicleManager() const { return VehicleManager; }

    /**
     * Enable/disable physics simulation globally
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsEnabled(bool bEnabled);

    /**
     * Get current physics performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FString GetPhysicsPerformanceReport() const;

protected:
    /** Collision detection and response manager */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Subsystems")
    TObjectPtr<UPhysicsCollisionManager> CollisionManager;

    /** Ragdoll physics and death animations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Subsystems")
    TObjectPtr<URagdollSystemManager> RagdollManager;

    /** Destruction and fracture system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Subsystems")
    TObjectPtr<UDestructionSystemManager> DestructionManager;

    /** Vehicle and mount physics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Subsystems")
    TObjectPtr<UVehiclePhysicsManager> VehicleManager;

    /** Current physics LOD level (0=highest, 3=lowest) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentPhysicsLOD;

    /** Maximum number of active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100", ClampMax = "10000"))
    int32 MaxActivePhysicsBodies;

    /** Physics simulation enabled flag */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics System")
    bool bPhysicsEnabled;

    /** Performance monitoring */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float AveragePhysicsFrameTime;

    /** Frame time samples for averaging */
    TArray<float> PhysicsFrameTimeSamples;

    /** Maximum samples to keep for averaging */
    static constexpr int32 MaxFrameTimeSamples = 60;

private:
    /**
     * Create and initialize subsystem managers
     */
    void CreateSubsystemManagers();

    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics(float DeltaTime);

    /**
     * Determine optimal LOD level based on performance
     */
    int32 CalculateOptimalLOD(float CurrentFPS, float TargetFPS) const;

    /** World reference for physics simulation */
    TWeakObjectPtr<UWorld> WorldContext;

    /** Last frame time for performance tracking */
    float LastFrameTime;
};