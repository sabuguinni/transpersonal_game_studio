#include "Core_VehiclePhysicsManager.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_VehiclePhysicsManager::UCore_VehiclePhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentSpeed = 0.0f;
    CurrentRPM = 800.0f; // Idle RPM
    CurrentSteerAngle = 0.0f;
    bIsOnGround = false;
    bIsFlipped = false;
    VehicleVelocity = FVector::ZeroVector;
    
    // Initialize input state
    ThrottleInput = 0.0f;
    SteerInput = 0.0f;
    BrakeInput = 0.0f;
    bHandbrakeEngaged = false;
    
    // Initialize internal state
    AccumulatedDistance = 0.0f;
    LastUpdateTime = 0.0f;
    LastPosition = FVector::ZeroVector;
    LastRotation = FRotator::ZeroRotator;
    PhysicsUpdateCounter = 0;
    bNeedsFullUpdate = true;
    
    // Initialize cached references
    VehicleBody = nullptr;
    TerrainPhysicsManager = nullptr;
    CachedWorld = nullptr;
}

void UCore_VehiclePhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache world reference
    CachedWorld = GetWorld();
    
    // Initialize vehicle physics
    InitializeVehiclePhysics();
    
    // Find terrain physics manager
    if (CachedWorld)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            TerrainPhysicsManager = Owner->FindComponentByClass<UCore_TerrainPhysicsManager>();
        }
    }
    
    // Initialize wheel data if empty
    if (WheelData.Num() == 0)
    {
        // Default 4-wheel configuration
        for (int32 i = 0; i < 4; i++)
        {
            FCore_VehicleWheelData WheelInfo;
            WheelInfo.bIsDriveWheel = (i >= 2); // Rear wheels are drive wheels
            WheelInfo.bIsSteerWheel = (i < 2);  // Front wheels are steer wheels
            WheelData.Add(WheelInfo);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsManager: Initialized for %s"), 
           Owner ? *Owner->GetName() : TEXT("Unknown"));
}

void UCore_VehiclePhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CachedWorld || !GetOwner())
    {
        return;
    }
    
    // Update vehicle physics
    UpdateVehiclePhysics(DeltaTime);
    
    // Update performance counter
    PhysicsUpdateCounter++;
    if (PhysicsUpdateCounter >= 10)
    {
        PhysicsUpdateCounter = 0;
        bNeedsFullUpdate = true;
    }
}

