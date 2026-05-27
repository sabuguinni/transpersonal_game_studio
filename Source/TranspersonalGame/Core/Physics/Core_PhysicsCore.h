#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCorePhysics, Log, All);

/**
 * Physics Core Events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsImpact, AActor*, Actor, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsDestruction, AActor*, DestroyedActor);

/**
 * Core Physics Settings for Prehistoric World
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    // Global Physics Scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GlobalPhysicsScale = 1.0f;

    // Gravity Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float GravityZ = -980.0f;

    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float DefaultFriction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float DefaultRestitution = 0.3f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxSubstepDeltaTime = 0.016667f;

    FCore_PhysicsSettings()
    {
        GlobalPhysicsScale = 1.0f;
        GravityZ = -980.0f;
        DefaultFriction = 0.7f;
        DefaultRestitution = 0.3f;
        MaxPhysicsSubsteps = 6;
        MaxSubstepDeltaTime = 0.016667f;
    }
};

/**
 * Core Physics Component
 * Manages physics behavior for prehistoric world objects
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FCore_PhysicsSettings PhysicsSettings;

    // Mass Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
    bool bOverrideMass = false;

    // Material Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float Restitution = 0.3f;

    // Destruction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 1000.0f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsImpact OnPhysicsImpact;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsDestruction OnPhysicsDestruction;

    // Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyForce(const FVector& Force, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyImpulse(const FVector& Impulse, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetMass(float NewMass);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetMass() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetVelocity() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetVelocity(const FVector& NewVelocity);

    // Collision Functions
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse Response);

    // Destruction Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const { return bCanBeDestroyed; }

protected:
    // Internal Functions
    void InitializePhysics();
    void UpdatePhysicsProperties();
    void HandleCollision(const FHitResult& Hit, float ImpactForce);

    // Component References
    UPROPERTY()
    class UPrimitiveComponent* PhysicsComponent;

    // State Tracking
    bool bPhysicsInitialized = false;
    float AccumulatedDamage = 0.0f;
};