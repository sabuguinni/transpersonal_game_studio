#include "Core_CharacterPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

UCore_CharacterPhysics::UCore_CharacterPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default configuration
    PhysicsConfig = FCore_CharacterPhysicsConfig();
    CurrentPhysicsState = ECore_CharacterPhysicsState::Normal;
    
    bIsGrounded = true;
    bIsInWater = false;
    CurrentVelocityMagnitude = 0.0f;
    CurrentAcceleration = FVector::ZeroVector;
    
    RagdollTimer = 0.0f;
    RagdollDuration = 0.0f;
    bRagdollActive = false;
    
    LastGroundNormal = FVector::UpVector;
    LastGroundDistance = 0.0f;
    bGroundContactValid = false;
    
    WaterLevel = 0.0f;
    SubmergedDepth = 0.0f;
}

void UCore_CharacterPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    ValidateMovementSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Component initialized for %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UCore_CharacterPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update physics state and calculations
    UpdatePhysicsState(DeltaTime);
    UpdateGroundDetection();
    
    if (bIsInWater)
    {
        UpdateWaterPhysics(DeltaTime);
    }
    
    if (bRagdollActive)
    {
        ProcessRagdoll(DeltaTime);
    }
    
    // Update velocity tracking
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentVelocityMagnitude = CurrentVelocity.Size();
    CurrentAcceleration = MovementComponent->GetCurrentAcceleration();
}

void UCore_CharacterPhysics::InitializeComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysics: Owner is not a Character!"));
        return;
    }
    
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    MeshComponent = OwnerCharacter->GetMesh();
    CapsuleComponent = OwnerCharacter->GetCapsuleComponent();
    
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysics: No CharacterMovementComponent found!"));
    }
    
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysics: No SkeletalMeshComponent found!"));
    }
    
    if (!CapsuleComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysics: No CapsuleComponent found!"));
    }
}

void UCore_CharacterPhysics::UpdatePhysicsState(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    ECore_CharacterPhysicsState NewState = CurrentPhysicsState;
    
    // Determine new physics state based on conditions
    if (bRagdollActive)
    {
        NewState = ECore_CharacterPhysicsState::Ragdoll;
    }
    else if (bIsInWater)
    {
        NewState = ECore_CharacterPhysicsState::Swimming;
    }
    else if (MovementComponent->IsFalling())
    {
        NewState = ECore_CharacterPhysicsState::Falling;
    }
    else if (bIsGrounded)
    {
        NewState = ECore_CharacterPhysicsState::Normal;
    }
    
    // Update state if changed
    if (NewState != CurrentPhysicsState)
    {
        SetPhysicsState(NewState);
    }
}

void UCore_CharacterPhysics::SetMovementSpeed(float NewSpeed)
{
    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = NewSpeed;
        UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Movement speed set to %f"), NewSpeed);
    }
}

void UCore_CharacterPhysics::ApplyImpulse(const FVector& Impulse, bool bVelChange)
{
    if (!OwnerCharacter || !MeshComponent)
    {
        return;
    }
    
    // Apply impulse to the character's mesh
    MeshComponent->AddImpulse(Impulse, NAME_None, bVelChange);
    
    // Create impact data for event
    FCore_PhysicsImpactData ImpactData;
    ImpactData.ImpactLocation = OwnerCharacter->GetActorLocation();
    ImpactData.ImpactForce = Impulse.Size();
    ImpactData.ImpactTime = GetWorld()->GetTimeSeconds();
    
    // Check if impulse is strong enough to trigger ragdoll
    if (ShouldActivateRagdoll(ImpactData))
    {
        EnableRagdoll(PhysicsConfig.RagdollRecoveryTime);
    }
    
    OnImpactReceived(ImpactData);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Applied impulse %s (VelChange: %s)"), 
           *Impulse.ToString(), bVelChange ? TEXT("true") : TEXT("false"));
}

