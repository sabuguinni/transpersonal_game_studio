#include "Anim_TribalCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

UAnim_TribalCharacterAnimInstance::UAnim_TribalCharacterAnimInstance()
{
    // Initialize default values
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    LastUpdateTime = 0.0f;
    LastVelocity = FVector::ZeroVector;
    bWasInAir = false;
    
    // IK settings
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    
    // Animation thresholds
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    FearThreshold = 0.5f;
}

void UAnim_TribalCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Tribal Animation Instance initialized for: %s"), *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get owning character in Tribal Animation Instance"));
    }
}

void UAnim_TribalCharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation data
    UpdateMovementData();
    UpdateEmotionalState();
    UpdateSurvivalData();
    UpdateFootIK();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    LastVelocity = MovementComponent->Velocity;
    bWasInAir = AnimData.bIsInAir;
}

void UAnim_TribalCharacterAnimInstance::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate speed and direction
    FVector Velocity = MovementComponent->Velocity;
    AnimData.Speed = Velocity.Size();
    AnimData.Direction = CalculateDirection(Velocity, OwningCharacter->GetActorRotation());
    
    // Update movement state
    AnimData.bIsInAir = MovementComponent->IsFalling();
    AnimData.bIsCrouching = MovementComponent->IsCrouching();
    AnimData.MovementState = CalculateMovementState();
}

void UAnim_TribalCharacterAnimInstance::UpdateEmotionalState()
{
    // Calculate emotional state based on survival conditions
    AnimData.EmotionalState = CalculateEmotionalState();
}

void UAnim_TribalCharacterAnimInstance::UpdateSurvivalData()
{
    // Update survival-related animation data
    // This would typically get data from a survival component
    // For now, using placeholder values that can be connected via Blueprint
    
    // These values would come from the character's survival component
    AnimData.HealthPercentage = FMath::Clamp(AnimData.HealthPercentage, 0.0f, 1.0f);
    AnimData.StaminaPercentage = FMath::Clamp(AnimData.StaminaPercentage, 0.0f, 1.0f);
    AnimData.FearLevel = FMath::Clamp(AnimData.FearLevel, 0.0f, 1.0f);
}

void UAnim_TribalCharacterAnimInstance::UpdateFootIK()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Get foot socket locations
    USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh();
    if (!Mesh)
    {
        return;
    }
    
    // Perform foot IK traces
    FVector LeftFootLocation = Mesh->GetSocketLocation(TEXT("foot_l"));
    FVector RightFootLocation = Mesh->GetSocketLocation(TEXT("foot_r"));
    
    // Trace for ground
    FVector LeftFootIKTarget = PerformFootTrace(LeftFootLocation, TEXT("foot_l"));
    FVector RightFootIKTarget = PerformFootTrace(RightFootLocation, TEXT("foot_r"));
    
    // Interpolate to smooth IK movement
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    AnimData.LeftFootIKLocation = FMath::VInterpTo(AnimData.LeftFootIKLocation, LeftFootIKTarget, DeltaTime, FootIKInterpSpeed);
    AnimData.RightFootIKLocation = FMath::VInterpTo(AnimData.RightFootIKLocation, RightFootIKTarget, DeltaTime, FootIKInterpSpeed);
    
    // Calculate foot rotations based on ground normal
    // This would be expanded with proper ground normal calculation
    AnimData.LeftFootIKRotation = FRotator::ZeroRotator;
    AnimData.RightFootIKRotation = FRotator::ZeroRotator;
}

FVector UAnim_TribalCharacterAnimInstance::PerformFootTrace(FVector FootLocation, FName SocketName)
{
    if (!OwningCharacter)
    {
        return FVector::ZeroVector;
    }
    
    FVector StartLocation = FootLocation + FVector(0, 0, 20.0f);
    FVector EndLocation = FootLocation - FVector(0, 0, FootIKTraceDistance);
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    return FootLocation;
}

EAnim_TribalMovementState UAnim_TribalCharacterAnimInstance::CalculateMovementState()
{
    if (AnimData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_TribalMovementState::Jumping;
        }
        else
        {
            return EAnim_TribalMovementState::Falling;
        }
    }
    
    if (AnimData.bIsCrouching)
    {
        return EAnim_TribalMovementState::Crouching;
    }
    
    if (AnimData.Speed < 10.0f)
    {
        return EAnim_TribalMovementState::Idle;
    }
    else if (AnimData.Speed < WalkSpeedThreshold)
    {
        return EAnim_TribalMovementState::Walking;
    }
    else if (AnimData.Speed < RunSpeedThreshold)
    {
        return EAnim_TribalMovementState::Running;
    }
    
    return EAnim_TribalMovementState::Running;
}

