#include "AnimationSystemManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "CollisionQueryParams.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentTerrainType = EAnim_TerrainType::Flat;
    CurrentSpeed = 0.0f;
    CurrentDirection = 0.0f;
    LastVelocity = FVector::ZeroVector;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    TerrainSlope = 0.0f;
    TerrainNormal = FVector::UpVector;
    
    // Set default character animation data
    CharacterAnimData.CharacterName = TEXT("PrehistoricCharacter");
    CharacterAnimData.MovementSpeed = 300.0f;
    CharacterAnimData.TurnRate = 90.0f;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind to character components
    BindToCharacterComponents();
    
    // Initialize animation systems
    InitializeMotionMatching();
    InitializeIKSystem();
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update movement parameters
    UpdateMovementParameters();
    
    // Perform terrain analysis
    PerformTerrainTrace();
    
    // Update IK foot placement
    if (CharacterAnimData.IKConfig.bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    // Update animation blending
    UpdateAnimationBlending(DeltaTime);
    
    // Update trajectory prediction for Motion Matching
    UpdateTrajectoryPrediction();
}

void UAnimationSystemManager::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        EAnim_MovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Animation state changed: %d -> %d"), 
               (int32)PreviousState, (int32)CurrentMovementState);
        
        // Handle state-specific logic
        switch (CurrentMovementState)
        {
            case EAnim_MovementState::Gathering:
            case EAnim_MovementState::Crafting:
            case EAnim_MovementState::Combat:
                // Play appropriate action montage
                PlayActionMontage(CurrentMovementState);
                break;
            default:
                // Stop any playing montages for locomotion states
                StopCurrentMontage();
                break;
        }
    }
}

void UAnimationSystemManager::SetTerrainType(EAnim_TerrainType NewTerrainType)
{
    if (CurrentTerrainType != NewTerrainType)
    {
        CurrentTerrainType = NewTerrainType;
        UE_LOG(LogTemp, Log, TEXT("Terrain type changed: %d"), (int32)CurrentTerrainType);
    }
}

void UAnimationSystemManager::InitializeMotionMatching()
{
    if (!CharacterAnimData.MotionMatchingConfig.MotionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching Database not set for character: %s"), 
               *CharacterAnimData.CharacterName);
        return;
    }
    
    ConfigureMotionMatchingParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching initialized for character: %s"), 
           *CharacterAnimData.CharacterName);
}

void UAnimationSystemManager::UpdateMotionMatchingDatabase(class UPoseSearchDatabase* NewDatabase)
{
    CharacterAnimData.MotionMatchingConfig.MotionDatabase = NewDatabase;
    InitializeMotionMatching();
}

void UAnimationSystemManager::InitializeIKSystem()
{
    if (!CharacterAnimData.IKConfig.IKRig)
    {
        UE_LOG(LogTemp, Warning, TEXT("IK Rig not set for character: %s"), 
               *CharacterAnimData.CharacterName);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("IK System initialized for character: %s"), 
           *CharacterAnimData.CharacterName);
}

void UAnimationSystemManager::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComponent || !CharacterAnimData.IKConfig.bEnableFootIK)
    {
        return;
    }
    
    // Get foot bone locations
    FVector LeftFootLocation = SkeletalMeshComponent->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootLocation = SkeletalMeshComponent->GetBoneLocation(TEXT("foot_r"));
    
    // Perform foot traces
    FVector LeftFootTarget = GetFootPlacementOffset(true);
    FVector RightFootTarget = GetFootPlacementOffset(false);
    
    // Smoothly interpolate foot offsets
    float InterpSpeed = CharacterAnimData.IKConfig.FootAdjustmentSpeed;
    LeftFootOffset = FMath::VInterpTo(LeftFootOffset, LeftFootTarget, DeltaTime, InterpSpeed);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, RightFootTarget, DeltaTime, InterpSpeed);
    
    // Clamp foot offsets to maximum allowed
    float MaxOffset = CharacterAnimData.IKConfig.MaxFootOffset;
    LeftFootOffset = LeftFootOffset.GetClampedToMaxSize(MaxOffset);
    RightFootOffset = RightFootOffset.GetClampedToMaxSize(MaxOffset);
}

FVector UAnimationSystemManager::GetFootPlacementOffset(bool bIsLeftFoot) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    // Get foot bone name
    FName FootBoneName = bIsLeftFoot ? TEXT("foot_l") : TEXT("foot_r");
    FVector FootLocation = SkeletalMeshComponent->GetBoneLocation(FootBoneName);
    
    // Perform line trace downward
    FVector TraceStart = FootLocation;
    FVector TraceEnd = TraceStart - FVector(0, 0, CharacterAnimData.IKConfig.FootTraceDistance);
    
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
        // Calculate offset needed to place foot on ground
        float DistanceToGround = FVector::Dist(TraceStart, HitResult.Location);
        float TargetOffset = CharacterAnimData.IKConfig.FootTraceDistance - DistanceToGround;
        
        return FVector(0, 0, TargetOffset);
    }
    
    return FVector::ZeroVector;
}

