// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "WheeledVehiclePawn.h"
#include "ChaosWheeledVehicle.h"
#include "ChaosVehicleWheel.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "VehiclePhysicsSystemV2.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVehiclePhysics, Log, All);

/**
 * Vehicle types for different physics configurations
 */
UENUM(BlueprintType)
enum class EVehicleType : uint8
{
    LightVehicle = 0,       // Cars, motorcycles
    HeavyVehicle = 1,       // Trucks, tanks
    OffRoadVehicle = 2,     // ATVs, buggies
    AmphibiousVehicle = 3,  // Boats, amphibious vehicles
    AerialVehicle = 4,      // Helicopters, aircraft
    CustomVehicle = 5       // Custom configuration
};

/**
 * Vehicle drive modes
 */
UENUM(BlueprintType)
enum class EVehicleDriveMode : uint8
{
    FrontWheelDrive = 0,    // FWD
    RearWheelDrive = 1,     // RWD
    AllWheelDrive = 2,      // AWD
    FourWheelDrive = 3      // 4WD
};

/**
 * Vehicle transmission types
 */
UENUM(BlueprintType)
enum class EVehicleTransmissionType : uint8
{
    Manual = 0,             // Manual transmission
    Automatic = 1,          // Automatic transmission
    CVT = 2,               // Continuously Variable Transmission
    Electric = 3           // Electric motor (no gears)
};

/**
 * Vehicle surface types for different physics responses
 */
UENUM(BlueprintType)
enum class EVehicleSurfaceType : uint8
{
    Asphalt = 0,           // Road surface
    Dirt = 1,              // Dirt road
    Grass = 2,             // Grass terrain
    Sand = 3,              // Sandy surface
    Snow = 4,              // Snow/ice
    Mud = 5,               // Muddy terrain
    Rock = 6,              // Rocky surface
    Water = 7,             // Water surface
    Metal = 8,             // Metal grating
    Wood = 9               // Wooden surface
};

