#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS updates
    
    // Initialize motion matching weights
    VelocityWeight = 0.6f;
    DirectionWeight = 0.3f;
    AccelerationWeight = 0.1f;
    MinMatchThreshold = 0.7f;
    
    // Initialize state
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    SkeletalMeshComponent = nullptr;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    CachedPoseCandidates = BuildPoseCandidates();
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching Component initialized with %d pose candidates"), CachedPoseCandidates.Num());
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized || !OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update motion data every frame
    UpdateMotionData();
    
    // Find best matching pose
    FAnim_PoseCandidate BestPose = FindBestMatchingPose(CurrentMotionData);
    
    // Blend to new pose if it's significantly different
    if (BestPose.MatchScore > MinMatchThreshold && BestPose.AnimSequence != CurrentPose.AnimSequence)
    {
        BlendToPose(BestPose);
        CurrentPose = BestPose;
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionData()
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Get current velocity and movement state
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 10.0f;
    CurrentMotionData.bIsFalling = MovementComponent->IsFalling();
    CurrentMotionData.bIsJumping = MovementComponent->IsMovingOnGround() == false && CurrentVelocity.Z > 0.0f;
    
    // Calculate movement direction relative to character forward
    if (CurrentMotionData.bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        CurrentMotionData.Direction = FMath::Acos(DotProduct) * (180.0f / PI);
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Calculate acceleration
    static FVector PreviousVelocity = FVector::ZeroVector;
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        FVector AccelerationVector = (CurrentVelocity - PreviousVelocity) / DeltaTime;
        CurrentMotionData.Acceleration = AccelerationVector.Size();
    }
    PreviousVelocity = CurrentVelocity;
}

FAnim_PoseCandidate UAnim_MotionMatchingComponent::FindBestMatchingPose(const FAnim_MotionData& CurrentMotion)
{
    FAnim_PoseCandidate BestCandidate;
    float BestScore = 0.0f;
    
    for (const FAnim_PoseCandidate& Candidate : CachedPoseCandidates)
    {
        float Score = CalculatePoseMatchScore(CurrentMotion, Candidate.PoseMotionData);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestCandidate = Candidate;
            BestCandidate.MatchScore = Score;
        }
    }
    
    return BestCandidate;
}

float UAnim_MotionMatchingComponent::CalculatePoseMatchScore(const FAnim_MotionData& CurrentMotion, const FAnim_MotionData& PoseMotion)
{
    float VelocityScore = 1.0f - FMath::Abs(CurrentMotion.Speed - PoseMotion.Speed) / 600.0f; // Max speed assumption
    VelocityScore = FMath::Clamp(VelocityScore, 0.0f, 1.0f);
    
    float DirectionScore = 1.0f - FMath::Abs(CurrentMotion.Direction - PoseMotion.Direction) / 180.0f;
    DirectionScore = FMath::Clamp(DirectionScore, 0.0f, 1.0f);
    
    float AccelerationScore = 1.0f - FMath::Abs(CurrentMotion.Acceleration - PoseMotion.Acceleration) / 1000.0f; // Max accel assumption
    AccelerationScore = FMath::Clamp(AccelerationScore, 0.0f, 1.0f);
    
    // State matching bonuses
    float StateBonus = 0.0f;
    if (CurrentMotion.bIsMoving == PoseMotion.bIsMoving) StateBonus += 0.1f;
    if (CurrentMotion.bIsFalling == PoseMotion.bIsFalling) StateBonus += 0.1f;
    if (CurrentMotion.bIsJumping == PoseMotion.bIsJumping) StateBonus += 0.1f;
    
    // Weighted final score
    float FinalScore = (VelocityScore * VelocityWeight) + 
                      (DirectionScore * DirectionWeight) + 
                      (AccelerationScore * AccelerationWeight) + 
                      StateBonus;
    
    return FMath::Clamp(FinalScore, 0.0f, 1.0f);
}

void UAnim_MotionMatchingComponent::BlendToPose(const FAnim_PoseCandidate& TargetPose, float BlendTime)
{
    if (!TargetPose.AnimSequence || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Play the animation sequence at the specified start time
    SkeletalMeshComponent->GetAnimInstance()->Montage_Play(nullptr, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Blending to pose %s with score %f"), 
           *TargetPose.AnimSequence->GetName(), TargetPose.MatchScore);
}

void UAnim_MotionMatchingComponent::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
    }
}

TArray<FAnim_PoseCandidate> UAnim_MotionMatchingComponent::BuildPoseCandidates()
{
    TArray<FAnim_PoseCandidate> Candidates;
    
    // Build candidates from idle animations
    for (UAnimSequence* IdleAnim : IdleAnimations)
    {
        if (IdleAnim)
        {
            FAnim_PoseCandidate Candidate;
            Candidate.AnimSequence = IdleAnim;
            Candidate.StartTime = 0.0f;
            Candidate.PoseMotionData.Speed = 0.0f;
            Candidate.PoseMotionData.bIsMoving = false;
            Candidate.PoseMotionData.bIsFalling = false;
            Candidate.PoseMotionData.bIsJumping = false;
            Candidates.Add(Candidate);
        }
    }
    
    // Build candidates from walk animations
    for (UAnimSequence* WalkAnim : WalkAnimations)
    {
        if (WalkAnim)
        {
            FAnim_PoseCandidate Candidate;
            Candidate.AnimSequence = WalkAnim;
            Candidate.StartTime = 0.0f;
            Candidate.PoseMotionData.Speed = 150.0f; // Typical walk speed
            Candidate.PoseMotionData.bIsMoving = true;
            Candidate.PoseMotionData.bIsFalling = false;
            Candidate.PoseMotionData.bIsJumping = false;
            Candidates.Add(Candidate);
        }
    }
    
    // Build candidates from run animations
    for (UAnimSequence* RunAnim : RunAnimations)
    {
        if (RunAnim)
        {
            FAnim_PoseCandidate Candidate;
            Candidate.AnimSequence = RunAnim;
            Candidate.StartTime = 0.0f;
            Candidate.PoseMotionData.Speed = 400.0f; // Typical run speed
            Candidate.PoseMotionData.bIsMoving = true;
            Candidate.PoseMotionData.bIsFalling = false;
            Candidate.PoseMotionData.bIsJumping = false;
            Candidates.Add(Candidate);
        }
    }
    
    // Build candidates from jump animations
    for (UAnimSequence* JumpAnim : JumpAnimations)
    {
        if (JumpAnim)
        {
            FAnim_PoseCandidate Candidate;
            Candidate.AnimSequence = JumpAnim;
            Candidate.StartTime = 0.0f;
            Candidate.PoseMotionData.Speed = 200.0f; // Speed during jump
            Candidate.PoseMotionData.bIsMoving = true;
            Candidate.PoseMotionData.bIsFalling = false;
            Candidate.PoseMotionData.bIsJumping = true;
            Candidates.Add(Candidate);
        }
    }
    
    return Candidates;
}