void UCore_CharacterPhysics::SetPhysicsState(ECore_CharacterPhysicsState NewState)
{
    if (NewState == CurrentPhysicsState)
    {
        return;
    }
    
    ECore_CharacterPhysicsState OldState = CurrentPhysicsState;
    PreviousPhysicsState = CurrentPhysicsState;
    CurrentPhysicsState = NewState;
    
    // Apply state-specific settings
    switch (NewState)
    {
        case ECore_CharacterPhysicsState::Normal:
            if (MovementComponent)
            {
                MovementComponent->MaxWalkSpeed = PhysicsConfig.WalkSpeed;
                MovementComponent->JumpZVelocity = PhysicsConfig.JumpVelocity;
                MovementComponent->AirControl = PhysicsConfig.AirControl;
            }
            break;
            
        case ECore_CharacterPhysicsState::Swimming:
            if (MovementComponent)
            {
                MovementComponent->MaxSwimSpeed = PhysicsConfig.SwimSpeed;
            }
            break;
            
        case ECore_CharacterPhysicsState::Ragdoll:
            // Ragdoll state handled by EnableRagdoll function
            break;
            
        case ECore_CharacterPhysicsState::Falling:
            // Falling state uses default air control
            break;
            
        default:
            break;
    }
    
    OnPhysicsStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: State changed from %d to %d"), 
           (int32)OldState, (int32)NewState);
}

void UCore_CharacterPhysics::EnableRagdoll(float Duration)
{
    if (!MeshComponent || bRagdollActive)
    {
        return;
    }
    
    bRagdollActive = true;
    RagdollDuration = Duration;
    RagdollTimer = 0.0f;
    
    // Enable physics simulation on the mesh
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Disable character movement
    if (MovementComponent)
    {
        MovementComponent->DisableMovement();
    }
    
    // Disable capsule collision
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    OnRagdollActivated();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Ragdoll enabled for %f seconds"), Duration);
}

void UCore_CharacterPhysics::DisableRagdoll()
{
    if (!MeshComponent || !bRagdollActive)
    {
        return;
    }
    
    bRagdollActive = false;
    RagdollTimer = 0.0f;
    RagdollDuration = 0.0f;
    
    // Disable physics simulation on the mesh
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable character movement
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_Walking);
    }
    
    // Re-enable capsule collision
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Reset mesh to relative position
    if (MeshComponent && OwnerCharacter)
    {
        MeshComponent->SetRelativeLocation(FVector(0, 0, -90)); // Standard character mesh offset
        MeshComponent->SetRelativeRotation(FRotator(0, -90, 0)); // Standard character mesh rotation
    }
    
    OnRagdollDeactivated();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Ragdoll disabled"));
}

void UCore_CharacterPhysics::HandleImpact(const FCore_PhysicsImpactData& ImpactData)
{
    // Process impact effects
    if (ImpactData.ImpactForce > PhysicsConfig.RagdollImpulseThreshold)
    {
        EnableRagdoll(PhysicsConfig.RagdollRecoveryTime);
    }
    
    // Apply knockback if needed
    if (ImpactData.ImpactForce > 100.0f && MovementComponent)
    {
        FVector KnockbackDirection = (OwnerCharacter->GetActorLocation() - ImpactData.ImpactLocation).GetSafeNormal();
        FVector KnockbackForce = KnockbackDirection * ImpactData.ImpactForce * 0.5f;
        
        MovementComponent->AddImpulse(KnockbackForce, true);
    }
    
    OnImpactReceived(ImpactData);
}

void UCore_CharacterPhysics::EnterWater()
{
    if (bIsInWater)
    {
        return;
    }
    
    bIsInWater = true;
    
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_Swimming);
        MovementComponent->MaxSwimSpeed = PhysicsConfig.SwimSpeed;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Entered water"));
}

void UCore_CharacterPhysics::ExitWater()
{
    if (!bIsInWater)
    {
        return;
    }
    
    bIsInWater = false;
    
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_Walking);
        MovementComponent->MaxWalkSpeed = PhysicsConfig.WalkSpeed;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Exited water"));
}

void UCore_CharacterPhysics::ApplyBuoyancy(float DeltaTime)
{
    if (!bIsInWater || !MovementComponent)
    {
        return;
    }
    
    FVector BuoyancyForce = CalculateBuoyancyForce();
    MovementComponent->AddForce(BuoyancyForce);
}

bool UCore_CharacterPhysics::CheckGroundContact()
{
    if (!OwnerCharacter || !CapsuleComponent)
    {
        return false;
    }
    
    FVector Start = OwnerCharacter->GetActorLocation();
    FVector End = Start - FVector(0, 0, CapsuleComponent->GetScaledCapsuleHalfHeight() + 50.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        LastGroundNormal = HitResult.Normal;
        LastGroundDistance = HitResult.Distance;
        bGroundContactValid = true;
        bIsGrounded = true;
    }
    else
    {
        bGroundContactValid = false;
        bIsGrounded = false;
    }
    
    return bHit;
}

