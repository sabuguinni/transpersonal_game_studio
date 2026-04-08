#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentMovementState = ECharacterMovementState::Idle;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    bUseMotionMatching = true;
    bUseFootIK = true;
    FootIKInterpSpeed = 5.0f;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeAnimationSystems();
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationBlending(DeltaTime);
    ProcessMovementStateTransitions();
}

void UAnimationSystemManager::SetCharacterMovementState(ECharacterMovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        ECharacterMovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        
        // Log state transition for debugging
        UE_LOG(LogTemp, Log, TEXT("Character movement state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnimationSystemManager::UpdateCharacterProfile(const FCharacterAnimationProfile& NewProfile)
{
    CharacterProfile = NewProfile;
    
    // Update animation parameters based on profile
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                // Set animation variables based on character profile
                // This would be expanded with specific animation blueprint variables
            }
        }
    }
}

void UAnimationSystemManager::SetDinosaurBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur behavior state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnimationSystemManager::InitializeMotionMatching()
{
    if (bUseMotionMatching)
    {
        // Initialize Motion Matching system
        // This would connect to the Pose Search plugin
        UE_LOG(LogTemp, Log, TEXT("Motion Matching system initialized"));
    }
}

void UAnimationSystemManager::UpdateMotionMatchingQuery(FVector Velocity, FVector Acceleration, float TurnRate)
{
    if (!bUseMotionMatching) return;
    
    // Update motion matching query parameters
    // This would feed data to the Motion Matching node in the Animation Blueprint
    
    // Calculate movement characteristics for query
    float Speed = Velocity.Size();
    FVector Direction = Velocity.GetSafeNormal();
    
    // Apply character profile modifiers
    Speed *= CharacterProfile.MovementSpeed;
    
    // Modify based on fear and exhaustion
    if (CharacterProfile.FearLevel > 0.5f)
    {
        // Fear makes movements more erratic and faster
        Speed *= (1.0f + CharacterProfile.FearLevel * 0.3f);
    }
    
    if (CharacterProfile.ExhaustionLevel > 0.5f)
    {
        // Exhaustion slows down movements
        Speed *= (1.0f - CharacterProfile.ExhaustionLevel * 0.4f);
    }
}

void UAnimationSystemManager::EnableFootIK(bool bEnable)
{
    bUseFootIK = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Foot IK %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystemManager::UpdateFootIKTargets(FVector LeftFootTarget, FVector RightFootTarget)
{
    if (!bUseFootIK) return;
    
    // Update IK targets for foot placement
    // This would be used by the IK Rig system
}

void UAnimationSystemManager::InitializeAnimationSystems()
{
    InitializeMotionMatching();
    EnableFootIK(bUseFootIK);
    
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UAnimationSystemManager::UpdateAnimationBlending(float DeltaTime)
{
    // Update animation blending based on current states and profile
    // This is where we would implement the Richard Williams principles
    // of weight, timing, and follow-through
}

void UAnimationSystemManager::ProcessMovementStateTransitions()
{
    // Handle automatic state transitions based on gameplay conditions
    // For example, transitioning from walking to running based on speed
    
    if (AActor* Owner = GetOwner())
    {
        // Get movement data from owner
        // Process state transitions based on gameplay logic
    }
}