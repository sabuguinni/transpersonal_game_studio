#include "PrimitiveAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    PreviousMovementState = EAnim_MovementState::Idle;
    PreviousCombatState = EAnim_CombatState::Unarmed;
    
    // Initialize timing
    LastStateChangeTime = 0.0f;
    
    // Set default thresholds
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    StateTransitionTime = 0.2f;
    
    // Initialize animation assets to null
    IdleMontage = nullptr;
    WalkMontage = nullptr;
    RunMontage = nullptr;
    JumpMontage = nullptr;
    AttackMontage = nullptr;
    MovementBlendSpace = nullptr;
    AnimInstance = nullptr;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the animation instance from the character's mesh
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
            if (AnimInstance)
            {
                UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Found AnimInstance for %s"), *Character->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No AnimInstance found for %s"), *Character->GetName());
            }
        }
    }
    
    // Initialize data structures
    MovementData = FAnim_MovementData();
    CombatData = FAnim_CombatData();
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update movement data from character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            // Update movement data
            FVector Velocity = MovementComp->Velocity;
            MovementData.Speed = Velocity.Size2D();
            MovementData.Direction = FMath::Atan2(Velocity.Y, Velocity.X);
            MovementData.bIsInAir = MovementComp->IsFalling();
            MovementData.bIsCrouching = MovementComp->IsCrouching();
            
            // Calculate ground distance for landing animations
            if (MovementData.bIsInAir)
            {
                FHitResult HitResult;
                FVector StartLocation = Character->GetActorLocation();
                FVector EndLocation = StartLocation - FVector(0, 0, 1000.0f);
                
                if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
                {
                    MovementData.GroundDistance = HitResult.Distance;
                }
                else
                {
                    MovementData.GroundDistance = 1000.0f;
                }
            }
            else
            {
                MovementData.GroundDistance = 0.0f;
            }
        }
    }
    
    // Determine new states based on current data
    DetermineMovementState();
    DetermineCombatState();
    
    // Handle state transitions
    HandleStateTransitions();
}

void UPrimitiveAnimationController::UpdateMovementState(const FAnim_MovementData& NewMovementData)
{
    MovementData = NewMovementData;
    DetermineMovementState();
}

void UPrimitiveAnimationController::UpdateCombatState(const FAnim_CombatData& NewCombatData)
{
    CombatData = NewCombatData;
    DetermineCombatState();
}

void UPrimitiveAnimationController::TriggerMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Cannot play montage - missing AnimInstance or Montage"));
        return;
    }
    
    // Stop current montages before playing new one
    AnimInstance->StopAllMontages(0.2f);
    
    // Play the new montage
    float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
    if (Duration > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing montage %s for %.2f seconds"), 
               *Montage->GetName(), Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Failed to play montage %s"), 
               *Montage->GetName());
    }
}

void UPrimitiveAnimationController::StopAllMontages()
{
    if (AnimInstance)
    {
        AnimInstance->StopAllMontages(StateTransitionTime);
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Stopped all montages"));
    }
}

void UPrimitiveAnimationController::DetermineMovementState()
{
    EAnim_MovementState NewState = CurrentMovementState;
    
    // Determine state based on movement data
    if (MovementData.bIsInAir)
    {
        if (MovementData.Speed > 50.0f)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.Speed < 10.0f)
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Running;
    }
    
    // Update state if changed
    if (NewState != CurrentMovementState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Movement state changed from %d to %d"), 
               (int32)PreviousMovementState, (int32)CurrentMovementState);
    }
}

void UPrimitiveAnimationController::DetermineCombatState()
{
    EAnim_CombatState NewState = CurrentCombatState;
    
    // Determine state based on combat data
    if (CombatData.bIsAttacking)
    {
        NewState = EAnim_CombatState::Attacking;
    }
    else if (CombatData.bIsBlocking)
    {
        NewState = EAnim_CombatState::Blocking;
    }
    else if (CombatData.bHasWeapon)
    {
        NewState = EAnim_CombatState::MeleeReady;
    }
    else
    {
        NewState = EAnim_CombatState::Unarmed;
    }
    
    // Update state if changed
    if (NewState != CurrentCombatState)
    {
        PreviousCombatState = CurrentCombatState;
        CurrentCombatState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Combat state changed from %d to %d"), 
               (int32)PreviousCombatState, (int32)CurrentCombatState);
    }
}

void UPrimitiveAnimationController::HandleStateTransitions()
{
    if (!AnimInstance)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastChange = CurrentTime - LastStateChangeTime;
    
    // Only handle transitions if enough time has passed for smooth blending
    if (TimeSinceLastChange < StateTransitionTime)
    {
        return;
    }
    
    // Handle movement state transitions
    switch (CurrentMovementState)
    {
        case EAnim_MovementState::Idle:
            if (IdleMontage && PreviousMovementState != EAnim_MovementState::Idle)
            {
                TriggerMontage(IdleMontage, 1.0f);
            }
            break;
            
        case EAnim_MovementState::Walking:
            if (WalkMontage && PreviousMovementState != EAnim_MovementState::Walking)
            {
                TriggerMontage(WalkMontage, MovementData.Speed / WalkSpeedThreshold);
            }
            break;
            
        case EAnim_MovementState::Running:
            if (RunMontage && PreviousMovementState != EAnim_MovementState::Running)
            {
                TriggerMontage(RunMontage, MovementData.Speed / RunSpeedThreshold);
            }
            break;
            
        case EAnim_MovementState::Jumping:
        case EAnim_MovementState::Falling:
            if (JumpMontage && PreviousMovementState != EAnim_MovementState::Jumping && 
                PreviousMovementState != EAnim_MovementState::Falling)
            {
                TriggerMontage(JumpMontage, 1.0f);
            }
            break;
            
        default:
            break;
    }
    
    // Handle combat state transitions
    switch (CurrentCombatState)
    {
        case EAnim_CombatState::Attacking:
            if (AttackMontage && PreviousCombatState != EAnim_CombatState::Attacking)
            {
                TriggerMontage(AttackMontage, CombatData.AttackSpeed);
            }
            break;
            
        default:
            break;
    }
}