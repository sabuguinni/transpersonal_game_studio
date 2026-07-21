#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysicsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Mass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Deceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TurnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float SuspensionStiffness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float SuspensionDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TractionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float StabilityFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    bool bCanFlip;

    FCore_VehiclePhysicsProfile()
    {
        Mass = 1500.0f;
        MaxSpeed = 2000.0f;
        Acceleration = 800.0f;
        Deceleration = 1200.0f;
        TurnRadius = 8.0f;
        SuspensionStiffness = 50000.0f;
        SuspensionDamping = 4500.0f;
        TractionMultiplier = 1.0f;
        StabilityFactor = 0.8f;
        bCanFlip = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleWheelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float WheelRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float WheelWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float FrictionCoefficient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float SlipAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float RollingResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    bool bIsDriveWheel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    bool bIsSteerWheel;

    FCore_VehicleWheelData()
    {
        WheelRadius = 35.0f;
        WheelWidth = 20.0f;
        FrictionCoefficient = 1.0f;
        SlipAngle = 0.0f;
        RollingResistance = 0.02f;
        bIsDriveWheel = false;
        bIsSteerWheel = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleEngineData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Physics")
    float MaxTorque;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Physics")
    float MaxRPM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Physics")
    float IdleRPM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Physics")
    float TorqueCurveMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Physics")
    float EngineInertia;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Physics")
    float EngineBraking;

    FCore_VehicleEngineData()
    {
        MaxTorque = 400.0f;
        MaxRPM = 6000.0f;
        IdleRPM = 800.0f;
        TorqueCurveMultiplier = 1.0f;
        EngineInertia = 0.2f;
        EngineBraking = 0.05f;
    }
};

/**
 * Core_VehiclePhysicsManager
 * 
 * Manages realistic vehicle physics for prehistoric vehicles in the Transpersonal Game.
 * Handles primitive carts, sleds, and animal-drawn vehicles with realistic physics simulation.
 * Integrates with terrain physics for surface-appropriate vehicle behavior.
 * 
 * Features:
 * - Realistic vehicle dynamics with mass, acceleration, and suspension
 * - Terrain-aware traction and stability calculations
 * - Wheel physics simulation for different surface types
 * - Engine simulation for powered vehicles
 * - Vehicle stability and rollover prevention
 * - Integration with Core_TerrainPhysicsManager for surface properties
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Vehicle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FCore_VehiclePhysicsProfile VehicleProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    TArray<FCore_VehicleWheelData> WheelData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FCore_VehicleEngineData EngineData;

    // Current Vehicle State
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentRPM;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSteerAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    bool bIsOnGround;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    bool bIsFlipped;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    FVector VehicleVelocity;

    // Input State
    UPROPERTY(BlueprintReadWrite, Category = "Vehicle Input")
    float ThrottleInput;

    UPROPERTY(BlueprintReadWrite, Category = "Vehicle Input")
    float SteerInput;

    UPROPERTY(BlueprintReadWrite, Category = "Vehicle Input")
    float BrakeInput;

    UPROPERTY(BlueprintReadWrite, Category = "Vehicle Input")
    bool bHandbrakeEngaged;

    // Physics Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    class UPrimitiveComponent* VehicleBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    TArray<class UPrimitiveComponent*> WheelComponents;

    // Vehicle Physics Methods
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateVehiclePhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyEngineForce(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplySteeringForce(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyBrakingForce(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateSuspension(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void CalculateWheelTraction();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void CheckVehicleStability();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleTerrainInteraction();

    // Vehicle State Methods
    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    bool IsVehicleMoving() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    bool IsVehicleStable() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    float GetVehicleSpeedKMH() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    FVector GetVehicleForwardVector() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    FVector GetVehicleRightVector() const;

    // Vehicle Control Methods
    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetThrottleInput(float Throttle);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetSteerInput(float Steer);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetBrakeInput(float Brake);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetHandbrake(bool bEngaged);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void ResetVehicle();

    // Terrain Integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void UpdateTerrainTraction();

    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float GetTerrainFrictionMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    bool IsOnValidTerrain() const;

protected:
    // Internal Physics Calculations
    void CalculateEngineOutput(float DeltaTime);
    void CalculateWheelForces(float DeltaTime);
    void ApplyDownforce();
    void UpdateVehicleOrientation(float DeltaTime);
    void HandleCollisionResponse();

    // Terrain Detection
    FHitResult PerformGroundTrace() const;
    ETerrainType GetCurrentTerrainType() const;
    float GetSurfaceSlope() const;

    // Vehicle Dynamics
    FVector CalculateLateralForce() const;
    FVector CalculateLongitudinalForce() const;
    float CalculateAerodynamicDrag() const;
    float CalculateRollingResistance() const;

    // Safety Systems
    void CheckRolloverRisk();
    void ApplyStabilityControl();
    void LimitMaxSpeed();

private:
    // Internal State
    float AccumulatedDistance;
    float LastUpdateTime;
    FVector LastPosition;
    FRotator LastRotation;
    
    // Performance Optimization
    int32 PhysicsUpdateCounter;
    bool bNeedsFullUpdate;
    
    // Cached References
    class UCore_TerrainPhysicsManager* TerrainPhysicsManager;
    class UWorld* CachedWorld;
};