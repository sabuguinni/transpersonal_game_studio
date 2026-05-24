#include "Anim_SurvivalMovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_SurvivalMovementSystem::UAnim_SurvivalMovementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 500.0f;
    StaminaDepletionRate = 0.1f;
    FearIncreaseRate = 0.05f;
    InjuryMovementPenalty = 0.3f;
    
    // Initialize movement data
    MovementData = FAnim_SurvivalMovementData();
}

void UAnim_SurvivalMovementSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeReferences();
}

void UAnim_SurvivalMovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementData(DeltaTime);
        CalculateMovementState();
        UpdateStanceBasedOnSurvival();
        ApplySurvivalEffects();
    }
}

void UAnim_SurvivalMovementSystem::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (!MovementComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalMovementSystem: No CharacterMovementComponent found"));
        }
        
        if (!MeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalMovementSystem: No SkeletalMeshComponent found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UAnim_SurvivalMovementSystem: Owner is not a Character"));
    }
}

void UAnim_SurvivalMovementSystem::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent) return;
    
    // Get current velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityDirection);
        if (RightDot < 0.0f)
        {
            MovementData.Direction = -MovementData.Direction;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsMoving = MovementData.Speed > 1.0f;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Track movement state changes
    if (MovementData.bIsMoving != bWasMovingLastFrame)
    {
        LastMovementChangeTime = GetWorld()->GetTimeSeconds();
    }
    bWasMovingLastFrame = MovementData.bIsMoving;
}

void UAnim_SurvivalMovementSystem::CalculateMovementState()
{
    EAnim_SurvivalMovementState NewState = EAnim_SurvivalMovementState::Idle;
    
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            NewState = EAnim_SurvivalMovementState::Jumping;
        }
        else
        {
            NewState = EAnim_SurvivalMovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        if (MovementData.bIsMoving)
        {
            NewState = EAnim_SurvivalMovementState::Crawling;
        }
        else
        {
            NewState = EAnim_SurvivalMovementState::Crouching;
        }
    }
    else if (MovementData.bIsMoving)
    {
        // Determine movement speed state based on stamina and health
        float EffectiveSpeed = MovementData.Speed * GetMovementSpeedMultiplier();
        
        if (IsExhausted())
        {
            NewState = EAnim_SurvivalMovementState::Exhausted;
        }
        else if (EffectiveSpeed >= SprintSpeedThreshold)
        {
            NewState = EAnim_SurvivalMovementState::Sprinting;
        }
        else if (EffectiveSpeed >= RunSpeedThreshold)
        {
            NewState = EAnim_SurvivalMovementState::Running;
        }
        else if (EffectiveSpeed >= WalkSpeedThreshold)
        {
            NewState = EAnim_SurvivalMovementState::Walking;
        }
        else
        {
            NewState = EAnim_SurvivalMovementState::Walking; // Slow walk
        }
    }
    else
    {
        NewState = EAnim_SurvivalMovementState::Idle;
    }
    
    if (NewState != MovementData.MovementState)
    {
        SetMovementState(NewState);
    }
}

void UAnim_SurvivalMovementSystem::UpdateStanceBasedOnSurvival()
{
    EAnim_SurvivalStance NewStance = MovementData.CurrentStance;
    
    // Determine stance based on survival parameters
    if (IsInjured())
    {
        NewStance = EAnim_SurvivalStance::Injured;
    }
    else if (IsFearful())
    {
        NewStance = EAnim_SurvivalStance::Fearful;
    }
    else if (IsExhausted())
    {
        NewStance = EAnim_SurvivalStance::Cautious;
    }
    else if (MovementData.FearLevel > 0.3f)
    {
        NewStance = EAnim_SurvivalStance::Alert;
    }
    else if (MovementData.StaminaPercentage > 0.8f && MovementData.HealthPercentage > 0.8f)
    {
        NewStance = EAnim_SurvivalStance::Relaxed;
    }
    else
    {
        NewStance = EAnim_SurvivalStance::Cautious;
    }
    
    if (NewStance != MovementData.CurrentStance)
    {
        SetSurvivalStance(NewStance);
    }
}

