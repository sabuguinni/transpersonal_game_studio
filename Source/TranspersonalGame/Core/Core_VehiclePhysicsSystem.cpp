#include "Core_VehiclePhysicsSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_VehiclePhysicsSystem::UCore_VehiclePhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    PhysicsConfig = FCore_VehiclePhysicsConfig();
    CurrentState = FCore_VehicleState();
    
    VehicleBody = nullptr;
    GravityScale = 1.0f;
    AirResistance = 0.02f;
    WaterResistance = 0.8f;
    GroundFriction = 0.7f;
    
    bPerformanceMonitoring = false;
    AverageFrameTime = 0.0f;
    PhysicsCalculationsPerSecond = 0;
    
    AccumulatedForces = FVector::ZeroVector;
    LastFrameTime = 0.0f;
    FrameCounter = 0;
    PerformanceTimer = 0.0f;
}

void UCore_VehiclePhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the vehicle body component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Try to find a primitive component for physics
        VehicleBody = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (!VehicleBody)
        {
            VehicleBody = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
        if (!VehicleBody)
        {
            VehicleBody = Owner->GetRootComponent();
        }
        
        if (VehicleBody)
        {
            // Enable physics simulation
            VehicleBody->SetSimulatePhysics(true);
            VehicleBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            VehicleBody->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
            
            UE_LOG(LogTemp, Log, TEXT("Vehicle Physics System initialized for %s"), *Owner->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No suitable primitive component found for vehicle physics on %s"), *Owner->GetName());
        }
    }
}

void UCore_VehiclePhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bPerformanceMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
    }
    
    // Apply vehicle physics
    ApplyVehiclePhysics(DeltaTime);
}

void UCore_VehiclePhysicsSystem::InitializeVehicle(ECore_VehicleType InVehicleType, float InMass)
{
    PhysicsConfig.VehicleType = InVehicleType;
    PhysicsConfig.Mass = InMass;
    
    // Configure physics based on vehicle type
    switch (InVehicleType)
    {
        case ECore_VehicleType::Raft:
            PhysicsConfig.MaxSpeed = 300.0f;
            PhysicsConfig.Acceleration = 400.0f;
            PhysicsConfig.TurningRadius = 500.0f;
            PhysicsConfig.StabilityFactor = 0.9f;
            PhysicsConfig.TerrainAdaptation = 0.3f;
            WaterResistance = 0.5f;
            break;
            
        case ECore_VehicleType::Sled:
            PhysicsConfig.MaxSpeed = 600.0f;
            PhysicsConfig.Acceleration = 600.0f;
            PhysicsConfig.TurningRadius = 200.0f;
            PhysicsConfig.StabilityFactor = 0.7f;
            PhysicsConfig.TerrainAdaptation = 0.8f;
            GroundFriction = 0.3f;
            break;
            
        case ECore_VehicleType::Cart:
            PhysicsConfig.MaxSpeed = 800.0f;
            PhysicsConfig.Acceleration = 700.0f;
            PhysicsConfig.TurningRadius = 300.0f;
            PhysicsConfig.StabilityFactor = 0.8f;
            PhysicsConfig.TerrainAdaptation = 0.6f;
            GroundFriction = 0.6f;
            break;
            
        case ECore_VehicleType::Mount:
            PhysicsConfig.MaxSpeed = 1200.0f;
            PhysicsConfig.Acceleration = 1000.0f;
            PhysicsConfig.TurningRadius = 150.0f;
            PhysicsConfig.StabilityFactor = 0.9f;
            PhysicsConfig.TerrainAdaptation = 0.9f;
            GroundFriction = 0.8f;
            break;
            
        default:
            break;
    }
    
    // Apply mass to physics body
    if (VehicleBody)
    {
        VehicleBody->SetMassOverrideInKg(NAME_None, PhysicsConfig.Mass, true);
        UE_LOG(LogTemp, Log, TEXT("Vehicle initialized: Type=%d, Mass=%.1f"), (int32)InVehicleType, InMass);
    }
}

void UCore_VehiclePhysicsSystem::SetVehicleInput(float Throttle, float Steering)
{
    CurrentState.ThrottleInput = FMath::Clamp(Throttle, -1.0f, 1.0f);
    CurrentState.SteeringInput = FMath::Clamp(Steering, -1.0f, 1.0f);
}

void UCore_VehiclePhysicsSystem::ApplyVehiclePhysics(float DeltaTime)
{
    if (!VehicleBody || PhysicsConfig.VehicleType == ECore_VehicleType::None)
    {
        return;
    }
    
    // Update terrain interaction
    UpdateTerrainInteraction();
    
    // Calculate movement forces
    CalculateMovementForces(DeltaTime);
    
    // Apply environmental forces
    ApplyEnvironmentalForces(DeltaTime);
    
    // Handle specific physics based on environment
    if (CurrentState.bIsInWater)
    {
        HandleWaterPhysics(DeltaTime);
    }
    else if (CurrentState.bIsGrounded)
    {
        HandleGroundPhysics(DeltaTime);
    }
    
    // Update vehicle orientation
    UpdateVehicleOrientation(DeltaTime);
    
    // Check for collisions
    CheckCollisions();
    
    // Update current state
    if (VehicleBody)
    {
        CurrentState.Velocity = VehicleBody->GetPhysicsLinearVelocity();
        CurrentState.CurrentSpeed = CurrentState.Velocity.Size();
    }
}

