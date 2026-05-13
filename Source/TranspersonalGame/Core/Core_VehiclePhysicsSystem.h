#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Vehicles/WheeledVehicle.h"
#include "Vehicles/WheeledVehicleMovementComponent.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysicsSystem.generated.h"

class AWheeledVehicle;
class UWheeledVehicleMovementComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Mass = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxEngineForce = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxBrakingForce = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxSteeringAngle = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float SuspensionStiffness = 5.88f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float SuspensionDamping = 0.88f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float SuspensionMaxRaise = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float SuspensionMaxDrop = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float WheelRadius = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float WheelWidth = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TireFriction = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float RollResistance = 0.015f;

    FCore_VehiclePhysicsConfig()
    {
        Mass = 1500.0f;
        MaxEngineForce = 3000.0f;
        MaxBrakingForce = 1500.0f;
        MaxSteeringAngle = 50.0f;
        SuspensionStiffness = 5.88f;
        SuspensionDamping = 0.88f;
        SuspensionMaxRaise = 0.3f;
        SuspensionMaxDrop = 0.3f;
        WheelRadius = 35.0f;
        WheelWidth = 20.0f;
        TireFriction = 3.0f;
        RollResistance = 0.015f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float EngineRPM = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float SteeringAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float ThrottleInput = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float BrakeInput = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    bool bIsOnGround = true;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    bool bIsSkidding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    FVector AngularVelocity = FVector::ZeroVector;

    FCore_VehiclePhysicsState()
    {
        CurrentSpeed = 0.0f;
        EngineRPM = 0.0f;
        SteeringAngle = 0.0f;
        ThrottleInput = 0.0f;
        BrakeInput = 0.0f;
        bIsOnGround = true;
        bIsSkidding = false;
        Velocity = FVector::ZeroVector;
        AngularVelocity = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FCore_VehiclePhysicsConfig PhysicsConfig;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics")
    FCore_VehiclePhysicsState CurrentState;

    // Vehicle reference
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics")
    TObjectPtr<AWheeledVehicle> OwnerVehicle;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics")
    TObjectPtr<UWheeledVehicleMovementComponent> MovementComponent;

    // Core vehicle physics functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateVehiclePhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyEngineForce(float Force);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyBrakingForce(float Force);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplySteeringInput(float SteeringInput);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetVehiclePhysicsConfig(const FCore_VehiclePhysicsConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    FCore_VehiclePhysicsState GetVehiclePhysicsState() const;

    // Suspension system
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateSuspensionSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void AdjustSuspensionStiffness(float NewStiffness);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void AdjustSuspensionDamping(float NewDamping);

    // Tire physics
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateTirePhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetTireFriction(float NewFriction);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool IsVehicleSkidding() const;

    // Terrain interaction
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    float GetTerrainFrictionMultiplier() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void LogVehiclePhysicsPerformance() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ResetVehiclePhysics();

private:
    // Internal physics calculations
    void CalculateEngineForces(float DeltaTime);
    void CalculateSuspensionForces(float DeltaTime);
    void CalculateTireForces(float DeltaTime);
    void UpdateVehicleState(float DeltaTime);

    // Performance tracking
    float LastUpdateTime;
    int32 PhysicsUpdateCount;
    float AverageUpdateTime;

    // Cached references
    UPrimitiveComponent* VehicleRootComponent;
    TArray<UPrimitiveComponent*> WheelComponents;
};