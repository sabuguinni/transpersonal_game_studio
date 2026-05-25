#include "Anim_SurvivalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_SurvivalAnimInstance::UAnim_SurvivalAnimInstance()
{
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentSurvivalAction = EAnim_SurvivalAction::None;
    
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsMoving = false;
    bIsInAir = false;
    bIsCrouching = false;
    bIsClimbing = false;
    bIsSwimming = false;
    
    bIsHunting = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsInCombat = false;
    StaminaLevel = 100.0f;
    HealthLevel = 100.0f;
    
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    
    LocomotionBlendSpace = nullptr;
    IdleAnimation = nullptr;
    JumpStartAnimation = nullptr;
    JumpLoopAnimation = nullptr;
    JumpEndAnimation = nullptr;
}

void UAnim_SurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_SurvivalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMovementState();
    UpdateMotionMatchingData();
    UpdateIKFootPlacement();
}

void UAnim_SurvivalAnimInstance::UpdateMovementState()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get movement data
    FVector Velocity = MovementComponent->Velocity;
    MovementSpeed = Velocity.Size();
    bIsMoving = MovementSpeed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate movement direction relative to actor
    if (bIsMoving)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        MovementDirection = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, NormalizedVelocity)));
        
        // Determine if moving left or right
        FVector RightVector = OwningCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        if (RightDot < 0.0f)
        {
            MovementDirection = -MovementDirection;
        }
    }
    else
    {
        MovementDirection = 0.0f;
    }
    
    // Update movement state
    if (bIsInAir)
    {
        if (Velocity.Z > 0.0f)
        {
            CurrentMovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        CurrentMovementState = EAnim_MovementState::Crouching;
    }
    else if (bIsMoving)
    {
        // Determine movement speed state
        float MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
        float SpeedRatio = MovementSpeed / MaxWalkSpeed;
        
        if (SpeedRatio > 0.8f)
        {
            CurrentMovementState = EAnim_MovementState::Sprinting;
        }
        else if (SpeedRatio > 0.4f)
        {
            CurrentMovementState = EAnim_MovementState::Running;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Walking;
        }
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_SurvivalAnimInstance::UpdateMotionMatchingData()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update motion matching data structure
    MotionData.Velocity = MovementComponent->Velocity;
    MotionData.Acceleration = MovementComponent->GetCurrentAcceleration();
    MotionData.Speed = MovementSpeed;
    MotionData.Direction = MovementDirection;
    MotionData.bIsMoving = bIsMoving;
    MotionData.bIsInAir = bIsInAir;
    MotionData.bIsCrouching = bIsCrouching;
}

void UAnim_SurvivalAnimInstance::UpdateIKFootPlacement()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Perform foot IK calculations
    CalculateFootIK(TEXT("foot_l"), TEXT("ik_foot_l"), IKData.LeftFootAlpha, IKData.LeftFootLocation, IKData.LeftFootRotation);
    CalculateFootIK(TEXT("foot_r"), TEXT("ik_foot_r"), IKData.RightFootAlpha, IKData.RightFootLocation, IKData.RightFootRotation);
    
    // Calculate hip offset based on foot positions
    float LeftFootDistance = IKData.LeftFootLocation.Z;
    float RightFootDistance = IKData.RightFootLocation.Z;
    IKData.HipOffset = FMath::Min(LeftFootDistance, RightFootDistance);
}

void UAnim_SurvivalAnimInstance::CalculateFootIK(const FName& FootBoneName, const FName& IKBoneName, float& FootAlpha, FVector& FootLocation, FRotator& FootRotation)
{
    if (!OwningCharacter)
    {
        FootAlpha = 0.0f;
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        FootAlpha = 0.0f;
        return;
    }
    
    // Get foot bone location in world space
    FVector FootBoneLocation = MeshComp->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    // Perform line trace downward from foot
    float TraceDistance = 50.0f;
    FVector TraceStart = FootBoneLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector TraceEnd = FootBoneLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate foot adjustment
        float DistanceToGround = FVector::Dist(FootBoneLocation, HitResult.Location);
        FootLocation = HitResult.Location;
        FootRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();
        FootAlpha = FMath::Clamp(1.0f - (DistanceToGround / TraceDistance), 0.0f, 1.0f);
    }
    else
    {
        FootAlpha = 0.0f;
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
    }
}

FVector UAnim_SurvivalAnimInstance::PerformLineTrace(const FVector& StartLocation, float TraceDistance) const
{
    if (!GetWorld())
    {
        return FVector::ZeroVector;
    }
    
    FVector TraceEnd = StartLocation - FVector(0.0f, 0.0f, TraceDistance);
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    return bHit ? HitResult.Location : TraceEnd;
}

void UAnim_SurvivalAnimInstance::PlaySurvivalAction(EAnim_SurvivalAction Action)
{
    CurrentSurvivalAction = Action;
    
    // Update survival action flags
    bIsHunting = (Action == EAnim_SurvivalAction::Hunting);
    bIsGathering = (Action == EAnim_SurvivalAction::Gathering);
    bIsCrafting = (Action == EAnim_SurvivalAction::Crafting);
    bIsInCombat = (Action == EAnim_SurvivalAction::Combat);
    
    // Play corresponding montage if available
    if (SurvivalActionMontages.Contains(Action))
    {
        UAnimMontage* Montage = SurvivalActionMontages[Action];
        if (Montage)
        {
            Montage_Play(Montage);
        }
    }
}

void UAnim_SurvivalAnimInstance::StopSurvivalAction()
{
    CurrentSurvivalAction = EAnim_SurvivalAction::None;
    bIsHunting = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsInCombat = false;
    
    // Stop any playing montage
    if (IsAnyMontagePlaying())
    {
        Montage_Stop(0.2f);
    }
}

bool UAnim_SurvivalAnimInstance::IsSurvivalActionPlaying() const
{
    return CurrentSurvivalAction != EAnim_SurvivalAction::None;
}

void UAnim_SurvivalAnimInstance::SetMovementState(EAnim_MovementState NewState)
{
    CurrentMovementState = NewState;
}

void UAnim_SurvivalAnimInstance::EnableFootIK(bool bEnable)
{
    if (!bEnable)
    {
        IKData.LeftFootAlpha = 0.0f;
        IKData.RightFootAlpha = 0.0f;
        IKData.HipOffset = 0.0f;
    }
}

float UAnim_SurvivalAnimInstance::GetMovementSpeedRatio() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }
    
    return MovementSpeed / MovementComponent->MaxWalkSpeed;
}

FVector UAnim_SurvivalAnimInstance::GetMovementDirection() const
{
    if (!OwningCharacter || !bIsMoving)
    {
        return FVector::ZeroVector;
    }
    
    return OwningCharacter->GetActorTransform().InverseTransformVectorNoScale(MovementComponent->Velocity.GetSafeNormal());
}