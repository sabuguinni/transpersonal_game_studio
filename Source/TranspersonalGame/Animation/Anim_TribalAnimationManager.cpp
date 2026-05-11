#include "Anim_TribalAnimationManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

UAnim_TribalAnimationManager::UAnim_TribalAnimationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentMovementState = EAnim_TribalMovementState::Idle;
    CurrentActionState = EAnim_TribalActionState::None;
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    CachedAnimInstance = nullptr;
}

void UAnim_TribalAnimationManager::BeginPlay()
{
    Super::BeginPlay();
    
    CacheAnimInstance();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Tribal Animation Manager initialized"));
    }
}

void UAnim_TribalAnimationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationStates();
}

void UAnim_TribalAnimationManager::CacheAnimInstance()
{
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            CachedAnimInstance = MeshComp->GetAnimInstance();
            
            if (CachedAnimInstance)
            {
                UE_LOG(LogTemp, Log, TEXT("Tribal Animation Manager: Cached AnimInstance successfully"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Tribal Animation Manager: Failed to cache AnimInstance"));
            }
        }
    }
}

void UAnim_TribalAnimationManager::UpdateAnimationStates()
{
    if (!CachedAnimInstance)
    {
        CacheAnimInstance();
        return;
    }
    
    // Update movement state based on speed
    EAnim_TribalMovementState NewMovementState = CurrentMovementState;
    
    if (MovementSpeed <= 0.1f)
    {
        NewMovementState = EAnim_TribalMovementState::Idle;
    }
    else if (MovementSpeed <= 300.0f)
    {
        NewMovementState = EAnim_TribalMovementState::Walking;
    }
    else
    {
        NewMovementState = EAnim_TribalMovementState::Running;
    }
    
    if (NewMovementState != CurrentMovementState)
    {
        TransitionToMovementState(NewMovementState);
    }
}

void UAnim_TribalAnimationManager::SetMovementState(EAnim_TribalMovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        TransitionToMovementState(NewState);
    }
}

void UAnim_TribalAnimationManager::TransitionToMovementState(EAnim_TribalMovementState NewState)
{
    CurrentMovementState = NewState;
    
    // Stop current action montage when transitioning movement states
    if (CurrentActionState != EAnim_TribalActionState::None)
    {
        StopCurrentMontage(0.2f);
        CurrentActionState = EAnim_TribalActionState::None;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Animation: Transitioned to movement state %d"), (int32)NewState);
}

void UAnim_TribalAnimationManager::SetActionState(EAnim_TribalActionState NewState)
{
    if (CurrentActionState != NewState)
    {
        CurrentActionState = NewState;
        
        // Play appropriate montage for the new action state
        if (UAnimMontage* ActionMontage = GetMontageForAction(NewState))
        {
            PlayTribalMontage(ActionMontage);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Tribal Animation: Set action state to %d"), (int32)NewState);
    }
}

void UAnim_TribalAnimationManager::PlayTribalMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!CachedAnimInstance || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tribal Animation: Cannot play montage - missing AnimInstance or Montage"));
        return;
    }
    
    // Stop any currently playing montage
    if (IsPlayingMontage())
    {
        StopCurrentMontage(0.1f);
    }
    
    // Play the new montage
    float MontageLength = CachedAnimInstance->Montage_Play(Montage, PlayRate);
    
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Tribal Animation: Playing montage %s (Length: %.2f)"), 
            *Montage->GetName(), MontageLength);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Tribal Animation: Failed to play montage %s"), 
            *Montage->GetName());
    }
}

void UAnim_TribalAnimationManager::StopCurrentMontage(float BlendOutTime)
{
    if (CachedAnimInstance && IsPlayingMontage())
    {
        CachedAnimInstance->Montage_Stop(BlendOutTime);
        UE_LOG(LogTemp, Log, TEXT("Tribal Animation: Stopped current montage"));
    }
}

bool UAnim_TribalAnimationManager::IsPlayingMontage() const
{
    if (!CachedAnimInstance)
    {
        return false;
    }
    
    return CachedAnimInstance->IsAnyMontagePlaying();
}

void UAnim_TribalAnimationManager::PlayGatheringAnimation()
{
    SetActionState(EAnim_TribalActionState::Gathering);
}

void UAnim_TribalAnimationManager::PlayCraftingAnimation()
{
    SetActionState(EAnim_TribalActionState::Crafting);
}

void UAnim_TribalAnimationManager::PlayHuntingAnimation()
{
    SetActionState(EAnim_TribalActionState::Hunting);
}

void UAnim_TribalAnimationManager::PlayCombatAnimation(bool bIsAttacking)
{
    if (bIsAttacking)
    {
        SetActionState(EAnim_TribalActionState::Fighting);
    }
    else
    {
        // Play defend animation if we have one
        if (AnimationSet.DefendMontage)
        {
            PlayTribalMontage(AnimationSet.DefendMontage);
        }
    }
}

void UAnim_TribalAnimationManager::UpdateMovementBlendSpace(float Speed, float Direction)
{
    MovementSpeed = Speed;
    MovementDirection = Direction;
    
    // Update blend space parameters in the animation instance
    if (CachedAnimInstance && AnimationSet.MovementBlendSpace)
    {
        // Set blend space parameters - these would typically be used in the AnimBP
        // The actual implementation depends on how the blend space is set up
        UE_LOG(LogTemp, VeryVerbose, TEXT("Tribal Animation: Updated blend space - Speed: %.2f, Direction: %.2f"), 
            Speed, Direction);
    }
}

UAnimMontage* UAnim_TribalAnimationManager::GetMontageForAction(EAnim_TribalActionState Action)
{
    switch (Action)
    {
        case EAnim_TribalActionState::Gathering:
            return AnimationSet.GatheringMontage;
        case EAnim_TribalActionState::Crafting:
            return AnimationSet.CraftingMontage;
        case EAnim_TribalActionState::Hunting:
            return AnimationSet.HuntingMontage;
        case EAnim_TribalActionState::Fighting:
            return AnimationSet.AttackMontage;
        default:
            return nullptr;
    }
}