#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysicsSystem.generated.h"

/**
 * Core Vehicle Physics System
 * Handles realistic vehicle physics for prehistoric transportation
 * Supports rafts, sleds, and primitive wheeled vehicles
 * Integrates with terrain physics and material systems
 */

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    None            UMETA(DisplayName = "None"),
    Raft            UMETA(DisplayName = "Water Raft"),
    Sled            UMETA(DisplayName = "Ground Sled"), 
    Cart            UMETA(DisplayName = "Wheeled Cart"),
    Mount           UMETA(DisplayName = "Animal Mount")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    ECore_VehicleType VehicleType = ECore_VehicleType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxSpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Acceleration = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TurningRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float StabilityFactor = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TerrainAdaptation = 0.6f;

    FCore_VehiclePhysicsConfig()
    {
        VehicleType = ECore_VehicleType::None;
        Mass = 100.0f;
        MaxSpeed = 500.0f;
        Acceleration = 800.0f;
        TurningRadius = 300.0f;
        StabilityFactor = 0.8f;
        TerrainAdaptation = 0.6f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float SteeringInput = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float ThrottleInput = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    bool bIsGrounded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    bool bIsInWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float TerrainGrip = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float DamageLevel = 0.0f;

    FCore_VehicleState()
    {
        Velocity = FVector::ZeroVector;
        CurrentSpeed = 0.0f;
        SteeringInput = 0.0f;
        ThrottleInput = 0.0f;
        bIsGrounded = false;
        bIsInWater = false;
        TerrainGrip = 1.0f;
        DamageLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Vehicle Physics Methods
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehicle(ECore_VehicleType InVehicleType, float InMass = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetVehicleInput(float Throttle, float Steering);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyVehiclePhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleWaterPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleGroundPhysics(float DeltaTime);

    // Vehicle State Management
    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    FCore_VehicleState GetVehicleState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    ECore_VehicleType GetVehicleType() const { return PhysicsConfig.VehicleType; }

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    bool IsVehicleOperational() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle State")
    void RepairVehicle(float RepairAmount);

    // Terrain Integration
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float CalculateTerrainGrip(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void HandleTerrainDamage(float DamageAmount);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMonitoring(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

protected:
    // Vehicle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Config", meta = (AllowPrivateAccess = "true"))
    FCore_VehiclePhysicsConfig PhysicsConfig;

    // Current Vehicle State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State", meta = (AllowPrivateAccess = "true"))
    FCore_VehicleState CurrentState;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UPrimitiveComponent* VehicleBody;

    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float AirResistance = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float WaterResistance = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float GroundFriction = 0.7f;

    // Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bPerformanceMonitoring = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AverageFrameTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 PhysicsCalculationsPerSecond = 0;

private:
    // Internal Physics Calculations
    void CalculateMovementForces(float DeltaTime);
    void ApplyEnvironmentalForces(float DeltaTime);
    void UpdateVehicleOrientation(float DeltaTime);
    void CheckCollisions();
    void UpdatePerformanceMetrics(float DeltaTime);

    // Internal State
    FVector AccumulatedForces;
    float LastFrameTime;
    int32 FrameCounter;
    float PerformanceTimer;
};