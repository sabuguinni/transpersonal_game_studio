#include "Anim_MotionMatchingSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching parameters
    VelocityWeight = 1.0f;
    DirectionWeight = 0.8f;
    StateWeight = 1.2f;

    // Initialize IK parameters
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;

    // Initialize state
    CurrentMovementState = ESurvival_MovementState::Idle;

    // Initialize animation assets to null
    LocomotionBlendSpace = nullptr;
    IdleMontage = nullptr;
    WalkMontage = nullptr;
    RunMontage = nullptr;
    JumpMontage = nullptr;
    CrouchMontage = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Build animation database on start
    BuildAnimationDatabase();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update foot IK every frame
    UpdateFootIK(DeltaTime);

    // Update motion data from character movement
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector CurrentVelocity = MovementComp->Velocity;
            bool bInAir = MovementComp->IsFalling();
            bool bCrouching = MovementComp->IsCrouching();

            UpdateMotionData(CurrentVelocity, bInAir, bCrouching);
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& CurrentVelocity, bool bInAir, bool bCrouching)
{
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    CurrentMotionData.bIsInAir = bInAir;
    CurrentMotionData.bIsCrouching = bCrouching;

    // Calculate movement direction relative to actor forward
    if (AActor* Owner = GetOwner())
    {
        FVector ForwardVector = Owner->GetActorForwardVector();
        FVector VelocityNormalized = CurrentVelocity.GetSafeNormal();
        CurrentMotionData.Direction = FVector::DotProduct(ForwardVector, VelocityNormalized);
    }

    // Determine movement state based on motion data
    ESurvival_MovementState NewState = ESurvival_MovementState::Idle;

    if (bInAir)
    {
        NewState = ESurvival_MovementState::Jumping;
    }
    else if (bCrouching)
    {
        NewState = ESurvival_MovementState::Crouching;
    }
    else if (CurrentMotionData.Speed > 300.0f)
    {
        NewState = ESurvival_MovementState::Running;
    }
    else if (CurrentMotionData.Speed > 50.0f)
    {
        NewState = ESurvival_MovementState::Walking;
    }
    else
    {
        NewState = ESurvival_MovementState::Idle;
    }

    if (NewState != CurrentMovementState)
    {
        SetMovementState(NewState);
    }

    CurrentMotionData.MovementState = CurrentMovementState;
}

FAnim_StateData UAnim_MotionMatchingSystem::FindBestMatchingAnimation(const FAnim_MotionData& TargetMotion)
{
    if (AnimationDatabase.Num() == 0)
    {
        return FAnim_StateData();
    }

    float BestScore = FLT_MAX;
    int32 BestIndex = 0;

    for (int32 i = 0; i < AnimationDatabase.Num(); i++)
    {
        float Score = CalculateMotionScore(TargetMotion, AnimationDatabase[i].MotionData);
        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = i;
        }
    }

    return AnimationDatabase[BestIndex];
}

void UAnim_MotionMatchingSystem::BlendToAnimation(UAnimSequence* TargetAnimation, float BlendTime)
{
    if (!TargetAnimation)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetOwnerSkeletalMesh();
    if (!MeshComp || !MeshComp->GetAnimInstance())
    {
        return;
    }

    // Play the animation with blend time
    MeshComp->GetAnimInstance()->Montage_Play(nullptr, 1.0f / BlendTime);
}

void UAnim_MotionMatchingSystem::SetMovementState(ESurvival_MovementState NewState)
{
    if (CurrentMovementState == NewState)
    {
        return;
    }

    CurrentMovementState = NewState;

    // Play appropriate montage based on state
    UAnimMontage* MontageToPlay = nullptr;

    switch (NewState)
    {
        case ESurvival_MovementState::Idle:
            MontageToPlay = IdleMontage;
            break;
        case ESurvival_MovementState::Walking:
            MontageToPlay = WalkMontage;
            break;
        case ESurvival_MovementState::Running:
            MontageToPlay = RunMontage;
            break;
        case ESurvival_MovementState::Jumping:
            MontageToPlay = JumpMontage;
            break;
        case ESurvival_MovementState::Crouching:
            MontageToPlay = CrouchMontage;
            break;
        default:
            break;
    }

    if (MontageToPlay)
    {
        PlayMontage(MontageToPlay);
    }
}

void UAnim_MotionMatchingSystem::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetOwnerSkeletalMesh();
    if (!MeshComp || !MeshComp->GetAnimInstance())
    {
        return;
    }

    MeshComp->GetAnimInstance()->Montage_Play(Montage, PlayRate);
}