void UCore_VehiclePhysicsSystem::UpdateTerrainInteraction()
{
    if (!VehicleBody)
    {
        return;
    }
    
    FVector VehicleLocation = VehicleBody->GetComponentLocation();
    
    // Trace down to check terrain
    FHitResult HitResult;
    FVector TraceStart = VehicleLocation;
    FVector TraceEnd = VehicleLocation - FVector(0, 0, 200.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    UWorld* World = GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        CurrentState.bIsGrounded = true;
        CurrentState.TerrainGrip = CalculateTerrainGrip(HitResult.Location);
        
        // Check if we're in water
        if (HitResult.GetComponent() && HitResult.GetComponent()->ComponentHasTag("Water"))
        {
            CurrentState.bIsInWater = true;
            CurrentState.bIsGrounded = false;
        }
        else
        {
            CurrentState.bIsInWater = false;
        }
    }
    else
    {
        CurrentState.bIsGrounded = false;
        CurrentState.bIsInWater = false;
        CurrentState.TerrainGrip = 0.0f;
    }
}

void UCore_VehiclePhysicsSystem::HandleWaterPhysics(float DeltaTime)
{
    if (!VehicleBody || PhysicsConfig.VehicleType != ECore_VehicleType::Raft)
    {
        return;
    }
    
    // Apply buoyancy
    FVector BuoyancyForce = FVector(0, 0, PhysicsConfig.Mass * 980.0f * 1.2f); // Slight positive buoyancy
    VehicleBody->AddForce(BuoyancyForce);
    
    // Apply water resistance
    FVector Velocity = VehicleBody->GetPhysicsLinearVelocity();
    FVector WaterDrag = -Velocity * WaterResistance * PhysicsConfig.Mass;
    VehicleBody->AddForce(WaterDrag);
    
    // Apply movement force if throttle input
    if (FMath::Abs(CurrentState.ThrottleInput) > 0.1f)
    {
        FVector ForwardVector = VehicleBody->GetForwardVector();
        FVector MovementForce = ForwardVector * CurrentState.ThrottleInput * PhysicsConfig.Acceleration * PhysicsConfig.Mass;
        VehicleBody->AddForce(MovementForce);
    }
}

void UCore_VehiclePhysicsSystem::HandleGroundPhysics(float DeltaTime)
{
    if (!VehicleBody)
    {
        return;
    }
    
    // Apply movement force
    if (FMath::Abs(CurrentState.ThrottleInput) > 0.1f)
    {
        FVector ForwardVector = VehicleBody->GetForwardVector();
        float EffectiveAcceleration = PhysicsConfig.Acceleration * CurrentState.TerrainGrip * PhysicsConfig.TerrainAdaptation;
        FVector MovementForce = ForwardVector * CurrentState.ThrottleInput * EffectiveAcceleration * PhysicsConfig.Mass;
        VehicleBody->AddForce(MovementForce);
    }
    
    // Apply ground friction
    FVector Velocity = VehicleBody->GetPhysicsLinearVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0);
    FVector FrictionForce = -HorizontalVelocity * GroundFriction * CurrentState.TerrainGrip * PhysicsConfig.Mass;
    VehicleBody->AddForce(FrictionForce);
    
    // Apply steering torque
    if (FMath::Abs(CurrentState.SteeringInput) > 0.1f && CurrentState.CurrentSpeed > 50.0f)
    {
        float SteeringTorque = CurrentState.SteeringInput * PhysicsConfig.Mass * 1000.0f;
        SteeringTorque *= FMath::Clamp(CurrentState.CurrentSpeed / PhysicsConfig.MaxSpeed, 0.1f, 1.0f);
        VehicleBody->AddTorqueInRadians(FVector(0, 0, SteeringTorque));
    }
}

float UCore_VehiclePhysicsSystem::CalculateTerrainGrip(const FVector& Location)
{
    // Base grip calculation - can be enhanced with terrain material system
    float BaseGrip = 1.0f;
    
    // Adjust based on vehicle type
    switch (PhysicsConfig.VehicleType)
    {
        case ECore_VehicleType::Raft:
            BaseGrip = 0.0f; // No terrain grip for rafts
            break;
        case ECore_VehicleType::Sled:
            BaseGrip = 0.8f; // Good on snow/ice, poor on rough terrain
            break;
        case ECore_VehicleType::Cart:
            BaseGrip = 0.7f; // Moderate grip with wheels
            break;
        case ECore_VehicleType::Mount:
            BaseGrip = 0.9f; // Excellent grip with animal feet
            break;
        default:
            break;
    }
    
    return BaseGrip * PhysicsConfig.TerrainAdaptation;
}

