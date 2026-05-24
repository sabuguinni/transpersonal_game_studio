#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Core_RagdollSystem.generated.h"

/**
 * Core Ragdoll Physics System
 * Handles realistic ragdoll physics for dinosaurs and characters
 * Manages death animations, physics transitions, and body simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UObject
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

    // Core ragdoll functions
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void EnableRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void DisableRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const;

    // Dinosaur-specific ragdoll
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Dinosaur")
    void EnableDinosaurRagdoll(ACharacter* DinosaurCharacter, float ImpactForce = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Dinosaur")
    void SetDinosaurRagdollProperties(USkeletalMeshComponent* SkeletalMesh, float Mass = 500.0f, float Damping = 0.1f);

    // Physics impulse and forces
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Forces")
    void ApplyRagdollImpulse(USkeletalMeshComponent* SkeletalMesh, FVector Impulse, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Forces")
    void ApplyRadialImpulse(USkeletalMeshComponent* SkeletalMesh, FVector Origin, float Radius, float Strength);

protected:
    // Ragdoll configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float DefaultRagdollMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float DefaultLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float DefaultAngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    bool bEnableGravity;

    // Dinosaur-specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    TMap<FString, float> DinosaurMassOverrides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float TRexMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float VelociraptorMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float BrachiosaurusMass;

private:
    // Internal ragdoll management
    void ConfigureRagdollPhysics(USkeletalMeshComponent* SkeletalMesh);
    void StoreAnimationState(USkeletalMeshComponent* SkeletalMesh);
    void RestoreAnimationState(USkeletalMeshComponent* SkeletalMesh);

    // Stored animation states
    UPROPERTY()
    TMap<USkeletalMeshComponent*, UAnimInstance*> StoredAnimInstances;
};