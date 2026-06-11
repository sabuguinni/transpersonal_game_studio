#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsPreset : uint8
{
    Default         UMETA(DisplayName = "Default"),
    HighPrecision   UMETA(DisplayName = "High Precision"),
    Performance     UMETA(DisplayName = "Performance Optimized"),
    Destruction     UMETA(DisplayName = "Destruction Heavy"),
    Ragdoll         UMETA(DisplayName = "Ragdoll Optimized")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float MaxAngularVelocity = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    bool bEnableCCD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float SleepThreshold = 0.005f;

    FCore_PhysicsProfile()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        MaxAngularVelocity = 3600.0f;
        bEnableCCD = false;
        SleepThreshold = 0.005f;
    }
};

/**
 * Core Physics Manager - Handles advanced physics systems for dinosaur survival gameplay
 * Manages ragdoll physics, destruction, collision optimization, and physics materials
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS PROFILES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    ECore_PhysicsPreset CurrentPreset = ECore_PhysicsPreset::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    FCore_PhysicsProfile DefaultProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    FCore_PhysicsProfile HighPrecisionProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    FCore_PhysicsProfile PerformanceProfile;

    // === PHYSICS OPTIMIZATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxSimulatingBodies = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsLOD = true;

    // === RAGDOLL SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoCleanupRagdolls = true;

    // === DESTRUCTION SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionForceThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxDestructionChunks = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionCleanupTime = 60.0f;

    // === PHYSICS FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void SetPhysicsPreset(ECore_PhysicsPreset NewPreset);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void ApplyPhysicsProfile(const FCore_PhysicsProfile& Profile, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void OptimizePhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void EnableRagdollOnActor(AActor* TargetActor, float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void DisableRagdollOnActor(AActor* TargetActor, float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void TriggerDestruction(AActor* TargetActor, FVector ImpactPoint, float Force);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    int32 GetActivePhysicsBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void CleanupInactivePhysicsBodies();

    // === COLLISION OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void OptimizeCollisionForActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionLOD(AActor* TargetActor, int32 LODLevel);

private:
    // Internal tracking
    TArray<TWeakObjectPtr<AActor>> ActiveRagdolls;
    TArray<TWeakObjectPtr<AActor>> DestructionActors;
    
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 5.0f;

    // Helper functions
    void UpdatePhysicsOptimization();
    void CleanupExpiredRagdolls();
    void CleanupDestructionActors();
    FCore_PhysicsProfile GetProfileForPreset(ECore_PhysicsPreset Preset) const;
};