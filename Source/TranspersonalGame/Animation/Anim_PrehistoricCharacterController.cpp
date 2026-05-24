#include "Anim_PrehistoricCharacterController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PrehistoricCharacterController::UAnim_PrehistoricCharacterController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize animation data
    AnimationData = FAnim_PrehistoricAnimationData();
    
    // Initialize state tracking
    PreviousMovementState = EAnim_PrehistoricMovementState::Idle;
    PreviousActionState = EAnim_PrehistoricActionState::None;
    PreviousSpeed = 0.0f;
    PreviousDirection = 0.0f;
    SmoothedDirection = 0.0f;
    SmoothedSpeed = 0.0f;
    
    // Set default thresholds
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    AnimationBlendSpeed = 10.0f;
    DirectionSmoothingSpeed = 5.0f;
    
    // Initialize survival modifiers
    FatigueAnimationScale = 1.0f;
    HungerAnimationScale = 1.0f;
    FearAnimationScale = 1.0f;
}

void UAnim_PrehistoricCharacterController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Prehistoric Character Animation Controller initialized for: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Prehistoric Character Animation Controller: Owner is not a Character!"));
    }
}

void UAnim_PrehistoricCharacterController::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateAnimationData(DeltaTime);
        SmoothAnimationValues(DeltaTime);
        ApplySurvivalModifiers();
        UpdateMovementState();
    }
}

void UAnim_PrehistoricCharacterController::UpdateAnimationData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
        return;
    
    // Update velocity and acceleration
    AnimationData.Velocity = MovementComponent->Velocity;
    AnimationData.Acceleration = MovementComponent->GetCurrentAcceleration();
    
    // Calculate speed
    AnimationData.Speed = AnimationData.Velocity.Size2D();
    
    // Calculate direction relative to character forward
    AnimationData.Direction = CalculateDirection();
    
    // Update movement flags
    AnimationData.bIsInAir = MovementComponent->IsFalling();
    AnimationData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate yaw delta for turning animations
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    static FRotator PreviousRotation = CurrentRotation;
    AnimationData.YawDelta = UKismetMathLibrary::NormalizeAxis(CurrentRotation.Yaw - PreviousRotation.Yaw);
    PreviousRotation = CurrentRotation;
}

void UAnim_PrehistoricCharacterController::SmoothAnimationValues(float DeltaTime)
{
    // Smooth direction for better animation blending
    SmoothedDirection = FMath::FInterpTo(SmoothedDirection, AnimationData.Direction, 
                                        DeltaTime, DirectionSmoothingSpeed);
    AnimationData.Direction = SmoothedDirection;
    
    // Smooth speed for better animation blending
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, AnimationData.Speed, 
                                   DeltaTime, AnimationBlendSpeed);
    AnimationData.Speed = SmoothedSpeed;
}

void UAnim_PrehistoricCharacterController::ApplySurvivalModifiers()
{
    // Apply fatigue to movement speed
    if (AnimationData.Fatigue > 0.5f)
    {
        FatigueAnimationScale = FMath::Lerp(1.0f, 0.7f, (AnimationData.Fatigue - 0.5f) * 2.0f);
    }
    else
    {
        FatigueAnimationScale = 1.0f;
    }
    
    // Apply hunger to animation intensity
    if (AnimationData.Hunger > 0.7f)
    {
        HungerAnimationScale = FMath::Lerp(1.0f, 0.8f, (AnimationData.Hunger - 0.7f) * 3.33f);
    }
    else
    {
        HungerAnimationScale = 1.0f;
    }
    
    // Apply fear to movement urgency
    if (AnimationData.Fear > 0.3f)
    {
        FearAnimationScale = FMath::Lerp(1.0f, 1.5f, (AnimationData.Fear - 0.3f) * 1.43f);
    }
    else
    {
        FearAnimationScale = 1.0f;
    }
}

void UAnim_PrehistoricCharacterController::UpdateMovementState()
{
    EAnim_PrehistoricMovementState NewState = CalculateMovementState();
    
    if (NewState != PreviousMovementState)
    {
        AnimationData.MovementState = NewState;
        OnMovementStateChanged(NewState);
        PreviousMovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Movement State Changed to: %d"), (int32)NewState);
    }
}

