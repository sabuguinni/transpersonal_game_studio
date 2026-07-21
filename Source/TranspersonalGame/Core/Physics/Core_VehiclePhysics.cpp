#include "Core_VehiclePhysics.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCore_VehiclePhysics::UCore_VehiclePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default vehicle settings for wooden cart
    VehicleSettings.MaxSpeed = 800.0f;
    VehicleSettings.Acceleration = 1200.0f;
    VehicleSettings.Deceleration = 2000.0f;
    VehicleSettings.TurningRadius = 500.0f;
    VehicleSettings.Mass = 100.0f;
    VehicleSettings.VehicleType = ECore_VehicleType::Cart;
    VehicleSettings.bRequiresWater = false;
    VehicleSettings.bCanClimbSlopes = true;
    VehicleSettings.MaxSlopeAngle = 30.0f;
}

void UCore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        LastFrameLocation = OwnerPawn->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Initialized for %s"), *OwnerPawn->GetName());
    }
    
    CheckTerrainValidity();
}

void UCore_VehiclePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn)
    {
        return;
    }
    
    // Update current speed and velocity
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    CurrentVelocity = (CurrentLocation - LastFrameLocation) / DeltaTime;
    CurrentSpeed = CurrentVelocity.Size();
    bIsMoving = CurrentSpeed > 10.0f;
    LastFrameLocation = CurrentLocation;
    
    // Periodic terrain validation
    TerrainCheckTimer += DeltaTime;
    if (TerrainCheckTimer >= TerrainCheckInterval)
    {
        CheckTerrainValidity();
        TerrainCheckTimer = 0.0f;
    }
    
    // Update movement if there's input
    if (FMath::Abs(ThrottleInput) > 0.01f || FMath::Abs(SteeringInput) > 0.01f || bBrakeInput)
    {
        UpdateMovement(DeltaTime);
    }
}

