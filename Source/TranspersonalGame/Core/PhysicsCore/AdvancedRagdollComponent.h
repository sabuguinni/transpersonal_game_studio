#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "AdvancedRagdollComponent.generated.h"

/**
 * @brief Advanced ragdoll physics component with LOD and performance optimization
 * 
 * This component provides enhanced ragdoll physics simulation with:
 * - Multiple LOD levels for performance scaling
 * - Intelligent bone culling based on distance
 * - Smooth transitions between animated and ragdoll states
 * - Performance monitoring and automatic quality adjustment
 * 
 * Key features:
 * - LOD 0: Full ragdoll simulation with all bones
 * - LOD 1: Reduced bone count, simplified constraints
 * - LOD 2: Minimal simulation or static pose
 * 
 * @author Core Systems Programmer - Agent #03
 * @version 1.0
 * @date 2024
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdvancedRagdollComponent : public USkeletalMeshComponent
{
    GENERATED_BODY()

public:
    UAdvancedRagdollComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Activate ragdoll physics with specified intensity
     * @param RagdollIntensity Strength of ragdoll activation (0.0 = none, 1.0 = full)
     * @param bBlendFromAnimation Whether to blend from current animation pose
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(float RagdollIntensity = 1.0f, bool bBlendFromAnimation = true);

    /**
     * @brief Deactivate ragdoll and return to animation
     * @param BlendTime Time to blend back to animation
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll(float BlendTime = 1.0f);

    /**
     * @brief Set physics LOD level for performance optimization
     * @param LODLevel 0 = full detail, 1 = medium, 2 = minimal
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetPhysicsLODLevel(int32 LODLevel);

    /**
     * @brief Apply impulse to specific bone
     * @param BoneName Name of the bone to apply impulse to
     * @param Impulse Impulse vector to apply
     * @param bVelChange Whether to treat as velocity change
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToBone(FName BoneName, FVector Impulse, bool bVelChange = false);

    /**
     * @brief Get current ragdoll state
     * @return True if ragdoll is currently active
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const { return bRagdollActive; }

    /**
     * @brief Get current physics LOD level
     * @return Current LOD level (0-2)
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    int32 GetCurrentLODLevel() const { return CurrentLODLevel; }

    /**
     * @brief Set ragdoll damping parameters
     * @param LinearDamping Linear damping factor
     * @param AngularDamping Angular damping factor
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollDamping(float LinearDamping, float AngularDamping);

protected:
    /** Whether ragdoll physics is currently active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bRagdollActive;

    /** Current physics LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    int32 CurrentLODLevel;

    /** Ragdoll blend weight (0.0 = animation, 1.0 = ragdoll) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendWeight;

    /** Time to blend between animation and ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime;

    /** Linear damping for ragdoll bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping;

    /** Angular damping for ragdoll bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping;

    /** Bones to exclude at different LOD levels */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll LOD")
    TArray<FName> LOD1ExcludedBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll LOD")
    TArray<FName> LOD2ExcludedBones;

    /** Minimum time between LOD changes to prevent thrashing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll LOD")
    float LODChangeMinInterval;

private:
    /** Target blend weight for smooth transitions */
    float TargetBlendWeight;

    /** Current blend speed */
    float BlendSpeed;

    /** Last time LOD was changed */
    float LastLODChangeTime;

    /** Cached animation pose for blending */
    FPoseSnapshot CachedAnimationPose;

    /** Whether we have a valid cached pose */
    bool bHasCachedPose;

    /**
     * @brief Update ragdoll blending
     */
    void UpdateRagdollBlending(float DeltaTime);

    /**
     * @brief Apply LOD-specific bone exclusions
     */
    void ApplyLODBoneExclusions();

    /**
     * @brief Cache current animation pose for blending
     */
    void CacheAnimationPose();

    /**
     * @brief Apply cached pose with specified weight
     */
    void ApplyCachedPose(float Weight);

    /**
     * @brief Update physics properties based on current settings
     */
    void UpdatePhysicsProperties();
};