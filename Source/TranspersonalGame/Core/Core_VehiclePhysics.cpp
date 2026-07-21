#include "Core_VehiclePhysics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "LandscapeComponent.h"

UCore_VehiclePhysics::UCore_VehiclePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default wheel configuration for a simple cart
    WheelConfiguration.SetNum(4);
    
    // Front wheels (steerable)
    WheelConfiguration[0].bCanSteer = true;
    WheelConfiguration[1].bCanSteer = true;
    
    // Rear wheels (powered)
    WheelConfiguration[2].bIsPowered = true;
    WheelConfiguration[3].bIsPowered = true;
    
    // Set default physics data for prehistoric cart
    PhysicsData.Mass = 300.0f;  // Lighter than modern vehicles
    PhysicsData.MaxSpeed = 500.0f;  // Slow prehistoric transportation
    PhysicsData.Acceleration = 400.0f;
    PhysicsData.TurningRadius = 800.0f;  // Wide turning radius
    PhysicsData.DragCoefficient = 0.8f;  // High drag for primitive design
    PhysicsData.RollingResistance = 0.05f;  // High resistance on rough terrain
}

void UCore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    SetupWheelMeshes();
    
    // Cache initial position and rotation
    if (AActor* Owner = GetOwner())
    {
        LastPosition = Owner->GetActorLocation();
        LastRotation = Owner->GetActorRotation();
    }
    
    CurrentState = ECore_VehicleState::Idle;
    HealthPercentage = 100.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Initialized %s vehicle"), 
           *UEnum::GetValueAsString(VehicleType));
}

void UCore_VehiclePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CanOperate())
    {
        return;
    }
    
    // Update vehicle physics simulation
    UpdateVehiclePhysics(DeltaTime);
    
    // Update suspension and wheels
    UpdateSuspension(DeltaTime);
    UpdateWheelPhysics(DeltaTime);
    
    // Check terrain interactions
    CheckTerrainInteraction();
    
    // Update vehicle state
    UpdateVehicleState();
    
    // Update wheel mesh transforms
    UpdateWheelTransforms();
}

void UCore_VehiclePhysics::SetThrottleInput(float ThrottleValue)
{
    ThrottleInput = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::SetSteeringInput(float SteeringValue)
{
    SteeringInput = FMath::Clamp(SteeringValue, -1.0f, 1.0f);
    CurrentSteerAngle = SteeringInput * 30.0f; // Max 30 degree steering
}

void UCore_VehiclePhysics::SetBrakeInput(float BrakeValue)
{
    BrakeInput = FMath::Clamp(BrakeValue, 0.0f, 1.0f);
}

void UCore_VehiclePhysics::StartEngine()
{
    if (CanOperate())
    {
        bEngineRunning = true;
        CurrentState = ECore_VehicleState::Idle;
        UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Engine started"));
    }
}

void UCore_VehiclePhysics::StopEngine()
{
    bEngineRunning = false;
    ThrottleInput = 0.0f;
    CurrentState = ECore_VehicleState::Idle;
    UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Engine stopped"));
}

void UCore_VehiclePhysics::ApplyForce(const FVector& Force, const FVector& Location)
{
    if (VehicleMesh && VehicleMesh->GetBodyInstance())
    {
        VehicleMesh->GetBodyInstance()->AddForceAtPosition(Force, Location, false, true);
    }
}

void UCore_VehiclePhysics::ApplyTorque(const FVector& Torque)
{
    if (VehicleMesh && VehicleMesh->GetBodyInstance())
    {
        VehicleMesh->GetBodyInstance()->AddTorqueInRadians(Torque, false, true);
    }
}

void UCore_VehiclePhysics::UpdateVehiclePhysics(float DeltaTime)
{
    if (!bEngineRunning || !VehicleMesh)
    {
        return;
    }
    
    FVector TotalForce = FVector::ZeroVector;
    FVector TotalTorque = FVector::ZeroVector;
    
    // Calculate forces and torques
    CalculateForces(DeltaTime, TotalForce, TotalTorque);
    
    // Apply aerodynamics
    ApplyAerodynamics(TotalForce);
    
    // Apply rolling resistance
    ApplyRollingResistance(TotalForce);
    
    // Apply final forces
    if (!TotalForce.IsNearlyZero())
    {
        ApplyForce(TotalForce, GetOwner()->GetActorLocation());
    }
    
    if (!TotalTorque.IsNearlyZero())
    {
        ApplyTorque(TotalTorque);
    }
    
    // Update speed
    if (AActor* Owner = GetOwner())
    {
        FVector Velocity = (Owner->GetActorLocation() - LastPosition) / DeltaTime;
        CurrentSpeed = Velocity.Size();
        LastPosition = Owner->GetActorLocation();
    }
}

void UCore_VehiclePhysics::CalculateForces(float DeltaTime, FVector& OutForce, FVector& OutTorque)
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = GetOwner()->GetActorRightVector();
    
    // Throttle force
    if (FMath::Abs(ThrottleInput) > 0.01f)
    {
        float ThrottleForce = ThrottleInput * PhysicsData.Acceleration * PhysicsData.Mass;
        OutForce += ForwardVector * ThrottleForce;
    }
    
    // Brake force
    if (BrakeInput > 0.01f)
    {
        FVector BrakeForce = -ForwardVector * BrakeInput * PhysicsData.Deceleration * PhysicsData.Mass;
        OutForce += BrakeForce;
    }
    
    // Steering torque
    if (FMath::Abs(SteeringInput) > 0.01f && CurrentSpeed > 50.0f)
    {
        float SteeringTorque = SteeringInput * (CurrentSpeed / PhysicsData.MaxSpeed) * 1000000.0f;
        OutTorque += FVector(0.0f, 0.0f, SteeringTorque);
    }
}

