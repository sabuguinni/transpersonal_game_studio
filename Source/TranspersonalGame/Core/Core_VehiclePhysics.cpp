#include "Core_VehiclePhysics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"

ACore_VehiclePhysics::ACore_VehiclePhysics()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create vehicle body mesh component
    VehicleBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleBodyMesh"));
    RootComponent = VehicleBodyMesh;

    // Configure physics
    VehicleBodyMesh->SetSimulatePhysics(true);
    VehicleBodyMesh->SetMassOverrideInKg(NAME_None, VehicleMass);
    VehicleBodyMesh->SetCenterOfMass(CenterOfMassOffset);
    VehicleBodyMesh->SetLinearDamping(0.1f);
    VehicleBodyMesh->SetAngularDamping(0.3f);

    // Initialize wheel configurations (default 4-wheel setup)
    WheelConfigurations.SetNum(4);
    for (int32 i = 0; i < 4; i++)
    {
        WheelConfigurations[i] = FCore_WheelConfiguration();
    }

    // Initialize arrays
    WheelContactPoints.SetNum(4);
    WheelContactNormals.SetNum(4);
    WheelCompressionRatios.SetNum(4);

    // Reset input states
    CurrentThrottle = 0.0f;
    CurrentBrake = 0.0f;
    CurrentSteering = 0.0f;
}

void ACore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial vehicle mass
    if (VehicleBodyMesh)
    {
        VehicleBodyMesh->SetMassOverrideInKg(NAME_None, VehicleMass);
        VehicleBodyMesh->SetCenterOfMass(CenterOfMassOffset);
    }

    UE_LOG(LogTemp, Warning, TEXT("Core Vehicle Physics initialized with mass: %f kg"), VehicleMass);
}

void ACore_VehiclePhysics::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!VehicleBodyMesh || !VehicleBodyMesh->IsSimulatingPhysics())
    {
        return;
    }

    // Calculate wheel contacts with ground
    CalculateWheelContacts();

    // Calculate and apply suspension forces
    CalculateSuspensionForces();

    // Apply engine force based on throttle input
    ApplyEngineForce(DeltaTime);

    // Apply aerodynamic drag
    ApplyAerodynamicDrag(DeltaTime);

    // Apply wheel forces (traction, braking)
    ApplyWheelForces(DeltaTime);

    // Update vehicle statistics
    UpdateVehicleStats(DeltaTime);
}

void ACore_VehiclePhysics::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind input actions
    PlayerInputComponent->BindAction("Throttle", IE_Pressed, this, &ACore_VehiclePhysics::ThrottlePressed);
    PlayerInputComponent->BindAction("Throttle", IE_Released, this, &ACore_VehiclePhysics::ThrottleReleased);
    PlayerInputComponent->BindAction("Brake", IE_Pressed, this, &ACore_VehiclePhysics::BrakePressed);
    PlayerInputComponent->BindAction("Brake", IE_Released, this, &ACore_VehiclePhysics::BrakeReleased);

    // Bind input axes
    PlayerInputComponent->BindAxis("Steering", this, &ACore_VehiclePhysics::ApplySteering);
}

void ACore_VehiclePhysics::ApplyThrottle(float ThrottleValue)
{
    CurrentThrottle = FMath::Clamp(ThrottleValue, 0.0f, 1.0f);
}

void ACore_VehiclePhysics::ApplyBrake(float BrakeValue)
{
    CurrentBrake = FMath::Clamp(BrakeValue, 0.0f, 1.0f);
}

void ACore_VehiclePhysics::ApplySteering(float SteeringValue)
{
    CurrentSteering = FMath::Clamp(SteeringValue, -1.0f, 1.0f);
}

bool ACore_VehiclePhysics::IsOnGround() const
{
    // Check if any wheel is in contact with ground
    for (float CompressionRatio : WheelCompressionRatios)
    {
        if (CompressionRatio > 0.0f)
        {
            return true;
        }
    }
    return false;
}

float ACore_VehiclePhysics::GetTerrainTraction() const
{
    if (!IsOnGround())
    {
        return 0.0f;
    }

    // Perform line trace to determine surface material
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Determine traction based on surface material
        if (HitResult.GetComponent())
        {
            FString SurfaceName = HitResult.GetComponent()->GetName();
            
            // Basic surface traction mapping
            if (SurfaceName.Contains("Grass") || SurfaceName.Contains("Dirt"))
            {
                return 0.7f;
            }
            else if (SurfaceName.Contains("Rock") || SurfaceName.Contains("Stone"))
            {
                return 0.9f;
            }
            else if (SurfaceName.Contains("Mud") || SurfaceName.Contains("Swamp"))
            {
                return 0.3f;
            }
            else if (SurfaceName.Contains("Ice") || SurfaceName.Contains("Snow"))
            {
                return 0.2f;
            }
        }
    }

    // Default traction for unknown surfaces
    return 0.6f;
}

