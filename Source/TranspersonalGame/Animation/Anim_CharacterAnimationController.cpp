#include "Anim_CharacterAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    // Initialize state tracking
    StateTransitionAlpha = 0.0f;
    TimeSinceLastStateChange = 0.0f;
    
    // Initialize references
    CharacterMesh = nullptr;
    AnimInstance = nullptr;
    
    // Initialize animation assets to null
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    AttackMontage = nullptr;
    DeathMontage = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to character's mesh component
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            CharacterMesh = Character->GetMesh();
            if (CharacterMesh)
            {
                AnimInstance = CharacterMesh->GetAnimInstance();
                UE_LOG(LogTemp, Log, TEXT("Animation Controller: Found character mesh and anim instance"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Animation Controller: Character mesh not found"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Animation Controller: Owner is not a Character"));
        }
    }
    
    // Validate animation assets
    ValidateAnimationAssets();
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update state transition timing
    UpdateStateTransition(DeltaTime);
    
    // Auto-update movement data from character if available
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                // Calculate speed and direction
                FVector Velocity = MovementComp->Velocity;
                float CurrentSpeed = Velocity.Size();
                float CurrentDirection = 0.0f;
                
                if (CurrentSpeed > 0.1f)
                {
                    FVector Forward = Character->GetActorForwardVector();
                    FVector Right = Character->GetActorRightVector();
                    
                    FVector NormalizedVelocity = Velocity.GetSafeNormal();
                    float ForwardDot = FVector::DotProduct(Forward, NormalizedVelocity);
                    float RightDot = FVector::DotProduct(Right, NormalizedVelocity);
                    
                    CurrentDirection = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
                }
                
                // Update movement data
                bool bInAir = MovementComp->IsFalling();
                bool bCrouching = MovementComp->IsCrouching();
                
                UpdateMovementData(CurrentSpeed, CurrentDirection, bInAir, bCrouching);
                
                // Auto-determine character state based on movement
                ECharacterState NewState = ECharacterState::Idle;
                
                if (bInAir)
                {
                    NewState = ECharacterState::Jumping;
                }
                else if (CurrentSpeed > 0.1f)
                {
                    if (CurrentSpeed > 400.0f) // Running threshold
                    {
                        NewState = ECharacterState::Running;
                        MovementData.bIsRunning = true;
                    }
                    else
                    {
                        NewState = ECharacterState::Walking;
                        MovementData.bIsRunning = false;
                    }
                }
                else
                {
                    NewState = ECharacterState::Idle;
                    MovementData.bIsRunning = false;
                }
                
                if (MovementData.CharacterState != NewState)
                {
                    SetCharacterState(NewState);
                }
            }
        }
    }
}

void UAnim_CharacterAnimationController::UpdateMovementData(float NewSpeed, float NewDirection, bool bInAir, bool bCrouching)
{
    MovementData.Speed = NewSpeed;
    MovementData.Direction = NewDirection;
    MovementData.bIsInAir = bInAir;
    MovementData.bIsCrouching = bCrouching;
    
    // Log movement data for debugging
    UE_LOG(LogTemp, VeryVerbose, TEXT("Movement Data - Speed: %.2f, Direction: %.2f, InAir: %s, Crouching: %s"), 
           NewSpeed, NewDirection, bInAir ? TEXT("true") : TEXT("false"), bCrouching ? TEXT("true") : TEXT("false"));
}

void UAnim_CharacterAnimationController::SetCharacterState(ECharacterState NewState)
{
    if (MovementData.CharacterState != NewState)
    {
        ECharacterState OldState = MovementData.CharacterState;
        MovementData.CharacterState = NewState;
        TimeSinceLastStateChange = 0.0f;
        StateTransitionAlpha = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Character State Changed: %d -> %d"), (int32)OldState, (int32)NewState);
        
        // Handle state-specific logic
        switch (NewState)
        {
            case ECharacterState::Jumping:
                if (JumpMontage && AnimInstance)
                {
                    PlayAnimationMontage(JumpMontage);
                }
                break;
                
            case ECharacterState::Dead:
                if (DeathMontage && AnimInstance)
                {
                    PlayAnimationMontage(DeathMontage);
                }
                break;
                
            default:
                break;
        }
    }
}

void UAnim_CharacterAnimationController::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (Montage && AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing animation montage: %s"), *Montage->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage - Montage or AnimInstance is null"));
    }
}

void UAnim_CharacterAnimationController::StopAnimationMontage(UAnimMontage* Montage)
{
    if (Montage && AnimInstance)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        UE_LOG(LogTemp, Log, TEXT("Stopping animation montage: %s"), *Montage->GetName());
    }
}

void UAnim_CharacterAnimationController::UpdateStateTransition(float DeltaTime)
{
    TimeSinceLastStateChange += DeltaTime;
    
    // Calculate transition alpha (0 to 1 over 0.5 seconds)
    StateTransitionAlpha = FMath::Clamp(TimeSinceLastStateChange / 0.5f, 0.0f, 1.0f);
}

void UAnim_CharacterAnimationController::ValidateAnimationAssets()
{
    int32 MissingAssets = 0;
    
    if (!MovementBlendSpace)
    {
        UE_LOG(LogTemp, Warning, TEXT("MovementBlendSpace is not assigned"));
        MissingAssets++;
    }
    
    if (!JumpMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("JumpMontage is not assigned"));
        MissingAssets++;
    }
    
    if (!AttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttackMontage is not assigned"));
        MissingAssets++;
    }
    
    if (!DeathMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("DeathMontage is not assigned"));
        MissingAssets++;
    }
    
    if (MissingAssets > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Controller: %d animation assets are missing"), MissingAssets);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Animation Controller: All animation assets validated"));
    }
}