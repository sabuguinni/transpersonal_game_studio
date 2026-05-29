#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching parameters
    VelocityWeight = 1.0f;
    AccelerationWeight = 0.5f;
    DirectionWeight = 0.3f;
    SearchRadius = 100.0f;
    BlendTime = 0.2f;

    // Initialize state
    PreviousVelocity = FVector::ZeroVector;
    DeltaTimeAccumulator = 0.0f;
    bIsInitialized = false;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized)
    {
        return;
    }

    DeltaTimeAccumulator += DeltaTime;

    // Update motion data every frame
    UpdateMotionData();

    // Find best pose match every few frames for performance
    if (DeltaTimeAccumulator >= 0.033f) // ~30 FPS for motion matching
    {
        BestPoseCandidate = FindBestPoseMatch();
        ApplyPoseMatch(BestPoseCandidate);
        DeltaTimeAccumulator = 0.0f;
    }
}

void UAnim_MotionMatchingSystem::InitializeComponent()
{
    CacheCharacterReferences();
    
    if (OwnerCharacter && MovementComponent)
    {
        BuildPoseDatabase();
        ValidateDatabase();
        bIsInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching System failed to initialize - missing character references"));
    }
}

void UAnim_MotionMatchingSystem::CacheCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (USkeletalMeshComponent* SkeletalMesh = OwnerCharacter->GetMesh())
        {
            AnimInstance = SkeletalMesh->GetAnimInstance();
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get current velocity and calculate acceleration
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / GetWorld()->GetDeltaSeconds();
    
    // Update motion data
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Acceleration = CurrentAcceleration;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    CurrentMotionData.Direction = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, CurrentVelocity).Yaw;
    
    // Update movement states
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 10.0f;
    CurrentMotionData.bIsJumping = MovementComponent->IsFalling() && CurrentVelocity.Z > 0.0f;
    CurrentMotionData.bIsFalling = MovementComponent->IsFalling() && CurrentVelocity.Z < 0.0f;
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Store previous velocity for next frame
    PreviousVelocity = CurrentVelocity;
}

FAnim_PoseCandidate UAnim_MotionMatchingSystem::FindBestPoseMatch()
{
    FAnim_PoseCandidate BestCandidate;
    float BestCost = FLT_MAX;

    // Search through pose database
    for (int32 i = 0; i < PoseDatabase.Num() && i < PoseMotionDatabase.Num(); i++)
    {
        if (!PoseDatabase[i])
        {
            continue;
        }

        float Cost = CalculatePoseMatchCost(CurrentMotionData, PoseMotionDatabase[i]);
        
        if (Cost < BestCost)
        {
            BestCost = Cost;
            BestCandidate.PoseAsset = PoseDatabase[i];
            BestCandidate.PoseTime = 0.0f; // Could be randomized or calculated
            BestCandidate.MatchCost = Cost;
            BestCandidate.PoseMotionData = PoseMotionDatabase[i];
        }
    }

    return BestCandidate;
}

float UAnim_MotionMatchingSystem::CalculatePoseMatchCost(const FAnim_MotionData& TargetMotion, const FAnim_MotionData& PoseMotion)
{
    float VelocityCost = FVector::Dist(TargetMotion.Velocity, PoseMotion.Velocity) * VelocityWeight;
    float AccelerationCost = FVector::Dist(TargetMotion.Acceleration, PoseMotion.Acceleration) * AccelerationWeight;
    float DirectionCost = FMath::Abs(TargetMotion.Direction - PoseMotion.Direction) * DirectionWeight;
    
    // Boolean state penalties
    float StateCost = 0.0f;
    if (TargetMotion.bIsMoving != PoseMotion.bIsMoving) StateCost += 50.0f;
    if (TargetMotion.bIsJumping != PoseMotion.bIsJumping) StateCost += 100.0f;
    if (TargetMotion.bIsFalling != PoseMotion.bIsFalling) StateCost += 100.0f;
    if (TargetMotion.bIsCrouching != PoseMotion.bIsCrouching) StateCost += 75.0f;

    return VelocityCost + AccelerationCost + DirectionCost + StateCost;
}

