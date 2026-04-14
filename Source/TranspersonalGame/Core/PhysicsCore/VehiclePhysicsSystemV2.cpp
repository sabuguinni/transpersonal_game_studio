#include "VehiclePhysicsSystemV2.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

DEFINE_LOG_CATEGORY(LogVehiclePhysicsV2);

UVehiclePhysicsSystemV2::UVehiclePhysicsSystemV2()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    MaxSpeed = 2000.0f;
    Acceleration = 1200.0f;
    Deceleration = 2000.0f;
    TurnRate = 90.0f;
    MaxSteerAngle = 30.0f;
    WheelRadius = 35.0f;
    SuspensionStiffness = 10000.0f;
    SuspensionDamping = 500.0f;
    MaxSuspensionTravel = 20.0f;
    AntiRollForce = 5000.0f;
    DownforceCoefficient = 0.3f;
    DragCoefficient = 0.3f;
    RollingResistance = 30.0f;
    bUseAdvancedSuspension = true;
    bEnableDownforce = true;
    bEnableAntiRoll = true;
    bDebugDraw = false;
    
    // Initialize wheel data
    WheelData.SetNum(4);
    for (int32 i = 0; i < 4; i++)
    {
        WheelData[i].WheelIndex = i;
        WheelData[i].bIsGrounded = false;
        WheelData[i].SuspensionLength = MaxSuspensionTravel * 0.5f;
        WheelData[i].WheelRotation = 0.0f;
        WheelData[i].SteerAngle = 0.0f;
        WheelData[i].SlipRatio = 0.0f;
        WheelData[i].SlipAngle = 0.0f;
        WheelData[i].GripForce = 0.0f;
    }
    
    CurrentSpeed = 0.0f;
    ThrottleInput = 0.0f;
    SteerInput = 0.0f;
    BrakeInput = 0.0f;
    HandbrakeInput = 0.0f;
    
    VehicleState = ECore_VehicleState::Stationary;
}

void UVehiclePhysicsSystemV2::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the vehicle mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        VehicleMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (!VehicleMesh)
        {
            VehicleMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
        
        if (VehicleMesh)
        {
            // Enable physics simulation
            VehicleMesh->SetSimulatePhysics(true);
            VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Set up collision
            VehicleMesh->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
            VehicleMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            
            UE_LOG(LogVehiclePhysicsV2, Log, TEXT("Vehicle physics initialized for %s"), *Owner->GetName());
        }
        else
        {
            UE_LOG(LogVehiclePhysicsV2, Warning, TEXT("No mesh component found for vehicle %s"), *Owner->GetName());
        }
    }
    
    InitializePhysicsSystem();
}

void UVehiclePhysicsSystemV2::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!VehicleMesh || !VehicleMesh->IsSimulatingPhysics())
    {
        return;
    }
    
    // Update vehicle physics
    UpdateWheelPhysics(DeltaTime);
    UpdateSuspension(DeltaTime);
    UpdateEngineForces(DeltaTime);
    UpdateAerodynamics(DeltaTime);
    UpdateVehicleState(DeltaTime);
    
    if (bDebugDraw)
    {
        DrawDebugInfo();
    }
}

void UVehiclePhysicsSystemV2::InitializePhysicsSystem()
{
    if (!VehicleMesh)
    {
        return;
    }
    
    // Set up wheel positions (default layout for a standard vehicle)
    WheelPositions.SetNum(4);
    WheelPositions[0] = FVector(150.0f, -80.0f, -50.0f);   // Front Left
    WheelPositions[1] = FVector(150.0f, 80.0f, -50.0f);    // Front Right
    WheelPositions[2] = FVector(-150.0f, -80.0f, -50.0f);  // Rear Left
    WheelPositions[3] = FVector(-150.0f, 80.0f, -50.0f);   // Rear Right
    
    // Initialize physics materials
    if (!WheelPhysicsMaterial)
    {
        WheelPhysicsMaterial = LoadObject<UPhysicalMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultPhysicalMaterial.DefaultPhysicalMaterial"));
    }
    
    UE_LOG(LogVehiclePhysicsV2, Log, TEXT("Vehicle physics system initialized"));
}

