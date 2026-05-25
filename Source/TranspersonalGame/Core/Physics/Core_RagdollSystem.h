#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Core_RagdollSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRagdollSystem, Log, All);

/**
 * Ragdoll transition types for different death scenarios
 */
UENUM(BlueprintType)
enum class ECore_RagdollTransition : uint8
{
    None            UMETA(DisplayName = "None"),
    InstantDeath    UMETA(DisplayName = "Instant Death"),
    GradualCollapse UMETA(DisplayName = "Gradual Collapse"),
    ImpactDeath     UMETA(DisplayName = "Impact Death"),
    Unconscious     UMETA(DisplayName = "Unconscious")
};

/**
 * Ragdoll physics configuration for different character types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    /** Mass scale for ragdoll physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MassScale;

    /** Linear damping for ragdoll bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping;

    /** Angular damping for ragdoll bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping;

    /** Maximum angular velocity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxAngularVelocity;

    /** Collision response for ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionResponse> CollisionResponse;

    /** Time before ragdoll cleanup */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifecycle")
    float CleanupTime;

    FCore_RagdollConfig()
    {
        MassScale = 1.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        MaxAngularVelocity = 3600.0f;
        CollisionResponse = ECR_Block;
        CleanupTime = 30.0f;
    }
};

/**
 * Core Ragdoll System - Manages realistic physics-based character death and unconsciousness
 * 
 * Handles the transition from animated character to physics-driven ragdoll for:
 * - Character death scenarios (instant, gradual, impact-based)
 * - Unconsciousness states with recovery potential
 * - Dinosaur death physics with species-specific configurations
 * - Performance optimization through LOD and cleanup systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UObject
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

    /**
     * Initialize ragdoll system with character mesh component
     * @param InMeshComponent - The skeletal mesh component to manage
     * @param InConfig - Ragdoll configuration settings
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void InitializeRagdoll(USkeletalMeshComponent* InMeshComponent, const FCore_RagdollConfig& InConfig);

    /**
     * Activate ragdoll physics with specified transition type
     * @param TransitionType - How the ragdoll should activate
     * @param ImpactForce - Optional force to apply during activation
     * @param ImpactLocation - World location where force should be applied
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void ActivateRagdoll(ECore_RagdollTransition TransitionType, const FVector& ImpactForce = FVector::ZeroVector, const FVector& ImpactLocation = FVector::ZeroVector);

    /**
     * Deactivate ragdoll and return to animated state (for unconsciousness recovery)
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void DeactivateRagdoll();

    /**
     * Check if ragdoll is currently active
     */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    bool IsRagdollActive() const { return bRagdollActive; }

    /**
     * Get current ragdoll transition type
     */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    ECore_RagdollTransition GetTransitionType() const { return CurrentTransition; }

    /**
     * Force cleanup of ragdoll (immediate removal)
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void ForceCleanup();

    /**
     * Update ragdoll system (called from tick)
     * @param DeltaTime - Time since last update
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void UpdateRagdoll(float DeltaTime);

    /**
     * Set ragdoll configuration at runtime
     * @param NewConfig - New configuration to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    /**
     * Get current ragdoll configuration
     */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    FCore_RagdollConfig GetRagdollConfig() const { return RagdollConfig; }

protected:
    /** Skeletal mesh component being managed */
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    /** Current ragdoll configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCore_RagdollConfig RagdollConfig;

    /** Whether ragdoll is currently active */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bRagdollActive;

    /** Current transition type */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECore_RagdollTransition CurrentTransition;

    /** Time since ragdoll activation */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float RagdollActiveTime;

    /** Original collision settings (for restoration) */
    UPROPERTY()
    TEnumAsByte<ECollisionResponse> OriginalCollisionResponse;

    /** Original physics settings (for restoration) */
    UPROPERTY()
    bool bOriginalSimulatePhysics;

private:
    /**
     * Apply physics settings based on transition type
     * @param TransitionType - The type of ragdoll transition
     */
    void ApplyTransitionSettings(ECore_RagdollTransition TransitionType);

    /**
     * Configure physics bodies for ragdoll
     */
    void ConfigurePhysicsBodies();

    /**
     * Restore original animation state
     */
    void RestoreAnimationState();

    /**
     * Handle cleanup timer
     */
    void HandleCleanupTimer(float DeltaTime);

    /**
     * Apply impact force to ragdoll
     * @param Force - Force vector to apply
     * @param Location - World location to apply force
     */
    void ApplyImpactForce(const FVector& Force, const FVector& Location);

    /**
     * Validate mesh component and physics asset
     */
    bool ValidateRagdollSetup() const;
};