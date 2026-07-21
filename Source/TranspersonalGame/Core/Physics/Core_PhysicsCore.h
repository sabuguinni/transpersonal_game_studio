#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/Shared/SharedTypes.h"
#include "Core_PhysicsCore.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsProfile : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Character       UMETA(DisplayName = "Character"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Debris          UMETA(DisplayName = "Debris"),
    Ragdoll         UMETA(DisplayName = "Ragdoll"),
    Vehicle         UMETA(DisplayName = "Vehicle")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bGenerateHitEvents = true;

    FCore_PhysicsSettings()
    {
        Mass = 100.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        Restitution = 0.3f;
        Friction = 0.7f;
        bEnableGravity = true;
        bSimulatePhysics = true;
        bGenerateHitEvents = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    AActor* HitActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    UPrimitiveComponent* HitComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector ImpactPoint = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector ImpactNormal = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float ImpactForce = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float TimeStamp = 0.0f;

    FCore_CollisionData()
    {
        HitActor = nullptr;
        HitComponent = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        ImpactForce = 0.0f;
        TimeStamp = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnPhysicsCollision, const FCore_CollisionData&, CollisionData, ECore_PhysicsProfile, ProfileType);

/**
 * Core Physics System - Manages realistic physics simulation for survival gameplay
 * Handles collision detection, physics profiles, and force calculations
 * Integrates with destruction and terrain systems for immersive interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsCore : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsCore();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics Profile Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyPhysicsProfile(UPrimitiveComponent* Component, ECore_PhysicsProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FCore_PhysicsSettings GetPhysicsSettings(ECore_PhysicsProfile Profile) const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetCustomPhysicsSettings(UPrimitiveComponent* Component, const FCore_PhysicsSettings& Settings);

    // Force and Impulse System
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyForceAtLocation(UPrimitiveComponent* Component, FVector Force, FVector Location, bool bAccelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyImpulseAtLocation(UPrimitiveComponent* Component, FVector Impulse, FVector Location, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyRadialForce(FVector Origin, float Radius, float Strength, bool bImpulse = false, bool bIgnoreMass = false);

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool LineTrace(FVector Start, FVector End, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool SphereTrace(FVector Start, FVector End, float Radius, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool BoxTrace(FVector Start, FVector End, FVector HalfSize, FRotator Orientation, FHitResult& HitResult, bool bTraceComplex = false);

    // Physics State Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnablePhysicsSimulation(UPrimitiveComponent* Component, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetGravityEnabled(UPrimitiveComponent* Component, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetCollisionEnabled(UPrimitiveComponent* Component, bool bEnable);

    // Mass and Density
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetMass(UPrimitiveComponent* Component, float Mass);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    float GetMass(UPrimitiveComponent* Component) const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetDensity(UPrimitiveComponent* Component, float Density);

    // Velocity and Movement
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FVector GetVelocity(UPrimitiveComponent* Component) const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetVelocity(UPrimitiveComponent* Component, FVector NewVelocity);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FVector GetAngularVelocity(UPrimitiveComponent* Component) const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetAngularVelocity(UPrimitiveComponent* Component, FVector NewAngularVelocity);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsSubstepEnabled(bool bEnable);

    // Event Broadcasting
    UPROPERTY(BlueprintAssignable, Category = "Core Physics")
    FCore_OnPhysicsCollision OnPhysicsCollision;

protected:
    // Physics Profile Storage
    UPROPERTY()
    TMap<ECore_PhysicsProfile, FCore_PhysicsSettings> PhysicsProfiles;

    // Collision Tracking
    UPROPERTY()
    TArray<FCore_CollisionData> RecentCollisions;

    // Performance Metrics
    UPROPERTY()
    int32 ActivePhysicsObjects;

    UPROPERTY()
    float LastFrameTime;

    // Internal Methods
    void InitializePhysicsProfiles();
    void UpdatePerformanceMetrics();
    void ProcessCollisionEvent(const FHitResult& HitResult, float ImpactForce);

    // Collision Event Handlers
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
    // Internal state
    bool bInitialized;
    float CollisionHistoryDuration;
    FTimerHandle PerformanceUpdateTimer;
};