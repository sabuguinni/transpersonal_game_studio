#include "Anim_SurvivalActionController.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Characters/TranspersonalCharacter.h"

UAnim_SurvivalActionController::UAnim_SurvivalActionController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    bIsPerformingAction = false;
    bCanInterruptAction = true;
    CurrentActionType = EAnim_SurvivalActionType::None;
    ActionBlendInTime = 0.2f;
    ActionBlendOutTime = 0.2f;
    ActionPlayRate = 1.0f;
    
    // Initialize montage references to null
    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    HuntingMontage = nullptr;
    BuildingMontage = nullptr;
    RestingMontage = nullptr;
    ClimbingMontage = nullptr;
    SwimmingMontage = nullptr;
    
    // Initialize action durations
    ActionDurations.Add(EAnim_SurvivalActionType::Gathering, 3.0f);
    ActionDurations.Add(EAnim_SurvivalActionType::Crafting, 5.0f);
    ActionDurations.Add(EAnim_SurvivalActionType::Hunting, 2.0f);
    ActionDurations.Add(EAnim_SurvivalActionType::Building, 4.0f);
    ActionDurations.Add(EAnim_SurvivalActionType::Resting, 10.0f);
    ActionDurations.Add(EAnim_SurvivalActionType::Climbing, 1.5f);
    ActionDurations.Add(EAnim_SurvivalActionType::Swimming, 0.0f); // Continuous
}

void UAnim_SurvivalActionController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character
    OwnerCharacter = Cast&lt;ACharacter&gt;(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalActionController: Owner is not a Character"));
        return;
    }
    
    // Get skeletal mesh component
    SkeletalMeshComp = OwnerCharacter-&gt;GetMesh();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalActionController: No SkeletalMeshComponent found"));
        return;
    }
    
    // Get anim instance
    AnimInstance = SkeletalMeshComp-&gt;GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalActionController: No AnimInstance found"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalActionController initialized successfully"));
}

void UAnim_SurvivalActionController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update action timer if performing an action
    if (bIsPerformingAction && CurrentActionType != EAnim_SurvivalActionType::None)
    {
        ActionTimer += DeltaTime;
        
        // Check if action should complete
        float* ActionDuration = ActionDurations.Find(CurrentActionType);
        if (ActionDuration && *ActionDuration > 0.0f && ActionTimer >= *ActionDuration)
        {
            CompleteCurrentAction();
        }
    }
}

