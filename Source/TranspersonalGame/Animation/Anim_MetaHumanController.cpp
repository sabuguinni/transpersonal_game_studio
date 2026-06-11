#include "Anim_MetaHumanController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"

UAnim_MetaHumanController::UAnim_MetaHumanController()
{
    // Initialize default values
    AnimParams.Speed = 0.0f;
    AnimParams.Direction = 0.0f;
    AnimParams.CurrentState = EAnim_MetaHumanState::Idle;
    AnimParams.HealthPercent = 1.0f;
    AnimParams.StaminaPercent = 1.0f;
    AnimParams.FearLevel = 0.0f;
    AnimParams.bEnableFootIK = true;
    AnimParams.bEnableLookAtIK = true;
    AnimParams.bIsInCombat = false;
    AnimParams.bIsInteracting = false;

    FootIKInterpSpeed = 15.0f;
    LookAtIKInterpSpeed = 10.0f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LookAtRotation = FRotator::ZeroRotator;

    PreviousState = EAnim_MetaHumanState::Idle;
    StateTransitionTime = 0.0f;
    bIsTransitioning = false;
}

void UAnim_MetaHumanController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache the owning pawn
    if (APawn* Pawn = TryGetPawnOwner())
    {
        UE_LOG(LogTemp, Log, TEXT("MetaHuman Animation Controller initialized for: %s"), *Pawn->GetName());
    }
}

void UAnim_MetaHumanController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn)
    {
        return;
    }

    // Update movement parameters
    if (ACharacter* Character = Cast<ACharacter>(Pawn))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            AnimParams.Speed = Velocity.Size2D();
            
            // Calculate direction relative to character forward
            if (AnimParams.Speed > 0.1f)
            {
                FVector ForwardVector = Character->GetActorForwardVector();
                FVector VelocityNormalized = Velocity.GetSafeNormal2D();
                float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
                AnimParams.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
                
                // Determine if moving left or right
                FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized);
                if (CrossProduct.Z < 0.0f)
                {
                    AnimParams.Direction *= -1.0f;
                }
            }
            else
            {
                AnimParams.Direction = 0.0f;
            }

            // Update animation state based on speed
            if (AnimParams.Speed < 10.0f)
            {
                if (AnimParams.CurrentState != EAnim_MetaHumanState::Combat && 
                    AnimParams.CurrentState != EAnim_MetaHumanState::Crafting &&
                    AnimParams.CurrentState != EAnim_MetaHumanState::Gathering)
                {
                    SetAnimationState(EAnim_MetaHumanState::Idle);
                }
            }
            else if (AnimParams.Speed < 300.0f)
            {
                SetAnimationState(EAnim_MetaHumanState::Walking);
            }
            else
            {
                SetAnimationState(EAnim_MetaHumanState::Running);
            }

            // Check if jumping
            if (!MovementComp->IsMovingOnGround())
            {
                SetAnimationState(EAnim_MetaHumanState::Jumping);
            }
        }
    }

    // Update IK systems
    if (AnimParams.bEnableFootIK)
    {
        UpdateFootIK();
    }

    // Interpolate IK values for smooth transitions
    InterpolateIKValues(DeltaTimeX);

    // Handle state transitions
    if (bIsTransitioning)
    {
        StateTransitionTime += DeltaTimeX;
        if (StateTransitionTime >= 0.2f) // 200ms transition time
        {
            bIsTransitioning = false;
            StateTransitionTime = 0.0f;
        }
    }
}

