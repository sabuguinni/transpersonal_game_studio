#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Handles ragdoll physics, destruction, and enhanced movement for TranspersonalGame
 * Implements realistic physics simulation for prehistoric survival gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System") 
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool IsPhysicsSystemActive() const { return bPhysicsSystemActive; }

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void EnableRagdollForCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DisableRagdollForCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool IsCharacterInRagdoll(ACharacter* Character) const;

    // Enhanced Movement Physics
    UFUNCTION(BlueprintCallable, Category = "Movement Physics")
    void ApplyImpactForce(AActor* Actor, FVector Force, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Movement Physics")
    void ApplyExplosiveForce(FVector ExplosionCenter, float ExplosionRadius, float ExplosionStrength);

    UFUNCTION(BlueprintCallable, Category = "Movement Physics")
    void SetGravityScale(AActor* Actor, float GravityScale);

    // Collision Management
    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    void SetupCollisionForActor(AActor* Actor, ECore_CollisionType CollisionType);

    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    void EnablePhysicsSimulation(AActor* Actor, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    bool CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB);

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void TriggerDestruction(AActor* Actor, FVector ImpactPoint, float DestructionForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void CreateDebrisFromActor(AActor* OriginalActor, int32 DebrisCount);

    // Physics Performance
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsFrameTime() const;

protected:
    // Core System State
    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    bool bPhysicsSystemActive;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    float PhysicsSystemVersion;

    // Ragdoll Management
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Physics")
    TArray<TWeakObjectPtr<ACharacter>> ActiveRagdollCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RagdollActivationForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RagdollDeactivationDelay;

    // Physics Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxActivePhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePhysicsOptimization;

    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float DefaultCollisionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float DefaultMass;

    // Destruction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    float MinDestructionForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    int32 MaxDebrisCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    float DebrisLifetime;

private:
    // Internal Physics Management
    void UpdatePhysicsSystem(float DeltaTime);
    void CleanupInactivePhysicsObjects();
    void ValidatePhysicsSettings();
    
    // Ragdoll Helpers
    void SetupRagdollConstraints(ACharacter* Character);
    void CleanupRagdollConstraints(ACharacter* Character);
    
    // Performance Monitoring
    mutable float CachedPhysicsFrameTime;
    mutable int32 CachedActiveObjectCount;
    float LastPerformanceUpdateTime;
    
    // Timer Handles
    FTimerHandle PhysicsUpdateTimerHandle;
    FTimerHandle PerformanceMonitorTimerHandle;
};