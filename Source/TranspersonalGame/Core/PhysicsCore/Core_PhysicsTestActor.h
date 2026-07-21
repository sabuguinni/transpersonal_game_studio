#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Core_PhysicsTestActor.generated.h"

/**
 * Core_PhysicsTestActor - Test actor for physics system validation
 * Used to test destruction, collision, and ragdoll systems in each biome
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsTestActor : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsTestActor();

protected:
    virtual void BeginPlay() override;

    /** Main mesh component for physics testing */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Test", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* TestMesh;

    /** Collision sphere for interaction detection */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Test", meta = (AllowPrivateAccess = "true"))
    USphereComponent* CollisionSphere;

    /** Physics simulation enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Test")
    bool bEnablePhysics;

    /** Destruction threshold for impact damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Test")
    float DestructionThreshold;

    /** Current health for destruction testing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Test")
    float CurrentHealth;

    /** Maximum health */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Test")
    float MaxHealth;

    /** Biome type this test actor belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Test")
    FString BiomeType;

public:
    virtual void Tick(float DeltaTime) override;

    /** Handle collision events */
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    /** Apply damage to the test actor */
    UFUNCTION(BlueprintCallable, Category = "Physics Test")
    void ApplyDamage(float DamageAmount);

    /** Trigger destruction sequence */
    UFUNCTION(BlueprintCallable, Category = "Physics Test")
    void TriggerDestruction();

    /** Reset test actor to initial state */
    UFUNCTION(BlueprintCallable, Category = "Physics Test")
    void ResetTestActor();

    /** Get current physics state */
    UFUNCTION(BlueprintPure, Category = "Physics Test")
    bool IsPhysicsEnabled() const;

    /** Set physics simulation state */
    UFUNCTION(BlueprintCallable, Category = "Physics Test")
    void SetPhysicsEnabled(bool bEnabled);

    /** Get collision response info */
    UFUNCTION(BlueprintPure, Category = "Physics Test")
    FString GetCollisionInfo() const;
};