#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsArchitect.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPhysicsAsset;

/**
 * Physics architecture system for dinosaur survival gameplay
 * Manages collision channels, physics bodies, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable, Category = "Physics Architecture")
class TRANSPERSONALGAME_API APhysicsArchitect : public AActor
{
    GENERATED_BODY()

public:
    APhysicsArchitect();

protected:
    virtual void BeginPlay() override;

public:
    // === COLLISION CHANNEL MANAGEMENT ===
    
    /** Setup all required collision channels for dinosaur gameplay */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Collision Setup")
    void InitializeCollisionChannels();
    
    /** Configure collision responses for dinosaur interactions */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Collision Setup")
    void SetupDinosaurCollisionResponses();
    
    /** Setup player collision with environment and dinosaurs */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Collision Setup")
    void SetupPlayerCollisionResponses();

    // === PHYSICS BODY CONFIGURATION ===
    
    /** Configure physics bodies for large dinosaurs (T-Rex, Brachiosaurus) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Bodies")
    void ConfigureLargeDinosaurPhysics();
    
    /** Configure physics bodies for small dinosaurs (Velociraptor, Compsognathus) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Bodies")
    void ConfigureSmallDinosaurPhysics();
    
    /** Setup ragdoll physics for dinosaur death animations */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Bodies")
    void SetupDinosaurRagdollPhysics();

    // === PERFORMANCE OPTIMIZATION ===
    
    /** Optimize physics simulation for large numbers of actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void OptimizePhysicsPerformance();
    
    /** Setup LOD system for physics bodies based on distance */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void SetupPhysicsLODSystem();
    
    /** Configure physics sleeping for inactive objects */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ConfigurePhysicsSleeping();

    // === VALIDATION AND TESTING ===
    
    /** Test collision detection between dinosaurs and environment */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool TestDinosaurCollisionDetection();
    
    /** Validate physics performance meets target framerates */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidatePhysicsPerformance();
    
    /** Check for physics body penetration issues */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    TArray<AActor*> FindPhysicsPenetrationIssues();

protected:
    // === COLLISION CHANNEL DEFINITIONS ===
    
    /** Collision channel for dinosaur bodies */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> DinosaurChannel;
    
    /** Collision channel for player character */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> PlayerChannel;
    
    /** Collision channel for environment objects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> EnvironmentChannel;
    
    /** Collision channel for projectiles (spears, rocks) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> ProjectileChannel;
    
    /** Collision channel for interaction objects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> InteractionChannel;

    // === PHYSICS SETTINGS ===
    
    /** Physics simulation accuracy for large dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float LargeDinosaurPhysicsAccuracy = 1.0f;
    
    /** Physics simulation accuracy for small dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float SmallDinosaurPhysicsAccuracy = 0.8f;
    
    /** Maximum physics simulation distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float MaxPhysicsSimulationDistance = 5000.0f;
    
    /** Physics sleeping threshold */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float PhysicsSleepThreshold = 0.1f;

    // === PERFORMANCE LIMITS ===
    
    /** Maximum number of active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits")
    int32 MaxActivePhysicsBodies = 1000;
    
    /** Target physics simulation framerate */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits")
    float TargetPhysicsFramerate = 60.0f;
    
    /** Maximum physics substeps per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits")
    int32 MaxPhysicsSubsteps = 6;

private:
    // === INTERNAL SYSTEMS ===
    
    /** Initialize collision profiles for different object types */
    void InitializeCollisionProfiles();
    
    /** Setup physics materials for different surfaces */
    void SetupPhysicsMaterials();
    
    /** Configure physics constraints for ragdoll systems */
    void SetupPhysicsConstraints();
    
    /** Monitor physics performance and adjust settings */
    void MonitorPhysicsPerformance();
    
    // === VALIDATION HELPERS ===
    
    /** Check if actor has valid physics setup */
    bool ValidateActorPhysics(AActor* Actor) const;
    
    /** Get physics complexity score for actor */
    float GetPhysicsComplexityScore(AActor* Actor) const;
    
    /** Check for common physics issues */
    bool HasPhysicsIssues(AActor* Actor) const;
    
    // === CACHED DATA ===
    
    /** Cache of validated physics actors */
    UPROPERTY()
    TArray<AActor*> ValidatedPhysicsActors;
    
    /** Performance metrics history */
    UPROPERTY()
    TArray<float> PhysicsPerformanceHistory;
    
    /** Last validation timestamp */
    UPROPERTY()
    float LastValidationTime;
};