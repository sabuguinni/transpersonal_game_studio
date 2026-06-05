#include "Core_VehiclePhysics.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_VehiclePhysics::UCore_VehiclePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    VehicleType = ECore_VehicleType::None;
    DriveMode = ECore_VehicleDriveMode::Manual;
    
    // Set default physics data
    PhysicsData = FCore_VehiclePhysicsData();
    CurrentState = FCore_VehicleState();
    
    // Initialize internal state
    AccumulatedForces = FVector::ZeroVector;
    AccumulatedTorques = FVector::ZeroVector;
    InputForces = FVector::ZeroVector;
    SteeringInput = 0.0f;
    LastUpdateTime = 0.0f;
    bPhysicsInitialized = false;
}

void UCore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize vehicle physics
    InitializeVehiclePhysics();
    
    // Set up initial state
    CurrentState.Durability = 100.0f;
    CurrentState.CurrentLoad = 0.0f;
    CurrentState.bIsStable = true;
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Initialized for vehicle type %d"), (int32)VehicleType);
}

void UCore_VehiclePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsInitialized)
    {
        return;
    }
    
    // Update vehicle movement
    UpdateMovement(DeltaTime);
    
    // Update environmental physics
    CheckEnvironmentalConditions();
    UpdateWaterPhysics(DeltaTime);
    UpdateTerrainPhysics(DeltaTime);
    
    // Update stability
    UpdateStability(DeltaTime);
    
    // Validate physics state
    ValidatePhysicsState();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UCore_VehiclePhysics::InitializeVehiclePhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_VehiclePhysics: No owner found"));
        return;
    }
    
    // Find vehicle mesh component
    VehicleMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (VehicleMesh)
    {
        PhysicsBody = VehicleMesh;
        
        // Configure physics properties
        VehicleMesh->SetSimulatePhysics(true);
        VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        VehicleMesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Set mass
        VehicleMesh->SetMassOverrideInKg(NAME_None, PhysicsData.Mass, true);
        
        // Configure physics material properties
        VehicleMesh->SetLinearDamping(PhysicsData.AirResistance);
        VehicleMesh->SetAngularDamping(PhysicsData.Friction);
        
        UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Physics initialized for mesh component"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysics: No static mesh component found"));
    }
    
    bPhysicsInitialized = true;
}

void UCore_VehiclePhysics::ApplyForce(const FVector& Force, const FVector& Location)
{
    if (!PhysicsBody || !bPhysicsInitialized)
    {
        return;
    }
    
    // Apply force to physics body
    if (Location.IsZero())
    {
        PhysicsBody->AddForce(Force);
    }
    else
    {
        PhysicsBody->AddForceAtLocation(Force, Location);
    }
    
    // Accumulate for internal calculations
    AccumulatedForces += Force;
}

void UCore_VehiclePhysics::ApplyTorque(const FVector& Torque)
{
    if (!PhysicsBody || !bPhysicsInitialized)
    {
        return;
    }
    
    PhysicsBody->AddTorqueInRadians(Torque);
    AccumulatedTorques += Torque;
}

