#include "Core_VehiclePhysics.h"
#include "Core_TerrainPhysicsIntegrator.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ACore_VehiclePhysics::ACore_VehiclePhysics()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create vehicle mesh component
    VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
    RootComponent = VehicleMesh;
    
    // Enable physics simulation
    VehicleMesh->SetSimulatePhysics(true);
    VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    VehicleMesh->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
    VehicleMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // Create terrain physics integrator
    TerrainPhysicsIntegrator = CreateDefaultSubobject<UCore_TerrainPhysicsIntegrator>(TEXT("TerrainPhysicsIntegrator"));
    
    // Set default physics properties
    PhysicsProperties = FCore_VehiclePhysicsProperties();
    ConfigureVehiclePhysics();
    
    // Initialize state
    CurrentState = ECore_VehicleState::Idle;
    TerrainResponse = FCore_VehicleTerrainResponse();
    
    // Set default vehicle type
    VehicleType = ECore_VehicleType::WoodenCart;
}

void ACore_VehiclePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure initial physics properties
    ConfigureVehiclePhysics();
    
    // Initialize terrain response
    UpdateTerrainResponse();
    
    // Store initial position and rotation
    LastPosition = GetActorLocation();
    LastRotation = GetActorRotation();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void ACore_VehiclePhysics::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update vehicle state
    UpdateVehicleState(DeltaTime);
    
    // Update terrain response
    UpdateTerrainResponse();
    
    // Apply terrain effects
    ApplyTerrainEffects(DeltaTime);
    
    // Check vehicle stability
    CheckVehicleStability();
    
    // Apply physics forces
    ApplyPhysicsForces(DeltaTime);
    
    // Update timers
    StateTimer += DeltaTime;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void ACore_VehiclePhysics::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Bind movement inputs
    PlayerInputComponent->BindAxis("MoveForward", this, &ACore_VehiclePhysics::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACore_VehiclePhysics::MoveRight);
    PlayerInputComponent->BindAction("Brake", IE_Pressed, this, &ACore_VehiclePhysics::ApplyBrakes, true);
    PlayerInputComponent->BindAction("Brake", IE_Released, this, &ACore_VehiclePhysics::ApplyBrakes, false);
}

void ACore_VehiclePhysics::MoveForward(float Value)
{
    CurrentForwardInput = Value;
    HandleVehicleInput(Value, CurrentRightInput);
}

void ACore_VehiclePhysics::MoveRight(float Value)
{
    CurrentRightInput = Value;
    HandleVehicleInput(CurrentForwardInput, Value);
}

void ACore_VehiclePhysics::ApplyBrakes(bool bBraking)
{
    bIsBraking = bBraking;
    
    if (bBraking)
    {
        CurrentState = ECore_VehicleState::Braking;
    }
    else if (CurrentState == ECore_VehicleState::Braking)
    {
        CurrentState = ECore_VehicleState::Idle;
    }
}

void ACore_VehiclePhysics::UpdateTerrainResponse()
{
    if (!TerrainPhysicsIntegrator)
        return;
    
    // Get current terrain type
    FVector CurrentLocation = GetActorLocation();
    ETerrainType NewTerrainType = TerrainPhysicsIntegrator->GetTerrainTypeAtLocation(CurrentLocation);
    
    if (NewTerrainType != TerrainResponse.CurrentTerrain)
    {
        TerrainResponse.CurrentTerrain = NewTerrainType;
        CalculateTerrainEffects();
    }
    
    // Check if vehicle should get stuck
    TerrainResponse.bIsStuck = ShouldVehicleGetStuck();
    
    if (TerrainResponse.bIsStuck)
    {
        TerrainResponse.StuckTimer += GetWorld()->GetDeltaSeconds();
        CurrentState = ECore_VehicleState::Stuck;
    }
    else
    {
        TerrainResponse.StuckTimer = 0.0f;
        if (CurrentState == ECore_VehicleState::Stuck)
        {
            CurrentState = ECore_VehicleState::Idle;
        }
    }
}

