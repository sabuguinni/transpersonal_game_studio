#include "Core_VehiclePhysicsSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Vehicles/WheeledVehicle.h"
#include "Vehicles/WheeledVehicleMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

UCore_VehiclePhysicsSystem::UCore_VehiclePhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default physics configuration
    PhysicsConfig = FCore_VehiclePhysicsConfig();
    CurrentState = FCore_VehiclePhysicsState();

    // Performance tracking
    LastUpdateTime = 0.0f;
    PhysicsUpdateCount = 0;
    AverageUpdateTime = 0.0f;

    // Initialize pointers
    OwnerVehicle = nullptr;
    MovementComponent = nullptr;
    VehicleRootComponent = nullptr;
}

void UCore_VehiclePhysicsSystem::BeginPlay()
{
    Super::BeginPlay();

    // Get owner vehicle reference
    OwnerVehicle = Cast<AWheeledVehicle>(GetOwner());
    if (!OwnerVehicle)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysicsSystem: Owner is not a WheeledVehicle"));
        return;
    }

    // Get movement component
    MovementComponent = OwnerVehicle->GetVehicleMovementComponent();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysicsSystem: No WheeledVehicleMovementComponent found"));
        return;
    }

    // Get root component
    VehicleRootComponent = OwnerVehicle->GetRootComponent();

    // Initialize vehicle physics
    InitializeVehiclePhysics();

    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsSystem: Initialized successfully"));
}

void UCore_VehiclePhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerVehicle || !MovementComponent)
    {
        return;
    }

    // Update vehicle physics
    UpdateVehiclePhysics(DeltaTime);

    // Update performance tracking
    PhysicsUpdateCount++;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (LastUpdateTime > 0.0f)
    {
        float UpdateTime = CurrentTime - LastUpdateTime;
        AverageUpdateTime = (AverageUpdateTime * (PhysicsUpdateCount - 1) + UpdateTime) / PhysicsUpdateCount;
    }
    LastUpdateTime = CurrentTime;
}

void UCore_VehiclePhysicsSystem::InitializeVehiclePhysics()
{
    if (!MovementComponent)
    {
        return;
    }

    // Apply physics configuration to movement component
    MovementComponent->Mass = PhysicsConfig.Mass;
    MovementComponent->MaxEngineForce = PhysicsConfig.MaxEngineForce;
    MovementComponent->MaxBrakingForce = PhysicsConfig.MaxBrakingForce;

    // Initialize suspension settings
    AdjustSuspensionStiffness(PhysicsConfig.SuspensionStiffness);
    AdjustSuspensionDamping(PhysicsConfig.SuspensionDamping);

    // Initialize tire settings
    SetTireFriction(PhysicsConfig.TireFriction);

    // Reset vehicle state
    ResetVehiclePhysics();

    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsSystem: Vehicle physics initialized with Mass=%.1f, MaxEngineForce=%.1f"), 
           PhysicsConfig.Mass, PhysicsConfig.MaxEngineForce);
}

void UCore_VehiclePhysicsSystem::UpdateVehiclePhysics(float DeltaTime)
{
    if (!MovementComponent || !VehicleRootComponent)
    {
        return;
    }

    // Update vehicle state
    UpdateVehicleState(DeltaTime);

    // Calculate and apply physics forces
    CalculateEngineForces(DeltaTime);
    CalculateSuspensionForces(DeltaTime);
    CalculateTireForces(DeltaTime);

    // Handle terrain interaction
    HandleTerrainInteraction();

    // Update suspension system
    UpdateSuspensionSystem(DeltaTime);

    // Update tire physics
    UpdateTirePhysics(DeltaTime);
}

void UCore_VehiclePhysicsSystem::ApplyEngineForce(float Force)
{
    if (!MovementComponent)
    {
        return;
    }

    // Clamp force to maximum engine force
    float ClampedForce = FMath::Clamp(Force, -PhysicsConfig.MaxEngineForce, PhysicsConfig.MaxEngineForce);
    
    // Apply throttle input
    float ThrottleInput = ClampedForce / PhysicsConfig.MaxEngineForce;
    MovementComponent->SetThrottleInput(ThrottleInput);

    // Update current state
    CurrentState.ThrottleInput = ThrottleInput;
    CurrentState.EngineRPM = FMath::Abs(ThrottleInput) * 6000.0f; // Simulate RPM

    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Applied engine force %.1f (throttle %.2f)"), 
           ClampedForce, ThrottleInput);
}

void UCore_VehiclePhysicsSystem::ApplyBrakingForce(float Force)
{
    if (!MovementComponent)
    {
        return;
    }

    // Clamp force to maximum braking force
    float ClampedForce = FMath::Clamp(Force, 0.0f, PhysicsConfig.MaxBrakingForce);
    
    // Apply brake input
    float BrakeInput = ClampedForce / PhysicsConfig.MaxBrakingForce;
    MovementComponent->SetBrakeInput(BrakeInput);

    // Update current state
    CurrentState.BrakeInput = BrakeInput;

    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Applied braking force %.1f (brake %.2f)"), 
           ClampedForce, BrakeInput);
}