void UCore_VehiclePhysics::SetVehicleInput(const FVector& InputVector, float Steering)
{
    InputForces = InputVector * PhysicsData.Acceleration;
    SteeringInput = FMath::Clamp(Steering, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::StartMovement()
{
    if (!CanMove())
    {
        return;
    }
    
    CurrentState.bIsMoving = true;
    OnVehicleStartMoving();
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Vehicle started moving"));
}

void UCore_VehiclePhysics::StopMovement()
{
    CurrentState.bIsMoving = false;
    InputForces = FVector::ZeroVector;
    SteeringInput = 0.0f;
    
    OnVehicleStopMoving();
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Vehicle stopped moving"));
}

void UCore_VehiclePhysics::UpdateMovement(float DeltaTime)
{
    if (!bPhysicsInitialized || !PhysicsBody)
    {
        return;
    }
    
    // Calculate physics forces
    CalculatePhysicsForces(DeltaTime);
    
    // Update velocity and speed
    UpdateVelocity(DeltaTime);
    
    // Apply input forces if moving
    if (CurrentState.bIsMoving && !InputForces.IsZero())
    {
        ApplyForce(InputForces);
        
        // Apply steering torque
        if (FMath::Abs(SteeringInput) > 0.1f)
        {
            FVector SteeringTorque = FVector(0, 0, SteeringInput * PhysicsData.Acceleration * 0.5f);
            ApplyTorque(SteeringTorque);
        }
    }
    
    // Check collisions
    CheckCollisions();
}

bool UCore_VehiclePhysics::AddLoad(float LoadWeight)
{
    if (CurrentState.CurrentLoad + LoadWeight > PhysicsData.CarryCapacity)
    {
        OnVehicleOverloaded();
        return false;
    }
    
    CurrentState.CurrentLoad += LoadWeight;
    
    // Update mass
    if (PhysicsBody)
    {
        float TotalMass = PhysicsData.Mass + CurrentState.CurrentLoad;
        PhysicsBody->SetMassOverrideInKg(NAME_None, TotalMass, true);
    }
    
    return true;
}

bool UCore_VehiclePhysics::RemoveLoad(float LoadWeight)
{
    if (CurrentState.CurrentLoad < LoadWeight)
    {
        return false;
    }
    
    CurrentState.CurrentLoad -= LoadWeight;
    
    // Update mass
    if (PhysicsBody)
    {
        float TotalMass = PhysicsData.Mass + CurrentState.CurrentLoad;
        PhysicsBody->SetMassOverrideInKg(NAME_None, TotalMass, true);
    }
    
    return true;
}

float UCore_VehiclePhysics::GetLoadPercentage() const
{
    if (PhysicsData.CarryCapacity <= 0.0f)
    {
        return 0.0f;
    }
    
    return (CurrentState.CurrentLoad / PhysicsData.CarryCapacity) * 100.0f;
}

void UCore_VehiclePhysics::UpdateWaterPhysics(float DeltaTime)
{
    if (!PhysicsBody)
    {
        return;
    }
    
    // Check if vehicle is in water
    FVector VehicleLocation = PhysicsBody->GetComponentLocation();
    
    // Simple water detection (Z < 0 for now, should be improved with proper water volume detection)
    bool bWasInWater = CurrentState.bIsInWater;
    CurrentState.bIsInWater = VehicleLocation.Z < 50.0f; // Assume water level at Z=50
    
    if (CurrentState.bIsInWater && !bWasInWater)
    {
        OnVehicleEnterWater();
    }
    else if (!CurrentState.bIsInWater && bWasInWater)
    {
        OnVehicleExitWater();
    }
    
    if (CurrentState.bIsInWater)
    {
        // Apply buoyancy force if vehicle can float
        if (PhysicsData.bCanFloat)
        {
            FVector BuoyancyForce = FVector(0, 0, 980.0f * PhysicsData.Mass * 0.5f);
            ApplyForce(BuoyancyForce);
        }
        
        // Apply water resistance
        FVector Velocity = PhysicsBody->GetComponentVelocity();
        FVector WaterResistance = -Velocity * PhysicsData.WaterResistance;
        ApplyForce(WaterResistance);
    }
}

void UCore_VehiclePhysics::UpdateTerrainPhysics(float DeltaTime)
{
    if (!PhysicsBody)
    {
        return;
    }
    
    // Apply terrain-based friction and resistance
    ApplyFrictionForces(DeltaTime);
    ApplyResistanceForces(DeltaTime);
}

void UCore_VehiclePhysics::CheckEnvironmentalConditions()
{
    // This would be expanded to check for various environmental factors
    // like terrain type, weather, obstacles, etc.
}

void UCore_VehiclePhysics::UpdateStability(float DeltaTime)
{
    if (!PhysicsBody)
    {
        return;
    }
    
    // Calculate stability based on speed, load, and terrain
    float LoadFactor = GetLoadPercentage() / 100.0f;
    float SpeedFactor = CurrentState.CurrentSpeed / PhysicsData.MaxSpeed;
    
    float StabilityFactor = PhysicsData.Stability * (1.0f - LoadFactor * 0.3f) * (1.0f - SpeedFactor * 0.2f);
    
    CurrentState.bIsStable = StabilityFactor > 0.5f;
    
    // Apply instability effects if needed
    if (!CurrentState.bIsStable && CurrentState.bIsMoving)
    {
        // Add some random forces to simulate instability
        FVector InstabilityForce = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        ) * (1.0f - StabilityFactor);
        
        ApplyForce(InstabilityForce);
    }
}

void UCore_VehiclePhysics::ApplyDamage(float DamageAmount, const FVector& ImpactLocation)
{
    CurrentState.Durability = FMath::Clamp(CurrentState.Durability - DamageAmount, 0.0f, 100.0f);
    
    OnVehicleDamaged(DamageAmount);
    
    // Apply impact force
    if (!ImpactLocation.IsZero())
    {
        FVector ImpactForce = (PhysicsBody->GetComponentLocation() - ImpactLocation).GetSafeNormal() * DamageAmount * 10.0f;
        ApplyForce(ImpactForce, ImpactLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysics: Vehicle damaged, durability: %.1f"), CurrentState.Durability);
}

bool UCore_VehiclePhysics::IsVehicleStable() const
{
    return CurrentState.bIsStable && CurrentState.Durability > 10.0f;
}

bool UCore_VehiclePhysics::SetDriver(APawn* NewDriver)
{
    if (!NewDriver)
    {
        return false;
    }
    
    CurrentState.Driver = NewDriver;
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Driver set: %s"), *NewDriver->GetName());
    return true;
}

void UCore_VehiclePhysics::RemoveDriver()
{
    CurrentState.Driver = nullptr;
    StopMovement();
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Driver removed"));
}

APawn* UCore_VehiclePhysics::GetCurrentDriver() const
{
    return CurrentState.Driver;
}

FVector UCore_VehiclePhysics::CalculateResistanceForces() const
{
    if (!PhysicsBody)
    {
        return FVector::ZeroVector;
    }
    
    FVector Velocity = PhysicsBody->GetComponentVelocity();
    FVector AirResistance = -Velocity * PhysicsData.AirResistance;
    
    if (CurrentState.bIsInWater)
    {
        FVector WaterResistance = -Velocity * PhysicsData.WaterResistance;
        return AirResistance + WaterResistance;
    }
    
    return AirResistance;
}

float UCore_VehiclePhysics::CalculateEffectiveSpeed() const
{
    float LoadPenalty = GetLoadPercentage() * 0.01f * 0.3f; // 30% speed reduction at full load
    float DamagePenalty = (100.0f - CurrentState.Durability) * 0.01f * 0.5f; // 50% speed reduction when destroyed
    
    return PhysicsData.MaxSpeed * (1.0f - LoadPenalty - DamagePenalty);
}

bool UCore_VehiclePhysics::CanMove() const
{
    if (CurrentState.Durability <= 0.0f)
    {
        return false;
    }
    
    if (PhysicsData.bRequiresAnimal && !CurrentState.Driver)
    {
        return false;
    }
    
    return true;
}

void UCore_VehiclePhysics::ResetVehiclePhysics()
{
    CurrentState = FCore_VehicleState();
    AccumulatedForces = FVector::ZeroVector;
    AccumulatedTorques = FVector::ZeroVector;
    InputForces = FVector::ZeroVector;
    SteeringInput = 0.0f;
    
    if (PhysicsBody)
    {
        PhysicsBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PhysicsBody->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Physics reset"));
}

void UCore_VehiclePhysics::CalculatePhysicsForces(float DeltaTime)
{
    // Apply resistance forces
    FVector ResistanceForces = CalculateResistanceForces();
    ApplyForce(ResistanceForces);
    
    // Reset accumulated forces for next frame
    AccumulatedForces = FVector::ZeroVector;
    AccumulatedTorques = FVector::ZeroVector;
}

void UCore_VehiclePhysics::ApplyFrictionForces(float DeltaTime)
{
    if (!PhysicsBody)
    {
        return;
    }
    
    FVector Velocity = PhysicsBody->GetComponentVelocity();
    FVector FrictionForce = -Velocity * PhysicsData.Friction * PhysicsData.Mass;
    
    ApplyForce(FrictionForce);
}

void UCore_VehiclePhysics::ApplyResistanceForces(float DeltaTime)
{
    FVector ResistanceForces = CalculateResistanceForces();
    ApplyForce(ResistanceForces);
}

void UCore_VehiclePhysics::UpdateVelocity(float DeltaTime)
{
    if (!PhysicsBody)
    {
        return;
    }
    
    CurrentState.Velocity = PhysicsBody->GetComponentVelocity();
    CurrentState.CurrentSpeed = CurrentState.Velocity.Size();
    
    // Clamp to max speed
    float EffectiveMaxSpeed = CalculateEffectiveSpeed();
    if (CurrentState.CurrentSpeed > EffectiveMaxSpeed)
    {
        FVector ClampedVelocity = CurrentState.Velocity.GetSafeNormal() * EffectiveMaxSpeed;
        PhysicsBody->SetPhysicsLinearVelocity(ClampedVelocity);
        CurrentState.Velocity = ClampedVelocity;
        CurrentState.CurrentSpeed = EffectiveMaxSpeed;
    }
}

void UCore_VehiclePhysics::CheckCollisions()
{
    // This would be expanded to handle collision detection and response
    // For now, just basic collision handling through UE5's physics system
}

void UCore_VehiclePhysics::ValidatePhysicsState()
{
    // Ensure physics state remains valid
    if (CurrentState.Durability < 0.0f)
    {
        CurrentState.Durability = 0.0f;
    }
    
    if (CurrentState.CurrentLoad < 0.0f)
    {
        CurrentState.CurrentLoad = 0.0f;
    }
    
    if (CurrentState.CurrentSpeed < 0.0f)
    {
        CurrentState.CurrentSpeed = 0.0f;
    }
}