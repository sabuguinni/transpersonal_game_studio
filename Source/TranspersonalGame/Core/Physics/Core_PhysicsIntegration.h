#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "Core_PhysicsIntegration.generated.h"

/**
 * Core Physics Integration System
 * Manages integration between UE5 Chaos Physics and custom physics systems
 * Handles physics material properties, collision responses, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UCore_PhysicsIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsMaterial(UPhysicalMaterial* Material);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    UPhysicalMaterial* GetCurrentPhysicsMaterial() const;

    // Collision Response System
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ConfigureCollisionResponse(ECollisionResponse ResponseType);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetCollisionObjectType(ECollisionChannel ObjectType);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizePhysicsPerformance(bool bEnableOptimization);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsLOD(int32 LODLevel);

    // Physics State Management
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void EnablePhysicsSimulation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool IsPhysicsSimulationEnabled() const;

    // Force Application System
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyForce(const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyImpulse(const FVector& Impulse, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyTorque(const FVector& Torque);

    // Physics Query System
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, FHitResult& HitResult);

protected:
    // Physics Material Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material", meta = (AllowPrivateAccess = "true"))
    UPhysicalMaterial* CurrentPhysicsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material", meta = (AllowPrivateAccess = "true"))
    float Friction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material", meta = (AllowPrivateAccess = "true"))
    float Restitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material", meta = (AllowPrivateAccess = "true"))
    float Density;

    // Collision Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<ECollisionChannel> CollisionObjectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<ECollisionResponse> CollisionResponseToWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<ECollisionResponse> CollisionResponseToPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<ECollisionResponse> CollisionResponseToVehicle;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bOptimizePerformance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 PhysicsLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxSimulationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CullingDistance;

    // Physics State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    bool bGravityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    FVector CustomGravity;

    // Force Application
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float MaxForceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float MaxImpulseMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float MaxTorqueMultiplier;

private:
    // Internal Physics Management
    void InitializePhysicsSettings();
    void UpdatePhysicsLOD();
    void ValidatePhysicsState();
    
    // Performance Monitoring
    float LastPerformanceCheck;
    int32 PhysicsObjectCount;
    float AverageFrameTime;
};