#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalPhysics, Log, All);

// Forward declarations
class UDestructionSystemComponent;
class URagdollSystemComponent;
class UCollisionSystemComponent;

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