#include "PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    AttackCooldown = 1.0f;
    TimeSinceLastAttack = 0.0f;
    
    // Initialize component pointers
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner character and components
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController initialized for character: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Owner is not a Character!"));
    }
    
    // Initialize animation data
    MovementData = FAnim_MovementData();
    CombatData = FAnim_CombatData();
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update animation systems
    UpdateMovementAnimation(DeltaTime);
    UpdateCombatState();
    
    // Update timing
    TimeSinceLastAttack += DeltaTime;
}

void UPrimitiveAnimationController::UpdateMovementAnimation(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Calculate current speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size2D();
    
    // Calculate movement direction relative to character facing
    CalculateMovementDirection();
    
    // Update air state
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Update movement state based on speed and conditions
    UpdateMovementState();
}

void UPrimitiveAnimationController::UpdateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewState = EAnim_MovementState::Idle;
    
    // Check air states first (highest priority)
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    // Check crouching
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    // Check ground movement
    else if (MovementData.Speed > RunThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (MovementData.Speed > WalkThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }
    
    // Update state if changed
    if (NewState != MovementData.MovementState)
    {
        SetMovementState(NewState);
    }
}

void UPrimitiveAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (MovementData.MovementState != NewState)
    {
        EAnim_MovementState OldState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)OldState, (int32)NewState);
    }
}

void UPrimitiveAnimationController::UpdateCombatState()
{
    // Update combat timing
    if (CombatData.bIsAttacking && TimeSinceLastAttack > AttackCooldown)
    {
        CombatData.bIsAttacking = false;
    }
}

void UPrimitiveAnimationController::CalculateMovementDirection()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        MovementData.Direction = 0.0f;
        return;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    if (Velocity.Size2D() > 0.1f)
    {
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        MovementData.Direction = UKismetMathLibrary::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
}

void UPrimitiveAnimationController::SetCombatState(EAnim_CombatState NewState)
{
    if (CombatData.CombatState != NewState)
    {
        EAnim_CombatState OldState = CombatData.CombatState;
        CombatData.CombatState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), 
               (int32)OldState, (int32)NewState);
    }
}

void UPrimitiveAnimationController::TriggerAttackAnimation()
{
    if (TimeSinceLastAttack >= AttackCooldown)
    {
        CombatData.bIsAttacking = true;
        TimeSinceLastAttack = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Attack animation triggered"));
    }
}

void UPrimitiveAnimationController::TriggerBlockAnimation()
{
    CombatData.bIsBlocking = !CombatData.bIsBlocking;
    
    UE_LOG(LogTemp, Log, TEXT("Block state toggled: %s"), 
           CombatData.bIsBlocking ? TEXT("ON") : TEXT("OFF"));
}

void UPrimitiveAnimationController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!MeshComponent || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage: Missing mesh component or montage"));
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s"), *Montage->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage: No AnimInstance found"));
    }
}

void UPrimitiveAnimationController::StopAllMontages()
{
    if (!MeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.2f);
        UE_LOG(LogTemp, Log, TEXT("Stopped all montages"));
    }
}