#include "Anim_StateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"

UAnim_StateManager::UAnim_StateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize state data
    CurrentStateData = FAnim_StateData();
    PreviousStateData = FAnim_StateData();
    
    // Set default thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    
    // Initialize component references
    OwnerMeshComponent = nullptr;
    OwnerMovementComponent = nullptr;
}

void UAnim_StateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    CacheComponentReferences();
    
    // Initialize state
    if (OwnerMovementComponent)
    {
        UpdateMovementData();
        UpdateHealthAndStamina();
    }
}

void UAnim_StateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMovementComponent || !OwnerMeshComponent)
    {
        return;
    }
    
    // Store previous state for transition detection
    PreviousStateData = CurrentStateData;
    
    // Update current state data
    UpdateMovementData();
    UpdateHealthAndStamina();
    
    // Handle state transitions
    UpdateStateTransitions(DeltaTime);
}

void UAnim_StateManager::UpdateMovementState(float CurrentSpeed, bool bIsMoving, bool bIsInAir, bool bIsCrouching)
{
    EAnim_MovementState NewMovementState = CurrentStateData.MovementState;
    
    // Update basic state data
    CurrentStateData.Speed = CurrentSpeed;
    CurrentStateData.bIsInAir = bIsInAir;
    CurrentStateData.bIsCrouching = bIsCrouching;
    
    // Determine movement state based on conditions
    if (bIsInAir)
    {
        if (OwnerMovementComponent && OwnerMovementComponent->IsFalling())
        {
            NewMovementState = EAnim_MovementState::Falling;
        }
        else
        {
            NewMovementState = EAnim_MovementState::Jumping;
        }
    }
    else if (bIsCrouching)
    {
        NewMovementState = EAnim_MovementState::Crouching;
    }
    else if (bIsMoving && CurrentSpeed > 0.1f)
    {
        if (CurrentSpeed >= RunSpeedThreshold)
        {
            NewMovementState = EAnim_MovementState::Running;
        }
        else if (CurrentSpeed >= WalkSpeedThreshold)
        {
            NewMovementState = EAnim_MovementState::Walking;
        }
        else
        {
            NewMovementState = EAnim_MovementState::Idle;
        }
    }
    else
    {
        NewMovementState = EAnim_MovementState::Idle;
    }
    
    // Apply state change if different
    if (NewMovementState != CurrentStateData.MovementState)
    {
        EAnim_MovementState OldState = CurrentStateData.MovementState;
        CurrentStateData.MovementState = NewMovementState;
        OnMovementStateChanged(OldState, NewMovementState);
    }
}

void UAnim_StateManager::SetActionState(EAnim_ActionState NewActionState)
{
    if (NewActionState != CurrentStateData.ActionState)
    {
        EAnim_ActionState OldState = CurrentStateData.ActionState;
        CurrentStateData.ActionState = NewActionState;
        OnActionStateChanged(OldState, NewActionState);
    }
}

void UAnim_StateManager::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !OwnerMeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        
        // Set action state based on montage
        SetActionState(EAnim_ActionState::Attacking); // Default to attacking, can be customized
    }
}

void UAnim_StateManager::StopActionMontage()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.2f);
        SetActionState(EAnim_ActionState::None);
    }
}

bool UAnim_StateManager::IsPlayingActionMontage() const
{
    if (!OwnerMeshComponent)
    {
        return false;
    }
    
    UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
    
    return false;
}

void UAnim_StateManager::UpdateStateTransitions(float DeltaTime)
{
    // Check for automatic state transitions
    if (CurrentStateData.ActionState != EAnim_ActionState::None && !IsPlayingActionMontage())
    {
        // Action finished, return to none
        SetActionState(EAnim_ActionState::None);
    }
    
    // Update direction for blend spaces
    if (OwnerMovementComponent)
    {
        FVector Velocity = OwnerMovementComponent->Velocity;
        FVector Forward = GetOwner()->GetActorForwardVector();
        
        // Calculate direction relative to forward vector
        if (Velocity.SizeSquared() > 0.1f)
        {
            FVector NormalizedVelocity = Velocity.GetSafeNormal();
            float DotProduct = FVector::DotProduct(Forward, NormalizedVelocity);
            CurrentStateData.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
            
            // Determine if moving left or right
            FVector Right = GetOwner()->GetActorRightVector();
            float RightDot = FVector::DotProduct(Right, NormalizedVelocity);
            if (RightDot < 0.0f)
            {
                CurrentStateData.Direction *= -1.0f;
            }
        }
        else
        {
            CurrentStateData.Direction = 0.0f;
        }
    }
}

