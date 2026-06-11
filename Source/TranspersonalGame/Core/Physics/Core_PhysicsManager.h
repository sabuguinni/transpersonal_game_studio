#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/WorldSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core_PhysicsManager.generated.h"

/**
 * Physics simulation quality levels for performance scaling
 */
UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"), 
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

/**
 * Ragdoll activation modes for character physics
 */
UENUM(BlueprintType)
enum class ECore_RagdollMode : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Partial     UMETA(DisplayName = "Partial Ragdoll"),
    Full        UMETA(DisplayName = "Full Ragdoll"),
    Blended     UMETA(DisplayName = "Animation Blended")
};

/**
 * Physics material surface types for different terrain
 */
UENUM(BlueprintType)
enum class ECore_SurfaceType : uint8
{
    Rock        UMETA(DisplayName = "Rock Surface"),
    Dirt        UMETA(DisplayName = "Dirt Surface"),
    Grass       UMETA(DisplayName = "Grass Surface"),
    Wood        UMETA(DisplayName = "Wood Surface"),
    Water       UMETA(DisplayName = "Water Surface"),
    Ice         UMETA(DisplayName = "Ice Surface"),
    Sand        UMETA(DisplayName = "Sand Surface"),
    Mud         UMETA(DisplayName = "Mud Surface")
};

/**
 * Physics simulation parameters for world-wide settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    /** Global gravity multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float GravityMultiplier = 1.0f;

    /** Physics simulation quality level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsQuality QualityLevel = ECore_PhysicsQuality::High;

    /** Maximum physics simulation distance from player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float MaxSimulationDistance = 3000.0f;

    /** Enable advanced collision detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableAdvancedCollision = true;

    /** Enable physics material effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsMaterials = true;

    FCore_PhysicsSettings()
    {
        GravityMultiplier = 1.0f;
        QualityLevel = ECore_PhysicsQuality::High;
        MaxSimulationDistance = 3000.0f;
        bEnableAdvancedCollision = true;
        bEnablePhysicsMaterials = true;
    }
};

/**
 * Ragdoll configuration for character physics simulation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    /** Ragdoll activation mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    ECore_RagdollMode RagdollMode = ECore_RagdollMode::Disabled;

    /** Time to blend from animation to ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BlendInTime = 0.2f;

    /** Time to blend from ragdoll back to animation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BlendOutTime = 0.5f;

    /** Minimum impact velocity to trigger ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float MinImpactVelocity = 500.0f;

    /** Maximum ragdoll duration before recovery */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float MaxRagdollDuration = 5.0f;

    FCore_RagdollConfig()
    {
        RagdollMode = ECore_RagdollMode::Disabled;
        BlendInTime = 0.2f;
        BlendOutTime = 0.5f;
        MinImpactVelocity = 500.0f;
        MaxRagdollDuration = 5.0f;
    }
};

/**
 * Core Physics Manager - World Subsystem for managing physics simulation
 * Handles physics quality scaling, ragdoll systems, and collision optimization
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

    // Physics Settings Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsQuality(ECore_PhysicsQuality NewQuality);

    UFUNCTION(BlueprintPure, Category = "Physics")
    ECore_PhysicsQuality GetPhysicsQuality() const { return PhysicsSettings.QualityLevel; }

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGravityMultiplier(float Multiplier);

    UFUNCTION(BlueprintPure, Category = "Physics")
    float GetGravityMultiplier() const { return PhysicsSettings.GravityMultiplier; }

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool ActivateRagdoll(USkeletalMeshComponent* MeshComponent, const FCore_RagdollConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool DeactivateRagdoll(USkeletalMeshComponent* MeshComponent, float BlendTime = 0.5f);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive(USkeletalMeshComponent* MeshComponent) const;

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool PerformAdvancedLineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, 
        const TArray<AActor*>& IgnoreActors = TArray<AActor*>()) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    TArray<FHitResult> PerformMultiLineTrace(const FVector& Start, const FVector& End, 
        const TArray<AActor*>& IgnoreActors = TArray<AActor*>()) const;

    // Physics Materials
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsMaterial(UPrimitiveComponent* Component, ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintPure, Category = "Physics")
    UPhysicalMaterial* GetPhysicsMaterialForSurface(ECore_SurfaceType SurfaceType) const;

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsForDistance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxSimulationDistance(float Distance);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TogglePhysicsDebugVisualization(bool bEnabled);

protected:
    /** Current physics settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    FCore_PhysicsSettings PhysicsSettings;

    /** Physics materials for different surface types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    TMap<ECore_SurfaceType, TSoftObjectPtr<UPhysicalMaterial>> PhysicsMaterials;

    /** Active ragdoll components */
    UPROPERTY()
    TMap<USkeletalMeshComponent*, FCore_RagdollConfig> ActiveRagdolls;

    /** Timer handle for physics optimization updates */
    FTimerHandle OptimizationTimerHandle;

private:
    // Internal helper functions
    void ApplyPhysicsQualitySettings();
    void UpdatePhysicsSimulation();
    void CleanupInactiveRagdolls();
    
    // Physics material initialization
    void InitializePhysicsMaterials();
    
    // Performance monitoring
    void MonitorPhysicsPerformance();
    
    /** Last player location for distance-based optimization */
    FVector LastPlayerLocation;
    
    /** Physics debug visualization enabled */
    bool bDebugVisualizationEnabled = false;
};