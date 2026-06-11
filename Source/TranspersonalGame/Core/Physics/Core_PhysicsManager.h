#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Transitioning   UMETA(DisplayName = "Transitioning"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float TransitionTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollTime = 10.0f;

    FCore_RagdollSettings()
    {
        TransitionTime = 0.2f;
        RecoveryTime = 1.0f;
        ImpactThreshold = 500.0f;
        bAutoRecover = true;
        MaxRagdollTime = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    FCore_PhysicsProfile()
    {
        Mass = 75.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        GravityScale = 1.0f;
    }
};

/**
 * Core Physics Manager - Handles ragdoll physics, destruction, and advanced physics interactions
 * Manages physics state transitions, impact responses, and performance optimization
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

    // Ragdoll Management
    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    bool EnableRagdoll(USkeletalMeshComponent* MeshComponent, const FVector& ImpactForce = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    bool DisableRagdoll(USkeletalMeshComponent* MeshComponent, bool bBlendOut = true);

    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    bool IsRagdollActive(USkeletalMeshComponent* MeshComponent) const;

    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    ECore_RagdollState GetRagdollState(USkeletalMeshComponent* MeshComponent) const;

    // Physics Profiles
    UFUNCTION(BlueprintCallable, Category = "Physics|Profiles")
    void ApplyPhysicsProfile(UPrimitiveComponent* Component, const FCore_PhysicsProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Physics|Profiles")
    FCore_PhysicsProfile GetPhysicsProfile(UPrimitiveComponent* Component) const;

    // Impact and Collision
    UFUNCTION(BlueprintCallable, Category = "Physics|Impact")
    void HandleImpact(UPrimitiveComponent* HitComponent, const FVector& ImpactPoint, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics|Impact")
    bool ShouldTriggerRagdoll(const FVector& ImpactForce, float Threshold = 500.0f) const;

    // Physics Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics|Optimization")
    void OptimizePhysicsComponents(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics|Optimization")
    void SetPhysicsLOD(UPrimitiveComponent* Component, int32 LODLevel);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Physics|Debug", CallInEditor)
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Physics|Debug", CallInEditor)
    void DebugDrawPhysicsInfo(bool bEnabled = true);

protected:
    // Ragdoll tracking
    UPROPERTY()
    TMap<USkeletalMeshComponent*, ECore_RagdollState> RagdollStates;

    UPROPERTY()
    TMap<USkeletalMeshComponent*, float> RagdollTimers;

    // Default settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FCore_RagdollSettings DefaultRagdollSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FCore_PhysicsProfile DefaultPhysicsProfile;

    // Performance tracking
    UPROPERTY()
    TArray<UPrimitiveComponent*> ActivePhysicsComponents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActivePhysicsComponents = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float PhysicsOptimizationInterval = 1.0f;

    float LastOptimizationTime = 0.0f;

private:
    // Internal ragdoll management
    void UpdateRagdollStates(float DeltaTime);
    void TransitionRagdollState(USkeletalMeshComponent* MeshComponent, ECore_RagdollState NewState);
    
    // Physics optimization helpers
    void CullDistantPhysicsComponents();
    void AdjustPhysicsComplexity(UPrimitiveComponent* Component, float Distance);
    
    // Validation helpers
    bool ValidatePhysicsAsset(UPhysicsAsset* PhysicsAsset) const;
    bool ValidateCollisionSettings(UPrimitiveComponent* Component) const;

    // Timer handle for periodic updates
    FTimerHandle PhysicsUpdateTimer;
};