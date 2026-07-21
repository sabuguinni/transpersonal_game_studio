#include "Anim_PrimitiveHumanAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_PrimitiveHumanAnimInstance::UAnim_PrimitiveHumanAnimInstance()
{
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    bIsPerformingGesture = false;
    CurrentGestureName = NAME_None;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    // Initialize IK data
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    
    // Smoothing parameters
    DirectionSmoothingSpeed = 10.0f;
    LeanSmoothingSpeed = 5.0f;
    LastUpdateTime = 0.0f;
    LastVelocity = FVector::ZeroVector;
    
    // Initialize tribal gestures with default data
    TribalGestures.Empty();
    
    // Add default tribal gestures
    FAnim_TribalGestureData HuntingGesture;
    HuntingGesture.GestureName = FName("Hunting_Point");
    HuntingGesture.Duration = 2.5f;
    HuntingGesture.bIsLooping = false;
    HuntingGesture.Priority = 1.0f;
    TribalGestures.Add(HuntingGesture);
    
    FAnim_TribalGestureData GreetingGesture;
    GreetingGesture.GestureName = FName("Tribal_Greeting");
    GreetingGesture.Duration = 3.0f;
    GreetingGesture.bIsLooping = false;
    GreetingGesture.Priority = 0.8f;
    TribalGestures.Add(GreetingGesture);
    
    FAnim_TribalGestureData CraftingGesture;
    CraftingGesture.GestureName = FName("Crafting_Motion");
    CraftingGesture.Duration = 4.0f;
    CraftingGesture.bIsLooping = true;
    CraftingGesture.Priority = 0.6f;
    TribalGestures.Add(CraftingGesture);
}

void UAnim_PrimitiveHumanAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PrimitiveHumanAnimInstance: Initialized for character %s"), *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveHumanAnimInstance: Failed to get owning character"));
    }
    
    // Initialize timing
    if (UWorld* World = GetWorld())
    {
        LastUpdateTime = World->GetTimeSeconds();
    }
}

void UAnim_PrimitiveHumanAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data
    UpdateMovementData();
    
    // Update movement state
    UpdateMovementState();
    
    // Update direction and lean
    CalculateDirection();
    UpdateLeanAmount();
    
    // Cache current time
    if (UWorld* World = GetWorld())
    {
        LastUpdateTime = World->GetTimeSeconds();
    }
}

void UAnim_PrimitiveHumanAnimInstance::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity and calculate speed
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size2D();
    MovementData.bIsMoving = MovementData.Speed > 3.0f;
    
    // Calculate acceleration
    if (UWorld* World = GetWorld())
    {
        float CurrentTime = World->GetTimeSeconds();
        float TimeDelta = CurrentTime - LastUpdateTime;
        
        if (TimeDelta > 0.0f)
        {
            MovementData.Acceleration = (MovementData.Velocity - LastVelocity) / TimeDelta;
        }
        
        LastVelocity = MovementData.Velocity;
    }
    
    // Update movement flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_PrimitiveHumanAnimInstance::UpdateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewState = CurrentMovementState;
    
    // Determine new state based on movement component
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.Speed > 300.0f) // Running threshold
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (MovementData.Speed > 3.0f) // Walking threshold
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }
    
    // Update state if changed
    if (NewState != CurrentMovementState)
    {
        CurrentMovementState = NewState;
        UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman: Movement state changed to %d"), (int32)CurrentMovementState);
    }
}

void UAnim_PrimitiveHumanAnimInstance::CalculateDirection()
{
    if (!OwningCharacter || MovementData.Speed < 3.0f)
    {
        MovementData.Direction = 0.0f;
        return;
    }
    
    // Get forward vector and velocity direction
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    FVector VelocityDirection = MovementData.Velocity.GetSafeNormal2D();
    
    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    float TargetDirection = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    
    // Smooth the direction change
    if (UWorld* World = GetWorld())
    {
        float DeltaTime = World->GetDeltaSeconds();
        MovementData.Direction = FMath::FInterpTo(MovementData.Direction, TargetDirection, DeltaTime, DirectionSmoothingSpeed);
    }
}

