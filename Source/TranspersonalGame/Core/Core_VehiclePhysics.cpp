#include "Core_VehiclePhysics.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"

UCore_VehiclePhysics::UCore_VehiclePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default stats
    VehicleStats = FCore_VehicleStats();
    CurrentDurability = VehicleStats.Durability;
    CurrentState = ECore_VehicleState::Idle;
}

void UCore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Find vehicle body component if not set
    if (!VehicleBody)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            VehicleBody = Owner->FindComponentByClass<UPrimitiveComponent>();
            if (VehicleBody)
            {
                UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysics: Auto-detected vehicle body component"));
            }
        }
    }
    
    // Initialize vehicle based on type
    InitializeVehicle(VehicleType);
    
    LastPosition = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UCore_VehiclePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!VehicleBody || !GetOwner())
    {
        return;
    }
    
    // Update vehicle physics
    CheckTerrainType();
    ApplyForces(DeltaTime);
    UpdateVehicleState();
    HandleTerrainInteraction();
    
    if (bIsInWater)
    {
        HandleWaterPhysics();
    }
    
    UpdateDurability(DeltaTime);
    
    // Update velocity tracking
    FVector CurrentPosition = GetOwner()->GetActorLocation();
    if (DeltaTime > 0.0f)
    {
        Velocity = (CurrentPosition - LastPosition) / DeltaTime;
        CurrentSpeed = Velocity.Size();
    }
    LastPosition = CurrentPosition;
}

