#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Core/Shared/SharedTypes.h"
#include "Core_VehiclePhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    None            UMETA(DisplayName = "None"),
    Raft            UMETA(DisplayName = "Primitive Raft"),
    Sled            UMETA(DisplayName = "Stone Sled"),
    Cart            UMETA(DisplayName = "Wooden Cart"),
    Boat            UMETA(DisplayName = "Dugout Boat"),
    Mount           UMETA(DisplayName = "Dinosaur Mount")
};

UENUM(BlueprintType)
enum class ECore_VehicleDriveMode : uint8
{
    Manual          UMETA(DisplayName = "Manual Push/Pull"),
    AnimalPowered   UMETA(DisplayName = "Animal Powered"),
    WaterCurrent    UMETA(DisplayName = "Water Current"),
    Gravity         UMETA(DisplayName = "Gravity Powered")
};

USTRUCT(BlueprintType)
struct FCore_VehiclePhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float MaxSpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Acceleration = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float AirResistance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float WaterResistance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float TurningRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float Stability = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    float CarryCapacity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    bool bCanFloat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    bool bRequiresAnimal = false;

    FCore_VehiclePhysicsData()
    {
        Mass = 100.0f;
        MaxSpeed = 500.0f;
        Acceleration = 200.0f;
        Friction = 0.7f;
        AirResistance = 0.1f;
        WaterResistance = 0.3f;
        TurningRadius = 300.0f;
        Stability = 0.8f;
        CarryCapacity = 500.0f;
        bCanFloat = false;
        bRequiresAnimal = false;
    }
};

USTRUCT(BlueprintType)
struct FCore_VehicleState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float CurrentLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    bool bIsInWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    bool bIsStable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
    APawn* Driver = nullptr;

    FCore_VehicleState()
    {
        Velocity = FVector::ZeroVector;
        CurrentSpeed = 0.0f;
        CurrentLoad = 0.0f;
        Durability = 100.0f;
        bIsInWater = false;
        bIsMoving = false;
        bIsStable = true;
        Driver = nullptr;
    }
};

/**
 * Core_VehiclePhysics - Prehistoric vehicle physics system
 * 
 * Handles physics simulation for primitive vehicles used in prehistoric survival:
 * - Rafts and boats for water travel
 * - Sleds and carts for land transport
 * - Dinosaur mounts for riding
 * 
 * Features realistic physics including:
 * - Mass and momentum simulation
 * - Friction and resistance forces
 * - Water buoyancy and current effects
 * - Load capacity and stability
 * - Durability and damage
 */
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
    ECore_VehicleType VehicleType = ECore_VehicleType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    ECore_VehicleDriveMode DriveMode = ECore_VehicleDriveMode::Manual;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    FCore_VehiclePhysicsData PhysicsData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle State")
    FCore_VehicleState CurrentState;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VehicleMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPrimitiveComponent* PhysicsBody = nullptr;

    // Core Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyForce(const FVector& Force, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyTorque(const FVector& Torque);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetVehicleInput(const FVector& InputVector, float Steering = 0.0f);

    // Movement Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Movement")
    void StartMovement();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Movement")
    void StopMovement();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Movement")
    void UpdateMovement(float DeltaTime);

    // Load Management
    UFUNCTION(BlueprintCallable, Category = "Vehicle Load")
    bool AddLoad(float LoadWeight);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Load")
    bool RemoveLoad(float LoadWeight);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Load")
    float GetLoadPercentage() const;

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Vehicle Environment")
    void UpdateWaterPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Environment")
    void UpdateTerrainPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Environment")
    void CheckEnvironmentalConditions();

    // Stability and Damage
    UFUNCTION(BlueprintCallable, Category = "Vehicle Stability")
    void UpdateStability(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Stability")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Stability")
    bool IsVehicleStable() const;

    // Driver Management
    UFUNCTION(BlueprintCallable, Category = "Vehicle Driver")
    bool SetDriver(APawn* NewDriver);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Driver")
    void RemoveDriver();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Driver")
    APawn* GetCurrentDriver() const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    FVector CalculateResistanceForces() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    float CalculateEffectiveSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    bool CanMove() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Utility")
    void ResetVehiclePhysics();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Events")
    void OnVehicleStartMoving();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Events")
    void OnVehicleStopMoving();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Events")
    void OnVehicleDamaged(float DamageAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Events")
    void OnVehicleOverloaded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Events")
    void OnVehicleEnterWater();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Events")
    void OnVehicleExitWater();

private:
    // Internal physics calculations
    void CalculatePhysicsForces(float DeltaTime);
    void ApplyFrictionForces(float DeltaTime);
    void ApplyResistanceForces(float DeltaTime);
    void UpdateVelocity(float DeltaTime);
    void CheckCollisions();
    void ValidatePhysicsState();

    // Internal state
    FVector AccumulatedForces = FVector::ZeroVector;
    FVector AccumulatedTorques = FVector::ZeroVector;
    FVector InputForces = FVector::ZeroVector;
    float SteeringInput = 0.0f;
    float LastUpdateTime = 0.0f;
    bool bPhysicsInitialized = false;
};