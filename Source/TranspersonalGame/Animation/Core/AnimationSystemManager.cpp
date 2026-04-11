#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animation/AnimNodeBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogAnimationSystem);

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Motion Matching defaults
    MotionMatchingUpdateRate = 30.0f; // 30 FPS for motion matching
    PoseSearchRadius = 100.0f;
    MaxDatabaseSize = 10000;
    
    // IK defaults
    FootIKInterpSpeed = 15.0f;
    GroundTraceDistance = 200.0f;
    bEnableFootIK = true;
    
    // Animation state
    CurrentMovementState = ECharacterMovementState::Idle;
    TargetSkeletalMesh = nullptr;
    AnimInstance = nullptr;
    
    // Performance defaults
    bUseCrowdOptimization = false;
    AnimationUpdateDistance = 2000.0f; // 20 meters
    CurrentLODLevel = 0;
    
    // Internal tracking
    LastMotionMatchingUpdate = 0.0f;
    LastPerformanceUpdate = 0.0f;
    CurrentAnimationFrequency = 1.0f;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Animation System Manager initialized"));
    
    // Try to find skeletal mesh component on owner
    if (AActor* Owner = GetOwner())
    {
        TargetSkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (TargetSkeletalMesh)
        {
            AnimInstance = TargetSkeletalMesh->GetAnimInstance();
            InitializeMotionMatching(TargetSkeletalMesh);
            SetupFootIK(TargetSkeletalMesh);
            
            UE_LOG(LogAnimationSystem, Log, TEXT("Found skeletal mesh component and initialized systems"));
        }
        else
        {
            UE_LOG(LogAnimationSystem, Warning, TEXT("No skeletal mesh component found on owner"));
        }
    }
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!TargetSkeletalMesh || !AnimInstance)
    {
        return;
    }
    
    // Update performance optimization
    LastPerformanceUpdate += DeltaTime;
    if (LastPerformanceUpdate >= 1.0f) // Update every second
    {
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
                    UpdateAnimationFrequency(Distance);
                }
            }
        }
        LastPerformanceUpdate = 0.0f;
    }
    
    // Update IK system
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    // Update motion matching (at reduced frequency for performance)
    LastMotionMatchingUpdate += DeltaTime;
    float MotionMatchingInterval = 1.0f / (MotionMatchingUpdateRate * CurrentAnimationFrequency);
    
    if (LastMotionMatchingUpdate >= MotionMatchingInterval)
    {
        // Get desired movement from character
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                FVector DesiredVelocity = MovementComp->GetLastInputVector() * MovementComp->GetMaxSpeed();
                FVector DesiredDirection = DesiredVelocity.GetSafeNormal();
                
                if (DesiredVelocity.Size() > 10.0f) // Only update if moving
                {
                    FMotionMatchingPose BestPose = FindBestMatchingPose(DesiredVelocity, DesiredDirection);
                    BlendToMotionMatchingPose(BestPose, 0.2f);
                }
            }
        }
        
        LastMotionMatchingUpdate = 0.0f;
    }
}

void UAnimationSystemManager::InitializeMotionMatching(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogAnimationSystem, Error, TEXT("Cannot initialize motion matching: SkeletalMesh is null"));
        return;
    }
    
    TargetSkeletalMesh = SkeletalMesh;
    
    // Initialize motion database with basic poses
    MotionDatabase.Empty();
    
    // Create basic idle pose
    FMotionMatchingPose IdlePose;
    IdlePose.RootMotionVelocity = FVector::ZeroVector;
    IdlePose.RootMotionDirection = FVector::ForwardVector;
    IdlePose.AnimationName = FName("Idle");
    IdlePose.TimeStamp = 0.0f;
    MotionDatabase.Add(IdlePose);
    
    CurrentPose = IdlePose;
    TargetPose = IdlePose;
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Motion matching initialized with %d poses"), MotionDatabase.Num());
}

FMotionMatchingPose UAnimationSystemManager::FindBestMatchingPose(const FVector& DesiredVelocity, const FVector& DesiredDirection)
{
    if (MotionDatabase.Num() == 0)
    {
        UE_LOG(LogAnimationSystem, Warning, TEXT("Motion database is empty"));
        return FMotionMatchingPose();
    }
    
    float BestScore = FLT_MAX;
    int32 BestPoseIndex = 0;
    
    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        const FMotionMatchingPose& Pose = MotionDatabase[i];
        
        // Calculate velocity similarity
        float VelocityDiff = FVector::Dist(Pose.RootMotionVelocity, DesiredVelocity);
        
        // Calculate direction similarity
        float DirectionDiff = FVector::Dist(Pose.RootMotionDirection, DesiredDirection);
        
        // Combined score (lower is better)
        float Score = VelocityDiff + (DirectionDiff * 2.0f); // Weight direction more heavily
        
        if (Score < BestScore)
        {
            BestScore = Score;
            BestPoseIndex = i;
        }
    }
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Found best matching pose: %s (Score: %f)"), 
           *MotionDatabase[BestPoseIndex].AnimationName.ToString(), BestScore);
    
    return MotionDatabase[BestPoseIndex];
}

