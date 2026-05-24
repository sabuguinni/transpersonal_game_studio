#include "Anim_LocomotionBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_LocomotionBlueprint::UAnim_LocomotionBlueprint()
{
    // Set default thresholds
    WalkThreshold = 10.0f;
    RunThreshold = 300.0f;
    JumpThreshold = 100.0f;
    
    // Initialize references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Initialize animation assets to nullptr
    IdleAnimation = nullptr;
    WalkAnimation = nullptr;
    RunAnimation = nullptr;
    JumpStartAnimation = nullptr;
    JumpLoopAnimation = nullptr;
    JumpEndAnimation = nullptr;
    CrouchIdleAnimation = nullptr;
    CrouchWalkAnimation = nullptr;
}

void UAnim_LocomotionBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("Anim_LocomotionBlueprint: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Anim_LocomotionBlueprint: Failed to get owner character"));
    }
}

void UAnim_LocomotionBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateLocomotionData();
    UpdateAnimationState();
}

void UAnim_LocomotionBlueprint::UpdateLocomotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    LocomotionData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (LocomotionData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        LocomotionData.Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        LocomotionData.Direction = 0.0f;
    }
    
    // Update air state
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Update jump/fall data
    if (LocomotionData.bIsInAir)
    {
        LocomotionData.FallVelocity = Velocity.Z;
        LocomotionData.JumpHeight = OwnerCharacter->GetActorLocation().Z;
    }
    else
    {
        LocomotionData.FallVelocity = 0.0f;
        LocomotionData.JumpHeight = 0.0f;
    }
}

void UAnim_LocomotionBlueprint::UpdateAnimationState()
{
    EAnim_LocomotionState NewState = CalculateLocomotionState();
    
    if (NewState != LocomotionData.CurrentState)
    {
        LocomotionData.CurrentState = NewState;
        
        // Log state changes for debugging
        FString StateName;
        switch (NewState)
        {
            case EAnim_LocomotionState::Idle: StateName = TEXT("Idle"); break;
            case EAnim_LocomotionState::Walking: StateName = TEXT("Walking"); break;
            case EAnim_LocomotionState::Running: StateName = TEXT("Running"); break;
            case EAnim_LocomotionState::Jumping: StateName = TEXT("Jumping"); break;
            case EAnim_LocomotionState::Falling: StateName = TEXT("Falling"); break;
            case EAnim_LocomotionState::Landing: StateName = TEXT("Landing"); break;
            case EAnim_LocomotionState::Crouching: StateName = TEXT("Crouching"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Animation state changed to: %s (Speed: %.2f)"), 
               *StateName, LocomotionData.Speed);
    }
}

EAnim_LocomotionState UAnim_LocomotionBlueprint::CalculateLocomotionState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_LocomotionState::Idle;
    }
    
    // Check for crouching first
    if (LocomotionData.bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }
    
    // Check for air states
    if (LocomotionData.bIsInAir)
    {
        if (LocomotionData.FallVelocity > JumpThreshold)
        {
            return EAnim_LocomotionState::Jumping;
        }
        else if (LocomotionData.FallVelocity < -JumpThreshold)
        {
            return EAnim_LocomotionState::Falling;
        }
        else
        {
            return EAnim_LocomotionState::Jumping; // Default air state
        }
    }
    
    // Ground movement states
    if (LocomotionData.Speed < WalkThreshold)
    {
        return EAnim_LocomotionState::Idle;
    }
    else if (LocomotionData.Speed < RunThreshold)
    {
        return EAnim_LocomotionState::Walking;
    }
    else
    {
        return EAnim_LocomotionState::Running;
    }
}

void UAnim_LocomotionBlueprint::SetAnimationSequence(EAnim_LocomotionState State, UAnimSequence* Animation)
{
    if (!Animation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Trying to set null animation for state"));
        return;
    }
    
    switch (State)
    {
        case EAnim_LocomotionState::Idle:
            IdleAnimation = Animation;
            break;
        case EAnim_LocomotionState::Walking:
            WalkAnimation = Animation;
            break;
        case EAnim_LocomotionState::Running:
            RunAnimation = Animation;
            break;
        case EAnim_LocomotionState::Jumping:
            JumpStartAnimation = Animation;
            break;
        case EAnim_LocomotionState::Crouching:
            CrouchIdleAnimation = Animation;
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown animation state"));
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set animation sequence for state: %d"), (int32)State);
}

bool UAnim_LocomotionBlueprint::ShouldPlayIdleAnimation() const
{
    return LocomotionData.CurrentState == EAnim_LocomotionState::Idle && IdleAnimation != nullptr;
}

bool UAnim_LocomotionBlueprint::ShouldPlayWalkAnimation() const
{
    return LocomotionData.CurrentState == EAnim_LocomotionState::Walking && WalkAnimation != nullptr;
}

bool UAnim_LocomotionBlueprint::ShouldPlayRunAnimation() const
{
    return LocomotionData.CurrentState == EAnim_LocomotionState::Running && RunAnimation != nullptr;
}

bool UAnim_LocomotionBlueprint::ShouldPlayJumpAnimation() const
{
    return (LocomotionData.CurrentState == EAnim_LocomotionState::Jumping || 
            LocomotionData.CurrentState == EAnim_LocomotionState::Falling) && 
           JumpStartAnimation != nullptr;
}

bool UAnim_LocomotionBlueprint::ShouldPlayCrouchAnimation() const
{
    return LocomotionData.CurrentState == EAnim_LocomotionState::Crouching && CrouchIdleAnimation != nullptr;
}