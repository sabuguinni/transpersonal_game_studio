#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Vehicles/WheeledVehicle.h"
#include "Vehicles/WheeledVehicleMovementComponent.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysicsSystem.generated.h"

class AWheeledVehicle;
class UWheeledVehicleMovementComponent;

/**
 * Vehicle Physics Configuration for different vehicle types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxEngineForce = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxBrakeForce = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxSteerAngle = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float TireGrip = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SuspensionStiffness = 5.88f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SuspensionDamping = 0.88f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SuspensionMaxRaise = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SuspensionMaxDrop = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float RollResistance = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AirResistance = 0.3f;

    FCore_VehiclePhysicsConfig()
    {
        Mass = 1500.0f;
        MaxEngineForce = 3000.0f;
        MaxBrakeForce = 1500.0f;
        MaxSteerAngle = 50.0f;
        TireGrip = 2.0f;
        SuspensionStiffness = 5.88f;
        SuspensionDamping = 0.88f;
        SuspensionMaxRaise = 0.2f;
        SuspensionMaxDrop = 0.2f;
        RollResistance = 0.015f;
        AirResistance = 0.3f;
    }
};

/**
 * Vehicle Surface Type for terrain interaction
 */
UENUM(BlueprintType)
enum class ECore_VehicleSurfaceType : uint8
{
    Road        UMETA(DisplayName = "Road"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Ice         UMETA(DisplayName = "Ice"),
    Water       UMETA(DisplayName = "Water"),
    Rock        UMETA(DisplayName = "Rock")
};

/**
 * Vehicle Damage Information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleDamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float EngineHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float TransmissionHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float SuspensionHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float TireCondition = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsEngineDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsImmobilized = false;

    FCore_VehicleDamageInfo()
    {
        EngineHealth = 100.0f;
        TransmissionHealth = 100.0f;
        SuspensionHealth = 100.0f;
        TireCondition = 100.0f;
        bIsEngineDestroyed = false;
        bIsImmobilized = false;
    }
};

/**
 * Core Vehicle Physics System
 * Manages realistic vehicle physics for prehistoric transportation
 */
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
    // Vehicle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FCore_VehiclePhysicsConfig VehicleConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FCore_VehicleDamageInfo DamageInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    ECore_VehicleSurfaceType CurrentSurfaceType = ECore_VehicleSurfaceType::Dirt;

    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableAdvancedPhysics = true;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAsyncPhysics = true;

    // Vehicle Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void InitializeVehiclePhysics(AWheeledVehicle* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateVehiclePhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyEngineForce(float ForceAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyBrakeForce(float BrakeAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplySteeringInput(float SteerAmount);

    // Surface Interaction
    UFUNCTION(BlueprintCallable, Category = "Surface")
    void SetSurfaceType(ECore_VehicleSurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Surface")
    float GetSurfaceGripMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Surface")
    float GetSurfaceResistanceMultiplier() const;

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Damage")
    void ApplyDamage(float DamageAmount, const FString& DamageType);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void RepairVehicle(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    bool IsVehicleOperational() const;

    // Physics Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnablePhysicsDebug(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetVehiclePhysicsStatus() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsSettings();

protected:
    // Internal Physics Management
    void UpdateSuspensionPhysics(float DeltaTime);
    void UpdateTirePhysics(float DeltaTime);
    void UpdateEnginePhysics(float DeltaTime);
    void UpdateAerodynamics(float DeltaTime);
    void UpdateDamageEffects(float DeltaTime);

    // Surface Detection
    void DetectSurfaceType();
    void ApplySurfaceEffects();

    // Performance Optimization
    void OptimizePhysicsLOD();
    void UpdatePhysicsLOD(float DistanceToPlayer);

private:
    // Cached Components
    UPROPERTY()
    AWheeledVehicle* OwnerVehicle;

    UPROPERTY()
    UWheeledVehicleMovementComponent* VehicleMovement;

    // Physics State
    FVector LastVelocity;
    FVector LastAngularVelocity;
    float LastUpdateTime;
    float AccumulatedDeltaTime;

    // Performance Metrics
    float PhysicsUpdateTimer;
    int32 PhysicsCalculationsThisFrame;
    float AveragePhysicsTime;

    // Debug State
    bool bDebugPhysics;
    float DebugUpdateInterval;
    float LastDebugUpdate;
};