void UCore_VehiclePhysics::SetThrottleInput(float InThrottle)
{
    ThrottleInput = FMath::Clamp(InThrottle, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::SetSteeringInput(float InSteering)
{
    SteeringInput = FMath::Clamp(InSteering, -1.0f, 1.0f);
}

void UCore_VehiclePhysics::SetBrakeInput(bool bInBrake)
{
    bBrakeInput = bInBrake;
}

void UCore_VehiclePhysics::ApplyForces(float DeltaTime)
{
    if (!VehicleBody || DeltaTime <= 0.0f)
    {
        return;
    }
    
    CalculateMovementForces(DeltaTime);
    ApplyFriction(DeltaTime);
    ApplyAirResistance(DeltaTime);
    
    if (bIsInWater)
    {
        ApplyWaterResistance(DeltaTime);
    }
    
    UpdateWheelPhysics(DeltaTime);
}

void UCore_VehiclePhysics::CalculateMovementForces(float DeltaTime)
{
    if (!VehicleBody)
    {
        return;
    }
    
    // Calculate forward force from throttle input
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    float EffectiveAcceleration = VehicleStats.Acceleration;
    
    // Adjust for terrain and load
    if (bIsOnRoughTerrain)
    {
        EffectiveAcceleration *= (1.0f - TerrainRoughness * 0.5f);
    }
    
    float LoadFactor = 1.0f - (CurrentLoad / VehicleStats.CarryCapacity) * 0.3f;
    EffectiveAcceleration *= FMath::Clamp(LoadFactor, 0.1f, 1.0f);
    
    // Apply throttle force
    FVector ThrottleForce = ForwardVector * ThrottleInput * EffectiveAcceleration * VehicleBody->GetMass();
    VehicleBody->AddForce(ThrottleForce);
    
    // Apply steering torque
    if (FMath::Abs(SteeringInput) > 0.01f && CurrentSpeed > 10.0f)
    {
        float TurnTorque = SteeringInput * VehicleStats.TurnRate * VehicleBody->GetMass();
        FVector TurnAxis = GetOwner()->GetActorUpVector();
        VehicleBody->AddTorqueInRadians(TurnAxis * TurnTorque);
    }
    
    // Apply braking force
    if (bBrakeInput)
    {
        FVector BrakeForce = -Velocity.GetSafeNormal() * VehicleStats.BrakingForce * VehicleBody->GetMass();
        VehicleBody->AddForce(BrakeForce);
    }
}

void UCore_VehiclePhysics::ApplyFriction(float DeltaTime)
{
    if (!VehicleBody)
    {
        return;
    }
    
    float FrictionCoefficient = 0.1f;
    
    // Adjust friction based on terrain
    if (bIsOnRoughTerrain)
    {
        FrictionCoefficient += TerrainRoughness * 0.3f;
    }
    
    // Adjust friction based on vehicle type
    switch (VehicleType)
    {
        case ECore_VehicleType::Raft:
            FrictionCoefficient = bIsInWater ? 0.05f : 0.8f;
            break;
        case ECore_VehicleType::Sled:
            FrictionCoefficient = 0.15f;
            break;
        case ECore_VehicleType::Cart:
            FrictionCoefficient = 0.2f;
            break;
        case ECore_VehicleType::Boat:
            FrictionCoefficient = bIsInWater ? 0.08f : 1.0f;
            break;
        case ECore_VehicleType::RidingMount:
            FrictionCoefficient = 0.05f; // Animals have natural movement
            break;
    }
    
    FVector FrictionForce = -Velocity * FrictionCoefficient * VehicleBody->GetMass();
    VehicleBody->AddForce(FrictionForce);
}

void UCore_VehiclePhysics::ApplyAirResistance(float DeltaTime)
{
    if (!VehicleBody || CurrentSpeed < 50.0f)
    {
        return;
    }
    
    float AirDensity = 1.225f; // kg/m³ at sea level
    float DragCoefficient = 0.8f; // Primitive vehicles have poor aerodynamics
    float FrontalArea = 4.0f; // Estimated frontal area in m²
    
    float DragForce = 0.5f * AirDensity * DragCoefficient * FrontalArea * CurrentSpeed * CurrentSpeed;
    FVector DragVector = -Velocity.GetSafeNormal() * DragForce;
    
    VehicleBody->AddForce(DragVector);
}

void UCore_VehiclePhysics::ApplyWaterResistance(float DeltaTime)
{
    if (!VehicleBody || !bIsInWater)
    {
        return;
    }
    
    float WaterDensity = 1000.0f; // kg/m³
    float DragCoefficient = 1.2f; // Higher drag in water
    float SubmergedArea = FMath::Min(WaterDepth / 100.0f, 1.0f) * 6.0f; // Estimated submerged area
    
    float WaterDrag = 0.5f * WaterDensity * DragCoefficient * SubmergedArea * CurrentSpeed * CurrentSpeed * 0.01f;
    FVector DragVector = -Velocity.GetSafeNormal() * WaterDrag;
    
    VehicleBody->AddForce(DragVector);
    
    // Add buoyancy for water vehicles
    if (VehicleType == ECore_VehicleType::Raft || VehicleType == ECore_VehicleType::Boat)
    {
        float BuoyancyForce = WaterDensity * 9.81f * SubmergedArea * 100.0f; // Convert to UE units
        FVector BuoyancyVector = FVector(0, 0, BuoyancyForce);
        VehicleBody->AddForce(BuoyancyVector);
    }
}

void UCore_VehiclePhysics::UpdateWheelPhysics(float DeltaTime)
{
    // Update wheel components if they exist (for carts and sleds)
    for (UPrimitiveComponent* Wheel : WheelComponents)
    {
        if (Wheel && CurrentSpeed > 10.0f)
        {
            // Rotate wheels based on vehicle speed
            float RotationSpeed = CurrentSpeed / 50.0f; // Adjust wheel rotation rate
            FRotator WheelRotation = FRotator(RotationSpeed * DeltaTime * 180.0f, 0, 0);
            Wheel->AddRelativeRotation(WheelRotation);
        }
    }
}

void UCore_VehiclePhysics::UpdateVehicleState()
{
    ECore_VehicleState NewState = ECore_VehicleState::Idle;
    
    if (CurrentSpeed > 50.0f)
    {
        if (FMath::Abs(SteeringInput) > 0.3f)
        {
            NewState = ECore_VehicleState::Turning;
        }
        else
        {
            NewState = ECore_VehicleState::Moving;
        }
    }
    else if (bBrakeInput)
    {
        NewState = ECore_VehicleState::Braking;
    }
    else if (CurrentSpeed < 10.0f && FMath::Abs(ThrottleInput) > 0.1f)
    {
        NewState = ECore_VehicleState::Stuck;
    }
    
    if (CurrentDurability < 20.0f)
    {
        NewState = ECore_VehicleState::Damaged;
    }
    
    CurrentState = NewState;
}

void UCore_VehiclePhysics::HandleTerrainInteraction()
{
    // Apply terrain-based damage and performance changes
    if (bIsOnRoughTerrain && CurrentSpeed > 200.0f)
    {
        float DamageRate = TerrainRoughness * CurrentSpeed * 0.001f;
        ApplyDamage(DamageRate * GetWorld()->GetDeltaSeconds());
    }
}

void UCore_VehiclePhysics::HandleWaterPhysics()
{
    // Special handling for water-based vehicles
    if (VehicleType == ECore_VehicleType::Raft || VehicleType == ECore_VehicleType::Boat)
    {
        // Water vehicles are more stable in water
        if (VehicleBody)
        {
            FVector AngularVelocity = VehicleBody->GetPhysicsAngularVelocityInRadians();
            FVector StabilizingTorque = -AngularVelocity * VehicleBody->GetMass() * 0.5f;
            VehicleBody->AddTorqueInRadians(StabilizingTorque);
        }
    }
    else
    {
        // Land vehicles struggle in water
        float WaterPenalty = FMath::Min(WaterDepth / 50.0f, 1.0f);
        ApplyDamage(WaterPenalty * 2.0f * GetWorld()->GetDeltaSeconds());
    }
}

void UCore_VehiclePhysics::CheckTerrainType()
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    UWorld* World = GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Check for water
        if (HitResult.GetActor() && HitResult.GetActor()->GetName().Contains(TEXT("Water")))
        {
            bIsInWater = true;
            WaterDepth = FMath::Abs(HitResult.Location.Z - StartLocation.Z);
        }
        else
        {
            bIsInWater = false;
            WaterDepth = 0.0f;
        }
        
        // Determine terrain roughness based on surface normal
        FVector SurfaceNormal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector));
        TerrainRoughness = FMath::Clamp(SlopeAngle / (PI * 0.25f), 0.0f, 1.0f);
        bIsOnRoughTerrain = TerrainRoughness > 0.3f;
    }
}