void UCore_VehiclePhysics::SetThrottleInput(float Throttle)
{
    ThrottleInput = FMath::Clamp(Throttle, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::SetSteeringInput(float Steering)
{
    SteeringInput = FMath::Clamp(Steering, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::SetBrakeInput(bool bBrake)
{
    bBrakeInput = bBrake;
}

void UCore_VehiclePhysics::ApplyImpulse(const FVector& Impulse)
{
    if (OwnerPawn)
    {
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(OwnerPawn->GetRootComponent()))
        {
            if (RootPrimitive->IsSimulatingPhysics())
            {
                RootPrimitive->AddImpulse(Impulse);
                UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Applied impulse %s"), *Impulse.ToString());
            }
        }
    }
}

float UCore_VehiclePhysics::GetSpeedKMH() const
{
    // Convert cm/s to km/h
    return CurrentSpeed * 0.036f;
}

bool UCore_VehiclePhysics::CanMoveOnCurrentTerrain() const
{
    if (!bIsOnValidTerrain)
    {
        return false;
    }
    
    // Water vehicles need water
    if (VehicleSettings.bRequiresWater)
    {
        // TODO: Check for water volume when water system is implemented
        return true; // Placeholder
    }
    
    return true;
}

void UCore_VehiclePhysics::ConfigureForVehicleType(ECore_VehicleType NewType)
{
    VehicleSettings.VehicleType = NewType;
    
    switch (NewType)
    {
        case ECore_VehicleType::Cart:
            VehicleSettings.MaxSpeed = 800.0f;
            VehicleSettings.Acceleration = 1200.0f;
            VehicleSettings.TurningRadius = 500.0f;
            VehicleSettings.Mass = 100.0f;
            VehicleSettings.bRequiresWater = false;
            VehicleSettings.MaxSlopeAngle = 30.0f;
            break;
            
        case ECore_VehicleType::Raft:
            VehicleSettings.MaxSpeed = 400.0f;
            VehicleSettings.Acceleration = 600.0f;
            VehicleSettings.TurningRadius = 800.0f;
            VehicleSettings.Mass = 150.0f;
            VehicleSettings.bRequiresWater = true;
            VehicleSettings.MaxSlopeAngle = 0.0f;
            break;
            
        case ECore_VehicleType::Sled:
            VehicleSettings.MaxSpeed = 1200.0f;
            VehicleSettings.Acceleration = 800.0f;
            VehicleSettings.TurningRadius = 300.0f;
            VehicleSettings.Mass = 50.0f;
            VehicleSettings.bRequiresWater = false;
            VehicleSettings.MaxSlopeAngle = 45.0f;
            break;
            
        case ECore_VehicleType::Mount:
            VehicleSettings.MaxSpeed = 1500.0f;
            VehicleSettings.Acceleration = 2000.0f;
            VehicleSettings.TurningRadius = 200.0f;
            VehicleSettings.Mass = 500.0f;
            VehicleSettings.bRequiresWater = false;
            VehicleSettings.MaxSlopeAngle = 60.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Configured for vehicle type %d"), (int32)NewType);
}

void UCore_VehiclePhysics::UpdateMovement(float DeltaTime)
{
    if (!OwnerPawn || !CanMoveOnCurrentTerrain())
    {
        return;
    }
    
    ApplyPhysicsForces(DeltaTime);
}

void UCore_VehiclePhysics::CheckTerrainValidity()
{
    if (!OwnerPawn)
    {
        return;
    }
    
    FVector StartLocation = OwnerPawn->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Check slope angle
        FVector SurfaceNormal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
        
        bIsOnValidTerrain = SlopeAngle <= VehicleSettings.MaxSlopeAngle;
        
        // Special case for water vehicles
        if (VehicleSettings.bRequiresWater)
        {
            // TODO: Check for water surface when water system is implemented
            bIsOnValidTerrain = true; // Placeholder
        }
    }
    else
    {
        // No ground detected
        bIsOnValidTerrain = false;
    }
}

void UCore_VehiclePhysics::ApplyPhysicsForces(float DeltaTime)
{
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(OwnerPawn->GetRootComponent()))
    {
        FVector MovementForce = CalculateMovementForce(DeltaTime);
        FVector SteeringForce = CalculateSteeringForce(DeltaTime);
        FVector TotalForce = MovementForce + SteeringForce;
        
        // Apply braking
        if (bBrakeInput)
        {
            FVector BrakeForce = -CurrentVelocity.GetSafeNormal() * VehicleSettings.Deceleration * VehicleSettings.Mass;
            TotalForce += BrakeForce;
        }
        
        // Apply the force
        if (!TotalForce.IsNearlyZero())
        {
            if (RootPrimitive->IsSimulatingPhysics())
            {
                RootPrimitive->AddForce(TotalForce);
            }
            else
            {
                // Manual movement for non-physics objects
                FVector NewLocation = OwnerPawn->GetActorLocation() + (TotalForce / VehicleSettings.Mass) * DeltaTime * DeltaTime;
                OwnerPawn->SetActorLocation(NewLocation);
            }
        }
    }
}

FVector UCore_VehiclePhysics::CalculateMovementForce(float DeltaTime)
{
    if (FMath::Abs(ThrottleInput) < 0.01f)
    {
        return FVector::ZeroVector;
    }
    
    FVector ForwardVector = OwnerPawn->GetActorForwardVector();
    float DesiredSpeed = ThrottleInput * VehicleSettings.MaxSpeed;
    float CurrentForwardSpeed = FVector::DotProduct(CurrentVelocity, ForwardVector);
    float SpeedDifference = DesiredSpeed - CurrentForwardSpeed;
    
    float AccelerationToUse = (SpeedDifference > 0) ? VehicleSettings.Acceleration : VehicleSettings.Deceleration;
    FVector AccelerationForce = ForwardVector * SpeedDifference * AccelerationToUse * VehicleSettings.Mass / VehicleSettings.MaxSpeed;
    
    return AccelerationForce;
}

FVector UCore_VehiclePhysics::CalculateSteeringForce(float DeltaTime)
{
    if (FMath::Abs(SteeringInput) < 0.01f || CurrentSpeed < 50.0f)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate turning force based on current speed and turning radius
    float TurningAngle = SteeringInput * (CurrentSpeed / VehicleSettings.MaxSpeed) * 90.0f; // Max 90 degrees
    FVector RightVector = OwnerPawn->GetActorRightVector();
    
    // Apply centripetal force for turning
    float CentripetalForce = (VehicleSettings.Mass * CurrentSpeed * CurrentSpeed) / VehicleSettings.TurningRadius;
    FVector SteeringForce = RightVector * SteeringInput * CentripetalForce * 0.1f; // Scale down for control
    
    return SteeringForce;
}