EAnim_TribalEmotionalState UAnim_TribalCharacterAnimInstance::CalculateEmotionalState()
{
    // Determine emotional state based on survival conditions
    if (AnimData.FearLevel > FearThreshold)
    {
        return EAnim_TribalEmotionalState::Fearful;
    }
    
    if (AnimData.HealthPercentage < 0.3f)
    {
        return EAnim_TribalEmotionalState::Injured;
    }
    
    if (AnimData.StaminaPercentage < 0.2f)
    {
        return EAnim_TribalEmotionalState::Tired;
    }
    
    if (AnimData.Speed > RunSpeedThreshold)
    {
        return EAnim_TribalEmotionalState::Alert;
    }
    
    return EAnim_TribalEmotionalState::Calm;
}

float UAnim_TribalCharacterAnimInstance::CalculateDirection(FVector Velocity, FRotator Rotation)
{
    if (Velocity.SizeSquared() < 1.0f)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = Rotation.Vector();
    FVector VelocityNormalized = Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

void UAnim_TribalCharacterAnimInstance::SetMovementState(EAnim_TribalMovementState NewState)
{
    if (AnimData.MovementState != NewState)
    {
        AnimData.MovementState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Tribal character movement state changed to: %d"), (int32)NewState);
    }
}

void UAnim_TribalCharacterAnimInstance::SetEmotionalState(EAnim_TribalEmotionalState NewState)
{
    if (AnimData.EmotionalState != NewState)
    {
        AnimData.EmotionalState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Tribal character emotional state changed to: %d"), (int32)NewState);
    }
}

void UAnim_TribalCharacterAnimInstance::OnStartCrafting()
{
    SetMovementState(EAnim_TribalMovementState::Crafting);
    UE_LOG(LogTemp, Log, TEXT("Tribal character started crafting animation"));
}

void UAnim_TribalCharacterAnimInstance::OnStopCrafting()
{
    SetMovementState(EAnim_TribalMovementState::Idle);
    UE_LOG(LogTemp, Log, TEXT("Tribal character stopped crafting animation"));
}

void UAnim_TribalCharacterAnimInstance::OnStartHunting()
{
    SetMovementState(EAnim_TribalMovementState::Hunting);
    SetEmotionalState(EAnim_TribalEmotionalState::Alert);
    UE_LOG(LogTemp, Log, TEXT("Tribal character started hunting animation"));
}

void UAnim_TribalCharacterAnimInstance::OnStopHunting()
{
    SetMovementState(EAnim_TribalMovementState::Idle);
    SetEmotionalState(EAnim_TribalEmotionalState::Calm);
    UE_LOG(LogTemp, Log, TEXT("Tribal character stopped hunting animation"));
}

void UAnim_TribalCharacterAnimInstance::OnTakeDamage(float DamageAmount)
{
    // Reduce health percentage (this would typically be handled by a health component)
    AnimData.HealthPercentage = FMath::Clamp(AnimData.HealthPercentage - (DamageAmount * 0.01f), 0.0f, 1.0f);
    
    if (AnimData.HealthPercentage < 0.3f)
    {
        SetEmotionalState(EAnim_TribalEmotionalState::Injured);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal character took damage: %f, Health: %f"), DamageAmount, AnimData.HealthPercentage);
}

void UAnim_TribalCharacterAnimInstance::OnStaminaChanged(float NewStamina, float MaxStamina)
{
    AnimData.StaminaPercentage = FMath::Clamp(NewStamina / MaxStamina, 0.0f, 1.0f);
    
    if (AnimData.StaminaPercentage < 0.2f)
    {
        SetEmotionalState(EAnim_TribalEmotionalState::Tired);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal character stamina changed: %f%%"), AnimData.StaminaPercentage * 100.0f);
}

void UAnim_TribalCharacterAnimInstance::OnFearLevelChanged(float NewFearLevel)
{
    AnimData.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    if (AnimData.FearLevel > FearThreshold)
    {
        SetEmotionalState(EAnim_TribalEmotionalState::Fearful);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal character fear level changed: %f"), AnimData.FearLevel);
}