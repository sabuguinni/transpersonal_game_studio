#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Core_VehiclePhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    Cart,
    Sled,
    Raft,
    Glider
};

USTRUCT(BlueprintType)
struct FCore_WheelConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    float Radius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    float Mass = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    float Friction = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    float RollingResistance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    bool bCanSteer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    bool bCanBrake = true;

    FCore_WheelConfig()
    {
        Radius = 50.0f;
        Mass = 20.0f;
        Friction = 3.0f;
        RollingResistance = 0.1f;
        bCanSteer = false;
        bCanBrake = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_VehicleStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float MaxSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float Acceleration = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float BrakeForce = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float SteerAngle = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float Mass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float DragCoefficient = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float DownForce = 0.0f;

    FCore_VehicleStats()
    {
        MaxSpeed = 1000.0f;
        Acceleration = 500.0f;
        BrakeForce = 1500.0f;
        SteerAngle = 30.0f;
        Mass = 1000.0f;
        DragCoefficient = 0.3f;
        DownForce = 0.0f;
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

public:
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetThrottleInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetSteerInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetBrakeInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetHandbrakeInput(bool bPressed);

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    float GetEngineRPM() const { return CurrentRPM; }

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    FVector GetVelocity() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void ResetVehicle();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetVehicleStats(const FCore_VehicleStats& NewStats);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* VehicleRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    ECore_VehicleType VehicleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    FCore_VehicleStats VehicleStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    TArray<FCore_WheelConfig> WheelConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float ThrottleInput;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float SteerInput;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float BrakeInput;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    bool bHandbrakePressed;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float CurrentRPM;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float CurrentGear;

    TArray<UStaticMeshComponent*> WheelMeshes;
    TArray<UPhysicsConstraintComponent*> WheelConstraints;

    void UpdateVehiclePhysics(float DeltaTime);
    void UpdateWheelPhysics(float DeltaTime);
    void ApplyEngineForce(float DeltaTime);
    void ApplyBrakeForce(float DeltaTime);
    void ApplySteering(float DeltaTime);
    void ApplyAerodynamics(float DeltaTime);
    void UpdateEngineSound(float DeltaTime);
    void CreateWheels();
    void UpdateWheelRotation(float DeltaTime);

    // Input handling
    void OnThrottlePressed();
    void OnThrottleReleased();
    void OnBrakePressed();
    void OnBrakeReleased();
    void OnSteerLeft();
    void OnSteerRight();
    void OnSteerStop();
    void OnHandbrakePressed();
    void OnHandbrakeReleased();

    FVector CalculateDownforce() const;
    FVector CalculateDrag() const;
    float CalculateWheelLoad(int32 WheelIndex) const;
    bool IsGrounded() const;
};