void ACore_VehiclePhysics::CalculateSuspensionForces()
{
    if (!VehicleBodyMesh)
    {
        return;
    }

    for (int32 WheelIndex = 0; WheelIndex < WheelConfigurations.Num(); WheelIndex++)
    {
        if (WheelIndex >= WheelCompressionRatios.Num())
        {
            continue;
        }

        float CompressionRatio = WheelCompressionRatios[WheelIndex];
        if (CompressionRatio <= 0.0f)
        {
            continue;
        }

        const FCore_WheelConfiguration& WheelConfig = WheelConfigurations[WheelIndex];
        
        // Calculate spring force
        float SpringForce = WheelConfig.SpringStiffness * CompressionRatio * WheelConfig.SuspensionTravel;
        
        // Calculate damping force
        FVector WheelVelocity = VehicleBodyMesh->GetPhysicsLinearVelocityAtPoint(WheelContactPoints[WheelIndex]);
        float DampingForce = WheelConfig.DampingCoefficient * FVector::DotProduct(WheelVelocity, WheelContactNormals[WheelIndex]);
        
        // Total suspension force
        float TotalForce = SpringForce - DampingForce;
        FVector SuspensionForce = WheelContactNormals[WheelIndex] * TotalForce;
        
        // Apply force at wheel contact point
        VehicleBodyMesh->AddForceAtLocation(SuspensionForce, WheelContactPoints[WheelIndex]);
    }
}

void ACore_VehiclePhysics::ApplyEngineForce(float DeltaTime)
{
    if (!VehicleBodyMesh || CurrentThrottle <= 0.0f)
    {
        return;
    }

    // Calculate engine output based on current RPM
    float EngineOutput = CalculateEngineOutput(VehicleStats.EngineRPM);
    
    // Apply throttle modifier
    float TotalForce = EngineOutput * CurrentThrottle * MaxEnginePower * 100.0f; // Convert to Newtons
    
    // Apply force in forward direction
    FVector ForwardDirection = GetActorForwardVector();
    FVector EngineForce = ForwardDirection * TotalForce;
    
    VehicleBodyMesh->AddForce(EngineForce);
}

void ACore_VehiclePhysics::ApplyAerodynamicDrag(float DeltaTime)
{
    if (!VehicleBodyMesh)
    {
        return;
    }

    FVector Velocity = VehicleBodyMesh->GetPhysicsLinearVelocity();
    float Speed = Velocity.Size();
    
    if (Speed > 0.1f)
    {
        // Calculate drag force: F = 0.5 * ρ * Cd * A * v²
        float AirDensity = 1.225f; // kg/m³ at sea level
        float DragForce = 0.5f * AirDensity * DragCoefficient * FrontalArea * Speed * Speed;
        
        // Apply drag in opposite direction of velocity
        FVector DragDirection = -Velocity.GetSafeNormal();
        FVector TotalDrag = DragDirection * DragForce;
        
        VehicleBodyMesh->AddForce(TotalDrag);
    }
}

void ACore_VehiclePhysics::UpdateVehicleStats(float DeltaTime)
{
    if (!VehicleBodyMesh)
    {
        return;
    }

    // Update speed (convert cm/s to km/h)
    FVector Velocity = VehicleBodyMesh->GetPhysicsLinearVelocity();
    VehicleStats.CurrentSpeed = Velocity.Size() * 0.036f;
    
    // Update engine RPM based on speed and throttle
    float TargetRPM = 800.0f + (VehicleStats.CurrentSpeed * 50.0f) + (CurrentThrottle * 2000.0f);
    VehicleStats.EngineRPM = FMath::FInterpTo(VehicleStats.EngineRPM, TargetRPM, DeltaTime, 5.0f);
    
    // Consume fuel based on throttle input
    if (CurrentThrottle > 0.0f)
    {
        float FuelConsumption = CurrentThrottle * 2.0f * DeltaTime; // 2% per second at full throttle
        VehicleStats.FuelLevel = FMath::Max(0.0f, VehicleStats.FuelLevel - FuelConsumption);
    }
    
    // Update gear based on speed (simple automatic transmission)
    if (VehicleStats.CurrentSpeed < 20.0f)
    {
        VehicleStats.CurrentGear = 1;
    }
    else if (VehicleStats.CurrentSpeed < 40.0f)
    {
        VehicleStats.CurrentGear = 2;
    }
    else if (VehicleStats.CurrentSpeed < 60.0f)
    {
        VehicleStats.CurrentGear = 3;
    }
    else
    {
        VehicleStats.CurrentGear = 4;
    }
}

