#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Core_PhysicsManager.generated.h"

/**
 * Dinosaur-specific physics configuration for realistic prehistoric survival gameplay
 * Manages mass, collision, and physics properties for all dinosaur species
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DinosaurPhysicsConfig
{
    GENERATED_BODY()

    // Realistic mass values based on paleontological data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MassKg = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECollisionResponse CollisionResponse = ECollisionResponse::ECR_Block;

    FCore_DinosaurPhysicsConfig()
    {
        MassKg = 1000.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        CollisionResponse = ECollisionResponse::ECR_Block;
    }
};

/**
 * Core Physics Manager - Handles all physics simulation for the prehistoric world
 * Manages dinosaur physics, environmental destruction, and collision systems
 * Integrates with Engine Architect performance budgets (3ms frame time limit)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /**
     * Configure physics properties for a dinosaur actor
     * @param DinosaurActor - The dinosaur actor to configure
     * @param Species - Dinosaur species for mass/physics lookup
     * @return true if configuration was successful
     */
    UFUNCTION(BlueprintCallable, Category = "Physics", CallInEditor)
    bool ConfigureDinosaurPhysics(AActor* DinosaurActor, ECore_DinosaurSpecies Species);

    /**
     * Apply realistic physics impulse to a dinosaur (for combat, falls, impacts)
     * @param DinosaurActor - Target dinosaur
     * @param ImpulseVector - Force vector in Newtons
     * @param bVelChange - Whether to treat as velocity change
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyDinosaurImpulse(AActor* DinosaurActor, FVector ImpulseVector, bool bVelChange = false);

    /**
     * Enable/disable physics simulation for a dinosaur
     * @param DinosaurActor - Target dinosaur
     * @param bEnable - Enable or disable physics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetDinosaurPhysicsEnabled(AActor* DinosaurActor, bool bEnable);

    /**
     * Get physics configuration for a specific dinosaur species
     * @param Species - Dinosaur species
     * @return Physics configuration struct
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics")
    FCore_DinosaurPhysicsConfig GetSpeciesPhysicsConfig(ECore_DinosaurSpecies Species) const;

    /**
     * Register a physics actor with the manager for performance tracking
     * @param PhysicsActor - Actor with physics simulation
     * @param Priority - Update priority (0=highest, 10=lowest)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RegisterPhysicsActor(AActor* PhysicsActor, int32 Priority = 5);

    /**
     * Unregister a physics actor from the manager
     * @param PhysicsActor - Actor to remove from tracking
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UnregisterPhysicsActor(AActor* PhysicsActor);

    /**
     * Get current physics performance metrics
     * @param OutFrameTimeMs - Current frame time in milliseconds
     * @param OutActiveActors - Number of active physics actors
     * @param OutBudgetUsage - Percentage of performance budget used
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void GetPhysicsMetrics(float& OutFrameTimeMs, int32& OutActiveActors, float& OutBudgetUsage) const;

    /**
     * Force physics optimization (reduce quality for performance)
     * Called automatically when budget exceeds 3ms
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void OptimizePhysicsPerformance();

    /**
     * Restore full physics quality
     * Called when performance budget allows
     */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RestorePhysicsQuality();

protected:
    // Dinosaur species physics configurations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    TMap<ECore_DinosaurSpecies, FCore_DinosaurPhysicsConfig> SpeciesConfigs;

    // Registered physics actors for performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<AActor>> RegisteredPhysicsActors;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    float PhysicsBudgetMs;

    UPROPERTY(BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    bool bOptimizationActive;

private:
    /**
     * Initialize default physics configurations for all dinosaur species
     */
    void InitializeSpeciesConfigs();

    /**
     * Configure physics component with species-specific settings
     * @param Component - Physics component to configure
     * @param Config - Physics configuration to apply
     */
    void ApplyPhysicsConfig(UPrimitiveComponent* Component, const FCore_DinosaurPhysicsConfig& Config);

    /**
     * Update performance metrics (called per frame)
     */
    void UpdatePerformanceMetrics();

    // Performance budget enforcement (3ms from Engine Architect)
    static constexpr float MAX_PHYSICS_BUDGET_MS = 3.0f;
    
    // Species mass data based on paleontological research
    static constexpr float TREX_MASS_KG = 7000.0f;        // 7 tons
    static constexpr float VELOCIRAPTOR_MASS_KG = 15.0f;  // 15 kg
    static constexpr float BRACHIOSAURUS_MASS_KG = 50000.0f; // 50 tons
    static constexpr float TRICERATOPS_MASS_KG = 12000.0f;   // 12 tons
    static constexpr float ANKYLOSAURUS_MASS_KG = 6000.0f;   // 6 tons
};