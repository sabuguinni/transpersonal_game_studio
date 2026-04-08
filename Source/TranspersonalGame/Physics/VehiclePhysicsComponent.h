// VehiclePhysicsComponent.h
// Advanced vehicle physics system for transpersonal game
// Supports cars, motorcycles, boats, and flying vehicles

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "VehiclePhysicsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleEvent, FString, EventType);

/**
 * @enum EVehicleType
 * @brief Types of vehicles supported by the physics system
 */
UENUM(BlueprintType)
enum class EVehicleType : uint8
{
    Car             UMETA(DisplayName = "Car"),
    Motorcycle      UMETA(DisplayName = "Motorcycle"),
    Boat            UMETA(DisplayName = "Boat"),
    Aircraft        UMETA(DisplayName = "Aircraft"),
    Hovercraft      UMETA(DisplayName = "Hovercraft")
};

/**
 * @enum EVehicleGear
 * @brief Vehicle transmission gears
 */
UENUM(BlueprintType)
enum class EVehicleGear : uint8
{
    Reverse         UMETA(DisplayName = "Reverse"),
    Neutral         UMETA(DisplayName = "Neutral"),
    First           UMETA(DisplayName = "First"),
    Second          UMETA(DisplayName = "Second"),
    Third           UMETA(DisplayName = "Third"),
    Fourth          UMETA(DisplayName = "Fourth"),
    Fifth           UMETA(DisplayName = "Fifth"),
    Sixth           UMETA(DisplayName = "Sixth")
};

/**
 * @struct FVehicleEngineData
 * @brief Engine characteristics and performance data
 */
USTRUCT(BlueprintType)
struct FVehicleEngineData
{
    GENERATED_BODY()

    // Engine power curve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
    float MaxTorque = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
    float MaxRPM = 7000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
    float IdleRPM = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
    UCurveFloat* TorqueCurve = nullptr;

    // Transmission
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
    TArray<float> GearRatios = {-3.5f, 0.0f, 3.5f, 2.1f, 1.4f, 1.0f, 0.8f, 0.6f};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
    float FinalDriveRatio = 3.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
    bool bAutomatic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
    float ShiftTime = 0.5f;
};

/**
 * @struct FVehicleSuspensionData
 * @brief Suspension system parameters
 */
USTRUCT(BlueprintType)
struct FVehicleSuspensionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringStiffness = 35000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float DamperStiffness = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float MaxRaise = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float MaxDrop = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float WheelRadius = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float FrictionSlip = 1.3f;
};

/**
 * @class UVehiclePhysicsComponent
 * @brief Advanced vehicle physics simulation component
 * 
 * Provides realistic vehicle physics for various vehicle types including
 * engine simulation, transmission, suspension, and tire physics.
 * Optimized for 60fps performance on console platforms.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVehiclePhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVehiclePhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize vehicle physics with specified type
     * @param VehicleType Type of vehicle to simulate
     * @return true if initialization successful
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool InitializeVehicle(EVehicleType VehicleType);

    /**
     * @brief Apply throttle input (0.0 to 1.0)
     * @param ThrottleInput Throttle value
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetThrottleInput(float ThrottleInput);

    /**
     * @brief Apply steering input (-1.0 to 1.0)
     * @param SteeringInput Steering value
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetSteeringInput(float SteeringInput);

    /**
     * @brief Apply brake input (0.0 to 1.0)
     * @param BrakeInput Brake value
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetBrakeInput(float BrakeInput);

    /**
     * @brief Apply handbrake
     * @param bHandbrakeActive Whether handbrake is active
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetHandbrake(bool bHandbrakeActive);

    /**
     * @brief Shift gear manually (manual transmission only)
     * @param NewGear Target gear
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ShiftGear(EVehicleGear NewGear);

    /**
     * @brief Get current vehicle speed in km/h
     * @return Current speed
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    float GetSpeed() const;

    /**
     * @brief Get current engine RPM
     * @return Current RPM
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    float GetEngineRPM() const;

    /**
     * @brief Get current gear
     * @return Current gear
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    EVehicleGear GetCurrentGear() const;

    /**
     * @brief Check if vehicle is airborne
     * @return true if airborne
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool IsAirborne() const;

    /**
     * @brief Get vehicle performance data for UI/telemetry
     * @return Performance data struct
     */
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    FVehiclePerformanceData GetPerformanceData() const;

    // Vehicle configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    EVehicleType VehicleType = EVehicleType::Car;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    FVehicleEngineData EngineData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    FVehicleSuspensionData SuspensionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    float VehicleMass = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    float DragCoefficient = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    float DownforceCoefficient = 0.1f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Events")
    FOnVehicleEvent OnGearChanged;

    UPROPERTY(BlueprintAssignable, Category = "Vehicle Events")
    FOnVehicleEvent OnEngineStateChanged;

protected:
    /**
     * @brief Update engine simulation
     * @param DeltaTime Frame delta time
     */
    void UpdateEngine(float DeltaTime);

    /**
     * @brief Update transmission and gear shifting
     * @param DeltaTime Frame delta time
     */
    void UpdateTransmission(float DeltaTime);

    /**
     * @brief Update suspension forces
     * @param DeltaTime Frame delta time
     */
    void UpdateSuspension(float DeltaTime);

    /**
     * @brief Update aerodynamics (drag and downforce)
     * @param DeltaTime Frame delta time
     */
    void UpdateAerodynamics(float DeltaTime);

    /**
     * @brief Calculate engine torque at current RPM
     * @return Engine torque in Nm
     */
    float CalculateEngineTorque() const;

    /**
     * @brief Calculate wheel forces for traction
     * @param WheelIndex Index of the wheel
     * @return Wheel force vector
     */
    FVector CalculateWheelForce(int32 WheelIndex) const;

private:
    // Current state
    UPROPERTY(VisibleAnywhere, Category = "State")
    float CurrentThrottle = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "State")
    float CurrentSteering = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "State")
    float CurrentBrake = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "State")
    bool bHandbrakeActive = false;

    UPROPERTY(VisibleAnywhere, Category = "State")
    float EngineRPM = 800.0f;

    UPROPERTY(VisibleAnywhere, Category = "State")
    EVehicleGear CurrentGear = EVehicleGear::Neutral;

    UPROPERTY(VisibleAnywhere, Category = "State")
    float GearShiftTimer = 0.0f;

    // Physics components
    UPROPERTY()
    UPrimitiveComponent* VehicleBody = nullptr;

    UPROPERTY()
    TArray<UPrimitiveComponent*> WheelComponents;

    // Performance tracking
    float LastFrameTime = 0.0f;
    FVector LastVelocity = FVector::ZeroVector;
};

/**
 * @struct FVehiclePerformanceData
 * @brief Vehicle performance metrics for UI and telemetry
 */
USTRUCT(BlueprintType)
struct FVehiclePerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RPM = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EVehicleGear Gear = EVehicleGear::Neutral;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Acceleration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bAirborne = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float EngineTorque = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FuelConsumption = 0.0f;
};