void UAnim_SurvivalMovementSystem::ApplySurvivalEffects()
{
    if (!MovementComponent) return;
    
    // Apply movement speed penalties based on health and stamina
    float SpeedMultiplier = GetMovementSpeedMultiplier();
    
    // This would typically be handled by the character's movement component
    // For now, we just track the multiplier for animation purposes
}

void UAnim_SurvivalMovementSystem::SetMovementState(EAnim_SurvivalMovementState NewState)
{
    if (MovementData.MovementState != NewState)
    {
        EAnim_SurvivalMovementState PreviousState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        
        // Handle state-specific logic
        switch (NewState)
        {
            case EAnim_SurvivalMovementState::Jumping:
                if (JumpMontage)
                {
                    PlayMovementMontage(JumpMontage);
                }
                break;
                
            case EAnim_SurvivalMovementState::Landing:
                if (LandMontage)
                {
                    PlayMovementMontage(LandMontage);
                }
                break;
                
            case EAnim_SurvivalMovementState::Crouching:
                if (CrouchMontage && PreviousState != EAnim_SurvivalMovementState::Crawling)
                {
                    PlayMovementMontage(CrouchMontage);
                }
                break;
                
            case EAnim_SurvivalMovementState::Climbing:
                if (ClimbMontage)
                {
                    PlayMovementMontage(ClimbMontage);
                }
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_SurvivalMovementSystem::SetSurvivalStance(EAnim_SurvivalStance NewStance)
{
    if (MovementData.CurrentStance != NewStance)
    {
        MovementData.CurrentStance = NewStance;
        StanceTransitionTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Survival stance changed to %d"), (int32)NewStance);
    }
}

void UAnim_SurvivalMovementSystem::PlayMovementMontage(UAnimMontage* Montage)
{
    if (Montage && MeshComponent && MeshComponent->GetAnimInstance())
    {
        MeshComponent->GetAnimInstance()->Montage_Play(Montage);
    }
}

void UAnim_SurvivalMovementSystem::UpdateSurvivalParameters(float StaminaLevel, float HealthLevel, float CurrentFear)
{
    MovementData.StaminaPercentage = FMath::Clamp(StaminaLevel, 0.0f, 1.0f);
    MovementData.HealthPercentage = FMath::Clamp(HealthLevel, 0.0f, 1.0f);
    MovementData.FearLevel = FMath::Clamp(CurrentFear, 0.0f, 1.0f);
}

bool UAnim_SurvivalMovementSystem::IsExhausted() const
{
    return MovementData.StaminaPercentage <= 0.1f;
}

bool UAnim_SurvivalMovementSystem::IsInjured() const
{
    return MovementData.HealthPercentage <= 0.5f;
}

bool UAnim_SurvivalMovementSystem::IsFearful() const
{
    return MovementData.FearLevel >= 0.7f;
}

float UAnim_SurvivalMovementSystem::GetMovementSpeedMultiplier() const
{
    float Multiplier = 1.0f;
    
    // Apply health penalty
    if (IsInjured())
    {
        Multiplier *= (1.0f - InjuryMovementPenalty);
    }
    
    // Apply stamina penalty
    if (MovementData.StaminaPercentage < 0.3f)
    {
        Multiplier *= (0.5f + MovementData.StaminaPercentage);
    }
    
    // Apply fear effects
    if (IsFearful())
    {
        // Fear can either slow down (paralysis) or speed up (panic)
        if (MovementData.FearLevel > 0.9f)
        {
            Multiplier *= 0.6f; // Paralyzed by fear
        }
        else
        {
            Multiplier *= 1.2f; // Adrenaline boost
        }
    }
    
    return FMath::Clamp(Multiplier, 0.1f, 2.0f);
}