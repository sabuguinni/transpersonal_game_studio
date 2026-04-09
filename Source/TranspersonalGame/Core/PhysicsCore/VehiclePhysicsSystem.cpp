#include "VehiclePhysicsSystem.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogVehiclePhysics);

UVehiclePhysicsSystem::UVehiclePhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Default vehicle physics settings
    MaxSpeed = 2000.0f; // 20 m/s for prehistoric creatures
    Acceleration = 800.0f;
    Deceleration = 1200.0f;
    TurnRate = 90.0f;
    
    // Terrain adaptation settings
    TerrainAdaptationStrength = 1.0f;
    SlopeLimit = 45.0f;
    bEnableTerrainAdaptation = true;
    
    // Physics material settings
    FrictionCoefficient = 0.7f;
    RollingResistance = 0.02f;
    AirDensity = 1.225f; // kg/m³ at sea level
    
    CurrentSpeed = 0.0f;
    bIsGrounded = false;
    GroundNormal = FVector::UpVector;
}

void UVehiclePhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner and mesh component
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        MeshComponent = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
        if (!MeshComponent)
        {
            MeshComponent = OwnerPawn->FindComponentByClass<UStaticMeshComponent>();
        }
    }
    
    // Initialize physics materials
    InitializePhysicsMaterials();
    
    UE_LOG(LogVehiclePhysics, Log, TEXT("VehiclePhysicsSystem initialized for %s"), 
           OwnerPawn ? *OwnerPawn->GetName() : TEXT("Unknown"));
}

void UVehiclePhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn || !MeshComponent)
        return;
    
    // Update ground detection
    UpdateGroundDetection();
    
    // Apply physics forces
    ApplyMovementForces(DeltaTime);
    
    // Handle terrain adaptation
    if (bEnableTerrainAdaptation && bIsGrounded)
    {
        AdaptToTerrain(DeltaTime);
    }
    
    // Update current speed
    CurrentSpeed = OwnerPawn->GetVelocity().Size();
}

void UVehiclePhysicsSystem::UpdateGroundDetection()
{
    if (!OwnerPawn)
        return;
    
    FVector StartLocation = OwnerPawn->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f); // 2m ground check
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        bIsGrounded = true;
        GroundNormal = HitResult.Normal;
        GroundDistance = HitResult.Distance;
        
        // Store ground material for physics calculations
        if (HitResult.PhysMaterial.IsValid())
        {
            GroundMaterial = HitResult.PhysMaterial.Get();
        }
    }
    else
    {
        bIsGrounded = false;
        GroundNormal = FVector::UpVector;
        GroundDistance = 200.0f;
        GroundMaterial = nullptr;
    }
}

void UVehiclePhysicsSystem::ApplyMovementForces(float DeltaTime)
{
    if (!MeshComponent || !MeshComponent->GetBodyInstance())
        return;
    
    FBodyInstance* BodyInstance = MeshComponent->GetBodyInstance();
    
    // Calculate drag forces
    FVector Velocity = BodyInstance->GetUnrealWorldVelocity();
    FVector DragForce = -Velocity * Velocity.SizeSquared() * AirDensity * 0.5f * DragCoefficient;
    
    // Apply rolling resistance if grounded
    if (bIsGrounded)
    {
        FVector RollingResistanceForce = -Velocity.GetSafeNormal() * RollingResistance * 
                                       BodyInstance->GetBodyMass() * 9.81f; // g = 9.81 m/s²
        BodyInstance->AddForce(RollingResistanceForce, false);
    }
    
    // Apply drag
    BodyInstance->AddForce(DragForce, false);
}

void UVehiclePhysicsSystem::AdaptToTerrain(float DeltaTime)
{
    if (!OwnerPawn || !bIsGrounded)
        return;
    
    // Calculate slope angle
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
    
    if (SlopeAngle > SlopeLimit)
    {
        // Apply sliding force on steep slopes
        FVector SlideDirection = FVector::CrossProduct(GroundNormal, FVector::UpVector).GetSafeNormal();
        FVector SlideForce = SlideDirection * SlopeAngle * 10.0f; // Adjust multiplier as needed
        
        if (MeshComponent && MeshComponent->GetBodyInstance())
        {
            MeshComponent->GetBodyInstance()->AddForce(SlideForce, false);
        }
    }
    
    // Gradually align with terrain normal
    FRotator CurrentRotation = OwnerPawn->GetActorRotation();
    FRotator TargetRotation = FRotationMatrix::MakeFromZX(GroundNormal, OwnerPawn->GetActorForwardVector()).Rotator();
    
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TerrainAdaptationStrength);
    OwnerPawn->SetActorRotation(NewRotation);
}

void UVehiclePhysicsSystem::InitializePhysicsMaterials()
{
    // Create default physics material if none exists
    if (!DefaultPhysicsMaterial)
    {
        DefaultPhysicsMaterial = NewObject<UPhysicalMaterial>(this);
        DefaultPhysicsMaterial->Friction = FrictionCoefficient;
        DefaultPhysicsMaterial->Restitution = 0.3f;
        DefaultPhysicsMaterial->Density = 1000.0f; // kg/m³
    }
}

void UVehiclePhysicsSystem::SetMaxSpeed(float NewMaxSpeed)
{
    MaxSpeed = FMath::Max(0.0f, NewMaxSpeed);
    UE_LOG(LogVehiclePhysics, Log, TEXT("Max speed set to: %f"), MaxSpeed);
}

void UVehiclePhysicsSystem::SetAcceleration(float NewAcceleration)
{
    Acceleration = FMath::Max(0.0f, NewAcceleration);
    UE_LOG(LogVehiclePhysics, Log, TEXT("Acceleration set to: %f"), Acceleration);
}

void UVehiclePhysicsSystem::EnableTerrainAdaptation(bool bEnable)
{
    bEnableTerrainAdaptation = bEnable;
    UE_LOG(LogVehiclePhysics, Log, TEXT("Terrain adaptation %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FVector UVehiclePhysicsSystem::GetGroundNormal() const
{
    return GroundNormal;
}

bool UVehiclePhysicsSystem::IsGrounded() const
{
    return bIsGrounded;
}

float UVehiclePhysicsSystem::GetCurrentSpeed() const
{
    return CurrentSpeed;
}

float UVehiclePhysicsSystem::GetGroundDistance() const
{
    return GroundDistance;
}

UPhysicalMaterial* UVehiclePhysicsSystem::GetGroundMaterial() const
{
    return GroundMaterial;
}