void UCore_VehiclePhysicsSystem::HandleTerrainDamage(float DamageAmount)
{
    CurrentState.DamageLevel += DamageAmount;
    CurrentState.DamageLevel = FMath::Clamp(CurrentState.DamageLevel, 0.0f, 100.0f);
    
    // Reduce performance based on damage
    if (CurrentState.DamageLevel > 50.0f)
    {
        PhysicsConfig.MaxSpeed *= 0.8f;
        PhysicsConfig.Acceleration *= 0.7f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Vehicle damage: %.1f%%"), CurrentState.DamageLevel);
}

bool UCore_VehiclePhysicsSystem::IsVehicleOperational() const
{
    return CurrentState.DamageLevel < 90.0f && PhysicsConfig.VehicleType != ECore_VehicleType::None;
}

void UCore_VehiclePhysicsSystem::RepairVehicle(float RepairAmount)
{
    CurrentState.DamageLevel -= RepairAmount;
    CurrentState.DamageLevel = FMath::Clamp(CurrentState.DamageLevel, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle repaired: %.1f%% damage remaining"), CurrentState.DamageLevel);
}

void UCore_VehiclePhysicsSystem::EnablePerformanceMonitoring(bool bEnable)
{
    bPerformanceMonitoring = bEnable;
    if (bEnable)
    {
        FrameCounter = 0;
        PerformanceTimer = 0.0f;
        AverageFrameTime = 0.0f;
    }
}

FString UCore_VehiclePhysicsSystem::GetPerformanceReport() const
{
    return FString::Printf(TEXT("Vehicle Physics Performance: Avg Frame: %.2fms, Calc/sec: %d, Speed: %.1f/%.1f"), 
        AverageFrameTime * 1000.0f, PhysicsCalculationsPerSecond, CurrentState.CurrentSpeed, PhysicsConfig.MaxSpeed);
}

void UCore_VehiclePhysicsSystem::CalculateMovementForces(float DeltaTime)
{
    AccumulatedForces = FVector::ZeroVector;
    
    // Gravity
    AccumulatedForces += FVector(0, 0, -980.0f * PhysicsConfig.Mass * GravityScale);
    
    // Air resistance
    if (VehicleBody)
    {
        FVector Velocity = VehicleBody->GetPhysicsLinearVelocity();
        FVector AirDrag = -Velocity * AirResistance * PhysicsConfig.Mass;
        AccumulatedForces += AirDrag;
    }
}

void UCore_VehiclePhysicsSystem::ApplyEnvironmentalForces(float DeltaTime)
{
    if (VehicleBody && AccumulatedForces.SizeSquared() > 0.1f)
    {
        VehicleBody->AddForce(AccumulatedForces);
    }
}

void UCore_VehiclePhysicsSystem::UpdateVehicleOrientation(float DeltaTime)
{
    if (!VehicleBody || !CurrentState.bIsGrounded)
    {
        return;
    }
    
    // Stabilize vehicle orientation based on terrain
    FRotator CurrentRotation = VehicleBody->GetComponentRotation();
    FRotator TargetRotation = CurrentRotation;
    
    // Apply stability factor to reduce unwanted rotation
    float StabilityStrength = PhysicsConfig.StabilityFactor * 1000.0f;
    FVector AngularVelocity = VehicleBody->GetPhysicsAngularVelocityInRadians();
    FVector StabilizingTorque = -AngularVelocity * StabilityStrength * PhysicsConfig.Mass;
    
    VehicleBody->AddTorqueInRadians(StabilizingTorque);
}

void UCore_VehiclePhysicsSystem::CheckCollisions()
{
    // Collision handling can be expanded here
    // For now, basic damage on high-speed impacts
    if (CurrentState.CurrentSpeed > PhysicsConfig.MaxSpeed * 0.8f)
    {
        // Check for potential collision damage
        float DamageRisk = (CurrentState.CurrentSpeed / PhysicsConfig.MaxSpeed) * 0.1f;
        if (FMath::RandRange(0.0f, 1.0f) < DamageRisk)
        {
            HandleTerrainDamage(FMath::RandRange(1.0f, 5.0f));
        }
    }
}

void UCore_VehiclePhysicsSystem::UpdatePerformanceMetrics(float DeltaTime)
{
    FrameCounter++;
    PerformanceTimer += DeltaTime;
    LastFrameTime = DeltaTime;
    
    if (PerformanceTimer >= 1.0f)
    {
        AverageFrameTime = PerformanceTimer / FrameCounter;
        PhysicsCalculationsPerSecond = FrameCounter;
        
        FrameCounter = 0;
        PerformanceTimer = 0.0f;
    }
}