/**
 * Vehicle physics configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVehiclePhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVehicleType VehicleType = EVehicleType::LightVehicle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVehicleDriveMode DriveMode = EVehicleDriveMode::RearWheelDrive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVehicleTransmissionType TransmissionType = EVehicleTransmissionType::Automatic;

    // Mass and inertia
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Mass = 1500.0f; // kg

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterOfMass = FVector(0.0f, 0.0f, -50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector InertiaTensor = FVector(1000.0f, 2000.0f, 1500.0f);

    // Engine configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxEngineRPM = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTorque = 400.0f; // Nm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IdleRPM = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> TorqueCurve; // RPM vs Torque

    // Transmission configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> GearRatios;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FinalDriveRatio = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClutchStrength = 10.0f;

    // Suspension configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpringRate = 25000.0f; // N/m

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DampingRate = 3000.0f; // Ns/m

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuspensionTravel = 30.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AntiRollBarForce = 5000.0f;

    // Wheel configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WheelRadius = 35.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WheelWidth = 25.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WheelMass = 20.0f; // kg

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TireGrip = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TireFriction = 0.9f;

    // Aerodynamics
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DragCoefficient = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DownforceCoefficient = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FrontalArea = 2.5f; // m²

    // Stability systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableABS = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableTractionControl = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableESC = true; // Electronic Stability Control

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ABSThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TractionControlThreshold = 0.9f;
};

/**
 * Vehicle surface physics properties
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVehicleSurfaceProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVehicleSurfaceType SurfaceType = EVehicleSurfaceType::Asphalt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RollingResistance = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Bumpiness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCausesSplash = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCausesDust = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ParticleColor = FLinearColor::White;
};

/**
 * Vehicle event data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVehicleEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    class AWheeledVehiclePawn* Vehicle = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float EngineRPM = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentGear = 0;

    UPROPERTY(BlueprintReadOnly)
    EVehicleSurfaceType SurfaceType = EVehicleSurfaceType::Asphalt;

    UPROPERTY(BlueprintReadOnly)
    float TimeStamp = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleEvent, const FVehicleEventData&, EventData);

/**
 * Vehicle Physics System V2 - Advanced vehicle physics with Chaos Vehicles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVehiclePhysicsSystemV2 : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVehiclePhysicsSystemV2();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core vehicle management
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void InitializeVehicleSystem();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void ShutdownVehicleSystem();

    // Vehicle registration
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void RegisterVehicle(AWheeledVehiclePawn* Vehicle, const FVehiclePhysicsConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void UnregisterVehicle(AWheeledVehiclePawn* Vehicle);

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    bool IsVehicleRegistered(AWheeledVehiclePawn* Vehicle) const;

    // Vehicle configuration
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetVehicleConfiguration(AWheeledVehiclePawn* Vehicle, const FVehiclePhysicsConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    FVehiclePhysicsConfig GetVehicleConfiguration(AWheeledVehiclePawn* Vehicle) const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void ApplyVehiclePreset(AWheeledVehiclePawn* Vehicle, EVehicleType VehicleType);

    // Surface system
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetSurfaceProperties(EVehicleSurfaceType SurfaceType, const FVehicleSurfaceProperties& Properties);

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    FVehicleSurfaceProperties GetSurfaceProperties(EVehicleSurfaceType SurfaceType) const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetVehicleSurface(AWheeledVehiclePawn* Vehicle, EVehicleSurfaceType SurfaceType);

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    EVehicleSurfaceType DetectSurfaceType(const FVector& Location) const;

    // Vehicle control
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetVehicleThrottle(AWheeledVehiclePawn* Vehicle, float ThrottleInput);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetVehicleSteering(AWheeledVehiclePawn* Vehicle, float SteeringInput);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetVehicleBrake(AWheeledVehiclePawn* Vehicle, float BrakeInput);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetVehicleHandbrake(AWheeledVehiclePawn* Vehicle, bool bHandbrakeEngaged);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void ShiftGear(AWheeledVehiclePawn* Vehicle, int32 GearNumber);

    // Vehicle state queries
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    float GetVehicleSpeed(AWheeledVehiclePawn* Vehicle) const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    float GetVehicleEngineRPM(AWheeledVehiclePawn* Vehicle) const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    int32 GetVehicleCurrentGear(AWheeledVehiclePawn* Vehicle) const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    bool IsVehicleOnGround(AWheeledVehiclePawn* Vehicle) const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics V2")
    float GetVehicleWheelSlip(AWheeledVehiclePawn* Vehicle, int32 WheelIndex) const;

    // Stability systems
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void EnableABS(AWheeledVehiclePawn* Vehicle, bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void EnableTractionControl(AWheeledVehiclePawn* Vehicle, bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void EnableESC(AWheeledVehiclePawn* Vehicle, bool bEnable = true);

    // Damage system
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void ApplyVehicleDamage(AWheeledVehiclePawn* Vehicle, float Damage, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetWheelDamage(AWheeledVehiclePawn* Vehicle, int32 WheelIndex, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void SetEngineDamage(AWheeledVehiclePawn* Vehicle, float DamageAmount);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics V2")
    FOnVehicleEvent OnVehicleSpeedChanged;

    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics V2")
    FOnVehicleEvent OnVehicleGearChanged;

    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics V2")
    FOnVehicleEvent OnVehicleSurfaceChanged;

    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics V2")
    FOnVehicleEvent OnVehicleDamaged;

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void EnableVehicleDebugVisualization(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    void ValidateVehicleConfiguration(AWheeledVehiclePawn* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics V2")
    bool RunVehiclePhysicsTest(AWheeledVehiclePawn* Vehicle);

protected:
    // Default configurations for different vehicle types
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Presets")
    TMap<EVehicleType, FVehiclePhysicsConfig> VehiclePresets;

    // Surface configurations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface Settings")
    TMap<EVehicleSurfaceType, FVehicleSurfaceProperties> SurfaceConfigurations;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    int32 MaxActiveVehicles = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float MaxVehicleDistance = 5000.0f;

    // Runtime data
    UPROPERTY()
    TMap<TWeakObjectPtr<AWheeledVehiclePawn>, FVehiclePhysicsConfig> RegisteredVehicles;

private:
    // Internal vehicle state tracking
    struct FVehicleState
    {
        float LastSpeed = 0.0f;
        int32 LastGear = 0;
        EVehicleSurfaceType LastSurfaceType = EVehicleSurfaceType::Asphalt;
        float LastUpdateTime = 0.0f;
        float EngineDamage = 0.0f;
        TArray<float> WheelDamage;
        bool bABSActive = false;
        bool bTractionControlActive = false;
        bool bESCActive = false;
    };

    TMap<TWeakObjectPtr<AWheeledVehiclePawn>, FVehicleState> VehicleStates;

    // Internal vehicle logic
    void ApplyVehicleConfiguration(AWheeledVehiclePawn* Vehicle, const FVehiclePhysicsConfig& Config);
    void UpdateVehiclePhysics(AWheeledVehiclePawn* Vehicle, float DeltaTime);
    void UpdateVehicleSurface(AWheeledVehiclePawn* Vehicle);
    void UpdateStabilitySystems(AWheeledVehiclePawn* Vehicle, FVehicleState& State);
    
    // Stability system implementations
    void ProcessABS(AWheeledVehiclePawn* Vehicle, FVehicleState& State);
    void ProcessTractionControl(AWheeledVehiclePawn* Vehicle, FVehicleState& State);
    void ProcessESC(AWheeledVehiclePawn* Vehicle, FVehicleState& State);
    
    // Surface detection
    EVehicleSurfaceType DetectSurfaceTypeAtLocation(const FVector& Location) const;
    void ApplySurfaceEffects(AWheeledVehiclePawn* Vehicle, EVehicleSurfaceType SurfaceType);
    
    // Vehicle preset initialization
    void InitializeVehiclePresets();
    FVehiclePhysicsConfig CreatePresetConfig(EVehicleType VehicleType);
    
    // Surface configuration initialization
    void InitializeSurfaceConfigurations();
    FVehicleSurfaceProperties CreateSurfaceConfig(EVehicleSurfaceType SurfaceType);
    
    // Performance optimization
    void OptimizeVehiclePerformance();
    void CullDistantVehicles();
    void CleanupInvalidReferences();
    
    // Event handling
    void BroadcastVehicleEvent(const FVehicleEventData& EventData, FOnVehicleEvent& EventDelegate);
    void HandleVehicleDestroyed(AWheeledVehiclePawn* Vehicle);
    
    // System updates
    void SystemUpdate();
    void UpdateRegisteredVehicles(float DeltaTime);
    
    // Timers
    FTimerHandle SystemUpdateTimer;
    FTimerHandle CleanupTimer;
    FTimerHandle PerformanceOptimizationTimer;
    
    // Debug visualization
    bool bDebugVisualizationEnabled = false;
    void DrawVehicleDebugInfo();
    
    // System state
    bool bSystemInitialized = false;
};

/**
 * Vehicle Physics Component V2 - Attach to vehicles for advanced physics functionality
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVehiclePhysicsComponentV2 : public UActorComponent
{
    GENERATED_BODY()

public:
    UVehiclePhysicsComponentV2();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Component V2")
    void SetConfiguration(const FVehiclePhysicsConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics Component V2")
    FVehiclePhysicsConfig GetConfiguration() const { return Configuration; }

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Component V2")
    void ApplyVehiclePreset(EVehicleType VehicleType);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Vehicle Physics Component V2")
    float GetSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics Component V2")
    float GetEngineRPM() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics Component V2")
    int32 GetCurrentGear() const;

    UFUNCTION(BlueprintPure, Category = "Vehicle Physics Component V2")
    EVehicleSurfaceType GetCurrentSurfaceType() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Vehicle Physics Component V2")
    FOnVehicleEvent OnComponentVehicleEvent;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
    FVehiclePhysicsConfig Configuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
    bool bAutoRegisterWithSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
    EVehicleType DefaultVehicleType = EVehicleType::LightVehicle;

private:
    UPROPERTY()
    UVehiclePhysicsSystemV2* VehicleSystem = nullptr;

    UPROPERTY()
    AWheeledVehiclePawn* OwnerVehicle = nullptr;

    void RegisterWithVehicleSystem();
    void UnregisterFromVehicleSystem();
    void InitializeComponent();

    // Event handling
    UFUNCTION()
    void OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};