void UCore_VehiclePhysicsManager::InitializeVehiclePhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysicsManager: No owner actor"));
        return;
    }
    
    // Find vehicle body component
    VehicleBody = Owner->FindComponentByClass<UPrimitiveComponent>();
    if (!VehicleBody)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysicsManager: No primitive component found"));
        return;
    }
    
    // Configure physics properties
    if (VehicleBody->IsSimulatingPhysics())
    {
        VehicleBody->SetMassOverrideInKg(NAME_None, VehicleProfile.Mass);
        VehicleBody->SetLinearDamping(0.1f);
        VehicleBody->SetAngularDamping(0.3f);
        VehicleBody->SetCenterOfMass(FVector(0, 0, -20));
    }
    
    // Initialize wheel components
    WheelComponents.Empty();
    TArray<UActorComponent*> Components = Owner->GetRootComponent()->GetAttachChildren().Array();
    for (UActorComponent* Component : Components)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
        {
            if (PrimComp->GetName().Contains(TEXT("Wheel")))
            {
                WheelComponents.Add(PrimComp);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsManager: Found %d wheel components"), WheelComponents.Num());
}

void UCore_VehiclePhysicsManager::UpdateVehiclePhysics(float DeltaTime)
{
    if (!VehicleBody || !VehicleBody->IsSimulatingPhysics())
    {
        return;
    }
    
    // Update current state
    VehicleVelocity = VehicleBody->GetPhysicsLinearVelocity();
    CurrentSpeed = VehicleVelocity.Size();
    
    // Check ground contact
    FHitResult GroundHit = PerformGroundTrace();
    bIsOnGround = GroundHit.bBlockingHit;
    
    // Check if vehicle is flipped
    FVector UpVector = GetOwner()->GetActorUpVector();
    bIsFlipped = FVector::DotProduct(UpVector, FVector::UpVector) < 0.3f;
    
    if (bIsOnGround && !bIsFlipped)
    {
        // Apply engine force
        ApplyEngineForce(DeltaTime);
        
        // Apply steering
        ApplySteeringForce(DeltaTime);
        
        // Apply braking
        ApplyBrakingForce(DeltaTime);
        
        // Update suspension
        UpdateSuspension(DeltaTime);
        
        // Calculate traction
        CalculateWheelTraction();
        
        // Handle terrain interaction
        HandleTerrainInteraction();
    }
    
    // Check stability
    CheckVehicleStability();
    
    // Apply safety limits
    LimitMaxSpeed();
    
    // Update accumulated distance
    FVector CurrentPosition = GetOwner()->GetActorLocation();
    if (LastPosition != FVector::ZeroVector)
    {
        AccumulatedDistance += FVector::Dist(CurrentPosition, LastPosition);
    }
    LastPosition = CurrentPosition;
    LastUpdateTime += DeltaTime;
}

void UCore_VehiclePhysicsManager::ApplyEngineForce(float DeltaTime)
{
    if (FMath::Abs(ThrottleInput) < 0.01f)
    {
        return;
    }
    
    // Calculate engine output
    CalculateEngineOutput(DeltaTime);
    
    // Calculate force based on current RPM and throttle
    float TorqueOutput = EngineData.MaxTorque * (CurrentRPM / EngineData.MaxRPM) * ThrottleInput;
    float ForceMultiplier = TorqueOutput / VehicleProfile.Mass;
    
    // Apply force in forward direction
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector EngineForce = ForwardVector * ForceMultiplier * VehicleProfile.Acceleration;
    
    // Apply terrain friction modifier
    float FrictionMultiplier = GetTerrainFrictionMultiplier();
    EngineForce *= FrictionMultiplier;
    
    VehicleBody->AddForce(EngineForce, NAME_None, true);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Engine Force Applied: %s (RPM: %.1f, Throttle: %.2f)"), 
           *EngineForce.ToString(), CurrentRPM, ThrottleInput);
}

void UCore_VehiclePhysicsManager::ApplySteeringForce(float DeltaTime)
{
    if (FMath::Abs(SteerInput) < 0.01f || CurrentSpeed < 50.0f)
    {
        return;
    }
    
    // Calculate steering angle based on speed
    float SpeedFactor = FMath::Clamp(CurrentSpeed / VehicleProfile.MaxSpeed, 0.1f, 1.0f);
    CurrentSteerAngle = SteerInput * (90.0f / SpeedFactor); // Max 90 degrees at low speed
    
    // Apply lateral force for turning
    FVector RightVector = GetOwner()->GetActorRightVector();
    float LateralForce = CurrentSteerAngle * CurrentSpeed * 0.1f;
    
    VehicleBody->AddForce(RightVector * LateralForce, NAME_None, true);
    
    // Apply torque for rotation
    FVector TorqueVector = FVector::UpVector * CurrentSteerAngle * CurrentSpeed * 0.05f;
    VehicleBody->AddTorqueInRadians(TorqueVector, NAME_None, true);
}

void UCore_VehiclePhysicsManager::ApplyBrakingForce(float DeltaTime)
{
    if (BrakeInput < 0.01f && !bHandbrakeEngaged)
    {
        return;
    }
    
    // Calculate braking force
    float BrakingPower = bHandbrakeEngaged ? VehicleProfile.Deceleration * 2.0f : VehicleProfile.Deceleration * BrakeInput;
    
    // Apply force opposite to velocity
    if (VehicleVelocity.SizeSquared() > 1.0f)
    {
        FVector BrakingDirection = -VehicleVelocity.GetSafeNormal();
        FVector BrakingForce = BrakingDirection * BrakingPower;
        
        VehicleBody->AddForce(BrakingForce, NAME_None, true);
        
        // Reduce RPM when braking
        CurrentRPM = FMath::Max(EngineData.IdleRPM, CurrentRPM - (BrakingPower * DeltaTime * 0.1f));
    }
}