void UAnim_MetaHumanController::SetAnimationState(EAnim_MetaHumanState NewState)
{
    if (AnimParams.CurrentState != NewState)
    {
        PreviousState = AnimParams.CurrentState;
        AnimParams.CurrentState = NewState;
        bIsTransitioning = true;
        StateTransitionTime = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("Animation state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_MetaHumanController::UpdateMovementParams(float InSpeed, float InDirection)
{
    AnimParams.Speed = InSpeed;
    AnimParams.Direction = InDirection;
}

void UAnim_MetaHumanController::UpdateSurvivalParams(float Health, float Stamina, float Fear)
{
    AnimParams.HealthPercent = FMath::Clamp(Health, 0.0f, 1.0f);
    AnimParams.StaminaPercent = FMath::Clamp(Stamina, 0.0f, 1.0f);
    AnimParams.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);

    // Update animation state based on survival parameters
    if (AnimParams.FearLevel > 0.7f)
    {
        SetAnimationState(EAnim_MetaHumanState::Fearful);
    }
    else if (AnimParams.HealthPercent < 0.3f)
    {
        SetAnimationState(EAnim_MetaHumanState::Injured);
    }
}

void UAnim_MetaHumanController::SetCombatMode(bool bInCombat)
{
    AnimParams.bIsInCombat = bInCombat;
    if (bInCombat)
    {
        SetAnimationState(EAnim_MetaHumanState::Combat);
    }
    else if (AnimParams.CurrentState == EAnim_MetaHumanState::Combat)
    {
        SetAnimationState(EAnim_MetaHumanState::Idle);
    }
}

void UAnim_MetaHumanController::SetInteractionMode(bool bInteracting)
{
    AnimParams.bIsInteracting = bInteracting;
    if (bInteracting)
    {
        // Determine interaction type based on context
        // This could be expanded to check what the character is interacting with
        SetAnimationState(EAnim_MetaHumanState::Crafting);
    }
    else if (AnimParams.CurrentState == EAnim_MetaHumanState::Crafting ||
             AnimParams.CurrentState == EAnim_MetaHumanState::Gathering)
    {
        SetAnimationState(EAnim_MetaHumanState::Idle);
    }
}

void UAnim_MetaHumanController::UpdateFootIK()
{
    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn || !AnimParams.bEnableFootIK)
    {
        return;
    }

    // Calculate foot IK offsets for terrain adaptation
    LeftFootIKOffset = CalculateFootIKOffset(FName("foot_l"));
    RightFootIKOffset = CalculateFootIKOffset(FName("foot_r"));
}

void UAnim_MetaHumanController::UpdateLookAtIK(FVector TargetLocation)
{
    if (!AnimParams.bEnableLookAtIK)
    {
        return;
    }

    LookAtRotation = CalculateLookAtRotation(TargetLocation);
}

void UAnim_MetaHumanController::EnableFootIK(bool bEnable)
{
    AnimParams.bEnableFootIK = bEnable;
    if (!bEnable)
    {
        LeftFootIKOffset = FVector::ZeroVector;
        RightFootIKOffset = FVector::ZeroVector;
    }
}

void UAnim_MetaHumanController::EnableLookAtIK(bool bEnable)
{
    AnimParams.bEnableLookAtIK = bEnable;
    if (!bEnable)
    {
        LookAtRotation = FRotator::ZeroRotator;
    }
}

void UAnim_MetaHumanController::PlayIdleVariation(int32 VariationIndex)
{
    if (IdleMontage)
    {
        Montage_Play(IdleMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing idle variation %d"), VariationIndex);
    }
}

void UAnim_MetaHumanController::PlayCombatAction(int32 ActionIndex)
{
    if (CombatMontage)
    {
        Montage_Play(CombatMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing combat action %d"), ActionIndex);
    }
}

void UAnim_MetaHumanController::PlayInteractionAction(int32 ActionIndex)
{
    if (InteractionMontage)
    {
        Montage_Play(InteractionMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing interaction action %d"), ActionIndex);
    }
}

void UAnim_MetaHumanController::PlaySurvivalAction(int32 ActionIndex)
{
    if (SurvivalMontage)
    {
        Montage_Play(SurvivalMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing survival action %d"), ActionIndex);
    }
}

FVector UAnim_MetaHumanController::CalculateFootIKOffset(const FName& SocketName)
{
    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn)
    {
        return FVector::ZeroVector;
    }

    USkeletalMeshComponent* MeshComp = Pawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }

    // Get foot socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0, 0, 50);
    FVector TraceEnd = SocketLocation - FVector(0, 0, 100);

    // Perform line trace to find ground
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Pawn);

    bool bHit = Pawn->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        float IKOffset = HitResult.Location.Z - SocketLocation.Z;
        return FVector(0, 0, IKOffset);
    }

    return FVector::ZeroVector;
}

FRotator UAnim_MetaHumanController::CalculateLookAtRotation(const FVector& TargetLocation)
{
    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn)
    {
        return FRotator::ZeroRotator;
    }

    FVector PawnLocation = Pawn->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    return UKismetMathLibrary::FindLookAtRotation(PawnLocation, TargetLocation);
}

void UAnim_MetaHumanController::InterpolateIKValues(float DeltaTime)
{
    // Smooth interpolation for IK values to prevent jittery movement
    static FVector PreviousLeftFootOffset = FVector::ZeroVector;
    static FVector PreviousRightFootOffset = FVector::ZeroVector;
    static FRotator PreviousLookAtRotation = FRotator::ZeroRotator;

    LeftFootIKOffset = FMath::VInterpTo(PreviousLeftFootOffset, LeftFootIKOffset, DeltaTime, FootIKInterpSpeed);
    RightFootIKOffset = FMath::VInterpTo(PreviousRightFootOffset, RightFootIKOffset, DeltaTime, FootIKInterpSpeed);
    LookAtRotation = FMath::RInterpTo(PreviousLookAtRotation, LookAtRotation, DeltaTime, LookAtIKInterpSpeed);

    PreviousLeftFootOffset = LeftFootIKOffset;
    PreviousRightFootOffset = RightFootIKOffset;
    PreviousLookAtRotation = LookAtRotation;
}

float UAnim_MetaHumanController::CalculateSpeedBlendAlpha() const
{
    // Convert speed to blend alpha for animation blending
    return FMath::Clamp(AnimParams.Speed / 600.0f, 0.0f, 1.0f);
}

float UAnim_MetaHumanController::CalculateDirectionBlendAlpha() const
{
    // Convert direction to blend alpha (-1 to 1 range)
    return FMath::Clamp(AnimParams.Direction / 180.0f, -1.0f, 1.0f);
}

float UAnim_MetaHumanController::CalculateSurvivalBlendAlpha() const
{
    // Combine survival parameters for animation blending
    float SurvivalFactor = (AnimParams.HealthPercent + AnimParams.StaminaPercent) * 0.5f;
    SurvivalFactor -= AnimParams.FearLevel * 0.3f; // Fear reduces overall performance
    return FMath::Clamp(SurvivalFactor, 0.0f, 1.0f);
}