void UCore_VehiclePhysics::ApplyAerodynamics(FVector& Force)
{
    if (CurrentSpeed < 10.0f)
    {
        return;
    }
    
    // Simple drag calculation
    float DragMagnitude = 0.5f * PhysicsData.DragCoefficient * CurrentSpeed * CurrentSpeed;
    FVector DragDirection = -GetOwner()->GetVelocity().GetSafeNormal();
    Force += DragDirection * DragMagnitude;
}

void UCore_VehiclePhysics::ApplyRollingResistance(FVector& Force)
{
    if (CurrentSpeed < 1.0f)
    {
        return;
    }
    
    float ResistanceMagnitude = PhysicsData.RollingResistance * PhysicsData.Mass * 980.0f; // 9.8m/s^2
    FVector ResistanceDirection = -GetOwner()->GetVelocity().GetSafeNormal();
    Force += ResistanceDirection * ResistanceMagnitude;
}

void UCore_VehiclePhysics::UpdateSuspension(float DeltaTime)
{
    // Simple suspension simulation for each wheel
    for (int32 WheelIndex = 0; WheelIndex < WheelConfiguration.Num(); ++WheelIndex)
    {
        SimulateWheelPhysics(WheelIndex, DeltaTime);
    }
}

void UCore_VehiclePhysics::UpdateWheelPhysics(float DeltaTime)
{
    // Update wheel rotation based on vehicle speed
    float WheelRotationSpeed = CurrentSpeed / (2.0f * PI * 30.0f); // Assuming 30cm wheel radius
    
    for (int32 i = 0; i < WheelMeshes.Num(); ++i)
    {
        if (WheelMeshes[i])
        {
            FRotator CurrentRotation = WheelMeshes[i]->GetRelativeRotation();
            CurrentRotation.Pitch += WheelRotationSpeed * DeltaTime * 360.0f;
            WheelMeshes[i]->SetRelativeRotation(CurrentRotation);
        }
    }
}

void UCore_VehiclePhysics::CheckTerrainInteraction()
{
    PerformTerrainTrace();
    UpdateTerrainEffects();
}

float UCore_VehiclePhysics::GetTerrainFriction() const
{
    // Base friction value, can be modified by terrain type
    float BaseFriction = 1.0f;
    
    // Perform line trace to get terrain material
    if (UWorld* World = GetWorld())
    {
        FVector StartLocation = GetOwner()->GetActorLocation();
        FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 200.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
        {
            // Modify friction based on hit surface
            if (HitResult.GetActor() && HitResult.GetActor()->IsA<ALandscape>())
            {
                // Terrain-specific friction modifications
                BaseFriction = 0.8f; // Slightly reduced for natural terrain
            }
        }
    }
    
    return BaseFriction;
}

