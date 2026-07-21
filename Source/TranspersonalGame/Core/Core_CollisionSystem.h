#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "SharedTypes.h"
#include "Core_CollisionSystem.generated.h"

/**
 * Core Collision System - Handles advanced collision detection and response
 * Supports multi-layered collision channels for dinosaur interactions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Collision channel management
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupCollisionChannels();

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionObject(UPrimitiveComponent* Component, ECore_CollisionType CollisionType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool CheckCollisionBetween(AActor* ActorA, AActor* ActorB, float& Distance);

    // Advanced collision detection
    UFUNCTION(BlueprintCallable, Category = "Collision")
    TArray<FHitResult> PerformSphereTrace(FVector Start, FVector End, float Radius, ECore_CollisionType TraceType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformLineTrace(FVector Start, FVector End, FHitResult& OutHit, ECore_CollisionType TraceType);

    // Collision response
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void HandleCollisionResponse(const FHitResult& Hit, ECore_CollisionResponse ResponseType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ApplyImpactForce(AActor* Actor, FVector ImpactPoint, FVector Force);

    // Collision filtering
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionFilter(UPrimitiveComponent* Component, const TArray<ECore_CollisionType>& IgnoreTypes);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool ShouldIgnoreCollision(ECore_CollisionType TypeA, ECore_CollisionType TypeB);

protected:
    // Collision configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float CollisionUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MaxTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bEnableAdvancedCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bDebugCollision;

    // Collision tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision State")
    TArray<UPrimitiveComponent*> RegisteredComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision State")
    TMap<ECore_CollisionType, int32> CollisionCounts;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision State")
    float LastCollisionCheckTime;

    // Collision filters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Filters")
    TMap<ECore_CollisionType, TArray<ECore_CollisionType>> CollisionIgnoreMap;

private:
    // Internal collision management
    void UpdateCollisionTracking();
    void ProcessCollisionEvents();
    void CleanupInvalidComponents();
    
    // Collision response helpers
    void ApplyPhysicsResponse(const FHitResult& Hit, float ImpactStrength);
    void TriggerCollisionEvents(const FHitResult& Hit);
    
    // Performance optimization
    float LastUpdateTime;
    int32 CollisionCheckCounter;
};