void UAnimationSystemManager::UpdateMotionMatchingDatabase(UAnimSequence* AnimSequence)
{
    if (!AnimSequence)
    {
        UE_LOG(LogAnimationSystem, Error, TEXT("Cannot update motion database: AnimSequence is null"));
        return;
    }
    
    // Extract poses from animation sequence at regular intervals
    float AnimLength = AnimSequence->GetPlayLength();
    float SampleRate = 30.0f; // Sample at 30 FPS
    float TimeStep = 1.0f / SampleRate;
    
    for (float Time = 0.0f; Time < AnimLength; Time += TimeStep)
    {
        FMotionMatchingPose NewPose;
        ExtractPoseFromAnimation(AnimSequence, Time, NewPose);
        NewPose.AnimationName = AnimSequence->GetFName();
        NewPose.TimeStamp = Time;
        
        MotionDatabase.Add(NewPose);
        
        // Limit database size for performance
        if (MotionDatabase.Num() > MaxDatabaseSize)
        {
            MotionDatabase.RemoveAt(0); // Remove oldest pose
        }
    }
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Updated motion database with animation: %s. Total poses: %d"), 
           *AnimSequence->GetName(), MotionDatabase.Num());
}

void UAnimationSystemManager::BlendToMotionMatchingPose(const FMotionMatchingPose& TargetPose, float BlendTime)
{
    this->TargetPose = TargetPose;
    
    // In a full implementation, this would trigger animation blending
    // For now, we update the current pose directly
    CurrentPose = TargetPose;
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Blending to pose: %s"), *TargetPose.AnimationName.ToString());
}

void UAnimationSystemManager::SetupFootIK(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogAnimationSystem, Error, TEXT("Cannot setup foot IK: SkeletalMesh is null"));
        return;
    }
    
    // Setup default IK chains for feet
    IKChains.Empty();
    
    // Left foot IK chain
    FIKChainConfig LeftFootIK;
    LeftFootIK.StartBone = FName("thigh_l");
    LeftFootIK.EndBone = FName("foot_l");
    LeftFootIK.EffectorBone = FName("foot_l");
    LeftFootIK.BlendWeight = 1.0f;
    IKChains.Add(LeftFootIK);
    
    // Right foot IK chain
    FIKChainConfig RightFootIK;
    RightFootIK.StartBone = FName("thigh_r");
    RightFootIK.EndBone = FName("foot_r");
    RightFootIK.EffectorBone = FName("foot_r");
    RightFootIK.BlendWeight = 1.0f;
    IKChains.Add(RightFootIK);
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Foot IK setup complete with %d IK chains"), IKChains.Num());
}

void UAnimationSystemManager::UpdateFootIK(float DeltaTime)
{
    if (!TargetSkeletalMesh || IKChains.Num() == 0)
    {
        return;
    }
    
    // Update IK targets for each foot
    for (const FIKChainConfig& IKChain : IKChains)
    {
        if (IKChain.EffectorBone != NAME_None)
        {
            // Get current foot location
            FVector FootLocation = TargetSkeletalMesh->GetBoneLocation(IKChain.EffectorBone);
            
            // Perform ground trace
            FVector GroundLocation = PerformGroundTrace(FootLocation, GroundTraceDistance);
            
            // Smoothly interpolate to ground position
            FVector* CurrentTarget = IKTargets.Find(IKChain.EffectorBone);
            if (CurrentTarget)
            {
                *CurrentTarget = FMath::VInterpTo(*CurrentTarget, GroundLocation, DeltaTime, FootIKInterpSpeed);
            }
            else
            {
                IKTargets.Add(IKChain.EffectorBone, GroundLocation);
            }
        }
    }
    
    // Apply IK to skeleton
    ApplyIKToSkeleton(TargetSkeletalMesh);
}

FVector UAnimationSystemManager::PerformGroundTrace(const FVector& StartLocation, float TraceDistance)
{
    if (!GetWorld())
    {
        return StartLocation;
    }
    
    FVector TraceStart = StartLocation + FVector(0, 0, 50.0f); // Start slightly above
    FVector TraceEnd = StartLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return StartLocation; // Return original position if no ground found
}

void UAnimationSystemManager::SetIKTarget(FName BoneName, const FVector& TargetLocation, const FRotator& TargetRotation)
{
    IKTargets.FindOrAdd(BoneName) = TargetLocation;
    IKRotations.FindOrAdd(BoneName) = TargetRotation;
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Set IK target for bone %s: %s"), 
           *BoneName.ToString(), *TargetLocation.ToString());
}

void UAnimationSystemManager::SetMovementState(ECharacterMovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        ECharacterMovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        
        UE_LOG(LogAnimationSystem, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
        
        // Trigger animation state changes based on movement state
        // This would integrate with Animation Blueprints in a full implementation
    }
}

