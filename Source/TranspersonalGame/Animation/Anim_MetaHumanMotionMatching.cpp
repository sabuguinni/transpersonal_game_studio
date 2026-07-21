#include "Anim_MetaHumanMotionMatching.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_MetaHumanMotionMatching::UAnim_MetaHumanMotionMatching()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS updates
    
    CharacterType = EAnim_MetaHumanCharacterType::Player;
    CurrentState = EAnim_MotionMatchingState::Idle;
    CurrentSpeed = 0.0f;
    MovementDirection = FVector::ZeroVector;
    bIsInCombat = false;
    bIsInteracting = false;
    LastStateChangeTime = 0.0f;
    GestureTimer = 0.0f;
    
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MetaHumanMotionMatching::BeginPlay()
{
    Super::BeginPlay();
    
    // Find skeletal mesh component on owner
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }
    
    InitializeCharacterPreset();
}

void UAnim_MetaHumanMotionMatching::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComponent || !AnimInstance)
        return;
    
    // Update movement data
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            CurrentSpeed = Velocity.Size();
            MovementDirection = Velocity.GetSafeNormal();
            
            UpdateMotionMatchingState(CurrentSpeed, MovementDirection);
        }
    }
    
    UpdateAnimationBlending(DeltaTime);
    HandleGestureSystem(DeltaTime);
    ApplyIKFootPlacement();
}

void UAnim_MetaHumanMotionMatching::SetCharacterType(EAnim_MetaHumanCharacterType NewType)
{
    CharacterType = NewType;
    InitializeCharacterPreset();
}

void UAnim_MetaHumanMotionMatching::UpdateMotionMatchingState(float Speed, FVector Direction)
{
    EAnim_MotionMatchingState NewState = DetermineStateFromMovement(Speed, Direction);
    
    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        PlayAnimationMontage(CurrentState);
    }
}

void UAnim_MetaHumanMotionMatching::PlayAnimationMontage(EAnim_MotionMatchingState State)
{
    if (!AnimInstance)
        return;
    
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (State)
    {
        case EAnim_MotionMatchingState::Idle:
            MontageToPlay = AnimationSet.IdleMontage;
            break;
        case EAnim_MotionMatchingState::Walking:
            MontageToPlay = AnimationSet.WalkMontage;
            break;
        case EAnim_MotionMatchingState::Running:
            MontageToPlay = AnimationSet.RunMontage;
            break;
        case EAnim_MotionMatchingState::Jumping:
            MontageToPlay = AnimationSet.JumpMontage;
            break;
        case EAnim_MotionMatchingState::Combat:
            MontageToPlay = AnimationSet.CombatMontage;
            break;
        default:
            MontageToPlay = AnimationSet.IdleMontage;
            break;
    }
    
    if (MontageToPlay)
    {
        AnimInstance->Montage_Play(MontageToPlay, 1.0f);
    }
}

void UAnim_MetaHumanMotionMatching::SetCombatMode(bool bInCombat)
{
    bIsInCombat = bInCombat;
    if (bIsInCombat)
    {
        CurrentState = EAnim_MotionMatchingState::Combat;
        PlayAnimationMontage(CurrentState);
    }
}

void UAnim_MetaHumanMotionMatching::SetInteractionMode(bool bInteracting)
{
    bIsInteracting = bInteracting;
    if (bIsInteracting)
    {
        CurrentState = EAnim_MotionMatchingState::Interacting;
        PlayAnimationMontage(CurrentState);
    }
}

void UAnim_MetaHumanMotionMatching::InitializeCharacterPreset()
{
    MotionConfig = GetMotionConfigForCharacterType(CharacterType);
}

