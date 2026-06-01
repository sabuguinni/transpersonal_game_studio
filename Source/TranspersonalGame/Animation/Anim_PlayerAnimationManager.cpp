#include "Anim_PlayerAnimationManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PlayerAnimationManager::UAnim_PlayerAnimationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    BlendSpaceSmoothingSpeed = 5.0f;
    StateTransitionTime = 0.2f;
    bEnableRootMotion = true;
    TimeSinceLastStateChange = 0.0f;
    bIsInitialized = false;
    
    // Initialize references
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
    CharacterMovement = nullptr;
    AnimInstance = nullptr;
}

void UAnim_PlayerAnimationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-initialize with owner if it's a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        InitializeWithCharacter(Character);
    }
}

void UAnim_PlayerAnimationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized)
    {
        UpdateMovementAnimations(DeltaTime);
        SmoothBlendSpaceInputs(DeltaTime);
        TimeSinceLastStateChange += DeltaTime;
    }
}

void UAnim_PlayerAnimationManager::InitializeWithCharacter(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_PlayerAnimationManager: Cannot initialize with null character"));
        return;
    }
    
    OwnerCharacter = Character;
    CharacterMesh = Character->GetMesh();
    CharacterMovement = Character->GetCharacterMovement();
    
    if (CharacterMesh)
    {
        AnimInstance = CharacterMesh->GetAnimInstance();
        bIsInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimationManager: Successfully initialized with character %s"), 
               *Character->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_PlayerAnimationManager: Character has no skeletal mesh component"));
    }
}

void UAnim_PlayerAnimationManager::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState != CurrentMovementState && ShouldTransitionToState(NewState))
    {
        HandleStateTransition(NewState);
    }
}

void UAnim_PlayerAnimationManager::HandleStateTransition(EAnim_MovementState NewState)
{
    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    TimeSinceLastStateChange = 0.0f;
    
    // Broadcast state change event
    OnAnimationStateChanged.Broadcast(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimationManager: State transition from %d to %d"), 
           (int32)PreviousMovementState, (int32)CurrentMovementState);
}

bool UAnim_PlayerAnimationManager::ShouldTransitionToState(EAnim_MovementState NewState) const
{
    // Prevent rapid state changes
    if (TimeSinceLastStateChange < StateTransitionTime)
    {
        return false;
    }
    
    // Add specific transition rules here if needed
    return true;
}

void UAnim_PlayerAnimationManager::UpdateMovementAnimations(float DeltaTime)
{
    if (!CharacterMovement || !OwnerCharacter)
    {
        return;
    }
    
    // Calculate movement parameters
    FVector Velocity = CharacterMovement->Velocity;
    float Speed = Velocity.Size();
    float MaxWalkSpeed = CharacterMovement->MaxWalkSpeed;
    
    // Determine movement state based on character state
    EAnim_MovementState NewState = EAnim_MovementState::Idle;
    
    if (CharacterMovement->IsFalling())
    {
        if (Velocity.Z > 0)
        {
            NewState = EAnim_MovementState::JumpStart;
        }
        else
        {
            NewState = EAnim_MovementState::JumpLoop;
        }
    }
    else if (CharacterMovement->IsCrouching())
    {
        if (Speed > 10.0f)
        {
            NewState = EAnim_MovementState::CrouchWalk;
        }
        else
        {
            NewState = EAnim_MovementState::CrouchIdle;
        }
    }
    else if (Speed > MaxWalkSpeed * 0.8f)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (Speed > 10.0f)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }
    
    // Update state
    UpdateMovementStateFromCharacter();
    SetMovementState(NewState);
    
    // Update blend space input
    float NormalizedSpeed = FMath::Clamp(Speed / MaxWalkSpeed, 0.0f, 2.0f);
    
    // Calculate direction relative to character forward
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    FVector NormalizedVelocity = Velocity.GetSafeNormal();
    float ForwardAmount = FVector::DotProduct(NormalizedVelocity, ForwardVector);
    float RightAmount = FVector::DotProduct(NormalizedVelocity, RightVector);
    
    float Direction = UKismetMathLibrary::Atan2(RightAmount, ForwardAmount) * 180.0f / PI;
    
    UpdateBlendSpaceInput(NormalizedSpeed, Direction);
}

void UAnim_PlayerAnimationManager::UpdateMovementStateFromCharacter()
{
    // This method can be overridden for custom state logic
    // Base implementation relies on UpdateMovementAnimations
}

void UAnim_PlayerAnimationManager::UpdateBlendSpaceInput(float Speed, float Direction)
{
    BlendSpaceInput.Speed = Speed;
    BlendSpaceInput.Direction = Direction;
    
    // Calculate lean angle based on movement
    if (CharacterMovement)
    {
        FVector Acceleration = CharacterMovement->GetCurrentAcceleration();
        float AccelMagnitude = Acceleration.Size();
        
        if (AccelMagnitude > 0.0f)
        {
            FVector RightVector = OwnerCharacter->GetActorRightVector();
            float LeanAmount = FVector::DotProduct(Acceleration.GetSafeNormal(), RightVector);
            BlendSpaceInput.LeanAngle = FMath::Clamp(LeanAmount * 45.0f, -45.0f, 45.0f);
        }
        else
        {
            BlendSpaceInput.LeanAngle = 0.0f;
        }
    }
}

void UAnim_PlayerAnimationManager::SmoothBlendSpaceInputs(float DeltaTime)
{
    // Smooth the blend space inputs for more natural animation transitions
    float SmoothingFactor = FMath::Clamp(BlendSpaceSmoothingSpeed * DeltaTime, 0.0f, 1.0f);
    
    // This could be expanded to smooth all blend space inputs
    // For now, we keep the direct assignment for responsiveness
}

void UAnim_PlayerAnimationManager::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_PlayerAnimationManager: Cannot play montage - invalid montage or anim instance"));
        return;
    }
    
    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate);
    
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimationManager: Playing montage %s (length: %f)"), 
               *Montage->GetName(), MontageLength);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_PlayerAnimationManager: Failed to play montage %s"), 
               *Montage->GetName());
    }
}

void UAnim_PlayerAnimationManager::StopMontage(UAnimMontage* Montage)
{
    if (!AnimInstance)
    {
        return;
    }
    
    if (Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimationManager: Stopped specific montage %s"), *Montage->GetName());
    }
    else
    {
        AnimInstance->StopAllMontages(0.2f);
        UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimationManager: Stopped all montages"));
    }
}

void UAnim_PlayerAnimationManager::SetAnimationSet(const FAnim_AnimationSet& NewAnimationSet)
{
    AnimationSet = NewAnimationSet;
    UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimationManager: Animation set updated"));
}