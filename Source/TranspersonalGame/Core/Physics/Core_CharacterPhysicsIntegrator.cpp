#include "Core_CharacterPhysicsIntegrator.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCore_CharacterPhysicsIntegrator::UCore_CharacterPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default physics settings
    RagdollActivationThreshold = 500.0f;
    RagdollRecoveryTime = 3.0f;
    PhysicsBlendTime = 0.5f;
    bEnablePhysicsIntegration = true;
    
    // Default constraint settings
    MaxVelocityMagnitude = 2000.0f;
    MaxAngularVelocity = 10.0f;
    bLimitPhysicsVelocity = true;
    
    // Initialize state
    CurrentPhysicsState = ECore_PhysicsState::Normal;
    bIsRagdollActive = false;
    RagdollTimer = 0.0f;
}

void UCore_CharacterPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponentReferences();
    ValidateComponentReferences();
    
    if (ValidatePhysicsSetup())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysicsIntegrator: Successfully initialized for %s"), 
               OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CharacterPhysicsIntegrator: Failed to initialize properly"));
    }
}

void UCore_CharacterPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnablePhysicsIntegration || !OwnerCharacter)
    {
        return;
    }
    
    UpdatePhysicsState(DeltaTime);
    UpdateRagdollState(DeltaTime);
    
    if (bLimitPhysicsVelocity)
    {
        LimitPhysicsVelocity();
    }
}

void UCore_CharacterPhysicsIntegrator::InitializeComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        SkeletalMeshComp = OwnerCharacter->GetMesh();
        CapsuleComp = OwnerCharacter->GetCapsuleComponent();
        MovementComp = OwnerCharacter->GetCharacterMovement();
        
        // Cache initial position and rotation
        LastValidPosition = OwnerCharacter->GetActorLocation();
        LastValidRotation = OwnerCharacter->GetActorRotation();
        bHasValidPhysicsState = true;
    }
}

void UCore_CharacterPhysicsIntegrator::ValidateComponentReferences()
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysicsIntegrator: No valid Character owner"));
        return;
    }
    
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysicsIntegrator: No SkeletalMeshComponent found"));
    }
    
    if (!CapsuleComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysicsIntegrator: No CapsuleComponent found"));
    }
    
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CharacterPhysicsIntegrator: No CharacterMovementComponent found"));
    }
}

bool UCore_CharacterPhysicsIntegrator::ValidatePhysicsSetup()
{
    if (!OwnerCharacter || !SkeletalMeshComp || !CapsuleComp || !MovementComp)
    {
        return false;
    }
    
    // Check if skeletal mesh has a physics asset
    if (SkeletalMeshComp->GetPhysicsAsset() == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CharacterPhysicsIntegrator: No Physics Asset found on SkeletalMesh"));
        return false;
    }
    
    return true;
}

void UCore_CharacterPhysicsIntegrator::SetPhysicsState(ECore_PhysicsState NewState)
{
    if (CurrentPhysicsState == NewState)
    {
        return;
    }
    
    PreviousPhysicsState = CurrentPhysicsState;
    CurrentPhysicsState = NewState;
    bIsTransitioning = true;
    StateTransitionTimer = 0.0f;
    
    // Handle state transitions
    switch (NewState)
    {
        case ECore_PhysicsState::Normal:
            TransitionToNormal();
            break;
        case ECore_PhysicsState::Ragdoll:
            TransitionToRagdoll();
            break;
        case ECore_PhysicsState::Stunned:
            TransitionToStunned();
            break;
        case ECore_PhysicsState::Falling:
            TransitionToFalling();
            break;
        case ECore_PhysicsState::Swimming:
            TransitionToSwimming();
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysicsIntegrator: State changed from %d to %d"), 
           (int32)PreviousPhysicsState, (int32)CurrentPhysicsState);
}

bool UCore_CharacterPhysicsIntegrator::IsInPhysicsMode() const
{
    return CurrentPhysicsState == ECore_PhysicsState::Ragdoll || 
           CurrentPhysicsState == ECore_PhysicsState::Falling ||
           CurrentPhysicsState == ECore_PhysicsState::Swimming;
}

void UCore_CharacterPhysicsIntegrator::ActivateRagdoll(const FVector& ImpulseDirection, float ImpulseMagnitude)
{
    if (!SkeletalMeshComp || bIsRagdollActive)
    {
        return;
    }
    
    SetPhysicsState(ECore_PhysicsState::Ragdoll);
    bIsRagdollActive = true;
    RagdollTimer = 0.0f;
    
    // Store impulse for application
    RagdollImpulse = ImpulseDirection.GetSafeNormal() * ImpulseMagnitude;
    
    SetupRagdollPhysics();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysicsIntegrator: Ragdoll activated with impulse magnitude %f"), ImpulseMagnitude);
}