FVector UCore_CharacterPhysics::GetGroundNormal()
{
    return bGroundContactValid ? LastGroundNormal : FVector::UpVector;
}

float UCore_CharacterPhysics::GetDistanceToGround()
{
    return bGroundContactValid ? LastGroundDistance : -1.0f;
}

bool UCore_CharacterPhysics::ValidatePhysicsState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return false;
    }
    
    // Check for invalid velocities
    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.ContainsNaN() || Velocity.Size() > 10000.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CharacterPhysics: Invalid velocity detected, resetting"));
        MovementComponent->Velocity = FVector::ZeroVector;
        return false;
    }
    
    // Check for invalid position
    FVector Location = OwnerCharacter->GetActorLocation();
    if (Location.ContainsNaN() || Location.Z < -10000.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CharacterPhysics: Invalid position detected, resetting"));
        ResetPhysicsState();
        return false;
    }
    
    return true;
}

void UCore_CharacterPhysics::ResetPhysicsState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Reset velocity
    MovementComponent->Velocity = FVector::ZeroVector;
    
    // Reset position to a safe location (find ground or use spawn point)
    FVector SafeLocation = FVector(0, 0, 200); // Default safe location
    OwnerCharacter->SetActorLocation(SafeLocation);
    
    // Reset physics state
    if (bRagdollActive)
    {
        DisableRagdoll();
    }
    
    SetPhysicsState(ECore_CharacterPhysicsState::Normal);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysics: Physics state reset"));
}

void UCore_CharacterPhysics::UpdateGroundDetection()
{
    CheckGroundContact();
}

void UCore_CharacterPhysics::UpdateWaterPhysics(float DeltaTime)
{
    if (!bIsInWater)
    {
        return;
    }
    
    ApplyBuoyancy(DeltaTime);
    
    // Apply water resistance
    if (MovementComponent)
    {
        float WaterResistance = CalculateWaterResistance();
        FVector ResistanceForce = -MovementComponent->Velocity * WaterResistance;
        MovementComponent->AddForce(ResistanceForce);
    }
}

void UCore_CharacterPhysics::ProcessRagdoll(float DeltaTime)
{
    if (!bRagdollActive)
    {
        return;
    }
    
    RagdollTimer += DeltaTime;
    
    // Auto-recover from ragdoll after duration
    if (RagdollDuration > 0.0f && RagdollTimer >= RagdollDuration)
    {
        DisableRagdoll();
    }
}

void UCore_CharacterPhysics::ValidateMovementSettings()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Apply physics configuration to movement component
    MovementComponent->MaxWalkSpeed = PhysicsConfig.WalkSpeed;
    MovementComponent->JumpZVelocity = PhysicsConfig.JumpVelocity;
    MovementComponent->AirControl = PhysicsConfig.AirControl;
    MovementComponent->GroundFriction = PhysicsConfig.GroundFriction;
    MovementComponent->BrakingDecelerationWalking = PhysicsConfig.BrakingDeceleration;
    
    // Set mass if possible
    if (MeshComponent && MeshComponent->GetBodyInstance())
    {
        MeshComponent->GetBodyInstance()->SetMassOverride(PhysicsConfig.Mass);
    }
}

FVector UCore_CharacterPhysics::CalculateBuoyancyForce() const
{
    if (!bIsInWater)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate buoyancy based on submersion depth
    float BuoyancyMagnitude = PhysicsConfig.BuoyancyForce * FMath::Clamp(SubmergedDepth, 0.0f, 1.0f);
    return FVector(0, 0, BuoyancyMagnitude);
}

float UCore_CharacterPhysics::CalculateWaterResistance() const
{
    if (!bIsInWater || !MovementComponent)
    {
        return 0.0f;
    }
    
    // Water resistance increases with velocity
    float VelocityMagnitude = MovementComponent->Velocity.Size();
    return FMath::Clamp(VelocityMagnitude * 0.1f, 0.0f, 500.0f);
}

bool UCore_CharacterPhysics::ShouldActivateRagdoll(const FCore_PhysicsImpactData& ImpactData) const
{
    return ImpactData.ImpactForce >= PhysicsConfig.RagdollImpulseThreshold && !bRagdollActive;
}