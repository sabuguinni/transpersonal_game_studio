#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsSimulationType : uint8
{
    Static UMETA(DisplayName = "Static"),
    Kinematic UMETA(DisplayName = "Kinematic"),
    Dynamic UMETA(DisplayName = "Dynamic"),
    Ragdoll UMETA(DisplayName = "Ragdoll")
};

UENUM(BlueprintType)
enum class ECore_CollisionPreset : uint8
{
    Default UMETA(DisplayName = "Default"),
    BlockAll UMETA(DisplayName = "Block All"),
    OverlapAll UMETA(DisplayName = "Overlap All"),
    IgnoreAll UMETA(DisplayName = "Ignore All"),
    Pawn UMETA(DisplayName = "Pawn"),
    Trigger UMETA(DisplayName = "Trigger"),
    Projectile UMETA(DisplayName = "Projectile")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsSimulationType SimulationType = ECore_PhysicsSimulationType::Static;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_CollisionPreset CollisionPreset = ECore_CollisionPreset::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    FCore_PhysicsProperties()
    {
        SimulationType = ECore_PhysicsSimulationType::Static;
        Mass = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        bEnableGravity = true;
        CollisionPreset = ECore_CollisionPreset::Default;
        Restitution = 0.3f;
        Friction = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdoll = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneLinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneAngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneMassScale = 1.0f;

    FCore_RagdollSettings()
    {
        bEnableRagdoll = false;
        RagdollBlendTime = 0.2f;
        BoneLinearDamping = 0.1f;
        BoneAngularDamping = 0.1f;
        BoneMassScale = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDestruction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpulseStrength = 500.0f;

    FCore_DestructionSettings()
    {
        bEnableDestruction = false;
        DestructionThreshold = 100.0f;
        MaxFragments = 10;
        FragmentLifetime = 30.0f;
        ImpulseStrength = 500.0f;
    }
};

/**
 * Core Physics System Manager
 * Manages all physics simulation, ragdoll physics, and destruction systems
 * Handles collision detection, physics materials, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSimulation(float DeltaTime);

    // Actor Physics Configuration
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetActorPhysicsProperties(AActor* Actor, const FCore_PhysicsProperties& Properties);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FCore_PhysicsProperties GetActorPhysicsProperties(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnablePhysicsSimulation(AActor* Actor, bool bEnable = true);

    // Ragdoll Physics
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void EnableRagdoll(AActor* Actor, const FCore_RagdollSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DisableRagdoll(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool IsRagdollEnabled(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpulseToRagdoll(AActor* Actor, const FVector& Impulse, const FName& BoneName = NAME_None);

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionSettings(AActor* Actor, const FCore_DestructionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, const FVector& ImpactDirection, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanActorBeDestroyed(AActor* Actor, float Damage);

    // Collision and Physics Queries
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    TArray<AActor*> GetOverlappingActors(const FVector& Location, float Radius);

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsLOD(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsForDistance(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActivePhysicsActorCount();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void EnablePhysicsDebugVisualization(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DrawPhysicsDebugInfo(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsStats();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Physics")
    bool bIsInitialized = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    int32 MaxSimulatedActors = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsLODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsLODDistance2 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsLODDistance3 = 5000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bDebugVisualizationEnabled = false;

private:
    // Internal physics management
    void ConfigureCollisionPreset(UPrimitiveComponent* Component, ECore_CollisionPreset Preset);
    void ApplyPhysicsMaterial(UPrimitiveComponent* Component, const FCore_PhysicsProperties& Properties);
    void UpdatePhysicsLOD(AActor* Actor, float Distance);
    
    // Ragdoll helpers
    void ConfigureRagdollBone(USkeletalMeshComponent* SkeletalMesh, const FName& BoneName, const FCore_RagdollSettings& Settings);
    void BlendToRagdoll(USkeletalMeshComponent* SkeletalMesh, float BlendTime);
    
    // Destruction helpers
    void CreateDestructionFragments(AActor* Actor, const FVector& ImpactPoint, int32 FragmentCount);
    void ApplyFragmentPhysics(AActor* Fragment, const FVector& Impulse);

    // Performance tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RagdollActors;

    float LastPhysicsUpdateTime = 0.0f;
    int32 CurrentActiveActors = 0;
};