void UCore_CharacterPhysicsIntegrator::DeactivateRagdoll()
{
    if (!bIsRagdollActive)
    {
        return;
    }
    
    bIsRagdollActive = false;
    RagdollTimer = 0.0f;
    
    CleanupRagdollPhysics();
    SetPhysicsState(ECore_PhysicsState::Normal);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysicsIntegrator: Ragdoll deactivated"));
}

void UCore_CharacterPhysicsIntegrator::ProcessPhysicsImpact(const FCore_PhysicsImpact& Impact)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Check if impact is strong enough to trigger ragdoll
    if (Impact.bCausesRagdoll || Impact.ImpactForce >= RagdollActivationThreshold)
    {
        ActivateRagdoll(Impact.ImpactDirection, Impact.ImpactForce);
    }
    else
    {
        // Apply impulse without ragdoll
        ApplyImpulseToCharacter(Impact.ImpactDirection * Impact.ImpactForce, Impact.ImpactLocation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterPhysicsIntegrator: Processed impact with force %f"), Impact.ImpactForce);
}

void UCore_CharacterPhysicsIntegrator::ApplyImpulseToCharacter(const FVector& Impulse, const FVector& Location)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    FVector ImpulseLocation = Location.IsZero() ? OwnerCharacter->GetActorLocation() : Location;
    
    if (bIsRagdollActive)
    {
        // Apply to physics bodies
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, ImpulseLocation);
    }
    else if (MovementComp)
    {
        // Apply to movement component
        MovementComp->AddImpulse(Impulse, true);
    }
}

void UCore_CharacterPhysicsIntegrator::UpdatePhysicsState(float DeltaTime)
{
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        if (StateTransitionTimer >= PhysicsBlendTime)
        {
            bIsTransitioning = false;
            StateTransitionTimer = 0.0f;
        }
    }
    
    // Update physics constraints
    if (bLimitPhysicsVelocity)
    {
        ApplyPhysicsConstraints();
    }
    
    // Cache valid state
    if (OwnerCharacter && CurrentPhysicsState == ECore_PhysicsState::Normal)
    {
        LastValidPosition = OwnerCharacter->GetActorLocation();
        LastValidRotation = OwnerCharacter->GetActorRotation();
    }
}

void UCore_CharacterPhysicsIntegrator::UpdateRagdollState(float DeltaTime)
{
    if (!bIsRagdollActive)
    {
        return;
    }
    
    RagdollTimer += DeltaTime;
    
    // Apply initial impulse if we have one
    if (!RagdollImpulse.IsZero() && SkeletalMeshComp)
    {
        SkeletalMeshComp->AddImpulse(RagdollImpulse, NAME_None, true);
        RagdollImpulse = FVector::ZeroVector;
    }
    
    // Check for recovery
    if (RagdollTimer >= RagdollRecoveryTime && CanRecoverFromRagdoll())
    {
        UpdateRagdollRecovery(DeltaTime);
    }
}