void UVehiclePhysicsSystemV2::UpdateWheelPhysics(float DeltaTime)
{
    if (!VehicleMesh || WheelPositions.Num() != 4)
    {
        return;
    }
    
    FVector VehicleLocation = VehicleMesh->GetComponentLocation();
    FRotator VehicleRotation = VehicleMesh->GetComponentRotation();
    
    for (int32 i = 0; i < 4; i++)
    {
        FCore_WheelData& Wheel = WheelData[i];
        
        // Calculate world wheel position
        FVector WorldWheelPos = VehicleLocation + VehicleRotation.RotateVector(WheelPositions[i]);
        
        // Perform suspension raycast
        FVector TraceStart = WorldWheelPos + FVector(0, 0, MaxSuspensionTravel * 0.5f);
        FVector TraceEnd = WorldWheelPos - FVector(0, 0, MaxSuspensionTravel * 0.5f + WheelRadius);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECollisionChannel::ECC_WorldStatic,
            QueryParams
        );
        
        if (bHit)
        {
            Wheel.bIsGrounded = true;
            Wheel.SuspensionLength = FVector::Dist(TraceStart, HitResult.Location) - WheelRadius;
            Wheel.GroundNormal = HitResult.Normal;
            Wheel.GroundMaterial = HitResult.PhysMaterial.Get();
            
            // Apply suspension forces
            ApplySuspensionForce(i, HitResult.Location, HitResult.Normal, DeltaTime);
            
            // Update wheel rotation
            Wheel.WheelRotation += (CurrentSpeed / WheelRadius) * DeltaTime * 57.2958f; // Convert to degrees
        }
        else
        {
            Wheel.bIsGrounded = false;
            Wheel.SuspensionLength = MaxSuspensionTravel;
        }
    }
}

void UVehiclePhysicsSystemV2::UpdateSuspension(float DeltaTime)
{
    if (!bUseAdvancedSuspension)
    {
        return;
    }
    
    // Calculate anti-roll forces
    if (bEnableAntiRoll && WheelData[0].bIsGrounded && WheelData[1].bIsGrounded)
    {
        float FrontRollForce = (WheelData[0].SuspensionLength - WheelData[1].SuspensionLength) * AntiRollForce;
        ApplyAntiRollForce(0, 1, FrontRollForce);
    }
    
    if (bEnableAntiRoll && WheelData[2].bIsGrounded && WheelData[3].bIsGrounded)
    {
        float RearRollForce = (WheelData[2].SuspensionLength - WheelData[3].SuspensionLength) * AntiRollForce;
        ApplyAntiRollForce(2, 3, RearRollForce);
    }
}

void UVehiclePhysicsSystemV2::UpdateEngineForces(float DeltaTime)
{
    if (!VehicleMesh)
    {
        return;
    }
    
    // Calculate engine force
    float EngineForce = 0.0f;
    if (FMath::Abs(ThrottleInput) > 0.01f)
    {
        EngineForce = ThrottleInput * Acceleration;
        
        // Apply speed limiting
        if (CurrentSpeed > MaxSpeed)
        {
            EngineForce = FMath::Min(EngineForce, 0.0f);
        }
    }
    
    // Apply braking force
    float BrakeForce = BrakeInput * Deceleration;
    if (HandbrakeInput > 0.01f)
    {
        BrakeForce += HandbrakeInput * Deceleration * 0.7f; // Handbrake is less effective
    }
    
    // Calculate total force
    FVector ForwardVector = VehicleMesh->GetForwardVector();
    FVector TotalForce = ForwardVector * (EngineForce - BrakeForce);
    
    // Apply rolling resistance
    FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
    FVector ResistanceForce = -Velocity.GetSafeNormal() * RollingResistance;
    TotalForce += ResistanceForce;
    
    // Apply force to vehicle
    VehicleMesh->AddForce(TotalForce, NAME_None, true);
    
    // Update current speed
    CurrentSpeed = Velocity.Size();
}