void UCore_VehiclePhysicsSystem::ApplySteeringInput(float SteeringInput)
{
    if (!MovementComponent)
    {
        return;
    }

    // Clamp steering input
    float ClampedSteering = FMath::Clamp(SteeringInput, -1.0f, 1.0f);
    MovementComponent->SetSteeringInput(ClampedSteering);

    // Update current state
    CurrentState.SteeringAngle = ClampedSteering * PhysicsConfig.MaxSteeringAngle;

    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Applied steering input %.2f (angle %.1f)"), 
           ClampedSteering, CurrentState.SteeringAngle);
}

void UCore_VehiclePhysicsSystem::SetVehiclePhysicsConfig(const FCore_VehiclePhysicsConfig& NewConfig)
{
    PhysicsConfig = NewConfig;
    
    // Re-initialize physics with new configuration
    InitializeVehiclePhysics();

    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsSystem: Physics configuration updated"));
}

FCore_VehiclePhysicsState UCore_VehiclePhysicsSystem::GetVehiclePhysicsState() const
{
    return CurrentState;
}

void UCore_VehiclePhysicsSystem::UpdateSuspensionSystem(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Update suspension based on terrain and vehicle state
    float SpeedFactor = FMath::Clamp(CurrentState.CurrentSpeed / 1000.0f, 0.1f, 1.0f);
    float DynamicStiffness = PhysicsConfig.SuspensionStiffness * SpeedFactor;
    float DynamicDamping = PhysicsConfig.SuspensionDamping * (2.0f - SpeedFactor);

    // Apply dynamic suspension adjustments
    AdjustSuspensionStiffness(DynamicStiffness);
    AdjustSuspensionDamping(DynamicDamping);
}

void UCore_VehiclePhysicsSystem::AdjustSuspensionStiffness(float NewStiffness)
{
    PhysicsConfig.SuspensionStiffness = FMath::Clamp(NewStiffness, 0.1f, 20.0f);
    
    // Apply to movement component if available
    if (MovementComponent)
    {
        // Note: Specific suspension adjustment would depend on UE5 vehicle system API
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Suspension stiffness adjusted to %.2f"), 
               PhysicsConfig.SuspensionStiffness);
    }
}

void UCore_VehiclePhysicsSystem::AdjustSuspensionDamping(float NewDamping)
{
    PhysicsConfig.SuspensionDamping = FMath::Clamp(NewDamping, 0.1f, 5.0f);
    
    // Apply to movement component if available
    if (MovementComponent)
    {
        // Note: Specific suspension adjustment would depend on UE5 vehicle system API
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Suspension damping adjusted to %.2f"), 
               PhysicsConfig.SuspensionDamping);
    }
}

void UCore_VehiclePhysicsSystem::UpdateTirePhysics(float DeltaTime)
{
    if (!MovementComponent || !VehicleRootComponent)
    {
        return;
    }

    // Check for skidding based on lateral forces
    FVector Velocity = VehicleRootComponent->GetComponentVelocity();
    FVector ForwardVector = OwnerVehicle->GetActorForwardVector();
    
    float LateralVelocity = FVector::DotProduct(Velocity, OwnerVehicle->GetActorRightVector());
    float ForwardVelocity = FVector::DotProduct(Velocity, ForwardVector);
    
    // Determine if vehicle is skidding
    float SkidThreshold = 200.0f; // cm/s
    CurrentState.bIsSkidding = FMath::Abs(LateralVelocity) > SkidThreshold;

    // Adjust tire friction based on terrain
    float TerrainFrictionMultiplier = GetTerrainFrictionMultiplier();
    float EffectiveTireFriction = PhysicsConfig.TireFriction * TerrainFrictionMultiplier;
    
    SetTireFriction(EffectiveTireFriction);
}

void UCore_VehiclePhysicsSystem::SetTireFriction(float NewFriction)
{
    PhysicsConfig.TireFriction = FMath::Clamp(NewFriction, 0.1f, 5.0f);
    
    // Apply to movement component if available
    if (MovementComponent)
    {
        // Note: Specific tire friction adjustment would depend on UE5 vehicle system API
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Tire friction set to %.2f"), 
               PhysicsConfig.TireFriction);
    }
}

bool UCore_VehiclePhysicsSystem::IsVehicleSkidding() const
{
    return CurrentState.bIsSkidding;
}

void UCore_VehiclePhysicsSystem::HandleTerrainInteraction()
{
    if (!OwnerVehicle || !VehicleRootComponent)
    {
        return;
    }

    // Perform line trace to detect ground
    FVector StartLocation = OwnerVehicle->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f); // 2m down

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerVehicle);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );

    CurrentState.bIsOnGround = bHit && (HitResult.Distance < 150.0f); // Within 1.5m of ground

    if (bHit)
    {
        // Adjust physics based on surface type
        UPhysicalMaterial* SurfaceMaterial = HitResult.PhysMaterial.Get();
        if (SurfaceMaterial)
        {
            // Adjust friction based on surface material
            float SurfaceFriction = SurfaceMaterial->Friction;
            float AdjustedFriction = PhysicsConfig.TireFriction * SurfaceFriction;
            SetTireFriction(AdjustedFriction);
        }
    }
}