void ACore_VehiclePhysics::ApplyTerrainEffects(float DeltaTime)
{
    if (!VehicleMesh || TerrainResponse.bIsStuck)
        return;
    
    // Apply terrain-based force modifications
    FVector TerrainForce = CalculateTerrainForce();
    VehicleMesh->AddForce(TerrainForce);
    
    // Apply terrain friction
    FVector CurrentVelocity = VehicleMesh->GetPhysicsLinearVelocity();
    float FrictionForce = TerrainResponse.TerrainSpeedModifier * PhysicsProperties.TerrainFrictionMultiplier;
    FVector FrictionVector = -CurrentVelocity * FrictionForce * DeltaTime;
    VehicleMesh->AddForce(FrictionVector);
    
    // Apply water resistance for rafts
    if (VehicleType == ECore_VehicleType::WoodenRaft && TerrainResponse.CurrentTerrain == ETerrainType::Water)
    {
        FVector WaterResistance = -CurrentVelocity * PhysicsProperties.WaterResistance * DeltaTime;
        VehicleMesh->AddForce(WaterResistance);
    }
}

void ACore_VehiclePhysics::CheckVehicleStability()
{
    if (!VehicleMesh)
        return;
    
    // Calculate stability based on vehicle properties and terrain
    float StabilityThreshold = PhysicsProperties.StabilityFactor * TerrainResponse.TerrainStabilityModifier;
    
    // Check angular velocity for instability
    FVector AngularVelocity = VehicleMesh->GetPhysicsAngularVelocityInRadians();
    float AngularSpeed = AngularVelocity.Size();
    
    if (AngularSpeed > StabilityThreshold)
    {
        // Apply stabilizing force
        FVector StabilizingTorque = -AngularVelocity * PhysicsProperties.StabilityFactor;
        VehicleMesh->AddTorqueInRadians(StabilizingTorque);
    }
    
    // Check for vehicle rollover
    FRotator CurrentRotation = GetActorRotation();
    float RollAngle = FMath::Abs(CurrentRotation.Roll);
    float PitchAngle = FMath::Abs(CurrentRotation.Pitch);
    
    if (RollAngle > 45.0f || PitchAngle > 45.0f)
    {
        CurrentState = ECore_VehicleState::Damaged;
    }
}

void ACore_VehiclePhysics::HandleVehicleDamage(float DamageAmount)
{
    // Reduce vehicle performance based on damage
    PhysicsProperties.MaxSpeed *= (1.0f - DamageAmount * 0.1f);
    PhysicsProperties.Acceleration *= (1.0f - DamageAmount * 0.15f);
    PhysicsProperties.StabilityFactor *= (1.0f - DamageAmount * 0.2f);
    
    // Clamp values to prevent negative performance
    PhysicsProperties.MaxSpeed = FMath::Max(PhysicsProperties.MaxSpeed, 100.0f);
    PhysicsProperties.Acceleration = FMath::Max(PhysicsProperties.Acceleration, 50.0f);
    PhysicsProperties.StabilityFactor = FMath::Max(PhysicsProperties.StabilityFactor, 0.1f);
    
    CurrentState = ECore_VehicleState::Damaged;
}

void ACore_VehiclePhysics::SetVehicleType(ECore_VehicleType NewType)
{
    VehicleType = NewType;
    ConfigureVehiclePhysics();
}

