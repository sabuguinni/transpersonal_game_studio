#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "../SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

class UEngineArchitectureCore;

/**
 * Core Physics System Manager for Transpersonal Game Studio
 * Handles all physics simulation, collision detection, and material interactions
 * for the prehistoric survival game environment.
 * 
 * Key Features:
 * - Realistic physics simulation for dinosaurs and environment
 * - Dynamic collision detection and response
 * - Material-based physics properties (stone, wood, bone, flesh)
 * - Ragdoll physics for character death states
 * - Environmental destruction simulation
 * - Performance-optimized physics LOD system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APhysicsSystemManager : public AActor
{
    GENERATED_BODY()

public:
    APhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // === CORE PHYSICS MANAGEMENT ===
    
    /**
     * Initialize the physics system with world-specific parameters
     * @param World - The world to initialize physics for
     * @param bEnableAdvancedPhysics - Whether to enable advanced physics features
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem(UWorld* World, bool bEnableAdvancedPhysics = true);

    /**
     * Shutdown physics system and clean up resources
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ShutdownPhysicsSystem();

    /**
     * Register an object for physics simulation
     * @param Component - The primitive component to register
     * @param PhysicsType - Type of physics simulation to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterPhysicsObject(UPrimitiveComponent* Component, ECore_PhysicsType PhysicsType);

    /**
     * Unregister an object from physics simulation
     * @param Component - The component to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterPhysicsObject(UPrimitiveComponent* Component);

    // === COLLISION SYSTEM ===
    
    /**
     * Set up collision profiles for different object types
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void InitializeCollisionProfiles();

    /**
     * Handle collision between two objects
     * @param HitComponent - The component that was hit
     * @param OtherActor - The other actor in the collision
     * @param OtherComponent - The other component in the collision
     * @param HitResult - Detailed hit information
     */
    UFUNCTION()
    void HandleCollisionEvent(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                             UPrimitiveComponent* OtherComponent, const FHitResult& HitResult);

    /**
     * Apply collision response based on material types
     * @param HitResult - The collision hit result
     * @param ImpactForce - The force of the impact
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ApplyCollisionResponse(const FHitResult& HitResult, float ImpactForce);

    // === MATERIAL PHYSICS ===
    
    /**
     * Get physics properties for a specific material type
     * @param MaterialType - The type of material
     * @return Physics properties for the material
     */
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    FCore_PhysicsMaterial GetMaterialPhysicsProperties(ECore_MaterialType MaterialType);

    /**
     * Apply material-specific physics properties to a component
     * @param Component - The component to modify
     * @param MaterialType - The material type to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyMaterialPhysics(UPrimitiveComponent* Component, ECore_MaterialType MaterialType);

    // === RAGDOLL SYSTEM ===
    
    /**
     * Enable ragdoll physics on a character
     * @param Character - The character to enable ragdoll on
     * @param ImpactForce - Optional impact force to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void EnableRagdollPhysics(class ACharacter* Character, FVector ImpactForce = FVector::ZeroVector);

    /**
     * Disable ragdoll physics and return to animated state
     * @param Character - The character to disable ragdoll on
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void DisableRagdollPhysics(class ACharacter* Character);

    // === DESTRUCTION SYSTEM ===
    
    /**
     * Apply destruction to an object based on damage
     * @param Target - The object to potentially destroy
     * @param DamageAmount - Amount of damage applied
     * @param DamageLocation - World location where damage was applied
     * @return Whether the object was destroyed
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction System")
    bool ApplyDestructionDamage(AActor* Target, float DamageAmount, FVector DamageLocation);

    /**
     * Create debris from a destroyed object
     * @param OriginalActor - The actor that was destroyed
     * @param DestructionLocation - Where the destruction occurred
     * @param ExplosionForce - Force to apply to debris
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction System")
    void CreateDebris(AActor* OriginalActor, FVector DestructionLocation, float ExplosionForce);

    // === PERFORMANCE OPTIMIZATION ===
    
    /**
     * Update physics LOD based on distance and importance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsLOD();

    /**
     * Set physics simulation quality level
     * @param QualityLevel - 0=Low, 1=Medium, 2=High, 3=Ultra
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsQuality(int32 QualityLevel);

    /**
     * Get current physics performance metrics
     * @return Performance data structure
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PerformanceMetrics GetPhysicsPerformanceMetrics();

    // === SYSTEM INTEGRATION ===
    
    /**
     * Register with the Engine Architecture Core
     * @param ArchitectureCore - The core system to register with
     */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithArchitectureCore(UEngineArchitectureCore* ArchitectureCore);

    /**
     * Validate physics system health
     * @return True if system is healthy
     */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateSystemHealth();

protected:
    // === CORE PROPERTIES ===
    
    /** Reference to the world physics scene */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics System")
    UWorld* PhysicsWorld;

    /** Whether advanced physics features are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    bool bAdvancedPhysicsEnabled;

    /** Current physics quality level (0-3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "3"))
    int32 PhysicsQualityLevel;

    /** Maximum number of physics objects to simulate simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects;

    /** Distance threshold for physics LOD levels */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> PhysicsLODDistances;

    // === PHYSICS OBJECT TRACKING ===
    
    /** All registered physics objects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics System")
    TArray<UPrimitiveComponent*> RegisteredPhysicsObjects;

    /** Physics objects organized by type */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics System")
    TMap<ECore_PhysicsType, TArray<UPrimitiveComponent*>> PhysicsObjectsByType;

    /** Active ragdoll characters */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll System")
    TArray<class ACharacter*> ActiveRagdolls;

    // === MATERIAL PHYSICS PROPERTIES ===
    
    /** Physics properties for different material types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Physics")
    TMap<ECore_MaterialType, FCore_PhysicsMaterial> MaterialPhysicsMap;

    // === PERFORMANCE TRACKING ===
    
    /** Current frame's physics performance metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FCore_PerformanceMetrics CurrentPerformanceMetrics;

    /** Physics simulation time budget per frame (milliseconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTimeBudget;

    /** Reference to the Engine Architecture Core */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System Integration")
    UEngineArchitectureCore* ArchitectureCore;

private:
    // === INTERNAL METHODS ===
    
    /** Initialize default material physics properties */
    void InitializeMaterialPhysicsDefaults();

    /** Update performance metrics */
    void UpdatePerformanceMetrics(float DeltaTime);

    /** Clean up destroyed physics objects */
    void CleanupDestroyedObjects();

    /** Apply LOD settings to physics objects */
    void ApplyPhysicsLOD(UPrimitiveComponent* Component, float Distance);

    // === INTERNAL STATE ===
    
    /** Timer for performance metric updates */
    float PerformanceUpdateTimer;

    /** Timer for object cleanup */
    float CleanupTimer;

    /** Whether the system is currently initialized */
    bool bSystemInitialized;

    /** Frame counter for performance tracking */
    int32 FrameCounter;
};