bool UAnim_SurvivalActionController::StartSurvivalAction(EAnim_SurvivalActionType ActionType, bool bForceInterrupt)
{
    // Check if we can start a new action
    if (bIsPerformingAction && !bCanInterruptAction && !bForceInterrupt)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start action - current action cannot be interrupted"));
        return false;
    }
    
    // Stop current action if any
    if (bIsPerformingAction)
    {
        StopCurrentAction();
    }
    
    // Get the appropriate montage for this action
    UAnimMontage* MontageToPlay = GetMontageForAction(ActionType);
    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("No montage found for action type: %d"), (int32)ActionType);
        return false;
    }
    
    // Play the montage
    if (AnimInstance && SkeletalMeshComp)
    {
        float MontageLength = AnimInstance-&gt;Montage_Play(MontageToPlay, ActionPlayRate);
        if (MontageLength > 0.0f)
        {
            // Set action state
            CurrentActionType = ActionType;
            bIsPerformingAction = true;
            ActionTimer = 0.0f;
            
            // Bind montage end delegate
            FOnMontageEnded MontageEndDelegate;
            MontageEndDelegate.BindUObject(this, &UAnim_SurvivalActionController::OnMontageEnded);
            AnimInstance-&gt;Montage_SetEndDelegate(MontageEndDelegate, MontageToPlay);
            
            // Broadcast action started
            OnSurvivalActionStarted.Broadcast(ActionType);
            
            UE_LOG(LogTemp, Log, TEXT("Started survival action: %d"), (int32)ActionType);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to play montage for action: %d"), (int32)ActionType);
    return false;
}

void UAnim_SurvivalActionController::StopCurrentAction()
{
    if (!bIsPerformingAction || CurrentActionType == EAnim_SurvivalActionType::None)
    {
        return;
    }
    
    // Stop the current montage
    if (AnimInstance)
    {
        UAnimMontage* CurrentMontage = GetMontageForAction(CurrentActionType);
        if (CurrentMontage && AnimInstance-&gt;Montage_IsPlaying(CurrentMontage))
        {
            AnimInstance-&gt;Montage_Stop(ActionBlendOutTime, CurrentMontage);
        }
    }
    
    // Broadcast action stopped
    OnSurvivalActionStopped.Broadcast(CurrentActionType);
    
    // Reset state
    EAnim_SurvivalActionType PreviousAction = CurrentActionType;
    CurrentActionType = EAnim_SurvivalActionType::None;
    bIsPerformingAction = false;
    ActionTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Stopped survival action: %d"), (int32)PreviousAction);
}

void UAnim_SurvivalActionController::CompleteCurrentAction()
{
    if (!bIsPerformingAction || CurrentActionType == EAnim_SurvivalActionType::None)
    {
        return;
    }
    
    EAnim_SurvivalActionType CompletedAction = CurrentActionType;
    
    // Broadcast action completed before stopping
    OnSurvivalActionCompleted.Broadcast(CompletedAction);
    
    // Stop the action
    StopCurrentAction();
    
    UE_LOG(LogTemp, Log, TEXT("Completed survival action: %d"), (int32)CompletedAction);
}

bool UAnim_SurvivalActionController::IsPerformingAction() const
{
    return bIsPerformingAction;
}

EAnim_SurvivalActionType UAnim_SurvivalActionController::GetCurrentActionType() const
{
    return CurrentActionType;
}

float UAnim_SurvivalActionController::GetActionProgress() const
{
    if (!bIsPerformingAction || CurrentActionType == EAnim_SurvivalActionType::None)
    {
        return 0.0f;
    }
    
    float* ActionDuration = ActionDurations.Find(CurrentActionType);
    if (!ActionDuration || *ActionDuration <= 0.0f)
    {
        return 0.0f; // Continuous actions have no progress
    }
    
    return FMath::Clamp(ActionTimer / *ActionDuration, 0.0f, 1.0f);
}

void UAnim_SurvivalActionController::SetActionMontage(EAnim_SurvivalActionType ActionType, UAnimMontage* Montage)
{
    switch (ActionType)
    {
        case EAnim_SurvivalActionType::Gathering:
            GatheringMontage = Montage;
            break;
        case EAnim_SurvivalActionType::Crafting:
            CraftingMontage = Montage;
            break;
        case EAnim_SurvivalActionType::Hunting:
            HuntingMontage = Montage;
            break;
        case EAnim_SurvivalActionType::Building:
            BuildingMontage = Montage;
            break;
        case EAnim_SurvivalActionType::Resting:
            RestingMontage = Montage;
            break;
        case EAnim_SurvivalActionType::Climbing:
            ClimbingMontage = Montage;
            break;
        case EAnim_SurvivalActionType::Swimming:
            SwimmingMontage = Montage;
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown action type for montage assignment: %d"), (int32)ActionType);
            break;
    }
}

UAnimMontage* UAnim_SurvivalActionController::GetMontageForAction(EAnim_SurvivalActionType ActionType) const
{
    switch (ActionType)
    {
        case EAnim_SurvivalActionType::Gathering:
            return GatheringMontage;
        case EAnim_SurvivalActionType::Crafting:
            return CraftingMontage;
        case EAnim_SurvivalActionType::Hunting:
            return HuntingMontage;
        case EAnim_SurvivalActionType::Building:
            return BuildingMontage;
        case EAnim_SurvivalActionType::Resting:
            return RestingMontage;
        case EAnim_SurvivalActionType::Climbing:
            return ClimbingMontage;
        case EAnim_SurvivalActionType::Swimming:
            return SwimmingMontage;
        default:
            return nullptr;
    }
}

void UAnim_SurvivalActionController::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bIsPerformingAction)
    {
        return;
    }
    
    // Check if this is the montage for our current action
    UAnimMontage* CurrentMontage = GetMontageForAction(CurrentActionType);
    if (Montage == CurrentMontage)
    {
        if (bInterrupted)
        {
            OnSurvivalActionInterrupted.Broadcast(CurrentActionType);
            UE_LOG(LogTemp, Log, TEXT("Survival action interrupted: %d"), (int32)CurrentActionType);
        }
        else
        {
            CompleteCurrentAction();
        }
    }
}

void UAnim_SurvivalActionController::SetActionDuration(EAnim_SurvivalActionType ActionType, float Duration)
{
    ActionDurations.Add(ActionType, Duration);
}

float UAnim_SurvivalActionController::GetActionDuration(EAnim_SurvivalActionType ActionType) const
{
    const float* Duration = ActionDurations.Find(ActionType);
    return Duration ? *Duration : 0.0f;
}

void UAnim_SurvivalActionController::SetCanInterruptAction(bool bCanInterrupt)
{
    bCanInterruptAction = bCanInterrupt;
}

bool UAnim_SurvivalActionController::CanInterruptCurrentAction() const
{
    return bCanInterruptAction;
}