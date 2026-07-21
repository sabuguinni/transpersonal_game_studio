#include "Anim_PrehistoricAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "SharedTypes.h"

UAnim_PrehistoricAnimInstance::UAnim_PrehistoricAnimInstance()
{
    // Initialize movement variables
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    
    // Initialize rotation variables
    YawOffset = 0.0f;
    Lean = 0.0f;
    
    // Initialize state variables
    MovementState = EAnim_MovementState::Idle;
    CombatState = EAnim_CombatState::Unarmed;
    SurvivalState = EAnim_SurvivalState::None;
    
    // Initialize foot IK variables
    bEnableFootIK = true;
    LeftFootEffectorLocation = FVector::ZeroVector;
    RightFootEffectorLocation = FVector::ZeroVector;
    PelvisOffset = 0.0f;
    
    // Initialize look at variables
    bEnableLookAt = true;
    LookAtTarget = FVector::ZeroVector;
    LookAtAlpha = 0.0f;
    
    // Initialize blend space variables
    WalkRunBlend = 0.0f;
    StrafeBlend = 0.0f;
    
    // Initialize timing variables
    TimeToJumpApex = 0.0f;
    JumpPlayRate = 1.0f;
    
    // Initialize prehistoric-specific variables
    EnvironmentalAdaptation = 0.0f;
    TerrainDifficulty = 0.0f;
    FatigueLevel = 0.0f;
    AlertnessLevel = 0.0f;
    
    // Initialize cache variables
    PreviousVelocity = FVector::ZeroVector;
    DeltaTimeAccumulator = 0.0f;
    
    // Set update frequency
    UpdateRate = 30.0f; // 30 FPS for performance
    UpdateInterval = 1.0f / UpdateRate;
}

void UAnim_PrehistoricAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache owner references
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
        OwnerMesh = OwnerCharacter->GetMesh();
        
        UE_LOG(LogTemp, Log, TEXT("Prehistoric AnimInstance initialized for: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Prehistoric AnimInstance failed to get owner character"));
    }
}

void UAnim_PrehistoricAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    // Accumulate delta time for performance optimization
    DeltaTimeAccumulator += DeltaTime;
    if (DeltaTimeAccumulator < UpdateInterval)
        return;
    
    float UpdateDeltaTime = DeltaTimeAccumulator;
    DeltaTimeAccumulator = 0.0f;
    
    if (!OwnerCharacter || !OwnerMovement)
        return;
    
    // Update core movement data
    UpdateMovementData(UpdateDeltaTime);
    
    // Update rotation data
    UpdateRotationData(UpdateDeltaTime);
    
    // Update state machines
    UpdateMovementState(UpdateDeltaTime);
    UpdateCombatState(UpdateDeltaTime);
    UpdateSurvivalState(UpdateDeltaTime);
    
    // Update foot IK
    if (bEnableFootIK)
    {
        UpdateFootIK(UpdateDeltaTime);
    }
    
    // Update look at
    if (bEnableLookAt)
    {
        UpdateLookAt(UpdateDeltaTime);
    }
    
    // Update prehistoric-specific features
    UpdatePrehistoricFeatures(UpdateDeltaTime);
    
    // Update blend spaces
    UpdateBlendSpaces(UpdateDeltaTime);
}

void UAnim_PrehistoricAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerMovement)
        return;
    
    // Get current velocity
    FVector CurrentVelocity = OwnerMovement->Velocity;
    
    // Calculate speed
    Speed = CurrentVelocity.Size2D();
    
    // Calculate direction relative to actor rotation
    if (Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal2D();
        Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityDirection);
        if (RightDot < 0.0f)
        {
            Direction *= -1.0f;
        }
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Check if in air
    bIsInAir = OwnerMovement->IsFalling();
    
    // Check if accelerating
    FVector Acceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;
    bIsAccelerating = Acceleration.Size2D() > 10.0f;
    
    // Check if crouching
    bIsCrouching = OwnerMovement->IsCrouching();
    
    // Store previous velocity
    PreviousVelocity = CurrentVelocity;
}

void UAnim_PrehistoricAnimInstance::UpdateRotationData(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerMovement)
        return;
    
    // Calculate yaw offset between movement and actor rotation
    if (Speed > 0.1f)
    {
        FRotator MovementRotation = OwnerMovement->Velocity.GetSafeNormal2D().Rotation();
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        
        float YawDelta = UKismetMathLibrary::NormalizeAxis(MovementRotation.Yaw - ActorRotation.Yaw);
        YawOffset = FMath::FInterpTo(YawOffset, YawDelta, DeltaTime, 5.0f);
    }
    else
    {
        YawOffset = FMath::FInterpTo(YawOffset, 0.0f, DeltaTime, 10.0f);
    }
    
    // Calculate lean based on velocity change
    FVector VelocityChange = OwnerMovement->Velocity - PreviousVelocity;
    float LeanTarget = FMath::Clamp(VelocityChange.Y * 0.01f, -45.0f, 45.0f);
    Lean = FMath::FInterpTo(Lean, LeanTarget, DeltaTime, 3.0f);
}

