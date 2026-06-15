#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Core/Shared/SharedTypes.h"
#include "Core_CollisionSystem.generated.h"

/**
 * Advanced collision detection and response system for realistic physics interactions
 * Handles collision filtering, impact calculations, and damage distribution
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === COLLISION DETECTION ===
    
    /** Enable/disable collision system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    bool bCollisionEnabled = true;

    /** Collision response profiles for different object types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    TMap<ECore_ObjectType, FName> CollisionProfiles;

    /** Minimum impact velocity to trigger collision response */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision", meta = (ClampMin = "0.0"))
    float MinImpactVelocity = 50.0f;

    /** Maximum collision detection distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision", meta = (ClampMin = "0.0"))
    float MaxCollisionDistance = 1000.0f;

    // === IMPACT CALCULATION ===

    /** Base damage multiplier for collision impacts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Calculation", meta = (ClampMin = "0.0"))
    float BaseDamageMultiplier = 1.0f;

    /** Mass scaling factor for impact force calculation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Calculation", meta = (ClampMin = "0.0"))
    float MassScalingFactor = 0.1f;

    /** Velocity threshold for critical impacts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Calculation", meta = (ClampMin = "0.0"))
    float CriticalImpactThreshold = 500.0f;

    // === COLLISION FILTERING ===

    /** Objects to ignore during collision detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Filtering")
    TArray<AActor*> IgnoredActors;

    /** Collision channels to monitor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Filtering")
    TArray<TEnumAsByte<ECollisionChannel>> MonitoredChannels;

    /** Enable collision with static environment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Filtering")
    bool bCollideWithStatic = true;

    /** Enable collision with dynamic objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Filtering")
    bool bCollideWithDynamic = true;

    // === BLUEPRINT CALLABLE FUNCTIONS ===

    /** Register collision event callback */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void RegisterCollisionCallback(AActor* Actor);

    /** Unregister collision event callback */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void UnregisterCollisionCallback(AActor* Actor);

    /** Calculate impact damage based on collision parameters */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    float CalculateImpactDamage(float ImpactVelocity, float ObjectMass, ECore_ObjectType ObjectType);

    /** Apply collision impulse to actor */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void ApplyCollisionImpulse(AActor* Actor, const FVector& ImpactPoint, const FVector& ImpactForce);

    /** Check if collision should be processed */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool ShouldProcessCollision(AActor* ActorA, AActor* ActorB);

    /** Get collision profile for object type */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    FName GetCollisionProfileForType(ECore_ObjectType ObjectType);

    /** Set collision response for specific actor */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetActorCollisionResponse(AActor* Actor, ECore_ObjectType ObjectType);

    /** Enable/disable collision for specific actor */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetActorCollisionEnabled(AActor* Actor, bool bEnabled);

    /** Perform collision trace */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool PerformCollisionTrace(const FVector& Start, const FVector& End, FHitResult& OutHit);

    /** Get all overlapping actors */
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    TArray<AActor*> GetOverlappingActors(AActor* Actor);

    // === COLLISION EVENTS ===

    /** Called when collision is detected */
    UFUNCTION(BlueprintImplementableEvent, Category = "Core Collision Events")
    void OnCollisionDetected(AActor* ActorA, AActor* ActorB, const FVector& ImpactPoint, float ImpactVelocity);

    /** Called when critical impact occurs */
    UFUNCTION(BlueprintImplementableEvent, Category = "Core Collision Events")
    void OnCriticalImpact(AActor* Actor, float Damage, const FVector& ImpactPoint);

    /** Called when collision response is applied */
    UFUNCTION(BlueprintImplementableEvent, Category = "Core Collision Events")
    void OnCollisionResponse(AActor* Actor, const FVector& ResponseForce);

private:
    // === INTERNAL COLLISION HANDLING ===

    /** Handle collision between two actors */
    void HandleCollision(AActor* ActorA, AActor* ActorB, const FHitResult& HitResult);

    /** Calculate collision response force */
    FVector CalculateResponseForce(const FHitResult& HitResult, float ImpactVelocity, float ObjectMass);

    /** Apply damage from collision */
    void ApplyCollisionDamage(AActor* Actor, float Damage, const FVector& ImpactPoint);

    /** Update collision profiles */
    void UpdateCollisionProfiles();

    /** Initialize default collision profiles */
    void InitializeDefaultProfiles();

    // === COLLISION TRACKING ===

    /** Active collision callbacks */
    UPROPERTY()
    TArray<AActor*> RegisteredActors;

    /** Recent collision events (for avoiding duplicate processing) */
    TMap<TPair<AActor*, AActor*>, float> RecentCollisions;

    /** Time threshold for collision event deduplication */
    float CollisionDeduplicationTime = 0.1f;

    /** Last collision update time */
    float LastCollisionUpdate = 0.0f;
};