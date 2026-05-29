#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCorePhysics, Log, All);

/**
 * Core Physics System Manager
 * 
 * Manages all physics systems in the game including:
 * - Collision detection and response
 * - Ragdoll physics for characters and dinosaurs
 * - Destruction system for environmental objects
 * - Physics performance optimization
 * - Material physics properties
 * 
 * Design Philosophy:
 * - Physics should feel realistic and weighty
 * - Performance is critical - use LOD systems
 * - Destruction should be satisfying but controlled
 * - Ragdoll should activate seamlessly on death
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics System Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSystems(float DeltaTime);

    // Collision System
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Collision")
    void SetupCollisionChannels();

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Collision")
    void ConfigureCollisionProfiles();

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Collision")
    bool CheckCollisionBetween(AActor* ActorA, AActor* ActorB, FHitResult& HitResult);

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void EnableRagdollForCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void DisableRagdollForCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void BlendToRagdoll(ACharacter* Character, float BlendTime = 0.5f);

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Destruction")
    void DestroyObject(AActor* Actor, FVector ImpactLocation, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Destruction")
    void CreateDebris(AActor* OriginalActor, FVector ImpactLocation, int32 DebrisCount);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Performance")
    void SetPhysicsLOD(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Performance")
    void UpdatePhysicsLODs(const FVector& PlayerLocation);

    // Material Physics
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Materials")
    void SetupPhysicalMaterials();

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Materials")
    void ApplyPhysicalMaterial(UPrimitiveComponent* Component, class UPhysicalMaterial* Material);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Core Physics")
    bool IsPhysicsSystemActive() const { return bPhysicsSystemActive; }

    UFUNCTION(BlueprintPure, Category = "Core Physics")
    int32 GetActivePhysicsObjects() const { return ActivePhysicsObjects; }

    UFUNCTION(BlueprintPure, Category = "Core Physics")
    float GetPhysicsPerformanceMetric() const { return PhysicsPerformanceMetric; }

protected:
    // System State
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    bool bPhysicsSystemActive;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics")
    float PhysicsPerformanceMetric;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Performance")
    float PhysicsLODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Performance")
    float PhysicsLODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Performance")
    float PhysicsLODDistance3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Performance")
    int32 MaxPhysicsObjects;

    // Ragdoll Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Ragdoll")
    float RagdollBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Ragdoll")
    float RagdollLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Ragdoll")
    float RagdollAngularDamping;

    // Destruction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Destruction")
    float DestructionImpactThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Destruction")
    int32 MaxDebrisCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics|Destruction")
    float DebrisLifetime;

private:
    // Internal tracking
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TArray<TWeakObjectPtr<ACharacter>> RagdollCharacters;
    
    // Performance monitoring
    float LastPerformanceUpdate;
    float PerformanceUpdateInterval;

    // Helper functions
    void UpdatePerformanceMetrics();
    void CleanupInvalidActors();
    void ProcessPhysicsLOD(AActor* Actor, float DistanceToPlayer);
};