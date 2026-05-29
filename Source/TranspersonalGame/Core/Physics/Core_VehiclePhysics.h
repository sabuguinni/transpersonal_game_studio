#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Core_VehiclePhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    None = 0,
    Cart = 1,
    Raft = 2,
    Sled = 3,
    Mount = 4
};

USTRUCT(BlueprintType)
struct FCore_VehicleStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Acceleration = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Deceleration = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float TurnRate = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Mass = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Durability = 100.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_VehiclePhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehicle(ECore_VehicleType VehicleType);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetThrottleInput(float ThrottleValue);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetSteeringInput(float SteeringValue);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyBrakes(float BrakeForce);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetVehicleType(ECore_VehicleType NewType);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool IsVehicleMoving() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void RepairVehicle(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void DamageVehicle(float DamageAmount);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    ECore_VehicleType VehicleType = ECore_VehicleType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    FCore_VehicleStats VehicleStats;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    float CurrentDurability = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Input")
    float ThrottleInput = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Input")
    float SteeringInput = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Input")
    float BrakeInput = 0.0f;

private:
    void UpdateMovement(float DeltaTime);
    void ApplyPhysicsForces(float DeltaTime);
    void UpdateVehicleStats();
    void HandleTerrainInteraction();
    
    FVector Velocity = FVector::ZeroVector;
    FVector LastPosition = FVector::ZeroVector;
    float LastUpdateTime = 0.0f;
};

#include "Core_VehiclePhysics.generated.h"