void UCore_CharacterPhysicsIntegrator::SetupRagdollPhysics()
{
    if (!SkeletalMeshComp || !MovementComp)
    {
        return;
    }
    
    // Disable character movement
    MovementComp->SetMovementMode(MOVE_None);
    
    // Enable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Disable capsule collision to prevent conflicts
    if (CapsuleComp)
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UCore_CharacterPhysicsIntegrator::CleanupRagdollPhysics()
{
    if (!SkeletalMeshComp || !MovementComp)
    {
        return;
    }
    
    // Disable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable capsule collision
    if (CapsuleComp)
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Restore character movement
    MovementComp->SetMovementMode(MOVE_Walking);
    
    // Reset to last valid position if needed
    if (bHasValidPhysicsState && OwnerCharacter)
    {
        OwnerCharacter->SetActorLocation(LastValidPosition);
        OwnerCharacter->SetActorRotation(LastValidRotation);
    }
}

bool UCore_CharacterPhysicsIntegrator::CanRecoverFromRagdoll() const
{
    if (!SkeletalMeshComp)
    {
        return false;
    }
    
    // Check if character has settled (low velocity)
    FVector Velocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
    return Velocity.Size() < 100.0f; // Arbitrary threshold for "settled"
}

void UCore_CharacterPhysicsIntegrator::UpdateRagdollRecovery(float DeltaTime)
{
    // Gradual recovery - could be enhanced with animation blending
    DeactivateRagdoll();
}

void UCore_CharacterPhysicsIntegrator::LimitPhysicsVelocity()
{
    if (!SkeletalMeshComp || !bIsRagdollActive)
    {
        return;
    }
    
    // Limit linear velocity
    FVector LinearVelocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
    if (LinearVelocity.Size() > MaxVelocityMagnitude)
    {
        LinearVelocity = LinearVelocity.GetSafeNormal() * MaxVelocityMagnitude;
        SkeletalMeshComp->SetPhysicsLinearVelocity(LinearVelocity);
    }
    
    // Limit angular velocity
    FVector AngularVelocity = SkeletalMeshComp->GetPhysicsAngularVelocityInRadians();
    if (AngularVelocity.Size() > MaxAngularVelocity)
    {
        AngularVelocity = AngularVelocity.GetSafeNormal() * MaxAngularVelocity;
        SkeletalMeshComp->SetPhysicsAngularVelocityInRadians(AngularVelocity);
    }
}

void UCore_CharacterPhysicsIntegrator::ApplyPhysicsConstraints()
{
    UpdateVelocityLimits();
    ValidatePhysicsConstraints();
}

void UCore_CharacterPhysicsIntegrator::UpdateVelocityLimits()
{
    // Implementation for dynamic velocity limit updates based on state
    switch (CurrentPhysicsState)
    {
        case ECore_PhysicsState::Swimming:
            MaxVelocityMagnitude = 800.0f; // Slower in water
            break;
        case ECore_PhysicsState::Falling:
            MaxVelocityMagnitude = 3000.0f; // Higher for falling
            break;
        default:
            MaxVelocityMagnitude = 2000.0f; // Default
            break;
    }
}

void UCore_CharacterPhysicsIntegrator::ValidatePhysicsConstraints()
{
    // Ensure physics constraints are within reasonable bounds
    MaxVelocityMagnitude = FMath::Clamp(MaxVelocityMagnitude, 100.0f, 5000.0f);
    MaxAngularVelocity = FMath::Clamp(MaxAngularVelocity, 1.0f, 50.0f);
}

void UCore_CharacterPhysicsIntegrator::TransitionToNormal()
{
    if (bIsRagdollActive)
    {
        DeactivateRagdoll();
    }
}

void UCore_CharacterPhysicsIntegrator::TransitionToRagdoll()
{
    if (!bIsRagdollActive)
    {
        SetupRagdollPhysics();
    }
}

void UCore_CharacterPhysicsIntegrator::TransitionToStunned()
{
    // Disable movement but keep upright
    if (MovementComp)
    {
        MovementComp->SetMovementMode(MOVE_None);
    }
}

void UCore_CharacterPhysicsIntegrator::TransitionToFalling()
{
    // Enable falling physics
    if (MovementComp)
    {
        MovementComp->SetMovementMode(MOVE_Falling);
    }
}

void UCore_CharacterPhysicsIntegrator::TransitionToSwimming()
{
    // Enable swimming physics
    if (MovementComp)
    {
        MovementComp->SetMovementMode(MOVE_Swimming);
    }
}

void UCore_CharacterPhysicsIntegrator::HandleGroundImpact(float ImpactVelocity)
{
    if (ImpactVelocity > RagdollActivationThreshold)
    {
        FCore_PhysicsImpact Impact;
        Impact.ImpactForce = ImpactVelocity;
        Impact.ImpactDirection = FVector(0, 0, -1);
        Impact.bCausesRagdoll = true;
        ProcessPhysicsImpact(Impact);
    }
}

void UCore_CharacterPhysicsIntegrator::HandleWaterEntry()
{
    SetPhysicsState(ECore_PhysicsState::Swimming);
}

void UCore_CharacterPhysicsIntegrator::HandleWaterExit()
{
    SetPhysicsState(ECore_PhysicsState::Normal);
}

void UCore_CharacterPhysicsIntegrator::LogPhysicsState()
{
    UE_LOG(LogTemp, Log, TEXT("=== Core_CharacterPhysicsIntegrator State ==="));
    UE_LOG(LogTemp, Log, TEXT("Physics State: %d"), (int32)CurrentPhysicsState);
    UE_LOG(LogTemp, Log, TEXT("Ragdoll Active: %s"), bIsRagdollActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Ragdoll Timer: %f"), RagdollTimer);
    UE_LOG(LogTemp, Log, TEXT("Max Velocity: %f"), MaxVelocityMagnitude);
    UE_LOG(LogTemp, Log, TEXT("Physics Integration Enabled: %s"), bEnablePhysicsIntegration ? TEXT("Yes") : TEXT("No"));
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Character Location: %s"), *OwnerCharacter->GetActorLocation().ToString());
    }
}