void UAnim_PrehistoricAnimInstance::UpdateMovementState(float DeltaTime)
{
    EAnim_MovementState NewState = MovementState;
    
    if (bIsInAir)
    {
        NewState = EAnim_MovementState::InAir;
    }
    else if (bIsCrouching)
    {
        if (Speed > 1.0f)
        {
            NewState = EAnim_MovementState::Crouching;
        }
        else
        {
            NewState = EAnim_MovementState::CrouchIdle;
        }
    }
    else if (Speed < 1.0f)
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (Speed < 150.0f)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else if (Speed < 400.0f)
    {
        NewState = EAnim_MovementState::Running;
    }
    else
    {
        NewState = EAnim_MovementState::Sprinting;
    }
    
    // Add state transition logic
    if (NewState != MovementState)
    {
        OnMovementStateChanged(MovementState, NewState);
        MovementState = NewState;
    }
}

void UAnim_PrehistoricAnimInstance::UpdateCombatState(float DeltaTime)
{
    // This would be updated by the combat system
    // For now, maintain current state
    
    // Example logic for weapon state changes
    // if (OwnerCharacter->HasWeaponEquipped())
    // {
    //     CombatState = EAnim_CombatState::Armed;
    // }
    // else
    // {
    //     CombatState = EAnim_CombatState::Unarmed;
    // }
}

void UAnim_PrehistoricAnimInstance::UpdateSurvivalState(float DeltaTime)
{
    // This would be updated by the survival system
    // For now, maintain current state
    
    // Example logic for survival state changes
    // if (OwnerCharacter->IsPerformingSurvivalAction())
    // {
    //     SurvivalState = EAnim_SurvivalState::Crafting; // or other action
    // }
    // else
    // {
    //     SurvivalState = EAnim_SurvivalState::None;
    // }
}

void UAnim_PrehistoricAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerMesh)
        return;
    
    // Get foot bone locations
    FVector LeftFootLocation = OwnerMesh->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootLocation = OwnerMesh->GetBoneLocation(TEXT("foot_r"));
    
    // Perform ground traces for each foot
    FVector LeftGroundLocation = TraceForGround(LeftFootLocation);
    FVector RightGroundLocation = TraceForGround(RightFootLocation);
    
    // Calculate foot offsets
    float LeftFootOffset = LeftGroundLocation.Z - LeftFootLocation.Z;
    float RightFootOffset = RightGroundLocation.Z - RightFootLocation.Z;
    
    // Smooth the foot positions
    LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, 
                                               FVector(0, 0, LeftFootOffset), 
                                               DeltaTime, 15.0f);
    RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, 
                                                FVector(0, 0, RightFootOffset), 
                                                DeltaTime, 15.0f);
    
    // Calculate pelvis offset to keep character grounded
    float TargetPelvisOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, 10.0f);
}

FVector UAnim_PrehistoricAnimInstance::TraceForGround(const FVector& FootLocation)
{
    if (!OwnerCharacter)
        return FootLocation;
    
    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
        return FootLocation;
    
    // Setup trace parameters
    FVector TraceStart = FootLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    // Perform line trace
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, 
                                       ECC_WorldStatic, QueryParams))
    {
        return HitResult.ImpactPoint;
    }
    
    return FootLocation;
}

void UAnim_PrehistoricAnimInstance::UpdateLookAt(float DeltaTime)
{
    // This would be updated by an AI system or player input
    // For now, gradually reduce look at alpha
    LookAtAlpha = FMath::FInterpTo(LookAtAlpha, 0.0f, DeltaTime, 2.0f);
}

void UAnim_PrehistoricAnimInstance::UpdatePrehistoricFeatures(float DeltaTime)
{
    // Update environmental adaptation based on terrain
    UpdateEnvironmentalAdaptation(DeltaTime);
    
    // Update terrain difficulty
    UpdateTerrainDifficulty(DeltaTime);
    
    // Update fatigue level
    UpdateFatigueLevel(DeltaTime);
    
    // Update alertness level
    UpdateAlertnessLevel(DeltaTime);
}

void UAnim_PrehistoricAnimInstance::UpdateEnvironmentalAdaptation(float DeltaTime)
{
    // Calculate adaptation based on current environment
    float TargetAdaptation = 0.5f; // Base adaptation
    
    // Increase adaptation on difficult terrain
    if (TerrainDifficulty > 0.5f)
    {
        TargetAdaptation += 0.3f;
    }
    
    // Increase adaptation when moving fast
    if (Speed > 300.0f)
    {
        TargetAdaptation += 0.2f;
    }
    
    EnvironmentalAdaptation = FMath::FInterpTo(EnvironmentalAdaptation, TargetAdaptation, 
                                             DeltaTime, 1.0f);
}