float UCore_VehiclePhysicsSystem::GetTerrainFrictionMultiplier() const
{
    // Base friction multiplier
    float FrictionMultiplier = 1.0f;

    // Adjust based on vehicle state
    if (CurrentState.bIsSkidding)
    {
        FrictionMultiplier *= 0.7f; // Reduced friction when skidding
    }

    if (!CurrentState.bIsOnGround)
    {
        FrictionMultiplier *= 0.1f; // Minimal friction when airborne
    }

    // Adjust based on speed
    float SpeedFactor = FMath::Clamp(CurrentState.CurrentSpeed / 2000.0f, 0.5f, 1.2f);
    FrictionMultiplier *= SpeedFactor;

    return FMath::Clamp(FrictionMultiplier, 0.1f, 2.0f);
}

void UCore_VehiclePhysicsSystem::LogVehiclePhysicsPerformance() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Core_VehiclePhysicsSystem Performance ==="));
    UE_LOG(LogTemp, Log, TEXT("Physics Updates: %d"), PhysicsUpdateCount);
    UE_LOG(LogTemp, Log, TEXT("Average Update Time: %.4f ms"), AverageUpdateTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Current Speed: %.1f cm/s"), CurrentState.CurrentSpeed);
    UE_LOG(LogTemp, Log, TEXT("Engine RPM: %.1f"), CurrentState.EngineRPM);
    UE_LOG(LogTemp, Log, TEXT("Is On Ground: %s"), CurrentState.bIsOnGround ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Is Skidding: %s"), CurrentState.bIsSkidding ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Tire Friction: %.2f"), PhysicsConfig.TireFriction);
    UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void UCore_VehiclePhysicsSystem::ResetVehiclePhysics()
{
    // Reset current state
    CurrentState = FCore_VehiclePhysicsState();

    // Reset movement component inputs
    if (MovementComponent)
    {
        MovementComponent->SetThrottleInput(0.0f);
        MovementComponent->SetBrakeInput(0.0f);
        MovementComponent->SetSteeringInput(0.0f);
    }

    // Reset performance tracking
    PhysicsUpdateCount = 0;
    AverageUpdateTime = 0.0f;
    LastUpdateTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsSystem: Physics reset"));
}

// Private implementation methods

void UCore_VehiclePhysicsSystem::CalculateEngineForces(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Calculate engine force based on throttle input and RPM
    float EngineEfficiency = FMath::Clamp(CurrentState.EngineRPM / 4000.0f, 0.3f, 1.0f);
    float EffectiveEngineForce = PhysicsConfig.MaxEngineForce * CurrentState.ThrottleInput * EngineEfficiency;

    // Apply roll resistance
    float RollResistanceForce = CurrentState.CurrentSpeed * PhysicsConfig.RollResistance;
    EffectiveEngineForce -= RollResistanceForce;

    // Update engine force (this would be applied through the movement component)
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Engine force calculated: %.1f N"), 
           EffectiveEngineForce);
}

void UCore_VehiclePhysicsSystem::CalculateSuspensionForces(float DeltaTime)
{
    if (!VehicleRootComponent)
    {
        return;
    }

    // Calculate suspension compression based on vehicle position and ground distance
    // This is a simplified calculation - real implementation would check each wheel individually
    
    float SuspensionCompression = CurrentState.bIsOnGround ? 0.5f : 0.0f; // 50% compression when on ground
    float SuspensionForce = SuspensionCompression * PhysicsConfig.SuspensionStiffness * PhysicsConfig.Mass;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Suspension force: %.1f N"), SuspensionForce);
}

void UCore_VehiclePhysicsSystem::CalculateTireForces(float DeltaTime)
{
    if (!VehicleRootComponent)
    {
        return;
    }

    // Calculate tire forces based on current velocity and steering
    FVector CurrentVelocity = VehicleRootComponent->GetComponentVelocity();
    float LateralForce = CurrentState.SteeringAngle * PhysicsConfig.TireFriction * PhysicsConfig.Mass;
    float LongitudinalForce = CurrentState.ThrottleInput * PhysicsConfig.TireFriction * PhysicsConfig.Mass;

    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: Tire forces - Lateral: %.1f, Longitudinal: %.1f"), 
           LateralForce, LongitudinalForce);
}

void UCore_VehiclePhysicsSystem::UpdateVehicleState(float DeltaTime)
{
    if (!VehicleRootComponent)
    {
        return;
    }

    // Update velocity and speed
    CurrentState.Velocity = VehicleRootComponent->GetComponentVelocity();
    CurrentState.CurrentSpeed = CurrentState.Velocity.Size();

    // Update angular velocity
    CurrentState.AngularVelocity = VehicleRootComponent->GetPhysicsAngularVelocityInDegrees();

    // Update ground contact
    HandleTerrainInteraction();

    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_VehiclePhysicsSystem: State updated - Speed: %.1f cm/s"), 
           CurrentState.CurrentSpeed);
}