void UCore_VehiclePhysics::UpdateDurability(float DeltaTime)
{
    // Natural wear and tear
    float WearRate = 0.1f * DeltaTime;
    
    // Increased wear based on usage
    if (CurrentSpeed > 100.0f)
    {
        WearRate += (CurrentSpeed / 1000.0f) * DeltaTime;
    }
    
    // Increased wear on rough terrain
    if (bIsOnRoughTerrain)
    {
        WearRate += TerrainRoughness * DeltaTime;
    }
    
    CurrentDurability = FMath::Clamp(CurrentDurability - WearRate, 0.0f, VehicleStats.Durability);
}

void UCore_VehiclePhysics::ApplyDamage(float DamageAmount)
{
    CurrentDurability = FMath::Clamp(CurrentDurability - DamageAmount, 0.0f, VehicleStats.Durability);
    
    if (CurrentDurability <= 0.0f)
    {
        CurrentState = ECore_VehicleState::Damaged;
        // Disable vehicle movement
        ThrottleInput = 0.0f;
        SteeringInput = 0.0f;
    }
}

void UCore_VehiclePhysics::RepairVehicle(float RepairAmount)
{
    CurrentDurability = FMath::Clamp(CurrentDurability + RepairAmount, 0.0f, VehicleStats.Durability);
}

bool UCore_VehiclePhysics::CanCarryLoad(float AdditionalWeight) const
{
    return (CurrentLoad + AdditionalWeight) <= VehicleStats.CarryCapacity;
}

void UCore_VehiclePhysics::AddLoad(float Weight)
{
    if (CanCarryLoad(Weight))
    {
        CurrentLoad += Weight;
    }
}

void UCore_VehiclePhysics::RemoveLoad(float Weight)
{
    CurrentLoad = FMath::Max(0.0f, CurrentLoad - Weight);
}

float UCore_VehiclePhysics::GetSpeedKmh() const
{
    return CurrentSpeed * 0.036f; // Convert cm/s to km/h
}

float UCore_VehiclePhysics::GetDurabilityPercentage() const
{
    return (CurrentDurability / VehicleStats.Durability) * 100.0f;
}

float UCore_VehiclePhysics::GetLoadPercentage() const
{
    return (CurrentLoad / VehicleStats.CarryCapacity) * 100.0f;
}

bool UCore_VehiclePhysics::IsInWater() const
{
    return bIsInWater;
}

bool UCore_VehiclePhysics::IsOnRoughTerrain() const
{
    return bIsOnRoughTerrain;
}

void UCore_VehiclePhysics::InitializeVehicle(ECore_VehicleType InVehicleType)
{
    VehicleType = InVehicleType;
    
    // Configure vehicle stats based on type
    switch (VehicleType)
    {
        case ECore_VehicleType::Raft:
            VehicleStats.MaxSpeed = 300.0f;
            VehicleStats.Acceleration = 200.0f;
            VehicleStats.TurnRate = 45.0f;
            VehicleStats.WaterResistance = 0.2f;
            VehicleStats.CarryCapacity = 800.0f;
            break;
            
        case ECore_VehicleType::Sled:
            VehicleStats.MaxSpeed = 600.0f;
            VehicleStats.Acceleration = 400.0f;
            VehicleStats.TurnRate = 60.0f;
            VehicleStats.TerrainAdaptation = 0.8f;
            VehicleStats.CarryCapacity = 400.0f;
            break;
            
        case ECore_VehicleType::Cart:
            VehicleStats.MaxSpeed = 500.0f;
            VehicleStats.Acceleration = 300.0f;
            VehicleStats.TurnRate = 75.0f;
            VehicleStats.CarryCapacity = 1000.0f;
            break;
            
        case ECore_VehicleType::Boat:
            VehicleStats.MaxSpeed = 400.0f;
            VehicleStats.Acceleration = 250.0f;
            VehicleStats.TurnRate = 30.0f;
            VehicleStats.WaterResistance = 0.1f;
            VehicleStats.CarryCapacity = 1200.0f;
            break;
            
        case ECore_VehicleType::RidingMount:
            VehicleStats.MaxSpeed = 1200.0f;
            VehicleStats.Acceleration = 800.0f;
            VehicleStats.TurnRate = 120.0f;
            VehicleStats.TerrainAdaptation = 1.5f;
            VehicleStats.CarryCapacity = 200.0f;
            break;
    }
    
    CurrentDurability = VehicleStats.Durability;
    CurrentLoad = 0.0f;
}