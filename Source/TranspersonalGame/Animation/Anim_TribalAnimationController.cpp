#include "Anim_TribalAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_TribalAnimationController::UAnim_TribalAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentAction = EAnim_SurvivalAction::None;
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    LastUpdateTime = 0.0f;
    bIsPlayingAction = false;
    
    // IK initialization
    LeftFootOffset = 0.0f;
    RightFootOffset = 0.0f;
    HipOffset = FVector::ZeroVector;
    
    // Initialize pointers
    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
    
    // Initialize montage pointers
    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    HuntingMontage = nullptr;
    BuildingMontage = nullptr;
    EatingMontage = nullptr;
    DrinkingMontage = nullptr;
}

void UAnim_TribalAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAnimationReferences();
}

void UAnim_TribalAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationState(DeltaTime);
    
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_TribalAnimationController::InitializeAnimationReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter && OwnerCharacter->GetMesh())
    {
        AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        
        if (AnimInstance)
        {
            UE_LOG(LogTemp, Log, TEXT("Animation Controller initialized for character: %s"), *OwnerCharacter->GetName());
        }
    }
}

void UAnim_TribalAnimationController::UpdateAnimationState(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    
    // Update movement data
    MovementData.Speed = MovementComp->Velocity.Size();
    MovementData.Direction = FMath::Atan2(MovementComp->Velocity.Y, MovementComp->Velocity.X);
    MovementData.bIsInAir = MovementComp->IsFalling();
    MovementData.bIsCrouching = MovementComp->IsCrouching();
    
    // Determine movement state
    if (MovementData.bIsInAir)
    {
        MovementData.MovementState = EAnim_TribalMovementState::Jumping;
    }
    else if (MovementData.bIsCrouching)
    {
        MovementData.MovementState = EAnim_TribalMovementState::Crouching;
    }
    else if (MovementData.Speed > 300.0f)
    {
        MovementData.MovementState = EAnim_TribalMovementState::Running;
    }
    else if (MovementData.Speed > 50.0f)
    {
        MovementData.MovementState = EAnim_TribalMovementState::Walking;
    }
    else
    {
        MovementData.MovementState = EAnim_TribalMovementState::Idle;
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_TribalAnimationController::UpdateMovementData(float Speed, float Direction, bool bInAir, bool bCrouching)
{
    MovementData.Speed = Speed;
    MovementData.Direction = Direction;
    MovementData.bIsInAir = bInAir;
    MovementData.bIsCrouching = bCrouching;
}

void UAnim_TribalAnimationController::SetMovementState(EAnim_TribalMovementState NewState)
{
    MovementData.MovementState = NewState;
}

void UAnim_TribalAnimationController::PlaySurvivalAction(EAnim_SurvivalAction Action)
{
    if (bIsPlayingAction && CurrentAction == Action)
    {
        return; // Already playing this action
    }
    
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (Action)
    {
        case EAnim_SurvivalAction::Gathering:
            MontageToPlay = GatheringMontage;
            break;
        case EAnim_SurvivalAction::Crafting:
            MontageToPlay = CraftingMontage;
            break;
        case EAnim_SurvivalAction::Hunting:
            MontageToPlay = HuntingMontage;
            break;
        case EAnim_SurvivalAction::Building:
            MontageToPlay = BuildingMontage;
            break;
        case EAnim_SurvivalAction::Eating:
            MontageToPlay = EatingMontage;
            break;
        case EAnim_SurvivalAction::Drinking:
            MontageToPlay = DrinkingMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay)
    {
        PlayAnimationMontage(MontageToPlay);
        CurrentAction = Action;
        bIsPlayingAction = true;
        
        UE_LOG(LogTemp, Log, TEXT("Playing survival action: %d"), (int32)Action);
    }
}

void UAnim_TribalAnimationController::StopCurrentAction()
{
    if (bIsPlayingAction && AnimInstance)
    {
        AnimInstance->StopAllMontages(0.2f);
        CurrentAction = EAnim_SurvivalAction::None;
        bIsPlayingAction = false;
        
        UE_LOG(LogTemp, Log, TEXT("Stopped current survival action"));
    }
}

void UAnim_TribalAnimationController::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_TribalAnimationController::StopAnimationMontage(UAnimMontage* Montage)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
    }
}

void UAnim_TribalAnimationController::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
}

void UAnim_TribalAnimationController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerCharacter->GetMesh())
    {
        return;
    }
    
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    
    // Get foot bone locations
    FVector LeftFootLocation = Mesh->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootLocation = Mesh->GetBoneLocation(TEXT("foot_r"));
    
    // Calculate foot offsets
    float NewLeftFootOffset = CalculateFootOffset(LeftFootLocation, LeftFootLocation + FVector(0, 0, FootIKTraceDistance));
    float NewRightFootOffset = CalculateFootOffset(RightFootLocation, RightFootLocation + FVector(0, 0, FootIKTraceDistance));
    
    // Smooth interpolation
    LeftFootOffset = FMath::FInterpTo(LeftFootOffset, NewLeftFootOffset, DeltaTime, FootIKInterpSpeed);
    RightFootOffset = FMath::FInterpTo(RightFootOffset, NewRightFootOffset, DeltaTime, FootIKInterpSpeed);
    
    // Calculate hip offset to keep character grounded
    float HipOffsetZ = FMath::Min(LeftFootOffset, RightFootOffset);
    HipOffset = FMath::VInterpTo(HipOffset, FVector(0, 0, HipOffsetZ), DeltaTime, FootIKInterpSpeed);
    
    ApplyFootIK();
}

float UAnim_TribalAnimationController::CalculateFootOffset(const FVector& FootLocation, const FVector& TraceStart)
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    FVector TraceEnd = TraceStart - FVector(0, 0, FootIKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        float DistanceFromGround = (TraceStart - HitResult.Location).Z;
        return FootIKTraceDistance - DistanceFromGround;
    }
    
    return 0.0f;
}

void UAnim_TribalAnimationController::ApplyFootIK()
{
    // This would typically be handled in the Animation Blueprint
    // Here we just log the IK values for debugging
    
    if (LeftFootOffset != 0.0f || RightFootOffset != 0.0f)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Foot IK - Left: %f, Right: %f, Hip: %s"), 
               LeftFootOffset, RightFootOffset, *HipOffset.ToString());
    }
}