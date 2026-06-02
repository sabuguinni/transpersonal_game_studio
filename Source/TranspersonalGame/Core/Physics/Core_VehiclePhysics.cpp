#include "Core_VehiclePhysics.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

DEFINE_LOG_CATEGORY_STATIC(LogVehiclePhysics, Log, All);

UCore_VehiclePhysics::UCore_VehiclePhysics()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	// Initialize vehicle physics parameters
	MaxSpeed = 2000.0f; // 20 m/s max speed for primitive vehicles
	Acceleration = 800.0f;
	Deceleration = 1200.0f;
	TurnRate = 90.0f;
	
	// Stability settings
	StabilityForce = 500.0f;
	AntiRollForce = 300.0f;
	DownForce = 200.0f;
	
	// Terrain interaction
	TerrainFriction = 1.0f;
	WaterResistance = 0.8f;
	SlopeLimit = 45.0f;
	
	// Physics state
	CurrentSpeed = 0.0f;
	bIsOnGround = false;
	bIsInWater = false;
	CurrentTerrainType = ECore_TerrainType::Grass;
	
	// Initialize arrays
	WheelComponents.Empty();
	SuspensionPoints.Empty();
}

void UCore_VehiclePhysics::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache owner and mesh component
	OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		MeshComponent = OwnerPawn->FindComponentByClass<UStaticMeshComponent>();
		if (MeshComponent)
		{
			// Enable physics simulation
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			
			// Set mass for primitive vehicle (wooden cart/sled)
			MeshComponent->SetMassOverrideInKg(NAME_None, 200.0f, true);
		}
	}
	
	// Initialize suspension system
	InitializeSuspension();
	
	UE_LOG(LogVehiclePhysics, Log, TEXT("Vehicle Physics initialized for %s"), 
		OwnerPawn ? *OwnerPawn->GetName() : TEXT("Unknown"));
}

void UCore_VehiclePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!MeshComponent || !OwnerPawn)
		return;
	
	// Update physics state
	UpdatePhysicsState(DeltaTime);
	
	// Apply vehicle forces
	ApplyVehicleForces(DeltaTime);
	
	// Handle terrain interaction
	HandleTerrainInteraction(DeltaTime);
	
	// Update suspension
	UpdateSuspension(DeltaTime);
}

void UCore_VehiclePhysics::ApplyThrottle(float ThrottleInput)
{
	if (!MeshComponent)
		return;
	
	// Clamp input
	ThrottleInput = FMath::Clamp(ThrottleInput, -1.0f, 1.0f);
	
	// Calculate force based on current speed and terrain
	float SpeedRatio = FMath::Abs(CurrentSpeed) / MaxSpeed;
	float TerrainModifier = GetTerrainSpeedModifier();
	
	// Reduce force at higher speeds
	float ForceMultiplier = FMath::Lerp(1.0f, 0.3f, SpeedRatio);
	
	// Apply acceleration or deceleration
	float Force = ThrottleInput > 0.0f ? Acceleration : Deceleration;
	Force *= ForceMultiplier * TerrainModifier;
	
	// Get forward vector
	FVector ForwardVector = OwnerPawn->GetActorForwardVector();
	FVector ForceVector = ForwardVector * Force * ThrottleInput;
	
	// Apply force to mesh
	MeshComponent->AddForce(ForceVector, NAME_None, false);
	
	UE_LOG(LogVehiclePhysics, VeryVerbose, TEXT("Applied throttle: %f, Force: %f"), 
		ThrottleInput, Force);
}

void UCore_VehiclePhysics::ApplySteering(float SteeringInput)
{
	if (!MeshComponent || !bIsOnGround)
		return;
	
	// Clamp input
	SteeringInput = FMath::Clamp(SteeringInput, -1.0f, 1.0f);
	
	// Calculate turn force based on speed
	float SpeedRatio = FMath::Abs(CurrentSpeed) / MaxSpeed;
	float TurnForce = TurnRate * (1.0f - SpeedRatio * 0.5f); // Reduce turning at high speed
	
	// Apply torque for turning
	FVector TorqueVector = FVector(0.0f, 0.0f, TurnForce * SteeringInput);
	MeshComponent->AddTorqueInDegrees(TorqueVector, NAME_None, false);
}

