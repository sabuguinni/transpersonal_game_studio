#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_PhysicsManager.generated.h"

/**
 * Core Physics Manager - Handles physics simulation, collision detection, and ragdoll systems
 * Integrates with UE5 Chaos Physics for realistic prehistoric world simulation
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalGravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultLinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultAngularDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableAdvancedPhysics = true;

    // Collision Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> DefaultCollisionChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionTolerance = 0.1f;

    // Ragdoll System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdollSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollImpulseThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollRecoveryTime = 3.0f;

    // Physics Simulation Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGlobalPhysicsSettings(float NewGravityScale, float NewLinearDamping, float NewAngularDamping);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool PerformLineTrace(FVector Start, FVector End, FHitResult& OutHit, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool PerformSphereTrace(FVector Start, FVector End, float Radius, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    TArray<FHitResult> PerformMultiLineTrace(FVector Start, FVector End, bool bTraceComplex = false);

    // Ragdoll Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdoll(AActor* TargetActor, FVector ImpulseDirection = FVector::ZeroVector, float ImpulseStrength = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdoll(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsActorInRagdoll(AActor* TargetActor);

    // Physics Validation
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidatePhysicsSetup(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void RunPhysicsPerformanceTest();

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDestructiveForce(AActor* TargetActor, FVector ForceLocation, float ForceRadius, float ForceStrength);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreatePhysicsExplosion(FVector ExplosionLocation, float ExplosionRadius, float ExplosionStrength);

private:
    // Internal tracking
    UPROPERTY()
    TArray<AActor*> RagdollActors;

    UPROPERTY()
    TMap<AActor*, float> RagdollTimers;

    // Physics validation cache
    UPROPERTY()
    TMap<AActor*, bool> PhysicsValidationCache;

    // Helper functions
    void UpdateRagdollTimers(float DeltaTime);
    void CleanupInvalidActors();
    USkeletalMeshComponent* GetSkeletalMeshFromActor(AActor* Actor);
    void LogPhysicsState(const FString& Message);
};