void UAnim_StateManager::OnMovementStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState)
{
    // Log state transitions for debugging
    if (GEngine)
    {
        FString OldStateName = UEnum::GetValueAsString(OldState);
        FString NewStateName = UEnum::GetValueAsString(NewState);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
            FString::Printf(TEXT("Movement State: %s -> %s"), *OldStateName, *NewStateName));
    }
    
    // Handle specific state transitions
    switch (NewState)
    {
        case EAnim_MovementState::Jumping:
            // Could trigger jump sound or effect here
            break;
        case EAnim_MovementState::Running:
            // Could trigger running sound or breathing effect
            break;
        case EAnim_MovementState::Idle:
            // Could trigger idle animations or sounds
            break;
        default:
            break;
    }
}

void UAnim_StateManager::OnActionStateChanged(EAnim_ActionState OldState, EAnim_ActionState NewState)
{
    // Log action state transitions
    if (GEngine)
    {
        FString OldStateName = UEnum::GetValueAsString(OldState);
        FString NewStateName = UEnum::GetValueAsString(NewState);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
            FString::Printf(TEXT("Action State: %s -> %s"), *OldStateName, *NewStateName));
    }
    
    // Handle specific action transitions
    switch (NewState)
    {
        case EAnim_ActionState::Attacking:
            // Could trigger combat stance or weapon draw
            break;
        case EAnim_ActionState::Crafting:
            // Could trigger crafting stance or tool selection
            break;
        case EAnim_ActionState::None:
            // Return to normal stance
            break;
        default:
            break;
    }
}

void UAnim_StateManager::UpdateMovementData()
{
    if (!OwnerMovementComponent)
    {
        return;
    }
    
    // Update speed
    FVector Velocity = OwnerMovementComponent->Velocity;
    CurrentStateData.Speed = Velocity.Size();
    
    // Update air state
    CurrentStateData.bIsInAir = OwnerMovementComponent->IsFalling() || OwnerMovementComponent->IsFlying();
    
    // Update crouching state
    CurrentStateData.bIsCrouching = OwnerMovementComponent->IsCrouching();
    
    // Auto-update movement state based on current conditions
    bool bIsMoving = CurrentStateData.Speed > 0.1f;
    UpdateMovementState(CurrentStateData.Speed, bIsMoving, CurrentStateData.bIsInAir, CurrentStateData.bIsCrouching);
}

void UAnim_StateManager::UpdateHealthAndStamina()
{
    // Try to get health and stamina from character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        // For now, use placeholder values
        // In a real implementation, these would come from a health/stamina component
        CurrentStateData.HealthRatio = 1.0f; // Placeholder
        CurrentStateData.StaminaRatio = 1.0f; // Placeholder
        
        // Could implement actual health/stamina reading here:
        // if (UHealthComponent* HealthComp = OwnerCharacter->FindComponentByClass<UHealthComponent>())
        // {
        //     CurrentStateData.HealthRatio = HealthComp->GetHealthRatio();
        // }
    }
}

void UAnim_StateManager::CacheComponentReferences()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Cache skeletal mesh component
    OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    
    // Cache character movement component
    ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
    if (OwnerCharacter)
    {
        OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
    }
    
    // Log component status
    if (GEngine)
    {
        bool bHasMesh = OwnerMeshComponent != nullptr;
        bool bHasMovement = OwnerMovementComponent != nullptr;
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Anim_StateManager: Mesh=%s Movement=%s"), 
                bHasMesh ? TEXT("OK") : TEXT("MISSING"),
                bHasMovement ? TEXT("OK") : TEXT("MISSING")));
    }
}