void UCore_VehiclePhysics::ApplyBraking(float BrakeInput)
{
	if (!MeshComponent)
		return;
	
	BrakeInput = FMath::Clamp(BrakeInput, 0.0f, 1.0f);
	
	// Apply braking force opposite to velocity
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	if (!Velocity.IsNearlyZero())
	{
		FVector BrakeForce = -Velocity.GetSafeNormal() * Deceleration * BrakeInput * 2.0f;
		MeshComponent->AddForce(BrakeForce, NAME_None, false);
	}
	
	// Apply angular braking
	FVector AngularVelocity = MeshComponent->GetPhysicsAngularVelocityInDegrees();
	if (!AngularVelocity.IsNearlyZero())
	{
		FVector AngularBrake = -AngularVelocity * BrakeInput * 0.5f;
		MeshComponent->AddTorqueInDegrees(AngularBrake, NAME_None, false);
	}
}

void UCore_VehiclePhysics::UpdatePhysicsState(float DeltaTime)
{
	if (!MeshComponent)
		return;
	
	// Update current speed
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	CurrentSpeed = Velocity.Size();
	
	// Check ground contact
	UpdateGroundContact();
	
	// Check water contact
	UpdateWaterContact();
	
	// Update terrain type
	UpdateTerrainType();
}

void UCore_VehiclePhysics::ApplyVehicleForces(float DeltaTime)
{
	if (!MeshComponent)
		return;
	
	// Apply downforce for stability
	if (bIsOnGround && CurrentSpeed > 100.0f)
	{
		FVector DownForceVector = FVector(0.0f, 0.0f, -DownForce * (CurrentSpeed / MaxSpeed));
		MeshComponent->AddForce(DownForceVector, NAME_None, false);
	}
	
	// Apply stability force to prevent flipping
	ApplyStabilityForce();
	
	// Apply anti-roll force
	ApplyAntiRollForce();
}

void UCore_VehiclePhysics::HandleTerrainInteraction(float DeltaTime)
{
	if (!MeshComponent)
		return;
	
	// Apply terrain-specific forces
	switch (CurrentTerrainType)
	{
		case ECore_TerrainType::Sand:
			ApplySandResistance();
			break;
		case ECore_TerrainType::Mud:
			ApplyMudResistance();
			break;
		case ECore_TerrainType::Snow:
			ApplySnowResistance();
			break;
		case ECore_TerrainType::Water:
			ApplyWaterResistance();
			break;
		default:
			break;
	}
}

void UCore_VehiclePhysics::InitializeSuspension()
{
	// Create basic suspension points for primitive vehicle
	SuspensionPoints.Empty();
	
	if (MeshComponent)
	{
		FVector Bounds = MeshComponent->GetStaticMesh() ? 
			MeshComponent->GetStaticMesh()->GetBounds().BoxExtent : 
			FVector(100.0f, 100.0f, 50.0f);
		
		// Front suspension points
		SuspensionPoints.Add(FVector(Bounds.X * 0.8f, -Bounds.Y * 0.8f, -Bounds.Z));
		SuspensionPoints.Add(FVector(Bounds.X * 0.8f, Bounds.Y * 0.8f, -Bounds.Z));
		
		// Rear suspension points
		SuspensionPoints.Add(FVector(-Bounds.X * 0.8f, -Bounds.Y * 0.8f, -Bounds.Z));
		SuspensionPoints.Add(FVector(-Bounds.X * 0.8f, Bounds.Y * 0.8f, -Bounds.Z));
	}
}

void UCore_VehiclePhysics::UpdateSuspension(float DeltaTime)
{
	if (!MeshComponent || SuspensionPoints.Num() == 0)
		return;
	
	for (const FVector& SuspensionPoint : SuspensionPoints)
	{
		// Transform suspension point to world space
		FVector WorldPoint = MeshComponent->GetComponentTransform().TransformPosition(SuspensionPoint);
		
		// Perform line trace downward
		FHitResult HitResult;
		FVector StartLocation = WorldPoint;
		FVector EndLocation = StartLocation + FVector(0.0f, 0.0f, -200.0f); // 2m suspension travel
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwnerPawn);
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
			ECC_WorldStatic, QueryParams))
		{
			// Calculate suspension force
			float CompressionDistance = 200.0f - HitResult.Distance;
			if (CompressionDistance > 0.0f)
			{
				float SuspensionForce = CompressionDistance * 100.0f; // Spring constant
				FVector ForceVector = FVector(0.0f, 0.0f, SuspensionForce);
				
				// Apply force at suspension point
				MeshComponent->AddForceAtLocation(ForceVector, WorldPoint);
			}
		}
	}
}

