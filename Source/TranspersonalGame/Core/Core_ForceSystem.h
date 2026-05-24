#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_ForceSystem.generated.h"

/**
 * Enum for different types of forces in the prehistoric world
 */
UENUM(BlueprintType)
enum class ECore_ForceType : uint8
{
    Gravity         UMETA(DisplayName = "Gravity"),
    Wind            UMETA(DisplayName = "Wind"),
    Explosion       UMETA(DisplayName = "Explosion"),
    Magnetic        UMETA(DisplayName = "Magnetic"),
    Friction        UMETA(DisplayName = "Friction"),
    Buoyancy        UMETA(DisplayName = "Buoyancy"),
    Centrifugal     UMETA(DisplayName = "Centrifugal"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * Structure representing a force application
 */
USTRUCT(BlueprintType)
struct FCore_ForceApplication
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    ECore_ForceType ForceType = ECore_ForceType::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    FVector ForceVector = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    FVector ApplicationPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    float Magnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    bool bIsImpulse = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
    bool bAccelChange = false;

    FCore_ForceApplication()
    {
        ForceType = ECore_ForceType::Custom;
        ForceVector = FVector::ZeroVector;
        ApplicationPoint = FVector::ZeroVector;
        Magnitude = 0.0f;
        Duration = 0.0f;
        bIsImpulse = false;
        bAccelChange = false;
    }
};

/**
 * Core force application and management system for realistic physics.
 * Handles all types of forces: gravity, wind, explosions, friction, etc.
 * Provides realistic force calculations for the prehistoric environment.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_ForceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_ForceSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Force application methods
    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyForce(AActor* Actor, const FVector& Force, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyImpulse(AActor* Actor, const FVector& Impulse, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyForceAtLocation(AActor* Actor, const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyRadialForce(const FVector& Origin, float Radius, float Strength, bool bImpulse = false);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyDirectionalForce(const FVector& Direction, float Strength, float Radius, const FVector& Origin);

    // Specialized force types
    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyGravityForce(AActor* Actor, float GravityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyWindForce(AActor* Actor, const FVector& WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyExplosionForce(const FVector& ExplosionCenter, float ExplosionRadius, float ExplosionStrength);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyFrictionForce(AActor* Actor, float FrictionCoefficient);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ApplyBuoyancyForce(AActor* Actor, float FluidDensity, float SubmergedVolume);

    // Force field management
    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void CreateForceField(const FVector& Center, float Radius, const FVector& ForceDirection, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void RemoveForceField(int32 ForceFieldID);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void UpdateForceFields(float DeltaTime);

    // Force calculation utilities
    UFUNCTION(BlueprintCallable, Category = "Core Force")
    FVector CalculateGravityForce(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    FVector CalculateDragForce(AActor* Actor, const FVector& Velocity) const;

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    FVector CalculateNormalForce(AActor* Actor, const FVector& SurfaceNormal) const;

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    float CalculateForceFromAcceleration(AActor* Actor, const FVector& Acceleration) const;

    // Force registration and tracking
    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void RegisterForceTarget(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void UnregisterForceTarget(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void ProcessContinuousForces(float DeltaTime);

    // Performance and monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Force")
    int32 GetActiveForceCount() const;

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    float GetTotalForceApplied() const;

    UFUNCTION(BlueprintCallable, Category = "Core Force")
    void OptimizeForceCalculations();

protected:
    // Force tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Force", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_ForceApplication> ActiveForces;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Force", meta = (AllowPrivateAccess = "true"))
    TSet<AActor*> RegisteredActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Force", meta = (AllowPrivateAccess = "true"))
    TMap<int32, FVector> ForceFields;

    // Physics constants
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Physics")
    float DefaultGravity = -980.0f; // cm/s² (UE5 units)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Physics")
    float AirDensity = 1.225f; // kg/m³

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Physics")
    float DefaultDragCoefficient = 0.47f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Physics")
    float WaterDensity = 1000.0f; // kg/m³

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Performance")
    float MaxForceDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Performance")
    int32 MaxActiveForces = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Performance")
    float ForceUpdateFrequency = 60.0f;

    // Environmental forces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Environment")
    FVector GlobalWindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Environment")
    float GlobalWindStrength = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Force Environment")
    bool bApplyGlobalWind = false;

private:
    // Internal tracking
    int32 NextForceFieldID = 1;
    float TotalForceAppliedThisFrame = 0.0f;
    float LastUpdateTime = 0.0f;

    // Helper functions
    bool IsActorValidForForces(AActor* Actor) const;
    float GetActorMass(AActor* Actor) const;
    FVector GetActorVelocity(AActor* Actor) const;
    void ApplyForceToActor(AActor* Actor, const FCore_ForceApplication& ForceApp);
    void CleanupExpiredForces();
    void ProcessEnvironmentalForces(AActor* Actor, float DeltaTime);
};