void ACore_VehiclePhysics::ConfigureVehiclePhysics()
{
    if (!VehicleMesh)
        return;
    
    // Configure physics properties based on vehicle type
    switch (VehicleType)
    {
        case ECore_VehicleType::WoodenRaft:
            PhysicsProperties.Mass = 300.0f;
            PhysicsProperties.MaxSpeed = 400.0f;
            PhysicsProperties.Acceleration = 200.0f;
            PhysicsProperties.TurningRadius = 500.0f;
            PhysicsProperties.WaterResistance = 0.5f;
            PhysicsProperties.StabilityFactor = 0.8f;
            break;
            
        case ECore_VehicleType::StoneSled:
            PhysicsProperties.Mass = 800.0f;
            PhysicsProperties.MaxSpeed = 300.0f;
            PhysicsProperties.Acceleration = 150.0f;
            PhysicsProperties.TurningRadius = 200.0f;
            PhysicsProperties.TerrainFrictionMultiplier = 1.5f;
            PhysicsProperties.StabilityFactor = 0.9f;
            break;
            
        case ECore_VehicleType::WoodenCart:
            PhysicsProperties.Mass = 500.0f;
            PhysicsProperties.MaxSpeed = 600.0f;
            PhysicsProperties.Acceleration = 400.0f;
            PhysicsProperties.TurningRadius = 300.0f;
            PhysicsProperties.StabilityFactor = 0.7f;
            break;
            
        case ECore_VehicleType::AnimalMount:
            PhysicsProperties.Mass = 400.0f;
            PhysicsProperties.MaxSpeed = 1000.0f;
            PhysicsProperties.Acceleration = 800.0f;
            PhysicsProperties.TurningRadius = 150.0f;
            PhysicsProperties.StabilityFactor = 0.6f;
            break;
            
        default:
            // Use default properties
            break;
    }
    
    // Apply mass to physics component
    VehicleMesh->SetMassOverrideInKg(NAME_None, PhysicsProperties.Mass, true);
}

void ACore_VehiclePhysics::SetPhysicsProperties(const FCore_VehiclePhysicsProperties& NewProperties)
{
    PhysicsProperties = NewProperties;
    ConfigureVehiclePhysics();
}

float ACore_VehiclePhysics::GetCurrentSpeed() const
{
    if (!VehicleMesh)
        return 0.0f;
    
    FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
    return Velocity.Size();
}

void ACore_VehiclePhysics::DebugVehiclePhysics()
{
    if (!GEngine)
        return;
    
    FString DebugText = FString::Printf(TEXT("Vehicle Type: %s\nState: %s\nSpeed: %.1f\nTerrain: %s\nStuck: %s"),
        *UEnum::GetValueAsString(VehicleType),
        *UEnum::GetValueAsString(CurrentState),
        GetCurrentSpeed(),
        *UEnum::GetValueAsString(TerrainResponse.CurrentTerrain),
        TerrainResponse.bIsStuck ? TEXT("Yes") : TEXT("No")
    );
    
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugText);
}

void ACore_VehiclePhysics::DrawVehicleDebugInfo()
{
    if (!GetWorld())
        return;
    
    FVector Location = GetActorLocation();
    FVector Velocity = VehicleMesh ? VehicleMesh->GetPhysicsLinearVelocity() : FVector::ZeroVector;
    
    // Draw velocity vector
    DrawDebugLine(GetWorld(), Location, Location + Velocity * 0.1f, FColor::Green, false, 0.1f, 0, 2.0f);
    
    // Draw turning radius
    float Radius = PhysicsProperties.TurningRadius;
    DrawDebugCircle(GetWorld(), Location, Radius, 32, FColor::Blue, false, 0.1f, 0, 2.0f, FVector::ForwardVector, FVector::RightVector);
    
    // Draw terrain type indicator
    FColor TerrainColor = FColor::White;
    switch (TerrainResponse.CurrentTerrain)
    {
        case ETerrainType::Grass: TerrainColor = FColor::Green; break;
        case ETerrainType::Dirt: TerrainColor = FColor::Orange; break;
        case ETerrainType::Rock: TerrainColor = FColor::Red; break;
        case ETerrainType::Sand: TerrainColor = FColor::Yellow; break;
        case ETerrainType::Mud: TerrainColor = FColor::Magenta; break;
        case ETerrainType::Water: TerrainColor = FColor::Blue; break;
        default: break;
    }
    
    DrawDebugSphere(GetWorld(), Location + FVector(0, 0, 100), 50.0f, 12, TerrainColor, false, 0.1f, 0, 2.0f);
}

