#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCorePhysics, Log, All);

/**
 * Core Physics System Manager
 * 
 * Manages the core physics systems for the Transpersonal Game:
 * - Dynamic physics simulation for realistic object interactions
 * - Collision detection and response for survival gameplay
 * - Ragdoll physics for character death and impact
 * - Destruction system for environmental breakables
 * 
 * Design Philosophy:
 * - Physics should feel weighty and realistic (Casey Muratori approach)
 * - Performance is critical - no frame drops during physics events
 * - Predictable behavior for gameplay mechanics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalGravityScale;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultLinearDamping;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultAngularDamping;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsSimulation;
    
    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableComplexCollision;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionTolerance;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    int32 MaxCollisionIterations;
    
    // Ragdoll Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdollPhysics;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendWeight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollLifetime;
    
    // Destruction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDestruction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxDestructionParticles;

public:
    // Physics Management Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void InitializePhysicsSystem();
    
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UpdatePhysicsSettings();
    
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGlobalGravity(float NewGravity);
    
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsSimulation(bool bEnable);
    
    // Collision Functions
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool CheckCollisionAtLocation(FVector Location, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Collision")
    TArray<AActor*> GetOverlappingActors(FVector Location, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionResponseForActor(AActor* Actor, ECollisionResponse Response);
    
    // Ragdoll Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdollForCharacter(ACharacter* Character);
    
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdollForCharacter(ACharacter* Character);
    
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollBlendWeight(ACharacter* Character, float Weight);
    
    // Destruction Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void DestroyActor(AActor* Actor, FVector ImpactLocation, float Force);
    
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateDestructionEffect(FVector Location, float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanActorBeDestroyed(AActor* Actor);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsFrameTime() const;
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActivePhysicsObjects() const;
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

protected:
    // Internal Physics State
    UPROPERTY()
    TArray<AActor*> TrackedPhysicsActors;
    
    UPROPERTY()
    TArray<ACharacter*> RagdollCharacters;
    
    UPROPERTY()
    float LastPhysicsUpdateTime;
    
    UPROPERTY()
    int32 PhysicsObjectCount;
    
    // Internal Functions
    void UpdateTrackedActors();
    void ProcessRagdollCharacters(float DeltaTime);
    void MonitorPerformance();
    void CleanupDestroyedActors();
    
    // Physics Event Handlers
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
    
    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
    
    UFUNCTION()
    void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
};