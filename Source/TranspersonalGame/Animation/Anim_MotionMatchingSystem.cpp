#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    MotionMatchingThreshold = 50.0f;
    StateTransitionCooldown = 0.5f;
    CurrentAnimationState = TEXT("Idle");
    LastTransitionTime = 0.0f;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default blend spaces
    if (MovementBlendSpaces.Num() == 0)
    {
        // Setup default state mappings - these will be overridden by Blueprint
        MovementBlendSpaces.Add(TEXT("Idle"), nullptr);
        MovementBlendSpaces.Add(TEXT("Walk"), nullptr);
        MovementBlendSpaces.Add(TEXT("Run"), nullptr);
        MovementBlendSpaces.Add(TEXT("Jump"), nullptr);
        MovementBlendSpaces.Add(TEXT("Crouch"), nullptr);
    }
    
    // Initialize default action montages
    if (ActionMontages.Num() == 0)
    {
        ActionMontages.Add(TEXT("Attack"), nullptr);
        ActionMontages.Add(TEXT("Interact"), nullptr);
        ActionMontages.Add(TEXT("Craft"), nullptr);
        ActionMontages.Add(TEXT("Gather"), nullptr);
    }
    
    // Setup default state transitions
    if (StateTransitions.Num() == 0)
    {
        FAnim_StateTransition IdleToWalk;
        IdleToWalk.FromState = TEXT("Idle");
        IdleToWalk.ToState = TEXT("Walk");
        IdleToWalk.BlendTime = 0.2f;
        StateTransitions.Add(IdleToWalk);
        
        FAnim_StateTransition WalkToRun;
        WalkToRun.FromState = TEXT("Walk");
        WalkToRun.ToState = TEXT("Run");
        WalkToRun.BlendTime = 0.3f;
        StateTransitions.Add(WalkToRun);
        
        FAnim_StateTransition AnyToJump;
        AnyToJump.FromState = TEXT("Any");
        AnyToJump.ToState = TEXT("Jump");
        AnyToJump.BlendTime = 0.1f;
        StateTransitions.Add(AnyToJump);
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
        {
            // Update motion data from character movement
            FVector CurrentVelocity = MovementComp->Velocity;
            bool bIsInAir = MovementComp->IsFalling();
            bool bIsCrouching = MovementComp->IsCrouching();
            
            UpdateMotionData(CurrentVelocity, bIsInAir, bIsCrouching);
            ProcessMotionMatching();
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& InVelocity, bool bInIsInAir, bool bInIsCrouching)
{
    CurrentMotionData.Velocity = InVelocity;
    CurrentMotionData.Speed = InVelocity.Size();
    CurrentMotionData.bIsInAir = bInIsInAir;
    CurrentMotionData.bIsCrouching = bInIsCrouching;
    
    // Calculate movement direction relative to character forward
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = InVelocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Determine if moving left or right
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection);
        if (CrossProduct.Z < 0)
        {
            CurrentMotionData.Direction *= -1.0f;
        }
    }
}

void UAnim_MotionMatchingSystem::ProcessMotionMatching()
{
    FName OptimalState = DetermineOptimalState();
    
    if (OptimalState != CurrentAnimationState && CanTransitionToState(OptimalState))
    {
        RequestStateTransition(OptimalState);
    }
}

FName UAnim_MotionMatchingSystem::DetermineOptimalState() const
{
    // Priority-based state determination
    if (CurrentMotionData.bIsInAir)
    {
        return TEXT("Jump");
    }
    
    if (CurrentMotionData.bIsCrouching)
    {
        return TEXT("Crouch");
    }
    
    if (CurrentMotionData.Speed > MotionMatchingThreshold * 2.0f)
    {
        return TEXT("Run");
    }
    
    if (CurrentMotionData.Speed > 10.0f)
    {
        return TEXT("Walk");
    }
    
    return TEXT("Idle");
}

bool UAnim_MotionMatchingSystem::CanTransitionToState(FName TargetState) const
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Check cooldown
    if (CurrentTime - LastTransitionTime < StateTransitionCooldown)
    {
        return false;
    }
    
    // Check if transition is defined
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if ((Transition.FromState == CurrentAnimationState || Transition.FromState == TEXT("Any")) 
            && Transition.ToState == TargetState)
        {
            return true;
        }
    }
    
    return false;
}

void UAnim_MotionMatchingSystem::RequestStateTransition(FName NewState)
{
    if (NewState == CurrentAnimationState)
    {
        return;
    }
    
    FName OldState = CurrentAnimationState;
    CurrentAnimationState = NewState;
    LastTransitionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Fire Blueprint event
    OnStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Animation State Changed: %s -> %s"), *OldState.ToString(), *NewState.ToString());
}

UBlendSpace* UAnim_MotionMatchingSystem::GetCurrentBlendSpace() const
{
    if (UBlendSpace* const* BlendSpacePtr = MovementBlendSpaces.Find(CurrentAnimationState))
    {
        return *BlendSpacePtr;
    }
    
    return nullptr;
}

UAnimMontage* UAnim_MotionMatchingSystem::GetActionMontage(FName ActionName) const
{
    if (UAnimMontage* const* MontagePtr = ActionMontages.Find(ActionName))
    {
        return *MontagePtr;
    }
    
    return nullptr;
}