FAnim_MotionMatchingConfig UAnim_MetaHumanMotionMatching::GetMotionConfigForCharacterType(EAnim_MetaHumanCharacterType Type)
{
    FAnim_MotionMatchingConfig Config;
    
    switch (Type)
    {
        case EAnim_MetaHumanCharacterType::TribalElder:
            Config.IdleWeight = 0.8f;
            Config.WalkSpeed = 0.6f;
            Config.RunSpeed = 0.4f;
            Config.GestureFrequency = 0.9f;
            Config.BlendTime = 0.3f;
            break;
            
        case EAnim_MetaHumanCharacterType::Hunter:
            Config.IdleWeight = 0.3f;
            Config.WalkSpeed = 1.2f;
            Config.RunSpeed = 1.5f;
            Config.GestureFrequency = 0.4f;
            Config.BlendTime = 0.15f;
            break;
            
        case EAnim_MetaHumanCharacterType::Gatherer:
            Config.IdleWeight = 0.5f;
            Config.WalkSpeed = 0.9f;
            Config.RunSpeed = 1.1f;
            Config.GestureFrequency = 0.7f;
            Config.BlendTime = 0.2f;
            break;
            
        case EAnim_MetaHumanCharacterType::Crafter:
            Config.IdleWeight = 0.9f;
            Config.WalkSpeed = 0.5f;
            Config.RunSpeed = 0.7f;
            Config.GestureFrequency = 0.8f;
            Config.BlendTime = 0.25f;
            break;
            
        case EAnim_MetaHumanCharacterType::Scout:
            Config.IdleWeight = 0.2f;
            Config.WalkSpeed = 1.5f;
            Config.RunSpeed = 1.8f;
            Config.GestureFrequency = 0.3f;
            Config.BlendTime = 0.1f;
            break;
            
        case EAnim_MetaHumanCharacterType::Player:
        default:
            Config.IdleWeight = 0.5f;
            Config.WalkSpeed = 1.0f;
            Config.RunSpeed = 1.3f;
            Config.GestureFrequency = 0.5f;
            Config.BlendTime = 0.2f;
            break;
    }
    
    return Config;
}

void UAnim_MetaHumanMotionMatching::UpdateAnimationBlending(float DeltaTime)
{
    if (!AnimInstance)
        return;
    
    // Update blend space parameters for locomotion
    if (AnimationSet.LocomotionBlendSpace)
    {
        float SpeedRatio = CurrentSpeed / 600.0f; // Max run speed
        SpeedRatio = FMath::Clamp(SpeedRatio, 0.0f, 1.0f);
        
        // Set blend space parameters (this would be done through Animation Blueprint variables)
        // AnimInstance->SetBlendSpaceInput(SpeedRatio, MovementDirection);
    }
    
    // Handle directional movement blending
    if (AnimationSet.DirectionalBlendSpace && MovementDirection.Size() > 0.1f)
    {
        float MovementAngle = FMath::Atan2(MovementDirection.Y, MovementDirection.X);
        MovementAngle = FMath::RadiansToDegrees(MovementAngle);
        
        // Normalize angle to -180 to 180 range
        while (MovementAngle > 180.0f) MovementAngle -= 360.0f;
        while (MovementAngle < -180.0f) MovementAngle += 360.0f;
    }
}

void UAnim_MetaHumanMotionMatching::HandleGestureSystem(float DeltaTime)
{
    GestureTimer += DeltaTime;
    
    // Trigger random gestures based on character type
    float GestureInterval = 1.0f / MotionConfig.GestureFrequency;
    
    if (GestureTimer >= GestureInterval && CurrentState == EAnim_MotionMatchingState::Idle)
    {
        // Trigger gesture animation
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance
        {
            // Play gesture montage
            GestureTimer = 0.0f;
        }
    }
}

void UAnim_MetaHumanMotionMatching::ApplyIKFootPlacement()
{
    if (!MotionConfig.bEnableIKFootPlacement || !SkeletalMeshComponent)
        return;
    
    // Perform line traces for foot placement
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FVector TraceStart = ActorLocation + FVector(0, 0, 50);
    FVector TraceEnd = ActorLocation - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Calculate IK offset for foot placement
        float IKOffset = HitResult.Location.Z - ActorLocation.Z;
        
        // Apply IK offset (this would be done through Animation Blueprint)
        // AnimInstance->SetIKFootOffset(IKOffset);
    }
}

EAnim_MotionMatchingState UAnim_MetaHumanMotionMatching::DetermineStateFromMovement(float Speed, FVector Direction)
{
    if (bIsInCombat)
        return EAnim_MotionMatchingState::Combat;
    
    if (bIsInteracting)
        return EAnim_MotionMatchingState::Interacting;
    
    // Speed thresholds based on character configuration
    float WalkThreshold = 100.0f * MotionConfig.WalkSpeed;
    float RunThreshold = 300.0f * MotionConfig.RunSpeed;
    
    if (Speed < 10.0f)
        return EAnim_MotionMatchingState::Idle;
    else if (Speed < WalkThreshold)
        return EAnim_MotionMatchingState::Walking;
    else if (Speed < RunThreshold)
        return EAnim_MotionMatchingState::Running;
    else
        return EAnim_MotionMatchingState::Running;
}