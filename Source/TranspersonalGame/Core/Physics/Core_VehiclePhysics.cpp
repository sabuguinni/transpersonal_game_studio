#include "Core_VehiclePhysics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"

UCore_VehiclePhysics::UCore_VehiclePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default vehicle stats
    VehicleStats.MaxSpeed = 800.0f;
    VehicleStats.Acceleration = 1200.0f;
    VehicleStats.Deceleration = 2000.0f;
    VehicleStats.TurnRate = 45.0f;
    VehicleStats.Mass = 500.0f;
    VehicleStats.Durability = 100.0f;
    
    CurrentDurability = VehicleStats.Durability;
}

void UCore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    LastPosition = GetOwner()->GetActorLocation();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysics: Initialized for %s"), *GetOwner()->GetName());
}

void UCore_VehiclePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMovement(DeltaTime);
    ApplyPhysicsForces(DeltaTime);
    HandleTerrainInteraction();
}

void UCore_VehiclePhysics::InitializeVehicle(ECore_VehicleType NewVehicleType)
{
    VehicleType = NewVehicleType;
    UpdateVehicleStats();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysics: Vehicle initialized as type %d"), (int32)VehicleType);
}

void UCore_VehiclePhysics::SetThrottleInput(float ThrottleValue)
{
    ThrottleInput = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::SetSteeringInput(float SteeringValue)
{
    SteeringInput = FMath::Clamp(SteeringValue, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::ApplyBrakes(float BrakeForce)
{
    BrakeInput = FMath::Clamp(BrakeForce, 0.0f, 1.0f);
}

void UCore_VehiclePhysics::SetVehicleType(ECore_VehicleType NewType)
{
    VehicleType = NewType;
    UpdateVehicleStats();
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Vehicle type changed to %d"), (int32)NewType);
}

float UCore_VehiclePhysics::GetCurrentSpeed() const
{
    return CurrentSpeed;
}

bool UCore_VehiclePhysics::IsVehicleMoving() const
{
    return FMath::Abs(CurrentSpeed) > 10.0f;
}

void UCore_VehiclePhysics::RepairVehicle(float RepairAmount)
{
    CurrentDurability = FMath::Clamp(CurrentDurability + RepairAmount, 0.0f, VehicleStats.Durability);
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Vehicle repaired, durability now %f"), CurrentDurability);
}

void UCore_VehiclePhysics::DamageVehicle(float DamageAmount)
{
    CurrentDurability = FMath::Clamp(CurrentDurability - DamageAmount, 0.0f, VehicleStats.Durability);
    
    if (CurrentDurability <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysics: Vehicle destroyed!"));
        // Vehicle is destroyed, stop all movement
        ThrottleInput = 0.0f;
        CurrentSpeed = 0.0f;
    }
}

void UCore_VehiclePhysics::UpdateMovement(float DeltaTime)
{
    if (CurrentDurability <= 0.0f)
    {
        return; // Vehicle is destroyed
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Calculate speed based on throttle input
    float TargetSpeed = ThrottleInput * VehicleStats.MaxSpeed;
    
    // Apply acceleration or deceleration
    float AccelRate = (FMath::Abs(ThrottleInput) > 0.1f) ? VehicleStats.Acceleration : VehicleStats.Deceleration;
    
    // Apply braking
    if (BrakeInput > 0.1f)
    {
        AccelRate = VehicleStats.Deceleration * BrakeInput * 2.0f;
        TargetSpeed = 0.0f;
    }
    
    // Interpolate to target speed
    CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, AccelRate / VehicleStats.MaxSpeed);
    
    // Apply steering if moving
    if (FMath::Abs(CurrentSpeed) > 10.0f)
    {
        float TurnAmount = SteeringInput * VehicleStats.TurnRate * DeltaTime;
        FRotator CurrentRotation = Owner->GetActorRotation();
        FRotator NewRotation = CurrentRotation + FRotator(0.0f, TurnAmount, 0.0f);
        Owner->SetActorRotation(NewRotation);
    }
    
    // Move forward based on current speed and rotation
    FVector ForwardVector = Owner->GetActorForwardVector();
    FVector MovementDelta = ForwardVector * CurrentSpeed * DeltaTime;
    
    FVector NewLocation = Owner->GetActorLocation() + MovementDelta;
    Owner->SetActorLocation(NewLocation);
}

void UCore_VehiclePhysics::ApplyPhysicsForces(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Calculate actual velocity
    FVector CurrentPosition = Owner->GetActorLocation();
    Velocity = (CurrentPosition - LastPosition) / DeltaTime;
    LastPosition = CurrentPosition;
    
    // Apply terrain-based friction
    float TerrainFriction = 1.0f; // Default friction
    
    switch (VehicleType)
    {
        case ECore_VehicleType::Cart:
            TerrainFriction = 0.8f; // Wheels provide good traction
            break;
        case ECore_VehicleType::Raft:
            TerrainFriction = 0.3f; // Low friction on water
            break;
        case ECore_VehicleType::Sled:
            TerrainFriction = 0.5f; // Medium friction sliding
            break;
        case ECore_VehicleType::Mount:
            TerrainFriction = 0.9f; // Animals adapt to terrain
            break;
        default:
            break;
    }
    
    // Apply friction to reduce speed gradually
    if (FMath::Abs(ThrottleInput) < 0.1f && BrakeInput < 0.1f)
    {
        CurrentSpeed *= (1.0f - (TerrainFriction * DeltaTime));
    }
}

void UCore_VehiclePhysics::UpdateVehicleStats()
{
    switch (VehicleType)
    {
        case ECore_VehicleType::Cart:
            VehicleStats.MaxSpeed = 600.0f;
            VehicleStats.Acceleration = 800.0f;
            VehicleStats.TurnRate = 30.0f;
            VehicleStats.Mass = 800.0f;
            VehicleStats.Durability = 150.0f;
            break;
            
        case ECore_VehicleType::Raft:
            VehicleStats.MaxSpeed = 400.0f;
            VehicleStats.Acceleration = 400.0f;
            VehicleStats.TurnRate = 20.0f;
            VehicleStats.Mass = 300.0f;
            VehicleStats.Durability = 80.0f;
            break;
            
        case ECore_VehicleType::Sled:
            VehicleStats.MaxSpeed = 1000.0f;
            VehicleStats.Acceleration = 1500.0f;
            VehicleStats.TurnRate = 60.0f;
            VehicleStats.Mass = 200.0f;
            VehicleStats.Durability = 60.0f;
            break;
            
        case ECore_VehicleType::Mount:
            VehicleStats.MaxSpeed = 1200.0f;
            VehicleStats.Acceleration = 2000.0f;
            VehicleStats.TurnRate = 90.0f;
            VehicleStats.Mass = 600.0f;
            VehicleStats.Durability = 200.0f;
            break;
            
        default:
            // Keep default stats
            break;
    }
    
    CurrentDurability = VehicleStats.Durability;
}

void UCore_VehiclePhysics::HandleTerrainInteraction()
{
    // Basic terrain interaction - could be expanded with line traces
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Simple terrain effect based on Z position
    FVector Location = Owner->GetActorLocation();
    
    // Water detection (low Z values)
    if (Location.Z < 50.0f && VehicleType != ECore_VehicleType::Raft)
    {
        // Non-raft vehicles slow down in water
        CurrentSpeed *= 0.5f;
        
        // Damage non-waterproof vehicles
        if (VehicleType == ECore_VehicleType::Cart)
        {
            DamageVehicle(0.1f); // Slow water damage
        }
    }
    
    // Steep terrain detection (could use slope calculation)
    // For now, just reduce speed on very high terrain
    if (Location.Z > 1000.0f)
    {
        // High altitude reduces performance
        CurrentSpeed *= 0.8f;
    }
}