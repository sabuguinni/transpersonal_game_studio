#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "Core_RealTimePhysicsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsEventTriggered, AActor*, Actor, ECore_PhysicsEventType, EventType);

/**
 * Real-time physics manager that handles immediate physics responses for gameplay
 * Focuses on player-visible physics events: impacts, falls, collisions
 * Optimized for 60fps with minimal latency between input and physics response
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UCore_RealTimePhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RealTimePhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Real-time physics event handling
    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    void ProcessImpactEvent(AActor* Actor, const FVector& ImpactLocation, const FVector& ImpactVelocity, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    void ProcessFallEvent(AActor* Actor, const FVector& FallLocation, float FallHeight, float FallVelocity);

    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    void ProcessCollisionEvent(AActor* ActorA, AActor* ActorB, const FVector& CollisionPoint, const FVector& CollisionNormal);

    // Dynamic physics property adjustment
    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    void SetActorPhysicsProperties(AActor* Actor, float Mass, float LinearDamping, float AngularDamping, bool bEnableGravity);

    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    void ApplyInstantForce(AActor* Actor, const FVector& Force, const FVector& Location, bool bIsImpulse = true);

    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    void ApplyInstantTorque(AActor* Actor, const FVector& Torque, bool bIsImpulse = true);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    float GetCurrentPhysicsFrameTime() const { return CurrentPhysicsFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    int32 GetActivePhysicsActorCount() const { return ActivePhysicsActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Real-Time Physics")
    bool IsPhysicsOptimal() const { return CurrentPhysicsFrameTime < MaxAllowedPhysicsTime; }

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Real-Time Physics Events")
    FOnPhysicsEventTriggered OnPhysicsEventTriggered;

protected:
    // Core physics tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> ActivePhysicsActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    float CurrentPhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float MaxAllowedPhysicsTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxActivePhysicsActors;

    // Impact processing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings", meta = (AllowPrivateAccess = "true"))
    float MinImpactForceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings", meta = (AllowPrivateAccess = "true"))
    float MaxImpactForceProcessed;

    // Fall processing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Settings", meta = (AllowPrivateAccess = "true"))
    float MinFallHeightThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Settings", meta = (AllowPrivateAccess = "true"))
    float MaxFallVelocityProcessed;

    // Collision processing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings", meta = (AllowPrivateAccess = "true"))
    float MinCollisionVelocityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings", meta = (AllowPrivateAccess = "true"))
    bool bProcessStaticCollisions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings", meta = (AllowPrivateAccess = "true"))
    bool bProcessDynamicCollisions;

private:
    // Internal processing methods
    void UpdatePhysicsPerformanceMetrics(float DeltaTime);
    void ProcessPhysicsActorUpdates();
    void CleanupInactivePhysicsActors();
    
    // Event processing helpers
    void TriggerPhysicsEvent(AActor* Actor, ECore_PhysicsEventType EventType);
    bool ShouldProcessPhysicsEvent(AActor* Actor) const;
    
    // Performance optimization
    void OptimizePhysicsSettings();
    void LimitActivePhysicsActors();

    // Internal state
    float PhysicsFrameAccumulator;
    int32 PhysicsFrameCounter;
    TMap<AActor*, float> ActorLastUpdateTime;
};