#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

class USkeletalMeshComponent;
class UPhysicsAsset;
class UAnimInstance;

/**
 * Core_RagdollSystem - Advanced ragdoll physics management for dinosaurs and characters
 * Handles realistic death animations, impact responses, and physics-based character reactions
 * Optimized for performance with LOD-based activation and memory management
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === RAGDOLL ACTIVATION ===
    
    /** Activate ragdoll physics on the target skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void ActivateRagdoll(USkeletalMeshComponent* TargetMesh, bool bPreserveVelocity = true);
    
    /** Deactivate ragdoll and return to animation */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void DeactivateRagdoll(USkeletalMeshComponent* TargetMesh, float BlendTime = 1.0f);
    
    /** Apply impact force to ragdoll at specific bone */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void ApplyImpactForce(USkeletalMeshComponent* TargetMesh, FName BoneName, FVector Force, FVector Location);

    // === RAGDOLL CONFIGURATION ===
    
    /** Configure ragdoll physics properties */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void ConfigureRagdollPhysics(USkeletalMeshComponent* TargetMesh, float Mass = 80.0f, float LinearDamping = 0.1f, float AngularDamping = 0.1f);
    
    /** Set ragdoll bone constraints */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void SetBoneConstraints(USkeletalMeshComponent* TargetMesh, FName BoneName, bool bLockPosition, bool bLockRotation);

    // === PERFORMANCE OPTIMIZATION ===
    
    /** Check if ragdoll should be active based on distance */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    bool ShouldActivateRagdoll(AActor* TargetActor) const;
    
    /** Update ragdoll LOD based on distance */
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void UpdateRagdollLOD(USkeletalMeshComponent* TargetMesh, float DistanceToPlayer);

protected:
    // === RAGDOLL PROPERTIES ===
    
    /** Maximum distance for ragdoll activation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    float MaxRagdollDistance;
    
    /** Minimum impact force required to activate ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    float MinImpactForce;
    
    /** Default ragdoll mass */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    float DefaultRagdollMass;
    
    /** Ragdoll linear damping */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    float RagdollLinearDamping;
    
    /** Ragdoll angular damping */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    float RagdollAngularDamping;

    // === RAGDOLL STATE ===
    
    /** Currently active ragdolls */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    TArray<USkeletalMeshComponent*> ActiveRagdolls;
    
    /** Ragdoll activation timestamps */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    TMap<USkeletalMeshComponent*, float> RagdollActivationTimes;
    
    /** Maximum number of simultaneous ragdolls */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    int32 MaxSimultaneousRagdolls;

    // === PERFORMANCE MONITORING ===
    
    /** Current ragdoll count */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    int32 CurrentRagdollCount;
    
    /** Ragdoll performance budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics|Ragdoll", meta = (AllowPrivateAccess = "true"))
    float RagdollPerformanceBudget;

private:
    // === INTERNAL METHODS ===
    
    /** Internal ragdoll activation */
    void InternalActivateRagdoll(USkeletalMeshComponent* TargetMesh);
    
    /** Internal ragdoll deactivation */
    void InternalDeactivateRagdoll(USkeletalMeshComponent* TargetMesh);
    
    /** Cleanup inactive ragdolls */
    void CleanupInactiveRagdolls();
    
    /** Get player location for distance calculations */
    FVector GetPlayerLocation() const;
};