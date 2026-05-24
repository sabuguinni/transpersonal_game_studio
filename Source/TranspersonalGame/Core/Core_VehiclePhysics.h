#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysics.generated.h"

/**
 * Core_VehiclePhysics - Realistic vehicle physics system for prehistoric vehicles
 * Handles primitive carts, sleds, and basic transportation with realistic physics
 * Integrates with terrain deformation and material physics systems
 */

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    None            UMETA(DisplayName = "None"),
    WoodenCart      UMETA(DisplayName = "Wooden Cart"),
    StoneSled       UMETA(DisplayName = "Stone Sled"),
    LogRaft         UMETA(DisplayName = "Log Raft"),
    AnimalSled      UMETA(DisplayName = "Animal-Drawn Sled")
};

UENUM(BlueprintType)
enum class ECore_VehicleState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Moving          UMETA(DisplayName = "Moving"),
    Stuck           UMETA(DisplayName = "Stuck"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Destroyed       UMETA(DisplayName = "Destroyed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleWheelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float WheelRadius = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float WheelWidth = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float SuspensionStiffness = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float DampingCompression = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float DampingRelaxation = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    float FrictionSlip = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    bool bIsPowered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Physics")
    bool bCanSteer = false;

    FCore_VehicleWheelData()
    {
        WheelRadius = 30.0f;
        WheelWidth = 15.0f;
        SuspensionStiffness = 5000.0f;
        DampingCompression = 200.0f;
        DampingRelaxation = 300.0f;
        FrictionSlip = 2.0f;
        bIsPowered = false;
        bCanSteer = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Mass = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Acceleration = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Deceleration = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TurningRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float CenterOfMassOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float DragCoefficient = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float RollingResistance = 0.02f;

    FCore_VehiclePhysicsData()
    {
        Mass = 500.0f;
        MaxSpeed = 800.0f;
        Acceleration = 600.0f;
        Deceleration = 800.0f;
        TurningRadius = 500.0f;
        CenterOfMassOffset = 0.0f;
        DragCoefficient = 0.3f;
        RollingResistance = 0.02f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Vehicle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    ECore_VehicleType VehicleType = ECore_VehicleType::WoodenCart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    FCore_VehiclePhysicsData PhysicsData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    TArray<FCore_VehicleWheelData> WheelConfiguration;

    // Vehicle State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    ECore_VehicleState CurrentState = ECore_VehicleState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSteerAngle = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    float HealthPercentage = 100.0f;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VehicleMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UStaticMeshComponent*> WheelMeshes;

    // Vehicle Control Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetThrottleInput(float ThrottleValue);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetSteeringInput(float SteeringValue);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetBrakeInput(float BrakeValue);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void StartEngine();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void StopEngine();

    // Vehicle Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyForce(const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyTorque(const FVector& Torque);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateSuspension(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateWheelPhysics(float DeltaTime);

    // Terrain Integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void CheckTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float GetTerrainFriction() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void CreateTerrainDeformation();

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void RepairVehicle(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    bool CanOperate() const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    FVector GetCenterOfMass() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    float GetTotalMass() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    bool IsOnGround() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    void ResetVehicle();

    // Debug Functions
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugDrawVehicleInfo();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateVehicleSetup();

private:
    // Internal state
    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    bool bEngineRunning = false;
    
    FVector LastPosition = FVector::ZeroVector;
    FRotator LastRotation = FRotator::ZeroRotator;
    
    // Physics simulation
    void UpdateVehiclePhysics(float DeltaTime);
    void CalculateForces(float DeltaTime, FVector& OutForce, FVector& OutTorque);
    void ApplyAerodynamics(FVector& Force);
    void ApplyRollingResistance(FVector& Force);
    void UpdateVehicleState();
    
    // Wheel simulation
    void SimulateWheelPhysics(int32 WheelIndex, float DeltaTime);
    float CalculateWheelLoad(int32 WheelIndex) const;
    FVector CalculateWheelForce(int32 WheelIndex, float Load) const;
    
    // Terrain interaction
    void PerformTerrainTrace();
    void UpdateTerrainEffects();
    
    // Component management
    void InitializeComponents();
    void SetupWheelMeshes();
    void UpdateWheelTransforms();
};