void UCore_VehiclePhysics::CreateTerrainDeformation()
{
    // Create wheel tracks and deformation
    if (CurrentSpeed > 10.0f && VehicleMesh)
    {
        FVector VehicleLocation = GetOwner()->GetActorLocation();
        
        // Simple deformation effect - could be expanded with actual landscape modification
        UE_LOG(LogTemp, VeryVerbose, TEXT("Creating terrain deformation at %s"), 
               *VehicleLocation.ToString());
    }
}

void UCore_VehiclePhysics::ApplyDamage(float DamageAmount, const FVector& ImpactLocation)
{
    HealthPercentage = FMath::Clamp(HealthPercentage - DamageAmount, 0.0f, 100.0f);
    
    if (HealthPercentage <= 0.0f)
    {
        CurrentState = ECore_VehicleState::Destroyed;
        StopEngine();
    }
    else if (HealthPercentage < 25.0f)
    {
        CurrentState = ECore_VehicleState::Damaged;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Vehicle damaged: %.1f%% health remaining"), HealthPercentage);
}

void UCore_VehiclePhysics::RepairVehicle(float RepairAmount)
{
    HealthPercentage = FMath::Clamp(HealthPercentage + RepairAmount, 0.0f, 100.0f);
    
    if (HealthPercentage > 25.0f && CurrentState == ECore_VehicleState::Damaged)
    {
        CurrentState = ECore_VehicleState::Idle;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle repaired: %.1f%% health"), HealthPercentage);
}

bool UCore_VehiclePhysics::CanOperate() const
{
    return CurrentState != ECore_VehicleState::Destroyed && HealthPercentage > 0.0f;
}

FVector UCore_VehiclePhysics::GetCenterOfMass() const
{
    if (VehicleMesh)
    {
        return VehicleMesh->GetCenterOfMass();
    }
    return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

float UCore_VehiclePhysics::GetTotalMass() const
{
    return PhysicsData.Mass;
}

bool UCore_VehiclePhysics::IsOnGround() const
{
    if (!GetOwner())
    {
        return false;
    }
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams);
}

void UCore_VehiclePhysics::ResetVehicle()
{
    ThrottleInput = 0.0f;
    SteeringInput = 0.0f;
    BrakeInput = 0.0f;
    CurrentSpeed = 0.0f;
    CurrentSteerAngle = 0.0f;
    
    if (HealthPercentage > 0.0f)
    {
        CurrentState = ECore_VehicleState::Idle;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle reset"));
}

void UCore_VehiclePhysics::DebugDrawVehicleInfo()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    FVector Location = GetOwner()->GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);
    
    // Draw vehicle state info
    FString StateText = FString::Printf(TEXT("State: %s\nSpeed: %.1f\nHealth: %.1f%%"), 
                                       *UEnum::GetValueAsString(CurrentState),
                                       CurrentSpeed,
                                       HealthPercentage);
    
    DrawDebugString(GetWorld(), Location, StateText, nullptr, FColor::White, 0.0f, true);
    
    // Draw velocity vector
    if (CurrentSpeed > 1.0f)
    {
        FVector VelocityEnd = GetOwner()->GetActorLocation() + GetOwner()->GetVelocity().GetSafeNormal() * 200.0f;
        DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetActorLocation(), VelocityEnd, 
                                 50.0f, FColor::Green, false, 0.0f, 0, 3.0f);
    }
}

void UCore_VehiclePhysics::ValidateVehicleSetup()
{
    bool bIsValid = true;
    
    if (WheelConfiguration.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No wheel configuration found"));
        bIsValid = false;
    }
    
    if (PhysicsData.Mass <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid vehicle mass"));
        bIsValid = false;
    }
    
    if (!VehicleMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No vehicle mesh component found"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle setup validation: %s"), bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
}

