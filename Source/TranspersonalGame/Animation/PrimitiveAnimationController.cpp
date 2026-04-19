#include "PrimitiveAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    
    // Set default thresholds for prehistoric human movement
    WalkSpeedThreshold = 150.0f;  // Slower, more deliberate movement
    RunSpeedThreshold = 400.0f;   // Sprint speed for escaping predators
    JumpVelocityThreshold = 50.0f;
    
    // Initialize blend data with realistic timing
    IdleBlendData.BlendTime = 0.3f;
    IdleBlendData.PlayRate = 1.0f;
    IdleBlendData.bLooping = true;
    
    WalkBlendData.BlendTime = 0.2f;
    WalkBlendData.PlayRate = 1.0f;
    WalkBlendData.bLooping = true;
    
    RunBlendData.BlendTime = 0.15f;
    RunBlendData.PlayRate = 1.2f;  // Slightly faster for urgency
    RunBlendData.bLooping = true;
    
    JumpBlendData.BlendTime = 0.1f;
    JumpBlendData.PlayRate = 1.0f;
    JumpBlendData.bLooping = false;
    
    bDebugAnimationStates = false;
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSkeletalMeshComponent();
    
    if (bDebugAnimationStates)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: BeginPlay - Component initialized"));
    }
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle state transition timing
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        
        FAnim_BlendData CurrentBlendData = GetBlendDataForState(CurrentMovementState);
        if (StateTransitionTimer >= CurrentBlendData.BlendTime)
        {
            bIsTransitioning = false;
            StateTransitionTimer = 0.0f;
            
            if (bDebugAnimationStates)
            {
                UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Transition complete to %s"), 
                    *UEnum::GetValueAsString(CurrentMovementState));
            }
        }
    }
    
    // Auto-update animation based on character movement if we have a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            bool bIsJumping = MovementComp->IsFalling();
            bool bIsCrouching = MovementComp->IsCrouching();
            
            UpdateAnimationFromMovement(Velocity, bIsJumping, bIsCrouching);
        }
    }
}

void UPrimitiveAnimationController::InitializeSkeletalMeshComponent()
{
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
            
            if (bDebugAnimationStates)
            {
                UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Found SkeletalMeshComponent and AnimInstance"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PrimitiveAnimationController: No SkeletalMeshComponent found on owner %s"), 
                *Owner->GetName());
        }
    }
}

void UPrimitiveAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        HandleStateTransition(NewState);
    }
}

void UPrimitiveAnimationController::UpdateAnimationFromMovement(const FVector& Velocity, bool bIsJumping, bool bIsCrouching)
{
    EAnim_MovementState NewState = CalculateMovementStateFromVelocity(Velocity, bIsJumping, bIsCrouching);
    SetMovementState(NewState);
}

EAnim_MovementState UPrimitiveAnimationController::CalculateMovementStateFromVelocity(const FVector& Velocity, bool bIsJumping, bool bIsCrouching)
{
    // Handle jumping/falling first
    if (bIsJumping || Velocity.Z > JumpVelocityThreshold)
    {
        return EAnim_MovementState::Jumping;
    }
    
    if (Velocity.Z < -JumpVelocityThreshold)
    {
        return EAnim_MovementState::Falling;
    }
    
    // Handle crouching states
    if (bIsCrouching)
    {
        float HorizontalSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
        if (HorizontalSpeed > WalkSpeedThreshold * 0.5f) // Slower threshold for crouching
        {
            return EAnim_MovementState::Crawling;
        }
        else
        {
            return EAnim_MovementState::Crouching;
        }
    }
    
    // Handle normal movement states
    float HorizontalSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    
    if (HorizontalSpeed > RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else if (HorizontalSpeed > WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Idle;
    }
}

void UPrimitiveAnimationController::HandleStateTransition(EAnim_MovementState NewState)
{
    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    
    bIsTransitioning = true;
    StateTransitionTimer = 0.0f;
    
    ApplyAnimationForState(CurrentMovementState);
    
    if (bDebugAnimationStates)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: State transition from %s to %s"), 
            *UEnum::GetValueAsString(PreviousMovementState),
            *UEnum::GetValueAsString(CurrentMovementState));
    }
}

