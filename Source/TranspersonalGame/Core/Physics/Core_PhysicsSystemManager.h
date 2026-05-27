#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsSystemManager.generated.h"

class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

/**
 * Physics System Manager - Core physics subsystem
 * 
 * Manages all physics-related systems in the game:
 * - Collision detection and response
 * - Ragdoll physics for characters
 * - Destruction physics for objects
 * - Environmental physics interactions
 * 
 * This system ensures consistent physics behavior across
 * all game objects and provides centralized physics configuration.
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
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ShutdownPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UpdatePhysicsSystems(float DeltaTime);

    // Global Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalPhysicsTimeStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    int32 MaxPhysicsSubSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsSimulation;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveRagdolls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDestructionObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateDistance;

    // System References
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UCore_CollisionSystem* CollisionSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UCore_RagdollSystem* RagdollSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UCore_DestructionSystem* DestructionSystem;

    // Physics Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsEvent, AActor*, Actor, FVector, ImpactLocation);
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsEvent OnLargeImpact;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsEvent OnObjectDestroyed;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsEvent OnRagdollActivated;

    // Physics Queries
    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool IsPhysicsEnabled() const { return bEnablePhysicsSimulation; }

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    int32 GetActiveRagdollCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    int32 GetActiveDestructionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    float GetCurrentPhysicsLoad() const;

    // System Access
    UFUNCTION(BlueprintPure, Category = "System Access")
    UCore_CollisionSystem* GetCollisionSystem() const { return CollisionSystem; }

    UFUNCTION(BlueprintPure, Category = "System Access")
    UCore_RagdollSystem* GetRagdollSystem() const { return RagdollSystem; }

    UFUNCTION(BlueprintPure, Category = "System Access")
    UCore_DestructionSystem* GetDestructionSystem() const { return DestructionSystem; }

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawPhysicsInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogPhysicsStats();

protected:
    // Internal Management
    void CreatePhysicsSystems();
    void ConfigurePhysicsSettings();
    void RegisterPhysicsEvents();

    // Performance Monitoring
    UPROPERTY()
    float LastPhysicsUpdateTime;

    UPROPERTY()
    int32 CurrentActiveRagdolls;

    UPROPERTY()
    int32 CurrentDestructionObjects;

    UPROPERTY()
    bool bSystemsInitialized;

    // Physics World Settings
    void ApplyWorldPhysicsSettings(UWorld* World);
    void RestoreWorldPhysicsSettings(UWorld* World);

    // Event Handlers
    UFUNCTION()
    void HandleLargeImpact(AActor* Actor, const FVector& ImpactLocation);

    UFUNCTION()
    void HandleObjectDestruction(AActor* Actor, const FVector& DestructionLocation);

    UFUNCTION()
    void HandleRagdollActivation(AActor* Actor, const FVector& ActivationLocation);
};