void UAnimationSystemManager::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !AnimInstance)
    {
        UE_LOG(LogAnimationSystem, Error, TEXT("Cannot play montage: Montage or AnimInstance is null"));
        return;
    }
    
    AnimInstance->Montage_Play(Montage, PlayRate);
    UE_LOG(LogAnimationSystem, Log, TEXT("Playing animation montage: %s"), *Montage->GetName());
}

void UAnimationSystemManager::StopAnimationMontage(UAnimMontage* Montage, float BlendOutTime)
{
    if (!Montage || !AnimInstance)
    {
        UE_LOG(LogAnimationSystem, Error, TEXT("Cannot stop montage: Montage or AnimInstance is null"));
        return;
    }
    
    AnimInstance->Montage_Stop(BlendOutTime, Montage);
    UE_LOG(LogAnimationSystem, Log, TEXT("Stopping animation montage: %s"), *Montage->GetName());
}

void UAnimationSystemManager::SetAnimationLOD(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 3);
    
    if (TargetSkeletalMesh)
    {
        TargetSkeletalMesh->SetForcedLOD(CurrentLODLevel + 1); // UE5 LOD is 1-based
    }
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Animation LOD set to level %d"), CurrentLODLevel);
}

void UAnimationSystemManager::EnableCrowdOptimization(bool bEnable)
{
    bUseCrowdOptimization = bEnable;
    
    if (bEnable)
    {
        // Reduce animation update frequency for crowd characters
        CurrentAnimationFrequency = 0.5f;
        MotionMatchingUpdateRate = 15.0f; // Reduce to 15 FPS for crowds
    }
    else
    {
        // Full quality animation
        CurrentAnimationFrequency = 1.0f;
        MotionMatchingUpdateRate = 30.0f;
    }
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Crowd optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystemManager::UpdateAnimationFrequency(float DistanceFromCamera)
{
    if (!bUseCrowdOptimization)
    {
        return;
    }
    
    // Reduce animation quality based on distance
    if (DistanceFromCamera > AnimationUpdateDistance)
    {
        CurrentAnimationFrequency = 0.25f; // Very low quality for distant characters
        SetAnimationLOD(3); // Highest LOD (lowest quality)
    }
    else if (DistanceFromCamera > AnimationUpdateDistance * 0.5f)
    {
        CurrentAnimationFrequency = 0.5f; // Medium quality
        SetAnimationLOD(2);
    }
    else if (DistanceFromCamera > AnimationUpdateDistance * 0.25f)
    {
        CurrentAnimationFrequency = 0.75f; // Good quality
        SetAnimationLOD(1);
    }
    else
    {
        CurrentAnimationFrequency = 1.0f; // Full quality for close characters
        SetAnimationLOD(0);
    }
}

float UAnimationSystemManager::CalculatePoseSimilarity(const FMotionMatchingPose& PoseA, const FMotionMatchingPose& PoseB)
{
    // Simple similarity calculation based on velocity and direction
    float VelocitySimilarity = 1.0f - (FVector::Dist(PoseA.RootMotionVelocity, PoseB.RootMotionVelocity) / 1000.0f);
    float DirectionSimilarity = FVector::DotProduct(PoseA.RootMotionDirection, PoseB.RootMotionDirection);
    
    return (VelocitySimilarity + DirectionSimilarity) * 0.5f;
}

void UAnimationSystemManager::ExtractPoseFromAnimation(UAnimSequence* AnimSequence, float TimeStamp, FMotionMatchingPose& OutPose)
{
    if (!AnimSequence)
    {
        return;
    }
    
    // In a full implementation, this would extract actual bone transforms
    // For now, we create a simplified pose based on root motion
    OutPose.TimeStamp = TimeStamp;
    OutPose.AnimationName = AnimSequence->GetFName();
    
    // Extract root motion if available
    if (AnimSequence->HasRootMotion())
    {
        FTransform RootMotion = AnimSequence->ExtractRootMotion(TimeStamp, TimeStamp + 0.033f, false);
        OutPose.RootMotionVelocity = RootMotion.GetLocation() * 30.0f; // Convert to velocity (30 FPS)
        OutPose.RootMotionDirection = RootMotion.GetRotation().GetForwardVector();
    }
    else
    {
        OutPose.RootMotionVelocity = FVector::ZeroVector;
        OutPose.RootMotionDirection = FVector::ForwardVector;
    }
}

void UAnimationSystemManager::ApplyIKToSkeleton(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh || IKTargets.Num() == 0)
    {
        return;
    }
    
    // In a full implementation, this would apply IK constraints to the skeleton
    // This requires integration with the Animation Blueprint system
    // For now, we log the IK application
    
    for (const auto& IKPair : IKTargets)
    {
        UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Applying IK to bone %s at location %s"), 
               *IKPair.Key.ToString(), *IKPair.Value.ToString());
    }
}

void UAnimationSystemManager::OptimizeAnimationForDistance(float Distance)
{
    // Adjust animation quality based on distance from camera
    UpdateAnimationFrequency(Distance);
    
    // Additional optimizations could include:
    // - Disabling facial animation for distant characters
    // - Reducing bone count for IK calculations
    // - Switching to simpler animation states
}