void UPrimitiveAnimationController::ApplyAnimationForState(EAnim_MovementState State)
{
    if (!SkeletalMeshComponent || !AnimInstance)
    {
        return;
    }
    
    UAnimSequence* AnimToPlay = nullptr;
    FAnim_BlendData BlendData = GetBlendDataForState(State);
    
    // Select appropriate animation based on state
    switch (State)
    {
        case EAnim_MovementState::Idle:
            AnimToPlay = IdleAnimation;
            break;
            
        case EAnim_MovementState::Walking:
            AnimToPlay = WalkAnimation;
            break;
            
        case EAnim_MovementState::Running:
            AnimToPlay = RunAnimation;
            break;
            
        case EAnim_MovementState::Jumping:
            AnimToPlay = JumpStartAnimation;
            break;
            
        case EAnim_MovementState::Falling:
            AnimToPlay = JumpLoopAnimation;
            break;
            
        case EAnim_MovementState::Landing:
            AnimToPlay = JumpLandAnimation;
            break;
            
        case EAnim_MovementState::Crouching:
            AnimToPlay = CrouchIdleAnimation;
            break;
            
        case EAnim_MovementState::Crawling:
            AnimToPlay = CrouchWalkAnimation;
            break;
            
        default:
            AnimToPlay = IdleAnimation;
            break;
    }
    
    // Play the animation if we have one
    if (AnimToPlay)
    {
        if (UAnimSingleNodeInstance* SingleNodeInstance = Cast<UAnimSingleNodeInstance>(AnimInstance))
        {
            SingleNodeInstance->SetAnimationAsset(AnimToPlay);
            SingleNodeInstance->SetPlayRate(BlendData.PlayRate);
            SingleNodeInstance->SetLooping(BlendData.bLooping);
            
            if (bDebugAnimationStates)
            {
                UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing animation %s for state %s"), 
                    *AnimToPlay->GetName(),
                    *UEnum::GetValueAsString(State));
            }
        }
    }
    else if (bDebugAnimationStates)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No animation asset for state %s"), 
            *UEnum::GetValueAsString(State));
    }
}

FAnim_BlendData UPrimitiveAnimationController::GetBlendDataForState(EAnim_MovementState State)
{
    switch (State)
    {
        case EAnim_MovementState::Idle:
        case EAnim_MovementState::Crouching:
            return IdleBlendData;
            
        case EAnim_MovementState::Walking:
        case EAnim_MovementState::Crawling:
            return WalkBlendData;
            
        case EAnim_MovementState::Running:
            return RunBlendData;
            
        case EAnim_MovementState::Jumping:
        case EAnim_MovementState::Falling:
        case EAnim_MovementState::Landing:
            return JumpBlendData;
            
        default:
            return IdleBlendData;
    }
}

bool UPrimitiveAnimationController::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return false;
    }
    
    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate);
    
    if (bDebugAnimationStates)
    {
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Playing montage %s (length: %f)"), 
            *Montage->GetName(), MontageLength);
    }
    
    return MontageLength > 0.0f;
}

void UPrimitiveAnimationController::StopCurrentMontage(float BlendOutTime)
{
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(BlendOutTime);
        
        if (bDebugAnimationStates)
        {
            UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Stopped current montage"));
        }
    }
}

void UPrimitiveAnimationController::LogCurrentAnimationState()
{
    UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController Debug:"));
    UE_LOG(LogTemp, Warning, TEXT("  Current State: %s"), *UEnum::GetValueAsString(CurrentMovementState));
    UE_LOG(LogTemp, Warning, TEXT("  Previous State: %s"), *UEnum::GetValueAsString(PreviousMovementState));
    UE_LOG(LogTemp, Warning, TEXT("  Is Transitioning: %s"), bIsTransitioning ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Transition Timer: %f"), StateTransitionTimer);
    UE_LOG(LogTemp, Warning, TEXT("  Has SkeletalMesh: %s"), SkeletalMeshComponent ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Has AnimInstance: %s"), AnimInstance ? TEXT("Yes") : TEXT("No"));
}