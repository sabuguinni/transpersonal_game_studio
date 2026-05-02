#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    Simulating      UMETA(DisplayName = "Simulating"),
    Ragdoll         UMETA(DisplayName = "Ragdoll"),
    Destroyed       UMETA(DisplayName = "Destroyed")
};

UENUM(BlueprintType)
enum class ECore_CollisionPreset : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Player          UMETA(DisplayName = "Player"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Debris          UMETA(DisplayName = "Debris")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_CollisionPreset CollisionPreset = ECore_CollisionPreset::Default;

    FCore_PhysicsConfig()
    {
        Mass = 100.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        bSimulatePhysics = true;
        CollisionPreset = ECore_CollisionPreset::Default;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LifeSpan = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bPreserveVelocity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FVector ImpulseForce = FVector::ZeroVector;

    FCore_RagdollConfig()
    {
        BlendTime = 0.2f;
        LifeSpan = 10.0f;
        bPreserveVelocity = true;
        ImpulseForce = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FCore_DestructionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifeSpan = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionRadius = 500.0f;

    FCore_DestructionConfig()
    {
        MaxFragments = 20;
        FragmentLifeSpan = 30.0f;
        ExplosionForce = 1000.0f;
        ExplosionRadius = 500.0f;
    }
};

/**
 * Core Physics Manager - Handles physics simulation, ragdoll, and destruction systems
 * Manages all physics-related functionality for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics configuration
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ConfigurePhysicsForActor(AActor* Actor, const FCore_PhysicsConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsState(AActor* Actor, ECore_PhysicsState NewState);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    ECore_PhysicsState GetPhysicsState(AActor* Actor) const;

    // Ragdoll system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableRagdoll(AActor* Actor, const FCore_RagdollConfig& Config = FCore_RagdollConfig());

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisableRagdoll(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool IsRagdollActive(AActor* Actor) const;

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DestroyWithFragments(AActor* Actor, const FCore_DestructionConfig& Config = FCore_DestructionConfig());

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyExplosionForce(const FVector& Location, float Force, float Radius);

    // Collision presets
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyCollisionPreset(UPrimitiveComponent* Component, ECore_CollisionPreset Preset);

    // Physics validation and debugging
    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor)
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void LogPhysicsStats();

protected:
    // Internal physics state tracking
    UPROPERTY()
    TMap<AActor*, ECore_PhysicsState> PhysicsStateMap;

    // Configuration storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsConfig DefaultPhysicsConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    FCore_RagdollConfig DefaultRagdollConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionConfig DefaultDestructionConfig;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    int32 ActiveRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    float LastPhysicsUpdateTime;

private:
    // Internal helper functions
    void SetupCollisionChannels();
    void ConfigurePhysicsAsset(USkeletalMeshComponent* SkeletalMesh, const FCore_PhysicsConfig& Config);
    void CreateFragments(AActor* OriginalActor, const FCore_DestructionConfig& Config);
    void CleanupExpiredRagdolls();
    
    // Timer handles
    FTimerHandle RagdollCleanupTimer;
    FTimerHandle PhysicsStatsTimer;
};