void UCore_VehiclePhysics::UpdateVehicleState()
{
    if (!CanOperate())
    {
        return;
    }
    
    if (CurrentSpeed > 5.0f)
    {
        CurrentState = ECore_VehicleState::Moving;
    }
    else if (bEngineRunning)
    {
        CurrentState = ECore_VehicleState::Idle;
    }
    
    // Check if stuck (trying to move but not moving)
    if (FMath::Abs(ThrottleInput) > 0.5f && CurrentSpeed < 1.0f)
    {
        CurrentState = ECore_VehicleState::Stuck;
    }
}

void UCore_VehiclePhysics::SimulateWheelPhysics(int32 WheelIndex, float DeltaTime)
{
    if (WheelIndex < 0 || WheelIndex >= WheelConfiguration.Num())
    {
        return;
    }
    
    const FCore_VehicleWheelData& WheelData = WheelConfiguration[WheelIndex];
    
    // Calculate wheel load
    float WheelLoad = CalculateWheelLoad(WheelIndex);
    
    // Calculate wheel force
    FVector WheelForce = CalculateWheelForce(WheelIndex, WheelLoad);
    
    // Apply force to vehicle
    if (!WheelForce.IsNearlyZero())
    {
        FVector WheelLocation = GetOwner()->GetActorLocation(); // Simplified wheel position
        ApplyForce(WheelForce, WheelLocation);
    }
}

float UCore_VehiclePhysics::CalculateWheelLoad(int32 WheelIndex) const
{
    // Simplified load distribution - equal load on all wheels
    return PhysicsData.Mass * 980.0f / FMath::Max(1, WheelConfiguration.Num());
}

FVector UCore_VehiclePhysics::CalculateWheelForce(int32 WheelIndex, float Load) const
{
    if (WheelIndex < 0 || WheelIndex >= WheelConfiguration.Num())
    {
        return FVector::ZeroVector;
    }
    
    const FCore_VehicleWheelData& WheelData = WheelConfiguration[WheelIndex];
    FVector Force = FVector::ZeroVector;
    
    // Traction force for powered wheels
    if (WheelData.bIsPowered && FMath::Abs(ThrottleInput) > 0.01f)
    {
        float TractionForce = ThrottleInput * Load * WheelData.FrictionSlip * GetTerrainFriction();
        Force += GetOwner()->GetActorForwardVector() * TractionForce;
    }
    
    return Force;
}

void UCore_VehiclePhysics::PerformTerrainTrace()
{
    // Perform terrain analysis for each wheel
    for (int32 i = 0; i < WheelConfiguration.Num(); ++i)
    {
        // Simplified terrain trace - in full implementation would trace for each wheel position
        FVector WheelLocation = GetOwner()->GetActorLocation();
        FVector TraceEnd = WheelLocation - FVector(0.0f, 0.0f, 150.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, WheelLocation, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            // Process terrain hit result
            if (HitResult.GetActor())
            {
                // Could analyze terrain material, slope, etc.
            }
        }
    }
}

void UCore_VehiclePhysics::UpdateTerrainEffects()
{
    // Update terrain-based effects like dust, tracks, deformation
    if (CurrentSpeed > 10.0f)
    {
        CreateTerrainDeformation();
    }
}

void UCore_VehiclePhysics::InitializeComponents()
{
    // Find or create vehicle mesh component
    if (AActor* Owner = GetOwner())
    {
        VehicleMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (!VehicleMesh)
        {
            VehicleMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
}

void UCore_VehiclePhysics::SetupWheelMeshes()
{
    // Initialize wheel mesh array
    WheelMeshes.SetNum(WheelConfiguration.Num());
    
    // In a full implementation, would create or find wheel mesh components
    for (int32 i = 0; i < WheelConfiguration.Num(); ++i)
    {
        // Placeholder - would create actual wheel mesh components
        WheelMeshes[i] = nullptr;
    }
}

void UCore_VehiclePhysics::UpdateWheelTransforms()
{
    // Update wheel positions and rotations
    for (int32 i = 0; i < WheelMeshes.Num(); ++i)
    {
        if (WheelMeshes[i])
        {
            // Update steering for front wheels
            if (WheelConfiguration[i].bCanSteer)
            {
                FRotator WheelRotation = WheelMeshes[i]->GetRelativeRotation();
                WheelRotation.Yaw = CurrentSteerAngle;
                WheelMeshes[i]->SetRelativeRotation(WheelRotation);
            }
        }
    }
}