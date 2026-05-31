#include "Anim_CharacterAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize animation state
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::None;
    CurrentSpeed = 0.0f;
    CurrentDirection = 0.0f;
    bIsInAir = false;
    bIsClimbing = false;

    // Initialize internal state
    LastMontagePlayTime = 0.0f;
    CurrentlyPlayingMontage = nullptr;
    OwnerMesh = nullptr;
    AnimInstance = nullptr;

    // Initialize animation assets to nullptr (will be set in Blueprint or code)
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    AttackLightMontage = nullptr;
    AttackHeavyMontage = nullptr;
    BlockMontage = nullptr;
    DodgeLeftMontage = nullptr;
    DodgeRightMontage = nullptr;
    CraftingMontage = nullptr;
    GatheringMontage = nullptr;
    ClimbingMontage = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    if (OwnerMesh && AnimInstance)
    {
        LogAnimationEvent(TEXT("Animation Controller initialized successfully"));
    }
    else
    {
        LogAnimationEvent(TEXT("Warning: Failed to cache component references"));
    }
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update animation state based on character movement
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        // Update speed and direction
        FVector Velocity = OwnerCharacter->GetVelocity();
        CurrentSpeed = Velocity.Size2D();
        
        if (CurrentSpeed > 1.0f)
        {
            FVector Forward = OwnerCharacter->GetActorForwardVector();
            FVector VelNormalized = Velocity.GetSafeNormal2D();
            CurrentDirection = FVector::DotProduct(Forward, VelNormalized);
        }
        else
        {
            CurrentDirection = 0.0f;
        }

        // Update movement state based on speed
        if (CurrentSpeed < 1.0f)
        {
            SetMovementState(EAnim_MovementState::Idle);
        }
        else if (CurrentSpeed < 300.0f)
        {
            SetMovementState(EAnim_MovementState::Walking);
        }
        else
        {
            SetMovementState(EAnim_MovementState::Running);
        }

        // Update air state
        bIsInAir = OwnerCharacter->GetMovementComponent()->IsFalling();
    }
}

void UAnim_CharacterAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        CurrentMovementState = NewState;
        LogAnimationEvent(FString::Printf(TEXT("Movement state changed to: %d"), (int32)NewState));
    }
}

void UAnim_CharacterAnimationController::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        LogAnimationEvent(FString::Printf(TEXT("Combat state changed to: %d"), (int32)NewState));
    }
}

void UAnim_CharacterAnimationController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!IsValidAnimationAsset(Montage) || !AnimInstance)
    {
        LogAnimationEvent(TEXT("Cannot play montage: Invalid asset or AnimInstance"));
        return;
    }

    // Stop current montage if playing
    if (CurrentlyPlayingMontage && AnimInstance->Montage_IsPlaying(CurrentlyPlayingMontage))
    {
        AnimInstance->Montage_Stop(0.2f, CurrentlyPlayingMontage);
    }

    // Play new montage
    float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
    if (Duration > 0.0f)
    {
        CurrentlyPlayingMontage = Montage;
        LastMontagePlayTime = GetWorld()->GetTimeSeconds();
        LogAnimationEvent(FString::Printf(TEXT("Playing montage: %s"), *Montage->GetName()));
    }
    else
    {
        LogAnimationEvent(FString::Printf(TEXT("Failed to play montage: %s"), *Montage->GetName()));
    }
}

void UAnim_CharacterAnimationController::StopMontage(UAnimMontage* Montage)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }

    if (AnimInstance->Montage_IsPlaying(Montage))
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        LogAnimationEvent(FString::Printf(TEXT("Stopped montage: %s"), *Montage->GetName()));
        
        if (CurrentlyPlayingMontage == Montage)
        {
            CurrentlyPlayingMontage = nullptr;
        }
    }
}

void UAnim_CharacterAnimationController::UpdateMovementBlendSpace(float Speed, float Direction)
{
    CurrentSpeed = Speed;
    CurrentDirection = Direction;
    
    // The blend space will be updated through the AnimInstance
    // This function serves as a manual override if needed
}

void UAnim_CharacterAnimationController::TriggerJumpAnimation()
{
    if (JumpMontage)
    {
        PlayMontage(JumpMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Jump animation triggered but no JumpMontage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerLandAnimation()
{
    if (LandMontage)
    {
        PlayMontage(LandMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Land animation triggered but no LandMontage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerAttackAnimation(EAnim_AttackType AttackType)
{
    UAnimMontage* AttackMontage = nullptr;
    
    switch (AttackType)
    {
        case EAnim_AttackType::Light:
            AttackMontage = AttackLightMontage;
            break;
        case EAnim_AttackType::Heavy:
            AttackMontage = AttackHeavyMontage;
            break;
        default:
            LogAnimationEvent(TEXT("Unknown attack type"));
            return;
    }

    if (AttackMontage)
    {
        SetCombatState(EAnim_CombatState::Attacking);
        PlayMontage(AttackMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Attack animation triggered but no montage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerBlockAnimation()
{
    if (BlockMontage)
    {
        SetCombatState(EAnim_CombatState::Blocking);
        PlayMontage(BlockMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Block animation triggered but no BlockMontage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerDodgeAnimation(EAnim_DodgeDirection Direction)
{
    UAnimMontage* DodgeMontage = nullptr;
    
    switch (Direction)
    {
        case EAnim_DodgeDirection::Left:
            DodgeMontage = DodgeLeftMontage;
            break;
        case EAnim_DodgeDirection::Right:
            DodgeMontage = DodgeRightMontage;
            break;
        default:
            LogAnimationEvent(TEXT("Unknown dodge direction"));
            return;
    }

    if (DodgeMontage)
    {
        SetCombatState(EAnim_CombatState::Dodging);
        PlayMontage(DodgeMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Dodge animation triggered but no montage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerCraftingAnimation()
{
    if (CraftingMontage)
    {
        PlayMontage(CraftingMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Crafting animation triggered but no CraftingMontage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerGatheringAnimation()
{
    if (GatheringMontage)
    {
        PlayMontage(GatheringMontage, 1.0f);
    }
    else
    {
        LogAnimationEvent(TEXT("Gathering animation triggered but no GatheringMontage assigned"));
    }
}

void UAnim_CharacterAnimationController::TriggerClimbingAnimation(bool bIsClimbingNow)
{
    bIsClimbing = bIsClimbingNow;
    
    if (bIsClimbingNow && ClimbingMontage)
    {
        PlayMontage(ClimbingMontage, 1.0f);
    }
    else if (!bIsClimbingNow && CurrentlyPlayingMontage == ClimbingMontage)
    {
        StopMontage(ClimbingMontage);
    }
}

void UAnim_CharacterAnimationController::CacheComponentReferences()
{
    if (AActor* Owner = GetOwner())
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (OwnerMesh)
        {
            AnimInstance = OwnerMesh->GetAnimInstance();
        }
    }
}

bool UAnim_CharacterAnimationController::IsValidAnimationAsset(UAnimationAsset* Asset) const
{
    return Asset != nullptr && IsValid(Asset);
}

void UAnim_CharacterAnimationController::LogAnimationEvent(const FString& Event) const
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("AnimController [%s]: %s"), 
                GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
                *Event));
    }
    
    UE_LOG(LogTemp, Log, TEXT("AnimController [%s]: %s"), 
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
        *Event);
}