void UCore_VehiclePhysicsManager::UpdateSuspension(float DeltaTime)
{
    // Simple suspension simulation
    for (int32 i = 0; i < WheelComponents.Num() && i < WheelData.Num(); i++)
    {
        UPrimitiveComponent* Wheel = WheelComponents[i];
        if (!Wheel)
        {
            continue;
        }
        
        // Perform wheel trace
        FVector WheelLocation = Wheel->GetComponentLocation();
        FVector TraceStart = WheelLocation + FVector::UpVector * 50.0f;
        FVector TraceEnd = WheelLocation - FVector::UpVector * 100.0f;
        
        FHitResult WheelHit;
        bool bHit = CachedWorld->LineTraceSingleByChannel(
            WheelHit, TraceStart, TraceEnd, ECC_WorldStatic
        );
        
        if (bHit)
        {
            // Calculate suspension compression
            float CompressionDistance = FVector::Dist(TraceStart, WheelHit.Location);
            float CompressionRatio = FMath::Clamp((150.0f - CompressionDistance) / 100.0f, 0.0f, 1.0f);
            
            // Apply suspension force
            float SuspensionForce = VehicleProfile.SuspensionStiffness * CompressionRatio;
            FVector UpForce = FVector::UpVector * SuspensionForce;
            
            VehicleBody->AddForceAtLocation(UpForce, WheelLocation, NAME_None, true);
        }
    }
}

void UCore_VehiclePhysicsManager::CalculateWheelTraction()
{
    // Calculate traction based on terrain and wheel contact
    float TotalTraction = 0.0f;
    int32 ContactingWheels = 0;
    
    for (int32 i = 0; i < WheelData.Num(); i++)
    {
        // Simple traction calculation
        float WheelTraction = WheelData[i].FrictionCoefficient * GetTerrainFrictionMultiplier();
        TotalTraction += WheelTraction;
        ContactingWheels++;
    }
    
    if (ContactingWheels > 0)
    {
        VehicleProfile.TractionMultiplier = TotalTraction / ContactingWheels;
    }
}

void UCore_VehiclePhysicsManager::CheckVehicleStability()
{
    if (!VehicleBody)
    {
        return;
    }
    
    // Check angular velocity for stability
    FVector AngularVelocity = VehicleBody->GetPhysicsAngularVelocityInRadians();
    float AngularSpeed = AngularVelocity.Size();
    
    // Apply stability control if needed
    if (AngularSpeed > 5.0f) // Threshold for instability
    {
        ApplyStabilityControl();
    }
    
    // Check for rollover risk
    CheckRolloverRisk();
}

void UCore_VehiclePhysicsManager::HandleTerrainInteraction()
{
    if (!TerrainPhysicsManager)
    {
        return;
    }
    
    // Update terrain-based traction
    UpdateTerrainTraction();
    
    // Apply terrain-specific effects
    ETerrainType CurrentTerrain = GetCurrentTerrainType();
    switch (CurrentTerrain)
    {
        case ETerrainType::Mud:
            VehicleProfile.TractionMultiplier *= 0.6f;
            break;
        case ETerrainType::Sand:
            VehicleProfile.TractionMultiplier *= 0.7f;
            break;
        case ETerrainType::Snow:
            VehicleProfile.TractionMultiplier *= 0.5f;
            break;
        case ETerrainType::Rock:
            VehicleProfile.TractionMultiplier *= 1.2f;
            break;
        default:
            break;
    }
}

bool UCore_VehiclePhysicsManager::IsVehicleMoving() const
{
    return CurrentSpeed > 10.0f; // 10 cm/s threshold
}