void UCore_VehiclePhysics::UpdateGroundContact()
{
	if (!OwnerPawn)
		return;
	
	// Simple ground check using line trace
	FHitResult HitResult;
	FVector StartLocation = OwnerPawn->GetActorLocation();
	FVector EndLocation = StartLocation + FVector(0.0f, 0.0f, -150.0f);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);
	
	bIsOnGround = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation,
		ECC_WorldStatic, QueryParams);
}

void UCore_VehiclePhysics::UpdateWaterContact()
{
	// Simple water detection - check for water volume or specific material
	bIsInWater = false; // Placeholder - implement based on water system
}

void UCore_VehiclePhysics::UpdateTerrainType()
{
	// Placeholder - implement based on terrain system
	CurrentTerrainType = ECore_TerrainType::Grass;
}

float UCore_VehiclePhysics::GetTerrainSpeedModifier() const
{
	switch (CurrentTerrainType)
	{
		case ECore_TerrainType::Sand:
			return 0.6f;
		case ECore_TerrainType::Mud:
			return 0.4f;
		case ECore_TerrainType::Snow:
			return 0.7f;
		case ECore_TerrainType::Water:
			return 0.3f;
		case ECore_TerrainType::Rock:
			return 0.8f;
		default:
			return 1.0f;
	}
}

void UCore_VehiclePhysics::ApplyStabilityForce()
{
	if (!MeshComponent)
		return;
	
	// Get vehicle orientation
	FVector UpVector = OwnerPawn->GetActorUpVector();
	FVector WorldUp = FVector(0.0f, 0.0f, 1.0f);
	
	// Calculate correction force
	FVector CorrectionForce = FVector::CrossProduct(UpVector, WorldUp) * StabilityForce;
	
	// Apply as torque
	MeshComponent->AddTorqueInRadians(CorrectionForce, NAME_None, false);
}

void UCore_VehiclePhysics::ApplyAntiRollForce()
{
	// Placeholder for anti-roll implementation
	// Would require more complex suspension system
}

void UCore_VehiclePhysics::ApplySandResistance()
{
	if (!MeshComponent)
		return;
	
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	FVector Resistance = -Velocity * 0.3f; // Sand resistance
	MeshComponent->AddForce(Resistance, NAME_None, false);
}

void UCore_VehiclePhysics::ApplyMudResistance()
{
	if (!MeshComponent)
		return;
	
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	FVector Resistance = -Velocity * 0.5f; // Higher mud resistance
	MeshComponent->AddForce(Resistance, NAME_None, false);
}

void UCore_VehiclePhysics::ApplySnowResistance()
{
	if (!MeshComponent)
		return;
	
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	FVector Resistance = -Velocity * 0.2f; // Light snow resistance
	MeshComponent->AddForce(Resistance, NAME_None, false);
}

void UCore_VehiclePhysics::ApplyWaterResistance()
{
	if (!MeshComponent)
		return;
	
	FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();
	FVector Resistance = -Velocity * WaterResistance;
	MeshComponent->AddForce(Resistance, NAME_None, false);
}

bool UCore_VehiclePhysics::IsVehicleStable() const
{
	if (!OwnerPawn)
		return false;
	
	FVector UpVector = OwnerPawn->GetActorUpVector();
	float DotProduct = FVector::DotProduct(UpVector, FVector(0.0f, 0.0f, 1.0f));
	
	return DotProduct > 0.7f; // Vehicle is stable if up vector is mostly pointing up
}

float UCore_VehiclePhysics::GetCurrentSpeed() const
{
	return CurrentSpeed;
}

bool UCore_VehiclePhysics::IsOnGround() const
{
	return bIsOnGround;
}

bool UCore_VehiclePhysics::IsInWater() const
{
	return bIsInWater;
}

ECore_TerrainType UCore_VehiclePhysics::GetCurrentTerrainType() const
{
	return CurrentTerrainType;
}