void ACore_VehiclePhysics::CalculateWheelContacts()
{
    if (!VehicleBodyMesh)
    {
        return;
    }

    // Define wheel positions relative to vehicle center
    TArray<FVector> WheelOffsets = {
        FVector(120.0f, -80.0f, -50.0f),   // Front Left
        FVector(120.0f, 80.0f, -50.0f),    // Front Right
        FVector(-120.0f, -80.0f, -50.0f),  // Rear Left
        FVector(-120.0f, 80.0f, -50.0f)    // Rear Right
    };

    for (int32 WheelIndex = 0; WheelIndex < WheelOffsets.Num() && WheelIndex < WheelConfigurations.Num(); WheelIndex++)
    {
        FVector WheelWorldPosition = GetActorLocation() + GetActorRotation().RotateVector(WheelOffsets[WheelIndex]);
        FVector TraceStart = WheelWorldPosition + FVector(0, 0, WheelConfigurations[WheelIndex].WheelRadius);
        FVector TraceEnd = WheelWorldPosition - FVector(0, 0, WheelConfigurations[WheelIndex].SuspensionTravel + WheelConfigurations[WheelIndex].WheelRadius);

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECollisionChannel::ECC_WorldStatic,
            QueryParams
        );

        if (bHit)
        {
            WheelContactPoints[WheelIndex] = HitResult.ImpactPoint;
            WheelContactNormals[WheelIndex] = HitResult.ImpactNormal;
            
            float ContactDistance = FVector::Dist(TraceStart, HitResult.ImpactPoint);
            float MaxDistance = WheelConfigurations[WheelIndex].SuspensionTravel + WheelConfigurations[WheelIndex].WheelRadius;
            WheelCompressionRatios[WheelIndex] = FMath::Clamp((MaxDistance - ContactDistance) / WheelConfigurations[WheelIndex].SuspensionTravel, 0.0f, 1.0f);
        }
        else
        {
            WheelContactPoints[WheelIndex] = TraceEnd;
            WheelContactNormals[WheelIndex] = FVector::UpVector;
            WheelCompressionRatios[WheelIndex] = 0.0f;
        }
    }
}

void ACore_VehiclePhysics::ApplyWheelForces(float DeltaTime)
{
    if (!VehicleBodyMesh)
    {
        return;
    }

    float TerrainTraction = GetTerrainTraction();
    
    for (int32 WheelIndex = 0; WheelIndex < WheelConfigurations.Num(); WheelIndex++)
    {
        if (WheelCompressionRatios[WheelIndex] <= 0.0f)
        {
            continue;
        }

        const FCore_WheelConfiguration& WheelConfig = WheelConfigurations[WheelIndex];
        FVector WheelPosition = WheelContactPoints[WheelIndex];
        
        // Calculate wheel velocity
        FVector WheelVelocity = VehicleBodyMesh->GetPhysicsLinearVelocityAtPoint(WheelPosition);
        
        // Apply braking force
        if (CurrentBrake > 0.0f)
        {
            FVector BrakeForce = -WheelVelocity.GetSafeNormal() * CurrentBrake * 5000.0f * TerrainTraction;
            VehicleBodyMesh->AddForceAtLocation(BrakeForce, WheelPosition);
        }
        
        // Apply steering force (front wheels only)
        if (WheelIndex < 2 && FMath::Abs(CurrentSteering) > 0.01f)
        {
            FVector RightVector = GetActorRightVector();
            FVector SteeringForce = RightVector * CurrentSteering * 2000.0f * TerrainTraction;
            VehicleBodyMesh->AddForceAtLocation(SteeringForce, WheelPosition);
        }
        
        // Apply lateral friction
        FVector LateralVelocity = FVector::VectorPlaneProject(WheelVelocity, WheelContactNormals[WheelIndex]);
        FVector RightVector = GetActorRightVector();
        float LateralSpeed = FVector::DotProduct(LateralVelocity, RightVector);
        
        if (FMath::Abs(LateralSpeed) > 0.1f)
        {
            FVector LateralFriction = -RightVector * LateralSpeed * WheelConfig.FrictionCoefficient * 3000.0f * TerrainTraction;
            VehicleBodyMesh->AddForceAtLocation(LateralFriction, WheelPosition);
        }
    }
}

float ACore_VehiclePhysics::CalculateEngineOutput(float RPM) const
{
    // Simple torque curve simulation
    float OptimalRPM = 3000.0f;
    float MaxRPM = 6000.0f;
    
    if (RPM <= OptimalRPM)
    {
        return FMath::Lerp(0.5f, 1.0f, RPM / OptimalRPM);
    }
    else
    {
        return FMath::Lerp(1.0f, 0.3f, (RPM - OptimalRPM) / (MaxRPM - OptimalRPM));
    }
}

void ACore_VehiclePhysics::ThrottlePressed()
{
    ApplyThrottle(1.0f);
}

void ACore_VehiclePhysics::ThrottleReleased()
{
    ApplyThrottle(0.0f);
}

void ACore_VehiclePhysics::BrakePressed()
{
    ApplyBrake(1.0f);
}

void ACore_VehiclePhysics::BrakeReleased()
{
    ApplyBrake(0.0f);
}

void ACore_VehiclePhysics::SteerLeft()
{
    ApplySteering(-1.0f);
}

void ACore_VehiclePhysics::SteerRight()
{
    ApplySteering(1.0f);
}

void ACore_VehiclePhysics::SteerStop()
{
    ApplySteering(0.0f);
}