#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysics.generated.h"

/**
 * Core Vehicle Physics System - Handles realistic vehicle behavior for prehistoric carts, sleds, and rafts
 * Implements wheel physics, suspension, traction, and terrain interaction for survival gameplay
 * 
 * @author Core Systems Programmer
 * @version 1.0
 * @since CYCLE_010
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WheelConfiguration
{
    GENERATED_BODY()

    /** Wheel radius in centimeters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float WheelRadius = 30.0f;

    /** Wheel width in centimeters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float WheelWidth = 15.0f;

    /** Maximum suspension travel distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float SuspensionTravel = 20.0f;

    /** Spring stiffness for suspension */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float SpringStiffness = 50000.0f;

    /** Damping coefficient for suspension */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float DampingCoefficient = 3000.0f;

    /** Friction coefficient for this wheel */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float FrictionCoefficient = 0.8f;

    FCore_WheelConfiguration()
    {
        WheelRadius = 30.0f;
        WheelWidth = 15.0f;
        SuspensionTravel = 20.0f;
        SpringStiffness = 50000.0f;
        DampingCoefficient = 3000.0f;
        FrictionCoefficient = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleStats
{
    GENERATED_BODY()

    /** Current speed in km/h */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Stats")
    float CurrentSpeed = 0.0f;

    /** Engine RPM */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Stats")
    float EngineRPM = 0.0f;

    /** Current gear */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Stats")
    int32 CurrentGear = 1;

    /** Fuel/stamina level (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Stats")
    float FuelLevel = 100.0f;

    /** Vehicle health (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Stats")
    float VehicleHealth = 100.0f;

    FCore_VehicleStats()
    {
        CurrentSpeed = 0.0f;
        EngineRPM = 0.0f;
        CurrentGear = 1;
        FuelLevel = 100.0f;
        VehicleHealth = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_VehiclePhysics : public APawn
{
    GENERATED_BODY()

public:
    ACore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    /** Main vehicle body mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle Components")
    UStaticMeshComponent* VehicleBodyMesh;

    /** Wheel configurations for each wheel */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    TArray<FCore_WheelConfiguration> WheelConfigurations;

    /** Current vehicle statistics */
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Stats")
    FCore_VehicleStats VehicleStats;

    /** Maximum engine power in horsepower */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxEnginePower = 150.0f;

    /** Maximum torque in Nm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxTorque = 300.0f;

    /** Vehicle mass in kilograms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float VehicleMass = 1200.0f;

    /** Drag coefficient for air resistance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float DragCoefficient = 0.35f;

    /** Frontal area in square meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float FrontalArea = 2.5f;

    /** Center of mass offset from vehicle origin */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FVector CenterOfMassOffset = FVector(0.0f, 0.0f, -50.0f);

public:
    /** Apply throttle input (0-1) */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void ApplyThrottle(float ThrottleValue);

    /** Apply brake input (0-1) */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void ApplyBrake(float BrakeValue);

    /** Apply steering input (-1 to 1) */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void ApplySteering(float SteeringValue);

    /** Get current vehicle speed in km/h */
    UFUNCTION(BlueprintPure, Category = "Vehicle Stats")
    float GetCurrentSpeed() const { return VehicleStats.CurrentSpeed; }

    /** Get current engine RPM */
    UFUNCTION(BlueprintPure, Category = "Vehicle Stats")
    float GetEngineRPM() const { return VehicleStats.EngineRPM; }

    /** Check if vehicle is on ground */
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    bool IsOnGround() const;

    /** Get traction coefficient based on terrain */
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    float GetTerrainTraction() const;

    /** Calculate suspension forces for all wheels */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void CalculateSuspensionForces();

    /** Apply engine force based on throttle and RPM */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyEngineForce(float DeltaTime);

    /** Apply aerodynamic drag */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyAerodynamicDrag(float DeltaTime);

    /** Update vehicle statistics */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateVehicleStats(float DeltaTime);

protected:
    /** Current throttle input */
    float CurrentThrottle = 0.0f;

    /** Current brake input */
    float CurrentBrake = 0.0f;

    /** Current steering input */
    float CurrentSteering = 0.0f;

    /** Wheel contact points */
    TArray<FVector> WheelContactPoints;

    /** Wheel contact normals */
    TArray<FVector> WheelContactNormals;

    /** Wheel suspension compression ratios */
    TArray<float> WheelCompressionRatios;

    /** Calculate wheel contact with ground */
    void CalculateWheelContacts();

    /** Apply wheel forces to vehicle body */
    void ApplyWheelForces(float DeltaTime);

    /** Calculate engine output based on RPM */
    float CalculateEngineOutput(float RPM) const;

    /** Input binding functions */
    void ThrottlePressed();
    void ThrottleReleased();
    void BrakePressed();
    void BrakeReleased();
    void SteerLeft();
    void SteerRight();
    void SteerStop();
};