EAnim_PrehistoricMovementState UAnim_PrehistoricCharacterController::CalculateMovementState() const
{
    if (!MovementComponent)
        return EAnim_PrehistoricMovementState::Idle;
    
    // Check air state first
    if (MovementComponent->IsFalling())
    {
        if (AnimationData.Velocity.Z > 100.0f)
            return EAnim_PrehistoricMovementState::Jumping;
        else
            return EAnim_PrehistoricMovementState::Falling;
    }
    
    // Check crouching
    if (MovementComponent->IsCrouching())
    {
        return EAnim_PrehistoricMovementState::Crouching;
    }
    
    // Check movement speed
    float Speed = AnimationData.Speed;
    if (Speed < WalkThreshold)
    {
        return EAnim_PrehistoricMovementState::Idle;
    }
    else if (Speed < RunThreshold)
    {
        return EAnim_PrehistoricMovementState::Walking;
    }
    else
    {
        return EAnim_PrehistoricMovementState::Running;
    }
}

float UAnim_PrehistoricCharacterController::CalculateDirection() const
{
    if (!OwnerCharacter || AnimationData.Speed < 1.0f)
        return 0.0f;
    
    // Get character forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Get velocity direction (normalized)
    FVector VelocityDirection = AnimationData.Velocity.GetSafeNormal2D();
    
    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    // Convert to angle in degrees (-180 to 180)
    float Angle = FMath::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
    
    return Angle;
}

void UAnim_PrehistoricCharacterController::SetActionState(EAnim_PrehistoricActionState NewActionState)
{
    if (NewActionState != PreviousActionState)
    {
        AnimationData.ActionState = NewActionState;
        OnActionStateChanged(NewActionState);
        PreviousActionState = NewActionState;
        
        UE_LOG(LogTemp, Log, TEXT("Action State Changed to: %d"), (int32)NewActionState);
        
        // Auto-play montage if available
        if (NewActionState != EAnim_PrehistoricActionState::None)
        {
            PlayActionMontage(NewActionState);
        }
    }
}

void UAnim_PrehistoricCharacterController::PlayActionMontage(EAnim_PrehistoricActionState ActionType)
{
    if (!OwnerCharacter)
        return;
    
    UAnimMontage* MontageToPlay = GetMontageForAction(ActionType);
    if (MontageToPlay)
    {
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            MeshComp->GetAnimInstance()->Montage_Play(MontageToPlay);
            UE_LOG(LogTemp, Log, TEXT("Playing action montage for action: %d"), (int32)ActionType);
        }
    }
}

void UAnim_PrehistoricCharacterController::StopActionMontage()
{
    if (!OwnerCharacter)
        return;
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        MeshComp->GetAnimInstance()->Montage_Stop(0.2f);
        UE_LOG(LogTemp, Log, TEXT("Stopped action montage"));
    }
    
    SetActionState(EAnim_PrehistoricActionState::None);
}

UAnimMontage* UAnim_PrehistoricCharacterController::GetMontageForAction(EAnim_PrehistoricActionState ActionType) const
{
    switch (ActionType)
    {
        case EAnim_PrehistoricActionState::Gathering:
            return GatheringMontage;
        case EAnim_PrehistoricActionState::Hunting:
            return HuntingMontage;
        case EAnim_PrehistoricActionState::Crafting:
            return CraftingMontage;
        case EAnim_PrehistoricActionState::Eating:
            return EatingMontage;
        case EAnim_PrehistoricActionState::Drinking:
            return DrinkingMontage;
        case EAnim_PrehistoricActionState::Combat:
            return CombatMontage;
        default:
            return nullptr;
    }
}

void UAnim_PrehistoricCharacterController::SetFatigue(float NewFatigue)
{
    float ClampedFatigue = FMath::Clamp(NewFatigue, 0.0f, 1.0f);
    if (FMath::Abs(AnimationData.Fatigue - ClampedFatigue) > 0.01f)
    {
        AnimationData.Fatigue = ClampedFatigue;
        OnSurvivalStateChanged(AnimationData.Fatigue, AnimationData.Hunger, AnimationData.Fear);
    }
}

void UAnim_PrehistoricCharacterController::SetHunger(float NewHunger)
{
    float ClampedHunger = FMath::Clamp(NewHunger, 0.0f, 1.0f);
    if (FMath::Abs(AnimationData.Hunger - ClampedHunger) > 0.01f)
    {
        AnimationData.Hunger = ClampedHunger;
        OnSurvivalStateChanged(AnimationData.Fatigue, AnimationData.Hunger, AnimationData.Fear);
    }
}

void UAnim_PrehistoricCharacterController::SetFear(float NewFear)
{
    float ClampedFear = FMath::Clamp(NewFear, 0.0f, 1.0f);
    if (FMath::Abs(AnimationData.Fear - ClampedFear) > 0.01f)
    {
        AnimationData.Fear = ClampedFear;
        OnSurvivalStateChanged(AnimationData.Fatigue, AnimationData.Hunger, AnimationData.Fear);
    }
}