void UAnim_MotionMatchingSystem::UpdateFootIK(float DeltaTime)
{
    // Perform foot IK traces
    FVector NewLeftFootOffset;
    FVector NewRightFootOffset;

    PerformFootTrace(false, NewLeftFootOffset);  // Left foot
    PerformFootTrace(true, NewRightFootOffset);  // Right foot

    // Interpolate to smooth IK movement
    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, NewLeftFootOffset, DeltaTime, FootIKInterpSpeed);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, NewRightFootOffset, DeltaTime, FootIKInterpSpeed);
}

FVector UAnim_MotionMatchingSystem::GetFootIKOffset(bool bRightFoot) const
{
    return bRightFoot ? RightFootIKOffset : LeftFootIKOffset;
}

void UAnim_MotionMatchingSystem::BuildAnimationDatabase()
{
    AnimationDatabase.Empty();

    // Add idle animation data
    if (IdleMontage)
    {
        FAnim_StateData IdleData;
        IdleData.MotionData.MovementState = ESurvival_MovementState::Idle;
        IdleData.MotionData.Speed = 0.0f;
        IdleData.PlayRate = 1.0f;
        IdleData.BlendWeight = 1.0f;
        AddAnimationToDatabase(nullptr, IdleData.MotionData);
    }

    // Add walking animation data
    if (WalkMontage)
    {
        FAnim_StateData WalkData;
        WalkData.MotionData.MovementState = ESurvival_MovementState::Walking;
        WalkData.MotionData.Speed = 150.0f;
        WalkData.PlayRate = 1.0f;
        WalkData.BlendWeight = 1.0f;
        AddAnimationToDatabase(nullptr, WalkData.MotionData);
    }

    // Add running animation data
    if (RunMontage)
    {
        FAnim_StateData RunData;
        RunData.MotionData.MovementState = ESurvival_MovementState::Running;
        RunData.MotionData.Speed = 400.0f;
        RunData.PlayRate = 1.0f;
        RunData.BlendWeight = 1.0f;
        AddAnimationToDatabase(nullptr, RunData.MotionData);
    }

    // Add jumping animation data
    if (JumpMontage)
    {
        FAnim_StateData JumpData;
        JumpData.MotionData.MovementState = ESurvival_MovementState::Jumping;
        JumpData.MotionData.bIsInAir = true;
        JumpData.PlayRate = 1.0f;
        JumpData.BlendWeight = 1.0f;
        AddAnimationToDatabase(nullptr, JumpData.MotionData);
    }

    UE_LOG(LogTemp, Log, TEXT("Animation database built with %d entries"), AnimationDatabase.Num());
}

void UAnim_MotionMatchingSystem::AddAnimationToDatabase(UAnimSequence* Animation, const FAnim_MotionData& MotionData)
{
    FAnim_StateData NewEntry;
    NewEntry.AnimSequence = Animation;
    NewEntry.MotionData = MotionData;
    NewEntry.PlayRate = 1.0f;
    NewEntry.BlendWeight = 1.0f;

    AnimationDatabase.Add(NewEntry);
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B) const
{
    float Score = 0.0f;

    // Velocity difference
    float VelocityDiff = FVector::Dist(A.Velocity, B.Velocity);
    Score += VelocityDiff * VelocityWeight;

    // Speed difference
    float SpeedDiff = FMath::Abs(A.Speed - B.Speed);
    Score += SpeedDiff * VelocityWeight;

    // Direction difference
    float DirectionDiff = FMath::Abs(A.Direction - B.Direction);
    Score += DirectionDiff * DirectionWeight;

    // State matching
    if (A.MovementState != B.MovementState)
    {
        Score += 1000.0f * StateWeight;  // Heavy penalty for state mismatch
    }

    // Air state matching
    if (A.bIsInAir != B.bIsInAir)
    {
        Score += 500.0f * StateWeight;
    }

    // Crouch state matching
    if (A.bIsCrouching != B.bIsCrouching)
    {
        Score += 300.0f * StateWeight;
    }

    return Score;
}

void UAnim_MotionMatchingSystem::PerformFootTrace(bool bRightFoot, FVector& OutIKOffset) const
{
    OutIKOffset = FVector::ZeroVector;

    USkeletalMeshComponent* MeshComp = GetOwnerSkeletalMesh();
    if (!MeshComp)
    {
        return;
    }

    // Get foot bone location
    FName FootBoneName = bRightFoot ? TEXT("foot_r") : TEXT("foot_l");
    FVector FootLocation = MeshComp->GetBoneLocation(FootBoneName);

    if (FootLocation.IsZero())
    {
        return;  // Bone not found
    }

    // Perform downward trace
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calculate IK offset
        float DistanceToGround = FVector::Dist(FootLocation, HitResult.Location);
        OutIKOffset = FVector(0, 0, DistanceToGround - FootIKTraceDistance);
    }
}

USkeletalMeshComponent* UAnim_MotionMatchingSystem::GetOwnerSkeletalMesh() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetMesh();
    }
    
    return GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
}