bool UCore_VehiclePhysicsManager::IsVehicleStable() const
{
    if (!VehicleBody)
    {
        return false;
    }
    
    FVector AngularVelocity = VehicleBody->GetPhysicsAngularVelocityInRadians();
    return AngularVelocity.Size() < 2.0f && bIsOnGround && !bIsFlipped;
}

float UCore_VehiclePhysicsManager::GetVehicleSpeedKMH() const
{
    return CurrentSpeed * 0.036f; // Convert cm/s to km/h
}

FVector UCore_VehiclePhysicsManager::GetVehicleForwardVector() const
{
    return GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
}

FVector UCore_VehiclePhysicsManager::GetVehicleRightVector() const
{
    return GetOwner() ? GetOwner()->GetActorRightVector() : FVector::RightVector;
}

void UCore_VehiclePhysicsManager::SetThrottleInput(float Throttle)
{
    ThrottleInput = FMath::Clamp(Throttle, -1.0f, 1.0f);
}

void UCore_VehiclePhysicsManager::SetSteerInput(float Steer)
{
    SteerInput = FMath::Clamp(Steer, -1.0f, 1.0f);
}

void UCore_VehiclePhysicsManager::SetBrakeInput(float Brake)
{
    BrakeInput = FMath::Clamp(Brake, 0.0f, 1.0f);
}

void UCore_VehiclePhysicsManager::SetHandbrake(bool bEngaged)
{
    bHandbrakeEngaged = bEngaged;
}

void UCore_VehiclePhysicsManager::ResetVehicle()
{
    if (!VehicleBody || !GetOwner())
    {
        return;
    }
    
    // Reset position and rotation
    FVector ResetLocation = GetOwner()->GetActorLocation() + FVector::UpVector * 200.0f;
    FRotator ResetRotation = FRotator::ZeroRotator;
    
    GetOwner()->SetActorLocationAndRotation(ResetLocation, ResetRotation);
    
    // Reset physics state
    VehicleBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
    VehicleBody->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    
    // Reset input state
    ThrottleInput = 0.0f;
    SteerInput = 0.0f;
    BrakeInput = 0.0f;
    bHandbrakeEngaged = false;
    
    // Reset internal state
    CurrentSpeed = 0.0f;
    CurrentRPM = EngineData.IdleRPM;
    CurrentSteerAngle = 0.0f;
    bIsFlipped = false;
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsManager: Vehicle reset"));
}

void UCore_VehiclePhysicsManager::UpdateTerrainTraction()
{
    // Implementation depends on terrain physics manager
    // This is a placeholder for terrain integration
}

float UCore_VehiclePhysicsManager::GetTerrainFrictionMultiplier() const
{
    // Default friction multiplier
    return 1.0f;
}

bool UCore_VehiclePhysicsManager::IsOnValidTerrain() const
{
    return bIsOnGround;
}

void UCore_VehiclePhysicsManager::CalculateEngineOutput(float DeltaTime)
{
    // Calculate target RPM based on throttle and current speed
    float TargetRPM = EngineData.IdleRPM + (ThrottleInput * (EngineData.MaxRPM - EngineData.IdleRPM));
    
    // Smooth RPM transition
    float RPMDifference = TargetRPM - CurrentRPM;
    float RPMChangeRate = 1000.0f * DeltaTime; // RPM change per second
    
    if (FMath::Abs(RPMDifference) > RPMChangeRate)
    {
        CurrentRPM += FMath::Sign(RPMDifference) * RPMChangeRate;
    }
    else
    {
        CurrentRPM = TargetRPM;
    }
    
    // Apply engine braking when throttle is released
    if (ThrottleInput < 0.1f)
    {
        CurrentRPM = FMath::Max(EngineData.IdleRPM, CurrentRPM - (EngineData.EngineBraking * DeltaTime * 100.0f));
    }
}

void UCore_VehiclePhysicsManager::CalculateWheelForces(float DeltaTime)
{
    // Advanced wheel force calculation would go here
    // This is a simplified implementation
}