void UAnim_PrehistoricAnimInstance::UpdateTerrainDifficulty(float DeltaTime)
{
    // This would be calculated based on ground angle, surface type, etc.
    // For now, use a simple calculation based on foot IK offset difference
    float FootOffsetDifference = FMath::Abs(LeftFootEffectorLocation.Z - RightFootEffectorLocation.Z);
    float TargetDifficulty = FMath::Clamp(FootOffsetDifference / 20.0f, 0.0f, 1.0f);
    
    TerrainDifficulty = FMath::FInterpTo(TerrainDifficulty, TargetDifficulty, DeltaTime, 2.0f);
}

void UAnim_PrehistoricAnimInstance::UpdateFatigueLevel(float DeltaTime)
{
    // Increase fatigue when sprinting
    if (MovementState == EAnim_MovementState::Sprinting)
    {
        FatigueLevel = FMath::Clamp(FatigueLevel + DeltaTime * 0.1f, 0.0f, 1.0f);
    }
    else
    {
        // Recover fatigue when not sprinting
        FatigueLevel = FMath::Clamp(FatigueLevel - DeltaTime * 0.05f, 0.0f, 1.0f);
    }
}

void UAnim_PrehistoricAnimInstance::UpdateAlertnessLevel(float DeltaTime)
{
    // This would be updated by the AI system or danger detection
    // For now, gradually reduce alertness
    AlertnessLevel = FMath::FInterpTo(AlertnessLevel, 0.0f, DeltaTime, 1.0f);
}

void UAnim_PrehistoricAnimInstance::UpdateBlendSpaces(float DeltaTime)
{
    // Update walk/run blend based on speed
    if (Speed > 1.0f)
    {
        float WalkSpeed = 150.0f;
        float RunSpeed = 400.0f;
        WalkRunBlend = FMath::Clamp((Speed - WalkSpeed) / (RunSpeed - WalkSpeed), 0.0f, 1.0f);
    }
    else
    {
        WalkRunBlend = 0.0f;
    }
    
    // Update strafe blend based on direction
    StrafeBlend = FMath::Clamp(FMath::Abs(Direction) / 90.0f, 0.0f, 1.0f);
}

void UAnim_PrehistoricAnimInstance::OnMovementStateChanged(EAnim_MovementState OldState, 
                                                          EAnim_MovementState NewState)
{
    UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), (int32)OldState, (int32)NewState);
    
    // Handle specific state transitions
    if (NewState == EAnim_MovementState::InAir && OldState != EAnim_MovementState::InAir)
    {
        // Started jumping/falling
        CalculateJumpParameters();
    }
}

void UAnim_PrehistoricAnimInstance::CalculateJumpParameters()
{
    if (!OwnerMovement)
        return;
    
    // Calculate time to jump apex
    float VerticalVelocity = OwnerMovement->Velocity.Z;
    float Gravity = FMath::Abs(OwnerMovement->GetGravityZ());
    
    if (VerticalVelocity > 0.0f)
    {
        TimeToJumpApex = VerticalVelocity / Gravity;
    }
    else
    {
        TimeToJumpApex = 0.0f;
    }
    
    // Calculate jump play rate based on jump height
    JumpPlayRate = FMath::Clamp(VerticalVelocity / 500.0f, 0.5f, 2.0f);
}

void UAnim_PrehistoricAnimInstance::SetLookAtTarget(const FVector& Target, float Alpha)
{
    LookAtTarget = Target;
    LookAtAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
}

void UAnim_PrehistoricAnimInstance::SetAlertnessLevel(float NewAlertnessLevel)
{
    AlertnessLevel = FMath::Clamp(NewAlertnessLevel, 0.0f, 1.0f);
}

void UAnim_PrehistoricAnimInstance::SetCombatState(EAnim_CombatState NewCombatState)
{
    if (CombatState != NewCombatState)
    {
        EAnim_CombatState OldState = CombatState;
        CombatState = NewCombatState;
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), (int32)OldState, (int32)NewCombatState);
    }
}

void UAnim_PrehistoricAnimInstance::SetSurvivalState(EAnim_SurvivalState NewSurvivalState)
{
    if (SurvivalState != NewSurvivalState)
    {
        EAnim_SurvivalState OldState = SurvivalState;
        SurvivalState = NewSurvivalState;
        
        UE_LOG(LogTemp, Log, TEXT("Survival state changed from %d to %d"), (int32)OldState, (int32)NewSurvivalState);
    }
}