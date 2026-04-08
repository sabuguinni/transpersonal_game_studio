#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "ChaosVehicleMovementComponent.h"
#include "VehiclePhysicsSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleMounted, APawn*, Vehicle, APawn*, Rider);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleDismounted, APawn*, Vehicle, APawn*, Rider);

/**
 * Vehicle Physics System Component
 * Handles dinosaur riding mechanics and creature-based transportation
 * Integrates with Chaos Vehicle system for realistic physics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVehiclePhysicsSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVehiclePhysicsSystemComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Mount a rider onto this vehicle/creature */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool MountRider(APawn* Rider);

    /** Dismount the current rider */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool DismountRider();

    /** Check if this vehicle can be mounted */
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    bool CanBeMounted() const;

    /** Check if there's currently a rider */
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    bool HasRider() const { return CurrentRider != nullptr; }

    /** Get the current rider */
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    APawn* GetCurrentRider() const { return CurrentRider; }

    /** Configure vehicle physics properties */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ConfigureVehiclePhysics(float Mass, float MaxSpeed, float Acceleration, float TurnRate);

    /** Apply movement input to the vehicle */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyMovementInput(float ForwardInput, float RightInput);

    /** Apply steering input */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplySteeringInput(float SteeringInput);

    /** Emergency dismount (for danger situations) */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void EmergencyDismount();

    /** Set vehicle type (affects physics behavior) */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetVehicleType(const FString& VehicleType);

    /** Enable/disable vehicle physics simulation */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetPhysicsEnabled(bool bEnabled);

    /** Get vehicle physics metrics */
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    void GetVehicleMetrics(float& CurrentSpeed, float& CurrentAcceleration, FVector& Velocity) const;

public:
    /** Events */
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics")
    FOnVehicleMounted OnVehicleMounted;

    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics")
    FOnVehicleDismounted OnVehicleDismounted;

protected:
    /** Vehicle physics properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float VehicleMass = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Acceleration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TurnRate = 45.0f;

    /** Mounting properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounting")
    FVector MountOffset = FVector(0.0f, 0.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounting")
    bool bCanBeMounted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounting")
    float MountRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mounting")
    float DismountSafetyDistance = 150.0f;

    /** Vehicle type affects physics behavior */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FString VehicleType = TEXT("Dinosaur");

    /** Current rider reference */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics")
    TObjectPtr<APawn> CurrentRider;

    /** Vehicle movement component reference */
    UPROPERTY()
    TObjectPtr<UChaosVehicleMovementComponent> VehicleMovementComponent;

    /** Physics enabled state */
    UPROPERTY()
    bool bPhysicsEnabled = true;

    /** Current movement inputs */
    float CurrentForwardInput = 0.0f;
    float CurrentRightInput = 0.0f;
    float CurrentSteeringInput = 0.0f;

private:
    /** Initialize vehicle physics */
    void InitializeVehiclePhysics();

    /** Update vehicle physics based on type */
    void UpdateVehiclePhysicsForType();

    /** Find safe dismount location */
    FVector FindSafeDismountLocation() const;

    /** Check if location is safe for dismounting */
    bool IsLocationSafeForDismount(const FVector& Location) const;

    /** Apply physics settings to movement component */
    void ApplyPhysicsSettings();

    /** Handle rider input forwarding */
    void ForwardRiderInput();

    /** Timer for input updates */
    FTimerHandle InputUpdateTimer;
};