void UCore_VehiclePhysicsManager::ApplyDownforce()
{
    // Apply aerodynamic downforce at high speeds
    if (CurrentSpeed > 500.0f)
    {
        float DownforceAmount = (CurrentSpeed / VehicleProfile.MaxSpeed) * VehicleProfile.Mass * 0.1f;
        FVector DownForce = -FVector::UpVector * DownforceAmount;
        VehicleBody->AddForce(DownForce, NAME_None, true);
    }
}

void UCore_VehiclePhysicsManager::UpdateVehicleOrientation(float DeltaTime)
{
    // Vehicle orientation updates based on movement
}

void UCore_VehiclePhysicsManager::HandleCollisionResponse()
{
    // Handle collision responses and damage
}

FHitResult UCore_VehiclePhysicsManager::PerformGroundTrace() const
{
    FHitResult HitResult;
    
    if (!CachedWorld || !GetOwner())
    {
        return HitResult;
    }
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector::UpVector * 200.0f;
    
    CachedWorld->LineTraceSingleByChannel(
        HitResult, StartLocation, EndLocation, ECC_WorldStatic
    );
    
    return HitResult;
}

ETerrainType UCore_VehiclePhysicsManager::GetCurrentTerrainType() const
{
    // Default terrain type - should be integrated with terrain system
    return ETerrainType::Grass;
}

float UCore_VehiclePhysicsManager::GetSurfaceSlope() const
{
    FHitResult GroundHit = PerformGroundTrace();
    if (GroundHit.bBlockingHit)
    {
        FVector SurfaceNormal = GroundHit.Normal;
        return FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)) * (180.0f / PI);
    }
    
    return 0.0f;
}

FVector UCore_VehiclePhysicsManager::CalculateLateralForce() const
{
    // Calculate lateral forces for realistic vehicle dynamics
    return FVector::ZeroVector;
}

FVector UCore_VehiclePhysicsManager::CalculateLongitudinalForce() const
{
    // Calculate longitudinal forces
    return FVector::ZeroVector;
}

float UCore_VehiclePhysicsManager::CalculateAerodynamicDrag() const
{
    // Calculate air resistance
    return CurrentSpeed * CurrentSpeed * 0.001f;
}

float UCore_VehiclePhysicsManager::CalculateRollingResistance() const
{
    // Calculate rolling resistance
    return VehicleProfile.Mass * 9.81f * 0.02f; // 2% rolling resistance
}

void UCore_VehiclePhysicsManager::CheckRolloverRisk()
{
    if (!VehicleBody)
    {
        return;
    }
    
    // Check vehicle roll angle
    FRotator VehicleRotation = GetOwner()->GetActorRotation();
    float RollAngle = FMath::Abs(VehicleRotation.Roll);
    
    if (RollAngle > 45.0f && VehicleProfile.bCanFlip)
    {
        // High rollover risk - apply corrective forces
        FVector CorrectiveForce = FVector::UpVector * VehicleProfile.Mass * 2.0f;
        VehicleBody->AddForce(CorrectiveForce, NAME_None, true);
    }
}

void UCore_VehiclePhysicsManager::ApplyStabilityControl()
{
    if (!VehicleBody)
    {
        return;
    }
    
    // Apply stability control by dampening excessive rotation
    FVector AngularVelocity = VehicleBody->GetPhysicsAngularVelocityInRadians();
    FVector StabilityTorque = -AngularVelocity * VehicleProfile.StabilityFactor * VehicleProfile.Mass;
    
    VehicleBody->AddTorqueInRadians(StabilityTorque, NAME_None, true);
}

void UCore_VehiclePhysicsManager::LimitMaxSpeed()
{
    if (CurrentSpeed > VehicleProfile.MaxSpeed)
    {
        // Apply speed limiting force
        FVector VelocityDirection = VehicleVelocity.GetSafeNormal();
        FVector LimitingForce = -VelocityDirection * (CurrentSpeed - VehicleProfile.MaxSpeed) * 10.0f;
        
        VehicleBody->AddForce(LimitingForce, NAME_None, true);
    }
}