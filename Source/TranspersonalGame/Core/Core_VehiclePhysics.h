#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "Core_VehiclePhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    Raft              UMETA(DisplayName = "Primitive Raft"),
    Sled              UMETA(DisplayName = "Stone Sled"),
    Cart              UMETA(DisplayName = "Wooden Cart"),
    Boat              UMETA(DisplayName = "Dugout Boat"),
    RidingMount       UMETA(DisplayName = "Riding Mount")
};

UENUM(BlueprintType)
enum class ECore_VehicleState : uint8
{
    Idle              UMETA(DisplayName = "Idle"),
    Moving            UMETA(DisplayName = "Moving"),
    Turning           UMETA(DisplayName = "Turning"),
    Braking           UMETA(DisplayName = "Braking"),
    Stuck             UMETA(DisplayName = "Stuck"),
    Damaged           UMETA(DisplayName = "Damaged")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Acceleration = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float TurnRate = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float BrakingForce = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float CarryCapacity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float WaterResistance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float TerrainAdaptation = 1.0f;

    FCore_VehicleStats()
    {
        MaxSpeed = 800.0f;
        Acceleration = 600.0f;
        TurnRate = 90.0f;
        BrakingForce = 1200.0f;
        Durability = 100.0f;
        CarryCapacity = 500.0f;
        WaterResistance = 0.5f;
        TerrainAdaptation = 1.0f;
    }
};

/**
 * Core Vehicle Physics Component
 * Handles primitive prehistoric vehicle physics including rafts, sleds, carts, and riding mounts
 * Implements realistic physics simulation for prehistoric transportation methods
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Vehicle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Config")
    ECore_VehicleType VehicleType = ECore_VehicleType::Raft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Config")
    FCore_VehicleStats VehicleStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Config")
    class UPrimitiveComponent* VehicleBody = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Config")
    TArray<class UPrimitiveComponent*> WheelComponents;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    ECore_VehicleState CurrentState = ECore_VehicleState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    float CurrentDurability = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    float CurrentLoad = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    FVector Velocity = FVector::ZeroVector;

    // Input Values
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
    float ThrottleInput = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
    float SteeringInput = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
    bool bBrakeInput = false;

    // Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetThrottleInput(float InThrottle);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetSteeringInput(float InSteering);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetBrakeInput(bool bInBrake);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyForces(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateVehicleState();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleWaterPhysics();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void RepairVehicle(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    bool CanCarryLoad(float AdditionalWeight) const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void AddLoad(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void RemoveLoad(float Weight);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    float GetSpeedKmh() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    float GetDurabilityPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    float GetLoadPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    bool IsInWater() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    bool IsOnRoughTerrain() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehicle(ECore_VehicleType InVehicleType);

private:
    // Internal physics calculations
    void CalculateMovementForces(float DeltaTime);
    void ApplyFriction(float DeltaTime);
    void ApplyAirResistance(float DeltaTime);
    void ApplyWaterResistance(float DeltaTime);
    void UpdateWheelPhysics(float DeltaTime);
    void CheckTerrainType();
    void UpdateDurability(float DeltaTime);

    // Internal state tracking
    float LastUpdateTime = 0.0f;
    FVector LastPosition = FVector::ZeroVector;
    bool bIsInWater = false;
    bool bIsOnRoughTerrain = false;
    float TerrainRoughness = 0.0f;
    float WaterDepth = 0.0f;
};