void ACore_VehiclePhysics::UpdateVehicleState(float DeltaTime)
{
    // Update state based on input and movement
    float CurrentSpeed = GetCurrentSpeed();
    
    if (TerrainResponse.bIsStuck)
    {
        CurrentState = ECore_VehicleState::Stuck;
        return;
    }
    
    if (bIsBraking)
    {
        CurrentState = ECore_VehicleState::Braking;
    }
    else if (FMath::Abs(CurrentRightInput) > 0.1f)
    {
        CurrentState = ECore_VehicleState::Turning;
    }
    else if (FMath::Abs(CurrentForwardInput) > 0.1f || CurrentSpeed > 50.0f)
    {
        CurrentState = ECore_VehicleState::Moving;
    }
    else
    {
        CurrentState = ECore_VehicleState::Idle;
    }
    
    // Update position tracking
    LastPosition = GetActorLocation();
    LastRotation = GetActorRotation();
}

void ACore_VehiclePhysics::CalculateTerrainEffects()
{
    // Calculate speed and stability modifiers based on terrain
    switch (TerrainResponse.CurrentTerrain)
    {
        case ETerrainType::Grass:
            TerrainResponse.TerrainSpeedModifier = 1.0f;
            TerrainResponse.TerrainStabilityModifier = 1.0f;
            break;
            
        case ETerrainType::Dirt:
            TerrainResponse.TerrainSpeedModifier = 0.9f;
            TerrainResponse.TerrainStabilityModifier = 0.9f;
            break;
            
        case ETerrainType::Rock:
            TerrainResponse.TerrainSpeedModifier = 0.7f;
            TerrainResponse.TerrainStabilityModifier = 0.6f;
            break;
            
        case ETerrainType::Sand:
            TerrainResponse.TerrainSpeedModifier = 0.6f;
            TerrainResponse.TerrainStabilityModifier = 0.7f;
            break;
            
        case ETerrainType::Mud:
            TerrainResponse.TerrainSpeedModifier = 0.4f;
            TerrainResponse.TerrainStabilityModifier = 0.5f;
            break;
            
        case ETerrainType::Water:
            if (VehicleType == ECore_VehicleType::WoodenRaft)
            {
                TerrainResponse.TerrainSpeedModifier = 0.8f;
                TerrainResponse.TerrainStabilityModifier = 0.9f;
            }
            else
            {
                TerrainResponse.TerrainSpeedModifier = 0.2f;
                TerrainResponse.TerrainStabilityModifier = 0.3f;
            }
            break;
            
        default:
            TerrainResponse.TerrainSpeedModifier = 1.0f;
            TerrainResponse.TerrainStabilityModifier = 1.0f;
            break;
    }
}

void ACore_VehiclePhysics::ApplyPhysicsForces(float DeltaTime)
{
    if (!VehicleMesh || TerrainResponse.bIsStuck)
        return;
    
    // Apply movement forces
    if (FMath::Abs(CurrentForwardInput) > 0.1f)
    {
        FVector ForwardForce = GetActorForwardVector() * CurrentForwardInput * PhysicsProperties.Acceleration * TerrainResponse.TerrainSpeedModifier;
        VehicleMesh->AddForce(ForwardForce);
    }
    
    // Apply turning forces
    if (FMath::Abs(CurrentRightInput) > 0.1f)
    {
        FVector TorqueForce = GetActorUpVector() * CurrentRightInput * PhysicsProperties.Acceleration * 0.5f;
        VehicleMesh->AddTorqueInRadians(TorqueForce);
    }
    
    // Apply braking forces
    if (bIsBraking)
    {
        FVector CurrentVelocity = VehicleMesh->GetPhysicsLinearVelocity();
        FVector BrakingForce = -CurrentVelocity.GetSafeNormal() * PhysicsProperties.BrakingForce;
        VehicleMesh->AddForce(BrakingForce);
    }
    
    // Limit maximum speed
    FVector CurrentVelocity = VehicleMesh->GetPhysicsLinearVelocity();
    float MaxAllowedSpeed = PhysicsProperties.MaxSpeed * TerrainResponse.TerrainSpeedModifier;
    
    if (CurrentVelocity.Size() > MaxAllowedSpeed)
    {
        FVector LimitedVelocity = CurrentVelocity.GetSafeNormal() * MaxAllowedSpeed;
        VehicleMesh->SetPhysicsLinearVelocity(LimitedVelocity);
    }
}

