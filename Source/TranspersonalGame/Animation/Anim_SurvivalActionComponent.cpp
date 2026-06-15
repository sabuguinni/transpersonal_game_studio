#include "Anim_SurvivalActionComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAnim_SurvivalActionComponent::UAnim_SurvivalActionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    CurrentAction = EAnim_SurvivalAction::None;
    bIsPerformingAction = false;
    ActionProgress = 0.0f;
    ActionSpeedMultiplier = 1.0f;
    CurrentMontage = nullptr;
    SurvivalActionsTable = nullptr;
}

void UAnim_SurvivalActionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize component
    CurrentAction = EAnim_SurvivalAction::None;
    bIsPerformingAction = false;
    ActionProgress = 0.0f;
}

void UAnim_SurvivalActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsPerformingAction && CurrentMontage)
    {
        // Update action progress based on montage playback
        UAnimInstance* AnimInstance = GetOwnerAnimInstance();
        if (AnimInstance && AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            float MontagePosition = AnimInstance->Montage_GetPosition(CurrentMontage);
            float MontageLength = CurrentMontage->GetPlayLength();
            ActionProgress = FMath::Clamp(MontagePosition / MontageLength, 0.0f, 1.0f);
        }
    }
}

bool UAnim_SurvivalActionComponent::StartSurvivalAction(EAnim_SurvivalAction Action, AActor* TargetActor)
{
    if (bIsPerformingAction)
    {
        return false; // Already performing an action
    }

    if (!CanPerformAction(Action))
    {
        return false;
    }

    FAnim_SurvivalActionData* ActionData = GetActionData(Action);
    if (!ActionData || !ActionData->ActionMontage)
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetOwnerAnimInstance();
    if (!AnimInstance)
    {
        return false;
    }

    // Start the animation montage
    float MontageLength = AnimInstance->Montage_Play(ActionData->ActionMontage, ActionSpeedMultiplier);
    if (MontageLength <= 0.0f)
    {
        return false;
    }

    // Set up action state
    CurrentAction = Action;
    CurrentMontage = ActionData->ActionMontage;
    bIsPerformingAction = true;
    ActionProgress = 0.0f;

    // Bind montage end delegate
    FOnMontageEnded MontageEndDelegate;
    MontageEndDelegate.BindUFunction(this, FName("OnMontageEnded"));
    AnimInstance->Montage_SetEndDelegate(MontageEndDelegate, ActionData->ActionMontage);

    // Set timer for action completion
    float ActionDuration = ActionData->Duration / ActionSpeedMultiplier;
    GetWorld()->GetTimerManager().SetTimer(
        ActionTimerHandle,
        this,
        &UAnim_SurvivalActionComponent::OnActionCompleted,
        ActionDuration,
        false
    );

    // Play action sound
    if (ActionData->ActionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ActionData->ActionSound,
            GetOwner()->GetActorLocation()
        );
    }

    // Broadcast action started
    OnSurvivalActionStarted.Broadcast(Action, ActionDuration);

    return true;
}

void UAnim_SurvivalActionComponent::StopCurrentAction()
{
    if (!bIsPerformingAction)
    {
        return;
    }

    UAnimInstance* AnimInstance = GetOwnerAnimInstance();
    if (AnimInstance && CurrentMontage)
    {
        AnimInstance->Montage_Stop(0.2f, CurrentMontage);
    }

    // Clear timer
    if (ActionTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);
    }

    // Broadcast cancellation
    OnSurvivalActionCancelled.Broadcast(CurrentAction);

    // Reset state
    CurrentAction = EAnim_SurvivalAction::None;
    CurrentMontage = nullptr;
    bIsPerformingAction = false;
    ActionProgress = 0.0f;
}

bool UAnim_SurvivalActionComponent::CanPerformAction(EAnim_SurvivalAction Action) const
{
    if (bIsPerformingAction)
    {
        return false;
    }

    if (Action == EAnim_SurvivalAction::None)
    {
        return false;
    }

    FAnim_SurvivalActionData* ActionData = GetActionData(Action);
    if (!ActionData)
    {
        return false;
    }

    // Check if tools are required
    if (ActionData->bRequiresTools)
    {
        // TODO: Check if player has required tools
        // This would integrate with inventory system
    }

    // TODO: Check stamina requirements
    // This would integrate with character stats system

    return true;
}

void UAnim_SurvivalActionComponent::OnActionCompleted()
{
    if (!bIsPerformingAction)
    {
        return;
    }

    EAnim_SurvivalAction CompletedAction = CurrentAction;

    // Reset state
    CurrentAction = EAnim_SurvivalAction::None;
    CurrentMontage = nullptr;
    bIsPerformingAction = false;
    ActionProgress = 1.0f;

    // Clear timer
    if (ActionTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);
    }

    // Broadcast completion
    OnSurvivalActionCompleted.Broadcast(CompletedAction);
}

void UAnim_SurvivalActionComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        if (bInterrupted)
        {
            StopCurrentAction();
        }
        else
        {
            OnActionCompleted();
        }
    }
}

FAnim_SurvivalActionData* UAnim_SurvivalActionComponent::GetActionData(EAnim_SurvivalAction Action) const
{
    if (!SurvivalActionsTable)
    {
        return nullptr;
    }

    FString ActionName = UEnum::GetValueAsString(Action);
    ActionName = ActionName.Replace(TEXT("EAnim_SurvivalAction::"), TEXT(""));

    return SurvivalActionsTable->FindRow<FAnim_SurvivalActionData>(FName(*ActionName), TEXT(""));
}

UAnimInstance* UAnim_SurvivalActionComponent::GetOwnerAnimInstance() const
{
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            return SkeletalMesh->GetAnimInstance();
        }
    }
    return nullptr;
}