void UAnim_MotionMatchingSystem::ApplyPoseMatch(const FAnim_PoseCandidate& PoseCandidate)
{
    if (!AnimInstance || !PoseCandidate.PoseAsset)
    {
        return;
    }

    // In a full implementation, this would blend to the pose
    // For now, we just log the best match
    UE_LOG(LogTemp, VeryVerbose, TEXT("Best pose match: %s (Cost: %.2f)"), 
           *PoseCandidate.PoseAsset->GetName(), PoseCandidate.MatchCost);
}

void UAnim_MotionMatchingSystem::BuildPoseDatabase()
{
    // Clear existing database
    PoseDatabase.Empty();
    PoseMotionDatabase.Empty();

    // Add default poses with motion data
    // In a real implementation, these would be loaded from assets
    
    // Idle pose
    FAnim_MotionData IdleMotion;
    IdleMotion.Speed = 0.0f;
    IdleMotion.bIsMoving = false;
    AddPoseToDatabase(nullptr, IdleMotion); // Placeholder for now

    // Walking poses
    for (int32 i = 0; i < 8; i++)
    {
        FAnim_MotionData WalkMotion;
        WalkMotion.Speed = 150.0f;
        WalkMotion.Direction = i * 45.0f; // 8 directions
        WalkMotion.bIsMoving = true;
        WalkMotion.Velocity = FVector(
            FMath::Cos(FMath::DegreesToRadians(WalkMotion.Direction)) * WalkMotion.Speed,
            FMath::Sin(FMath::DegreesToRadians(WalkMotion.Direction)) * WalkMotion.Speed,
            0.0f
        );
        AddPoseToDatabase(nullptr, WalkMotion);
    }

    // Running poses
    for (int32 i = 0; i < 8; i++)
    {
        FAnim_MotionData RunMotion;
        RunMotion.Speed = 400.0f;
        RunMotion.Direction = i * 45.0f;
        RunMotion.bIsMoving = true;
        RunMotion.Velocity = FVector(
            FMath::Cos(FMath::DegreesToRadians(RunMotion.Direction)) * RunMotion.Speed,
            FMath::Sin(FMath::DegreesToRadians(RunMotion.Direction)) * RunMotion.Speed,
            0.0f
        );
        AddPoseToDatabase(nullptr, RunMotion);
    }

    // Jump poses
    FAnim_MotionData JumpMotion;
    JumpMotion.bIsJumping = true;
    JumpMotion.Velocity = FVector(0.0f, 0.0f, 300.0f);
    AddPoseToDatabase(nullptr, JumpMotion);

    // Fall poses
    FAnim_MotionData FallMotion;
    FallMotion.bIsFalling = true;
    FallMotion.Velocity = FVector(0.0f, 0.0f, -300.0f);
    AddPoseToDatabase(nullptr, FallMotion);

    UE_LOG(LogTemp, Log, TEXT("Built motion matching database with %d poses"), PoseDatabase.Num());
}

void UAnim_MotionMatchingSystem::AddPoseToDatabase(UPoseAsset* PoseAsset, const FAnim_MotionData& MotionData)
{
    PoseDatabase.Add(PoseAsset);
    PoseMotionDatabase.Add(MotionData);
}

void UAnim_MotionMatchingSystem::ValidateDatabase()
{
    if (PoseDatabase.Num() != PoseMotionDatabase.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Motion Matching Database mismatch: %d poses vs %d motion data entries"), 
               PoseDatabase.Num(), PoseMotionDatabase.Num());
    }
}

// Utility function implementations
FVector UAnim_MotionMatchingSystem::GetCharacterVelocity() const
{
    return MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
}

FVector UAnim_MotionMatchingSystem::GetCharacterAcceleration() const
{
    return CurrentMotionData.Acceleration;
}

float UAnim_MotionMatchingSystem::GetCharacterSpeed() const
{
    return CurrentMotionData.Speed;
}

float UAnim_MotionMatchingSystem::GetCharacterDirection() const
{
    return CurrentMotionData.Direction;
}

bool UAnim_MotionMatchingSystem::IsCharacterMoving() const
{
    return CurrentMotionData.bIsMoving;
}

bool UAnim_MotionMatchingSystem::IsCharacterJumping() const
{
    return CurrentMotionData.bIsJumping;
}

bool UAnim_MotionMatchingSystem::IsCharacterFalling() const
{
    return CurrentMotionData.bIsFalling;
}

bool UAnim_MotionMatchingSystem::IsCharacterCrouching() const
{
    return CurrentMotionData.bIsCrouching;
}