void UVehiclePhysicsSystemV2::UpdateAerodynamics(float DeltaTime)
{
    if (!VehicleMesh || !bEnableDownforce)
    {
        return;
    }
    
    FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
    float Speed = Velocity.Size();
    
    if (Speed > 100.0f) // Only apply aerodynamics at higher speeds
    {
        // Calculate downforce
        float DownforceAmount = DownforceCoefficient * Speed * Speed * 0.0001f;
        FVector DownforceVector = -VehicleMesh->GetUpVector() * DownforceAmount;
        VehicleMesh->AddForce(DownforceVector, NAME_None, true);
        
        // Calculate drag
        float DragAmount = DragCoefficient * Speed * Speed * 0.0001f;
        FVector DragVector = -Velocity.GetSafeNormal() * DragAmount;
        VehicleMesh->AddForce(DragVector, NAME_None, true);
    }
}

void UVehiclePhysicsSystemV2::UpdateVehicleState(float DeltaTime)
{
    // Update vehicle state based on current conditions
    if (CurrentSpeed < 10.0f)
    {
        VehicleState = ECore_VehicleState::Stationary;
    }
    else if (FMath::Abs(ThrottleInput) > 0.1f)
    {
        VehicleState = ECore_VehicleState::Accelerating;
    }
    else if (BrakeInput > 0.1f)
    {
        VehicleState = ECore_VehicleState::Braking;
    }
    else
    {
        VehicleState = ECore_VehicleState::Coasting;
    }
}

void UVehiclePhysicsSystemV2::ApplySuspensionForce(int32 WheelIndex, const FVector& ContactPoint, const FVector& Normal, float DeltaTime)
{
    if (!VehicleMesh || WheelIndex < 0 || WheelIndex >= WheelData.Num())
    {
        return;
    }
    
    FCore_WheelData& Wheel = WheelData[WheelIndex];
    
    // Calculate suspension compression
    float Compression = (MaxSuspensionTravel - Wheel.SuspensionLength) / MaxSuspensionTravel;
    Compression = FMath::Clamp(Compression, 0.0f, 1.0f);
    
    // Calculate spring force
    float SpringForce = Compression * SuspensionStiffness;
    
    // Calculate damping force
    float PreviousLength = Wheel.PreviousSuspensionLength;
    float CompressionVelocity = (PreviousLength - Wheel.SuspensionLength) / DeltaTime;
    float DampingForce = CompressionVelocity * SuspensionDamping;
    
    // Total suspension force
    float TotalForce = SpringForce + DampingForce;
    FVector SuspensionForceVector = Normal * TotalForce;
    
    // Apply force at wheel contact point
    VehicleMesh->AddForceAtLocation(SuspensionForceVector, ContactPoint, NAME_None, true);
    
    // Store previous length for next frame
    Wheel.PreviousSuspensionLength = Wheel.SuspensionLength;
}

void UVehiclePhysicsSystemV2::ApplyAntiRollForce(int32 LeftWheelIndex, int32 RightWheelIndex, float RollForce)
{
    if (!VehicleMesh || LeftWheelIndex >= WheelData.Num() || RightWheelIndex >= WheelData.Num())
    {
        return;
    }
    
    FVector LeftWheelPos = VehicleMesh->GetComponentLocation() + VehicleMesh->GetComponentRotation().RotateVector(WheelPositions[LeftWheelIndex]);
    FVector RightWheelPos = VehicleMesh->GetComponentLocation() + VehicleMesh->GetComponentRotation().RotateVector(WheelPositions[RightWheelIndex]);
    
    FVector UpVector = VehicleMesh->GetUpVector();
    
    // Apply opposing forces to reduce roll
    VehicleMesh->AddForceAtLocation(UpVector * RollForce, LeftWheelPos, NAME_None, true);
    VehicleMesh->AddForceAtLocation(-UpVector * RollForce, RightWheelPos, NAME_None, true);
}