bool UAnimationSystemManager::PlayActionMontage(EAnim_MovementState ActionType, float PlayRate)
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance())
    {
        return false;
    }
    
    UAnimMontage** MontagePtr = CharacterAnimData.ActionMontages.Find(ActionType);
    if (!MontagePtr || !*MontagePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("No montage found for action type: %d"), (int32)ActionType);
        return false;
    }
    
    UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    float MontageLength = AnimInstance->Montage_Play(*MontagePtr, PlayRate);
    
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Playing action montage for state: %d"), (int32)ActionType);
        return true;
    }
    
    return false;
}

void UAnimationSystemManager::StopCurrentMontage(float BlendOutTime)
{
    if (SkeletalMeshComponent && SkeletalMeshComponent->GetAnimInstance())
    {
        SkeletalMeshComponent->GetAnimInstance()->Montage_Stop(BlendOutTime);
    }
}

void UAnimationSystemManager::SetupCharacterAnimation(const FAnim_CharacterAnimData& NewAnimData)
{
    CharacterAnimData = NewAnimData;
    
    // Reinitialize systems with new data
    InitializeMotionMatching();
    InitializeIKSystem();
    
    UE_LOG(LogTemp, Log, TEXT("Character animation setup complete for: %s"), 
           *CharacterAnimData.CharacterName);
}

void UAnimationSystemManager::BindToCharacterComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Get skeletal mesh component
    SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No SkeletalMeshComponent found on owner: %s"), 
               *Owner->GetName());
    }
    
    // Get character movement component
    MovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No CharacterMovementComponent found on owner: %s"), 
               *Owner->GetName());
    }
}

EAnim_TerrainType UAnimationSystemManager::AnalyzeCurrentTerrain()
{
    // Simple terrain analysis based on slope and surface material
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(TerrainNormal, FVector::UpVector)));
    
    if (SlopeAngle > 30.0f)
    {
        return EAnim_TerrainType::Slope;
    }
    else if (SlopeAngle > 15.0f)
    {
        return EAnim_TerrainType::Rocky;
    }
    
    // Default to flat terrain
    return EAnim_TerrainType::Flat;
}

float UAnimationSystemManager::GetTerrainSlope() const
{
    return TerrainSlope;
}

void UAnimationSystemManager::UpdateMovementParameters()
{
    if (!MovementComponent)
    {
        return;
    }
    
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentSpeed = CurrentVelocity.Size();
    
    // Calculate direction relative to actor forward
    if (CurrentSpeed > 1.0f)
    {
        FVector ForwardVector = GetOwner()->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        CurrentDirection = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if turning left or right
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection);
        if (CrossProduct.Z < 0.0f)
        {
            CurrentDirection *= -1.0f;
        }
    }
    else
    {
        CurrentDirection = 0.0f;
    }
    
    // Update movement state based on speed
    if (CurrentSpeed < 1.0f)
    {
        SetMovementState(EAnim_MovementState::Idle);
    }
    else if (CurrentSpeed < 150.0f)
    {
        SetMovementState(EAnim_MovementState::Walking);
    }
    else if (CurrentSpeed < 400.0f)
    {
        SetMovementState(EAnim_MovementState::Running);
    }
    else
    {
        SetMovementState(EAnim_MovementState::Sprinting);
    }
    
    LastVelocity = CurrentVelocity;
}

void UAnimationSystemManager::PerformTerrainTrace()
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FVector TraceStart = ActorLocation + FVector(0, 0, 50);
    FVector TraceEnd = ActorLocation - FVector(0, 0, 100);
    
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
        TerrainNormal = HitResult.Normal;
        TerrainSlope = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(TerrainNormal, FVector::UpVector)));
        
        // Update terrain type
        EAnim_TerrainType NewTerrainType = AnalyzeCurrentTerrain();
        SetTerrainType(NewTerrainType);
    }
}

void UAnimationSystemManager::UpdateAnimationBlending(float DeltaTime)
{
    // Update animation parameters in the Animation Blueprint
    if (SkeletalMeshComponent && SkeletalMeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        
        // Set movement parameters
        AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
    }
}

void UAnimationSystemManager::ConfigureMotionMatchingParameters()
{
    // Configure Motion Matching specific parameters
    // This would interface with the Motion Matching system when available
    UE_LOG(LogTemp, Log, TEXT("Configuring Motion Matching parameters"));
}

void UAnimationSystemManager::UpdateTrajectoryPrediction()
{
    // Update trajectory prediction for Motion Matching
    // This calculates where the character will be in the near future
    if (MovementComponent && CharacterAnimData.MotionMatchingConfig.bUseTrajectoryMatching)
    {
        // Simple trajectory prediction based on current velocity
        FVector PredictedLocation = GetOwner()->GetActorLocation() + (MovementComponent->Velocity * 0.5f);
        
        // This would feed into the Motion Matching system
        // For now, we just log it for debugging
        UE_LOG(LogTemp, VeryVerbose, TEXT("Predicted location: %s"), *PredictedLocation.ToString());
    }
}