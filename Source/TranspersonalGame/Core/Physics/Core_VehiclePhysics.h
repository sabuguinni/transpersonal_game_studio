#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "WheeledVehiclePawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/Engine.h"
#include "Core_VehiclePhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    None UMETA(DisplayName = "None"),
    Cart UMETA(DisplayName = "Wooden Cart"),
    Sled UMETA(DisplayName = "Animal Sled"),
    Raft UMETA(DisplayName = "Water Raft"),
    Mount UMETA(DisplayName = "Animal Mount")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float MaxSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Acceleration = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float BrakeForce = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float TurnRate = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Mass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    float CarryCapacity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stats")
    bool bRequiresAnimal = false;

    FCore_VehicleStats()
    {
        MaxSpeed = 1000.0f;
        Acceleration = 500.0f;
        BrakeForce = 1500.0f;
        TurnRate = 45.0f;
        Mass = 1000.0f;
        Durability = 100.0f;
        CarryCapacity = 500.0f;
        bRequiresAnimal = false;
    }
};

/**
 * Core Vehicle Physics System
 * Handles primitive vehicle physics for carts, sleds, rafts, and animal mounts
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_VehiclePhysics : public APawn
{
    GENERATED_BODY()

public:
    ACore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Vehicle Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* AnimalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    class UFloatingPawnMovement* MovementComponent;

    // Vehicle Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    ECore_VehicleType VehicleType = ECore_VehicleType::Cart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    FCore_VehicleStats VehicleStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    float CurrentSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    float CurrentDurability = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    float CurrentCargoWeight = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    bool bIsOperational = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
    bool bHasAnimal = false;

    // Input Values
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    float ThrottleInput = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    float SteeringInput = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    bool bBrakePressed = false;

    // Vehicle Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetThrottleInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetSteeringInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void SetBrakeInput(bool bPressed);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void AttachAnimal(class APawn* Animal);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void DetachAnimal();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void AddCargo(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void RemoveCargo(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void RepairVehicle(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void DamageVehicle(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    bool CanOperate() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    float GetSpeedKmh() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    float GetCargoPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    float GetDurabilityPercentage() const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleDamaged, float, DamageAmount);
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Events")
    FOnVehicleDamaged OnVehicleDamaged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleDestroyed);
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Events")
    FOnVehicleDestroyed OnVehicleDestroyed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimalAttached, APawn*, Animal);
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Events")
    FOnAnimalAttached OnAnimalAttached;

protected:
    // Input Binding Functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void ApplyBrake();
    void ReleaseBrake();

    // Internal Functions
    void UpdateMovement(float DeltaTime);
    void UpdateVehiclePhysics(float DeltaTime);
    void ApplyTerrainEffects(float DeltaTime);
    void CheckVehicleIntegrity();
    void UpdateAnimalBehavior(float DeltaTime);

    // Physics Calculations
    FVector CalculateMovementForce() const;
    FVector CalculateBrakeForce() const;
    FVector CalculateTerrainResistance() const;
    float CalculateEffectiveAcceleration() const;
    float CalculateEffectiveMaxSpeed() const;

private:
    // Internal state
    FVector LastVelocity = FVector::ZeroVector;
    float AccumulatedDamage = 0.0f;
    float TerrainDifficulty = 1.0f;
    
    // Animal reference
    UPROPERTY()
    APawn* AttachedAnimal = nullptr;

    // Timers
    float DamageCheckTimer = 0.0f;
    float PhysicsUpdateTimer = 0.0f;
};