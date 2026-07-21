#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCorePhysics, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsImpact, AActor*, HitActor, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsDestruction, AActor*, DestroyedActor);

UENUM(BlueprintType)
enum class ECore_PhysicsType : uint8
{
    Static          UMETA(DisplayName = "Static"),
    Dynamic         UMETA(DisplayName = "Dynamic"),
    Kinematic       UMETA(DisplayName = "Kinematic"),
    Destructible    UMETA(DisplayName = "Destructible")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DestructionThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    FCore_PhysicsSettings()
    {
        Mass = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.01f;
        Restitution = 0.3f;
        Friction = 0.7f;
        DestructionThreshold = 1000.0f;
        bEnableGravity = true;
        bSimulatePhysics = true;
    }
};

/**
 * Core Physics Component - Handles realistic physics for prehistoric world objects
 * 
 * Features:
 * - Mass-based physics simulation
 * - Material property settings (friction, restitution)
 * - Destruction mechanics with force thresholds
 * - Impact event handling
 * - Performance optimization for large object counts
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    ECore_PhysicsType PhysicsType = ECore_PhysicsType::Dynamic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bAutoApplySettings = true;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsImpact OnPhysicsImpact;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsDestruction OnPhysicsDestruction;

    // Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddForceAtLocation(const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddImpulseAtLocation(const FVector& Impulse, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsType(ECore_PhysicsType NewType);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsSimulation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetMass(float NewMass);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetMass() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetVelocity() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void TriggerDestruction(float ImpactForce);

    // Collision Handling
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

protected:
    // Internal Functions
    void InitializePhysicsComponent();
    void UpdatePhysicsSettings();
    bool ShouldDestroyFromImpact(float ImpactForce) const;
    void HandlePhysicsDestruction();

    // Cached References
    UPROPERTY()
    UPrimitiveComponent* PhysicsComponent;

    // State Tracking
    bool bIsDestroyed = false;
    float LastImpactTime = 0.0f;
    float ImpactCooldown = 0.1f;
};