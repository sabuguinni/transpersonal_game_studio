#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalPhysics, Log, All);

/**
 * Physics System Manager
 * Handles Chaos Physics integration for destruction, ragdoll, and collision systems
 * Designed for large-scale dinosaur simulation with realistic physics
 */
UCLASS()
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the physics system manager instance */
    UFUNCTION(BlueprintPure, Category = "Physics System")
    static UPhysicsSystemManager* Get(const UObject* WorldContext);

    /** Initialize Chaos Physics for large-scale simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializeChaosPhysics();

    /** Configure physics settings for dinosaur simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ConfigureDinosaurPhysics();

    /** Enable destruction system for environmental interaction */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableDestructionSystem();

    /** Setup ragdoll physics for character death/unconsciousness */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetupRagdollSystem();

    /** Configure collision detection for large world */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ConfigureCollisionSystem();

    /** Register a physics actor for management */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterPhysicsActor(AActor* Actor, const FString& PhysicsType);

    /** Unregister a physics actor */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterPhysicsActor(AActor* Actor);

    /** Get current physics performance metrics */
    UFUNCTION(BlueprintPure, Category = "Physics System")
    void GetPhysicsMetrics(int32& ActiveBodies, float& SimulationTime, int32& CollisionPairs) const;

    /** Enable/disable physics simulation for performance */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsSimulationEnabled(bool bEnabled);

    /** Configure physics LOD based on distance from player */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetupPhysicsLOD();

protected:
    /** Registered physics actors by type */
    UPROPERTY()
    TMap<FString, TArray<TObjectPtr<AActor>>> PhysicsActorsByType;

    /** Physics simulation enabled state */
    UPROPERTY()
    bool bPhysicsSimulationEnabled = true;

    /** Performance metrics */
    UPROPERTY()
    int32 ActivePhysicsBodies = 0;

    UPROPERTY()
    float LastSimulationTime = 0.0f;

    UPROPERTY()
    int32 LastCollisionPairs = 0;

    /** Physics settings for different actor types */
    UPROPERTY()
    TMap<FString, float> PhysicsLODDistances;

private:
    void UpdatePhysicsMetrics();
    void ApplyPhysicsLOD();
    
    FTimerHandle PhysicsUpdateTimer;
    FTimerHandle PhysicsLODTimer;
};

/**
 * Destruction System Component
 * Handles environmental destruction for immersive dinosaur interactions
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDestructionSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDestructionSystemComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Configure destruction parameters */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetupDestruction(float DestructionThreshold, int32 MaxFragments);

    /** Trigger destruction at location */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactLocation, float ImpactForce);

    /** Check if object can be destroyed */
    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool CanBeDestroyed() const { return bCanBeDestroyed; }

    /** Enable/disable destruction */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionEnabled(bool bEnabled) { bCanBeDestroyed = bEnabled; }

protected:
    /** Destruction threshold force */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    float DestructionThreshold = 1000.0f;

    /** Maximum number of fragments */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    int32 MaxFragments = 50;

    /** Can this object be destroyed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    bool bCanBeDestroyed = true;

    /** Destruction effect to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    TObjectPtr<UParticleSystem> DestructionEffect;

    /** Sound to play on destruction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    TObjectPtr<USoundBase> DestructionSound;

private:
    void RegisterWithPhysicsSystem();
    void UnregisterFromPhysicsSystem();
};

/**
 * Ragdoll System Component
 * Handles realistic character physics for death and unconsciousness states
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URagdollSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URagdollSystemComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Activate ragdoll physics */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpulseLocation = FVector::ZeroVector, float ImpulseStrength = 0.0f);

    /** Deactivate ragdoll and return to normal physics */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    /** Check if ragdoll is currently active */
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return bRagdollActive; }

    /** Configure ragdoll parameters */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ConfigureRagdoll(float Mass, float LinearDamping, float AngularDamping);

protected:
    /** Is ragdoll currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    bool bRagdollActive = false;

    /** Ragdoll mass multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollMass = 1.0f;

    /** Linear damping for ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    /** Angular damping for ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    /** Auto-deactivate ragdoll after time */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float AutoDeactivateTime = 10.0f;

private:
    void RegisterWithPhysicsSystem();
    void UnregisterFromPhysicsSystem();
    void AutoDeactivateRagdoll();

    FTimerHandle AutoDeactivateTimer;
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
};

/**
 * Collision System Component
 * Optimized collision detection for large-scale dinosaur simulation
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCollisionSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCollisionSystemComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Setup collision for dinosaur */
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupDinosaurCollision(float CollisionRadius, float CollisionHeight);

    /** Setup collision for environment */
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupEnvironmentCollision();

    /** Enable/disable collision */
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionEnabled(bool bEnabled);

    /** Configure collision LOD based on distance */
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupCollisionLOD(float NearDistance, float FarDistance);

    /** Get collision bounds */
    UFUNCTION(BlueprintPure, Category = "Collision")
    FVector GetCollisionBounds() const;

protected:
    /** Collision radius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    float CollisionRadius = 100.0f;

    /** Collision height */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    float CollisionHeight = 200.0f;

    /** Collision LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    float NearLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    float FarLODDistance = 5000.0f;

    /** Current collision LOD level */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    int32 CurrentLODLevel = 0;

private:
    void RegisterWithPhysicsSystem();
    void UnregisterFromPhysicsSystem();
    void UpdateCollisionLOD();

    FTimerHandle LODUpdateTimer;
};