void UAnim_PrimitiveHumanAnimInstance::UpdateLeanAmount()
{
    if (!OwningCharacter)
    {
        MovementData.LeanAmount = 0.0f;
        return;
    }
    
    // Calculate lean based on acceleration and turning
    float AccelerationMagnitude = MovementData.Acceleration.Size2D();
    float TargetLean = FMath::Clamp(AccelerationMagnitude / 1000.0f, -1.0f, 1.0f);
    
    // Apply direction influence
    if (FMath::Abs(MovementData.Direction) > 45.0f)
    {
        TargetLean *= FMath::Sign(MovementData.Direction);
    }
    
    // Smooth the lean
    if (UWorld* World = GetWorld())
    {
        float DeltaTime = World->GetDeltaSeconds();
        MovementData.LeanAmount = FMath::FInterpTo(MovementData.LeanAmount, TargetLean, DeltaTime, LeanSmoothingSpeed);
    }
}

void UAnim_PrimitiveHumanAnimInstance::PlayTribalGesture(FName GestureName)
{
    if (bIsPerformingGesture)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveHuman: Cannot play gesture %s - already performing %s"), *GestureName.ToString(), *CurrentGestureName.ToString());
        return;
    }
    
    // Find the gesture data
    FAnim_TribalGestureData* GestureData = TribalGestures.FindByPredicate([GestureName](const FAnim_TribalGestureData& Gesture)
    {
        return Gesture.GestureName == GestureName;
    });
    
    if (!GestureData)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveHuman: Gesture %s not found"), *GestureName.ToString());
        return;
    }
    
    if (!GestureData->GestureMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveHuman: Gesture %s has no montage assigned"), *GestureName.ToString());
        return;
    }
    
    // Play the montage
    float MontageDuration = Montage_Play(GestureData->GestureMontage, 1.0f);
    if (MontageDuration > 0.0f)
    {
        bIsPerformingGesture = true;
        CurrentGestureName = GestureName;
        
        // Bind to montage end event
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UAnim_PrimitiveHumanAnimInstance::OnGestureMontageEnded);
        Montage_SetEndDelegate(EndDelegate, GestureData->GestureMontage);
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman: Playing gesture %s (duration: %f)"), *GestureName.ToString(), MontageDuration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveHuman: Failed to play gesture %s"), *GestureName.ToString());
    }
}

void UAnim_PrimitiveHumanAnimInstance::StopCurrentGesture()
{
    if (!bIsPerformingGesture)
    {
        return;
    }
    
    // Stop current montage
    Montage_Stop(0.2f);
    
    bIsPerformingGesture = false;
    CurrentGestureName = NAME_None;
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman: Stopped current gesture"));
}

void UAnim_PrimitiveHumanAnimInstance::SetActionState(EAnim_ActionState NewActionState)
{
    if (CurrentActionState != NewActionState)
    {
        CurrentActionState = NewActionState;
        UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman: Action state changed to %d"), (int32)CurrentActionState);
        
        // Auto-play appropriate gestures for certain actions
        switch (NewActionState)
        {
            case EAnim_ActionState::Crafting:
                PlayTribalGesture(FName("Crafting_Motion"));
                break;
            case EAnim_ActionState::Hunting:
                PlayTribalGesture(FName("Hunting_Point"));
                break;
            case EAnim_ActionState::Socializing:
                PlayTribalGesture(FName("Tribal_Greeting"));
                break;
            default:
                // Stop gesture for other states
                if (bIsPerformingGesture)
                {
                    StopCurrentGesture();
                }
                break;
        }
    }
}

void UAnim_PrimitiveHumanAnimInstance::UpdateFootIK(const FVector& LeftOffset, const FVector& RightOffset, float LeftAlpha, float RightAlpha)
{
    LeftFootIKOffset = LeftOffset;
    RightFootIKOffset = RightOffset;
    LeftFootIKAlpha = FMath::Clamp(LeftAlpha, 0.0f, 1.0f);
    RightFootIKAlpha = FMath::Clamp(RightAlpha, 0.0f, 1.0f);
}

void UAnim_PrimitiveHumanAnimInstance::OnGestureMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bIsPerformingGesture)
    {
        UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman: Gesture %s ended (interrupted: %s)"), 
               *CurrentGestureName.ToString(), 
               bInterrupted ? TEXT("true") : TEXT("false"));
        
        bIsPerformingGesture = false;
        CurrentGestureName = NAME_None;
    }
}