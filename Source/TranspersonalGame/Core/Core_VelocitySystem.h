#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_VelocitySystem.generated.h"

/**
 * Core velocity tracking and management system for realistic physics.
 * Handles velocity calculations, momentum conservation, and speed limits
 * for all physics bodies in the prehistoric world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_VelocitySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VelocitySystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Velocity tracking
    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    FVector GetActorVelocity(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    float GetActorSpeed(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void SetVelocityLimit(AActor* Actor, float MaxSpeed);

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void ApplyVelocityDamping(AActor* Actor, float DampingFactor);

    // Momentum system
    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void ApplyMomentumTransfer(AActor* FromActor, AActor* ToActor, float TransferRatio);

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    float CalculateMomentum(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void ConserveMomentum(const TArray<AActor*>& Actors);

    // Velocity-based effects
    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void ProcessVelocityEffects(AActor* Actor, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    bool ShouldApplyAirResistance(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void ApplyAirResistance(AActor* Actor, float DeltaTime);

    // Velocity state management
    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void RegisterVelocityTracker(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void UnregisterVelocityTracker(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void UpdateVelocityTracking(float DeltaTime);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    int32 GetTrackedActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    float GetAverageVelocity() const;

    UFUNCTION(BlueprintCallable, Category = "Core Velocity")
    void OptimizeVelocityTracking();

protected:
    // Velocity tracking data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Velocity", meta = (AllowPrivateAccess = "true"))
    TMap<AActor*, FVector> PreviousPositions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Velocity", meta = (AllowPrivateAccess = "true"))
    TMap<AActor*, FVector> CurrentVelocities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Velocity", meta = (AllowPrivateAccess = "true"))
    TMap<AActor*, float> VelocityLimits;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Velocity Performance")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Velocity Performance")
    float MaxTrackingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Velocity Performance")
    int32 MaxTrackedActors = 500;

    // Physics constants
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Velocity Physics")
    float AirDensity = 1.225f; // kg/m³ at sea level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Velocity Physics")
    float DefaultDragCoefficient = 0.47f; // Sphere drag coefficient

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Velocity Physics")
    float MinimumVelocityThreshold = 0.1f;

private:
    // Internal tracking
    float LastUpdateTime = 0.0f;
    int32 TrackedActorCount = 0;
    float TotalVelocityMagnitude = 0.0f;

    // Helper functions
    bool IsActorValidForTracking(AActor* Actor) const;
    float CalculateDragForce(AActor* Actor, const FVector& Velocity) const;
    void UpdateActorVelocity(AActor* Actor, float DeltaTime);
    void CleanupInvalidActors();
};