void ACore_VehiclePhysics::HandleVehicleInput(float ForwardInput, float RightInput)
{
    CurrentForwardInput = ForwardInput;
    CurrentRightInput = RightInput;
}

void ACore_VehiclePhysics::ProcessTerrainInteraction()
{
    // Additional terrain-specific interactions can be implemented here
    // For example, special effects when transitioning between terrain types
}

float ACore_VehiclePhysics::CalculateSpeedModifier() const
{
    return TerrainResponse.TerrainSpeedModifier;
}

float ACore_VehiclePhysics::CalculateStabilityModifier() const
{
    return TerrainResponse.TerrainStabilityModifier;
}

FVector ACore_VehiclePhysics::CalculateTerrainForce() const
{
    // Calculate additional forces based on terrain type
    FVector TerrainForce = FVector::ZeroVector;
    
    switch (TerrainResponse.CurrentTerrain)
    {
        case ETerrainType::Mud:
            // Mud creates additional drag
            if (VehicleMesh)
            {
                FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
                TerrainForce = -Velocity * 0.5f;
            }
            break;
            
        case ETerrainType::Water:
            // Water creates buoyancy for rafts
            if (VehicleType == ECore_VehicleType::WoodenRaft)
            {
                TerrainForce = FVector(0, 0, 500.0f); // Upward buoyancy force
            }
            break;
            
        default:
            break;
    }
    
    return TerrainForce;
}

bool ACore_VehiclePhysics::ShouldVehicleGetStuck() const
{
    // Determine if vehicle should get stuck based on terrain and vehicle type
    switch (TerrainResponse.CurrentTerrain)
    {
        case ETerrainType::Mud:
            return VehicleType != ECore_VehicleType::AnimalMount && GetCurrentSpeed() < 100.0f;
            
        case ETerrainType::Sand:
            return VehicleType == ECore_VehicleType::StoneSled && GetCurrentSpeed() < 50.0f;
            
        case ETerrainType::Water:
            return VehicleType != ECore_VehicleType::WoodenRaft;
            
        default:
            return false;
    }
}

// Component Implementation
UCore_VehiclePhysicsComponent::UCore_VehiclePhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    VehicleType = ECore_VehicleType::WoodenCart;
    PhysicsProperties = FCore_VehiclePhysicsProperties();
    TerrainResponse = FCore_VehicleTerrainResponse();
}

void UCore_VehiclePhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCore_VehiclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update terrain response for component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Basic terrain detection for component version
        FVector Location = Owner->GetActorLocation();
        // Simplified terrain detection - can be expanded with proper terrain system integration
    }
}

void UCore_VehiclePhysicsComponent::ApplyVehicleForce(const FVector& Force)
{
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddForce(Force);
        }
    }
}

void UCore_VehiclePhysicsComponent::SetVehiclePhysicsProperties(const FCore_VehiclePhysicsProperties& NewProperties)
{
    PhysicsProperties = NewProperties;
    
    // Apply mass to owner's primitive component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp)
        {
            PrimComp->SetMassOverrideInKg(NAME_None, PhysicsProperties.Mass, true);
        }
    }
}