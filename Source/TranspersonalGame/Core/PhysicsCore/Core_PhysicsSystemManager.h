#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/PhysicsVolume.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Static UMETA(DisplayName = "Static"),
    Dynamic UMETA(DisplayName = "Dynamic"),
    Kinematic UMETA(DisplayName = "Kinematic"),
    Ragdoll UMETA(DisplayName = "Ragdoll")
};

UENUM(BlueprintType)
enum class ECore_DestructionLevel : uint8
{
    None UMETA(DisplayName = "None"),
    Light UMETA(DisplayName = "Light"),
    Medium UMETA(DisplayName = "Medium"),
    Heavy UMETA(DisplayName = "Heavy"),
    Complete UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Static;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    FCore_PhysicsProfile()
    {
        PhysicsMode = ECore_PhysicsMode::Static;
        Mass = 100.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        bEnableGravity = true;
        Restitution = 0.3f;
        Friction = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionLevel DestructionLevel = ECore_DestructionLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float HealthPoints = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 FragmentCount = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDebris = true;

    FCore_DestructionProfile()
    {
        DestructionLevel = ECore_DestructionLevel::None;
        HealthPoints = 100.0f;
        ImpactThreshold = 500.0f;
        FragmentCount = 10;
        bCreateDebris = true;
    }
};

/**
 * Core Physics System Manager
 * Manages physics simulation, ragdoll physics, and destruction systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyPhysicsProfile(UStaticMeshComponent* MeshComponent, const FCore_PhysicsProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableRagdollPhysics(USkeletalMeshComponent* SkeletalMeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisableRagdollPhysics(USkeletalMeshComponent* SkeletalMeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyImpulse(UPrimitiveComponent* Component, const FVector& Impulse, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyForce(UPrimitiveComponent* Component, const FVector& Force);

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void RegisterDestructibleObject(AActor* Actor, const FCore_DestructionProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void TriggerDestruction(AActor* Actor, float DamageAmount, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    bool CanObjectBeDestroyed(AActor* Actor, float DamageAmount);

    // Physics Constraints
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UPhysicsConstraintComponent* CreatePhysicsConstraint(AActor* Actor1, AActor* Actor2, const FVector& ConstraintLocation);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void BreakPhysicsConstraint(UPhysicsConstraintComponent* Constraint);

    // Physics Materials
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UPhysicalMaterial* GetPhysicsMaterial(const FString& MaterialName);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyPhysicsMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* Material);

    // Physics Validation
    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor = true)
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor = true)
    void CreatePhysicsTestScene();

protected:
    // Physics profiles storage
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    TMap<AActor*, FCore_PhysicsProfile> PhysicsProfiles;

    // Destruction profiles storage
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    TMap<AActor*, FCore_DestructionProfile> DestructionProfiles;

    // Physics materials cache
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    TMap<FString, UPhysicalMaterial*> PhysicsMaterials;

    // Active constraints
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    TArray<UPhysicsConstraintComponent*> ActiveConstraints;

private:
    void InitializePhysicsMaterials();
    void CreateDefaultPhysicsProfiles();
    void SetupPhysicsCollisionChannels();
    void ValidatePhysicsComponents();
    
    // Destruction helpers
    void CreateDestructionFragments(AActor* Actor, const FCore_DestructionProfile& Profile, const FVector& ImpactLocation);
    void SpawnDebrisParticles(const FVector& Location, int32 FragmentCount);
};