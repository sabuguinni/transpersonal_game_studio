#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Core_VehiclePhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    Cart            UMETA(DisplayName = "Wooden Cart"),
    Raft            UMETA(DisplayName = "River Raft"),
    Sled            UMETA(DisplayName = "Snow Sled"),
    Mount           UMETA(DisplayName = "Dinosaur Mount")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float Acceleration = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float Deceleration = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float TurningRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    ECore_VehicleType VehicleType = ECore_VehicleType::Cart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    bool bRequiresWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    bool bCanClimbSlopes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float MaxSlopeAngle = 30.0f;
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    FCore_VehicleSettings VehicleSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    bool bIsOnValidTerrain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float ThrottleInput = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float SteeringInput = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bBrakeInput = false;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetThrottleInput(float Throttle);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetSteeringInput(float Steering);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void SetBrakeInput(bool bBrake);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Control")
    void ApplyImpulse(const FVector& Impulse);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Info")
    float GetSpeedKMH() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Info")
    bool CanMoveOnCurrentTerrain() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Setup")
    void ConfigureForVehicleType(ECore_VehicleType NewType);

private:
    UPROPERTY()
    APawn* OwnerPawn;

    FVector LastFrameLocation = FVector::ZeroVector;
    float TerrainCheckTimer = 0.0f;
    const float TerrainCheckInterval = 0.5f;

    void UpdateMovement(float DeltaTime);
    void CheckTerrainValidity();
    void ApplyPhysicsForces(float DeltaTime);
    FVector CalculateMovementForce(float DeltaTime);
    FVector CalculateSteeringForce(float DeltaTime);
};