void UVehiclePhysicsSystemV2::DrawDebugInfo()
{
    if (!VehicleMesh || !GetWorld())
    {
        return;
    }
    
    FVector VehicleLocation = VehicleMesh->GetComponentLocation();
    FRotator VehicleRotation = VehicleMesh->GetComponentRotation();
    
    // Draw wheel positions and suspension
    for (int32 i = 0; i < WheelPositions.Num() && i < WheelData.Num(); i++)
    {
        FVector WorldWheelPos = VehicleLocation + VehicleRotation.RotateVector(WheelPositions[i]);
        
        // Draw wheel
        FColor WheelColor = WheelData[i].bIsGrounded ? FColor::Green : FColor::Red;
        DrawDebugSphere(GetWorld(), WorldWheelPos, WheelRadius, 8, WheelColor, false, 0.0f, 0, 2.0f);
        
        // Draw suspension
        FVector SuspensionStart = WorldWheelPos + FVector(0, 0, MaxSuspensionTravel * 0.5f);
        FVector SuspensionEnd = WorldWheelPos - FVector(0, 0, WheelData[i].SuspensionLength);
        DrawDebugLine(GetWorld(), SuspensionStart, SuspensionEnd, FColor::Yellow, false, 0.0f, 0, 3.0f);
    }
    
    // Draw velocity vector
    FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
    DrawDebugLine(GetWorld(), VehicleLocation, VehicleLocation + Velocity * 0.1f, FColor::Blue, false, 0.0f, 0, 5.0f);
}

void UVehiclePhysicsSystemV2::SetThrottleInput(float InThrottle)
{
    ThrottleInput = FMath::Clamp(InThrottle, -1.0f, 1.0f);
}

void UVehiclePhysicsSystemV2::SetSteerInput(float InSteer)
{
    SteerInput = FMath::Clamp(InSteer, -1.0f, 1.0f);
    
    // Update front wheel steering
    if (WheelData.Num() >= 2)
    {
        float SteerAngle = SteerInput * MaxSteerAngle;
        WheelData[0].SteerAngle = SteerAngle; // Front Left
        WheelData[1].SteerAngle = SteerAngle; // Front Right
    }
}

void UVehiclePhysicsSystemV2::SetBrakeInput(float InBrake)
{
    BrakeInput = FMath::Clamp(InBrake, 0.0f, 1.0f);
}

void UVehiclePhysicsSystemV2::SetHandbrakeInput(float InHandbrake)
{
    HandbrakeInput = FMath::Clamp(InHandbrake, 0.0f, 1.0f);
}

float UVehiclePhysicsSystemV2::GetCurrentSpeed() const
{
    return CurrentSpeed;
}

ECore_VehicleState UVehiclePhysicsSystemV2::GetVehicleState() const
{
    return VehicleState;
}

bool UVehiclePhysicsSystemV2::IsGrounded() const
{
    for (const FCore_WheelData& Wheel : WheelData)
    {
        if (Wheel.bIsGrounded)
        {
            return true;
        }
    }
    return false;
}

int32 UVehiclePhysicsSystemV2::GetGroundedWheelCount() const
{
    int32 Count = 0;
    for (const FCore_WheelData& Wheel : WheelData)
    {
        if (Wheel.bIsGrounded)
        {
            Count++;
        }
    }
    return Count;
}

void UVehiclePhysicsSystemV2::ResetVehicle()
{
    if (!VehicleMesh)
    {
        return;
    }
    
    // Reset physics
    VehicleMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    VehicleMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    
    // Reset inputs
    ThrottleInput = 0.0f;
    SteerInput = 0.0f;
    BrakeInput = 0.0f;
    HandbrakeInput = 0.0f;
    
    // Reset wheel data
    for (FCore_WheelData& Wheel : WheelData)
    {
        Wheel.WheelRotation = 0.0f;
        Wheel.SteerAngle = 0.0f;
        Wheel.SlipRatio = 0.0f;
        Wheel.SlipAngle = 0.0f;
    }
    
    VehicleState = ECore_VehicleState::Stationary;
    CurrentSpeed = 0.0f;
    
    UE_LOG(LogVehiclePhysicsV2, Log, TEXT("Vehicle physics reset"));
}

void UVehiclePhysicsSystemV2::SetDebugDrawEnabled(bool bEnabled)
{
    bDebugDraw = bEnabled;
}