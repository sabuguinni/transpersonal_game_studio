#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WheeledVehicle.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysics.generated.h"

/**
 * Core_VehiclePhysics - Prehistoric vehicle physics system for stone-age transportation
 * Handles realistic physics for primitive vehicles: rafts, sleds, carts, and domesticated mounts
 * Integrates with terrain physics for authentic prehistoric movement mechanics
 */

UENUM(BlueprintType)
enum class ECore_VehicleType : uint8
{
    None            UMETA(DisplayName = "None"),
    WoodenRaft      UMETA(DisplayName = "Wooden Raft"),
    StoneSled       UMETA(DisplayName = "Stone Sled"),
    WoodenCart      UMETA(DisplayName = "Wooden Cart"),
    AnimalMount     UMETA(DisplayName = "Animal Mount"),
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECore_VehicleState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Moving          UMETA(DisplayName = "Moving"),
    Turning         UMETA(DisplayName = "Turning"),
    Braking         UMETA(DisplayName = "Braking"),
    Stuck           UMETA(DisplayName = "Stuck"),
    Damaged         UMETA(DisplayName = "Damaged"),
    MAX             UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehiclePhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float Mass = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float Acceleration = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float BrakingForce = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float TurningRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float TerrainFrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float WaterResistance = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    float StabilityFactor = 0.7f;

    FCore_VehiclePhysicsProperties()
    {
        Mass = 500.0f;
        MaxSpeed = 800.0f;
        Acceleration = 600.0f;
        BrakingForce = 1200.0f;
        TurningRadius = 300.0f;
        TerrainFrictionMultiplier = 1.0f;
        WaterResistance = 0.8f;
        StabilityFactor = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_VehicleTerrainResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Response")
    ETerrainType CurrentTerrain = ETerrainType::Grass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Response")
    float TerrainSpeedModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Response")
    float TerrainStabilityModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Response")
    bool bIsStuck = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Response")
    float StuckTimer = 0.0f;

    FCore_VehicleTerrainResponse()
    {
        CurrentTerrain = ETerrainType::Grass;
        TerrainSpeedModifier = 1.0f;
        TerrainStabilityModifier = 1.0f;
        bIsStuck = false;
        StuckTimer = 0.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACore_VehiclePhysics : public AWheeledVehicle
{
    GENERATED_BODY()

public:
    ACore_VehiclePhysics();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Vehicle Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    ECore_VehicleType VehicleType = ECore_VehicleType::WoodenCart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Setup")
    FCore_VehiclePhysicsProperties PhysicsProperties;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    ECore_VehicleState CurrentState = ECore_VehicleState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    FCore_VehicleTerrainResponse TerrainResponse;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCore_TerrainPhysicsIntegrator* TerrainPhysicsIntegrator;

public:
    // Movement Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Movement")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Movement")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Movement")
    void ApplyBrakes(bool bBraking);

    // Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void UpdateTerrainResponse();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyTerrainEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void CheckVehicleStability();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void HandleVehicleDamage(float DamageAmount);

    // Configuration Functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Setup")
    void SetVehicleType(ECore_VehicleType NewType);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Setup")
    void ConfigureVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Setup")
    void SetPhysicsProperties(const FCore_VehiclePhysicsProperties& NewProperties);

    // State Functions
    UFUNCTION(BlueprintPure, Category = "Vehicle State")
    ECore_VehicleState GetVehicleState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Vehicle State")
    FCore_VehicleTerrainResponse GetTerrainResponse() const { return TerrainResponse; }

    UFUNCTION(BlueprintPure, Category = "Vehicle State")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle State")
    bool IsVehicleStuck() const { return TerrainResponse.bIsStuck; }

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawVehicleDebugInfo();

protected:
    // Internal Functions
    void UpdateVehicleState(float DeltaTime);
    void CalculateTerrainEffects();
    void ApplyPhysicsForces(float DeltaTime);
    void HandleVehicleInput(float ForwardInput, float RightInput);
    void ProcessTerrainInteraction();

    // Physics Calculation Functions
    float CalculateSpeedModifier() const;
    float CalculateStabilityModifier() const;
    FVector CalculateTerrainForce() const;
    bool ShouldVehicleGetStuck() const;

    // Timer and State Management
    float StateTimer = 0.0f;
    float LastUpdateTime = 0.0f;
    FVector LastPosition = FVector::ZeroVector;
    FRotator LastRotation = FRotator::ZeroRotator;

    // Input Values
    float CurrentForwardInput = 0.0f;
    float CurrentRightInput = 0.0f;
    bool bIsBraking = false;
};

/**
 * Core_VehiclePhysicsComponent - Component for adding vehicle physics to any actor
 * Lightweight component version for objects that need vehicle-like behavior
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_VehiclePhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    ECore_VehicleType VehicleType = ECore_VehicleType::WoodenCart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics")
    FCore_VehiclePhysicsProperties PhysicsProperties;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
    FCore_VehicleTerrainResponse TerrainResponse;

public:
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void ApplyVehicleForce(const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
    void SetVehiclePhysicsProperties(const FCore_VehiclePhysicsProperties& NewProperties);

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    FCore_VehiclePhysicsProperties GetVehiclePhysicsProperties() const { return PhysicsProperties; }

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics")
    FCore_VehicleTerrainResponse GetTerrainResponse() const { return TerrainResponse; }
};