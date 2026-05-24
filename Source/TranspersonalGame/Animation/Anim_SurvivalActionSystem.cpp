#include "Anim_SurvivalActionSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_SurvivalActionSystem::UAnim_SurvivalActionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    CurrentAction = EAnim_SurvivalAction::None;
    ActionStartTime = 0.0f;
    ActionDuration = 0.0f;
    CharacterMesh = nullptr;
    AnimInstance = nullptr;
}

void UAnim_SurvivalActionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character mesh component
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        CharacterMesh = Character->GetMesh();
        if (CharacterMesh)
        {
            AnimInstance = CharacterMesh->GetAnimInstance();
        }
    }
    
    InitializeActionData();
}

void UAnim_SurvivalActionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update action progress if performing an action
    if (IsPerformingAction())
    {
        float ElapsedTime = GetWorld()->GetTimeSeconds() - ActionStartTime;
        if (ElapsedTime >= ActionDuration)
        {
            OnActionTimerComplete();
        }
    }
}

bool UAnim_SurvivalActionSystem::StartSurvivalAction(EAnim_SurvivalAction ActionType)
{
    if (!CanPerformAction(ActionType))
    {
        return false;
    }
    
    // Stop current action if any
    if (IsPerformingAction())
    {
        StopCurrentAction();
    }
    
    // Get action data
    FAnim_SurvivalActionData ActionData = GetActionData(ActionType);
    
    // Load and play montage
    UAnimMontage* Montage = ActionData.ActionMontage.LoadSynchronous();
    if (Montage && PlayActionMontage(Montage))
    {
        CurrentAction = ActionType;
        ActionStartTime = GetWorld()->GetTimeSeconds();
        ActionDuration = ActionData.ActionDuration;
        
        // Set timer for action completion
        GetWorld()->GetTimerManager().SetTimer(
            ActionTimerHandle,
            this,
            &UAnim_SurvivalActionSystem::OnActionTimerComplete,
            ActionDuration,
            false
        );
        
        // Broadcast action start event
        OnActionStart.Broadcast(ActionType, ActionDuration);
        
        UE_LOG(LogTemp, Log, TEXT("Started survival action: %d"), (int32)ActionType);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Failed to start survival action: %d"), (int32)ActionType);
    return false;
}

void UAnim_SurvivalActionSystem::StopCurrentAction()
{
    if (!IsPerformingAction())
    {
        return;
    }
    
    EAnim_SurvivalAction PreviousAction = CurrentAction;
    
    // Clear timer
    GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);
    
    // Stop animation
    StopActionMontage();
    
    // Reset state
    CurrentAction = EAnim_SurvivalAction::None;
    ActionStartTime = 0.0f;
    ActionDuration = 0.0f;
    
    // Broadcast completion with failure
    OnActionComplete.Broadcast(PreviousAction, false);
    
    UE_LOG(LogTemp, Log, TEXT("Stopped survival action: %d"), (int32)PreviousAction);
}

bool UAnim_SurvivalActionSystem::CanPerformAction(EAnim_SurvivalAction ActionType) const
{
    if (ActionType == EAnim_SurvivalAction::None)
    {
        return false;
    }
    
    // Check if action data exists
    if (!ActionDataMap.Contains(ActionType))
    {
        return false;
    }
    
    // Check if character has required components
    if (!CharacterMesh || !AnimInstance)
    {
        return false;
    }
    
    // Check if already performing an action
    if (IsPerformingAction())
    {
        return false;
    }
    
    // Check tool requirements
    FAnim_SurvivalActionData ActionData = ActionDataMap[ActionType];
    if (ActionData.bRequiresTool)
    {
        // TODO: Check if character has required tool
        // For now, assume tools are available
    }
    
    return true;
}

float UAnim_SurvivalActionSystem::GetActionProgress() const
{
    if (!IsPerformingAction() || ActionDuration <= 0.0f)
    {
        return 0.0f;
    }
    
    float ElapsedTime = GetWorld()->GetTimeSeconds() - ActionStartTime;
    return FMath::Clamp(ElapsedTime / ActionDuration, 0.0f, 1.0f);
}

void UAnim_SurvivalActionSystem::SetActionData(EAnim_SurvivalAction ActionType, const FAnim_SurvivalActionData& ActionData)
{
    ActionDataMap.Add(ActionType, ActionData);
}

FAnim_SurvivalActionData UAnim_SurvivalActionSystem::GetActionData(EAnim_SurvivalAction ActionType) const
{
    if (ActionDataMap.Contains(ActionType))
    {
        return ActionDataMap[ActionType];
    }
    
    return FAnim_SurvivalActionData();
}

void UAnim_SurvivalActionSystem::InitializeActionData()
{
    // Initialize default action data
    FAnim_SurvivalActionData CraftingData;
    CraftingData.ActionType = EAnim_SurvivalAction::Crafting;
    CraftingData.ActionDuration = 3.0f;
    CraftingData.StaminaCost = 15.0f;
    CraftingData.bRequiresTool = false;
    ActionDataMap.Add(EAnim_SurvivalAction::Crafting, CraftingData);
    
    FAnim_SurvivalActionData GatheringData;
    GatheringData.ActionType = EAnim_SurvivalAction::Gathering;
    GatheringData.ActionDuration = 2.0f;
    GatheringData.StaminaCost = 10.0f;
    GatheringData.bRequiresTool = false;
    ActionDataMap.Add(EAnim_SurvivalAction::Gathering, GatheringData);
    
    FAnim_SurvivalActionData BuildingData;
    BuildingData.ActionType = EAnim_SurvivalAction::Building;
    BuildingData.ActionDuration = 5.0f;
    BuildingData.StaminaCost = 25.0f;
    BuildingData.bRequiresTool = true;
    BuildingData.RequiredToolName = TEXT("Stone Axe");
    ActionDataMap.Add(EAnim_SurvivalAction::Building, BuildingData);
    
    FAnim_SurvivalActionData HuntingData;
    HuntingData.ActionType = EAnim_SurvivalAction::Hunting;
    HuntingData.ActionDuration = 1.5f;
    HuntingData.StaminaCost = 20.0f;
    HuntingData.bRequiresTool = true;
    HuntingData.RequiredToolName = TEXT("Spear");
    ActionDataMap.Add(EAnim_SurvivalAction::Hunting, HuntingData);
    
    FAnim_SurvivalActionData ClimbingData;
    ClimbingData.ActionType = EAnim_SurvivalAction::Climbing;
    ClimbingData.ActionDuration = 4.0f;
    ClimbingData.StaminaCost = 30.0f;
    ClimbingData.bRequiresTool = false;
    ActionDataMap.Add(EAnim_SurvivalAction::Climbing, ClimbingData);
    
    FAnim_SurvivalActionData SwimmingData;
    SwimmingData.ActionType = EAnim_SurvivalAction::Swimming;
    SwimmingData.ActionDuration = 1.0f;
    SwimmingData.StaminaCost = 8.0f;
    SwimmingData.bRequiresTool = false;
    ActionDataMap.Add(EAnim_SurvivalAction::Swimming, SwimmingData);
    
    UE_LOG(LogTemp, Log, TEXT("Survival action system initialized with %d actions"), ActionDataMap.Num());
}

void UAnim_SurvivalActionSystem::OnActionTimerComplete()
{
    if (!IsPerformingAction())
    {
        return;
    }
    
    EAnim_SurvivalAction CompletedAction = CurrentAction;
    
    // Stop animation
    StopActionMontage();
    
    // Reset state
    CurrentAction = EAnim_SurvivalAction::None;
    ActionStartTime = 0.0f;
    ActionDuration = 0.0f;
    
    // Broadcast completion with success
    OnActionComplete.Broadcast(CompletedAction, true);
    
    UE_LOG(LogTemp, Log, TEXT("Completed survival action: %d"), (int32)CompletedAction);
}

bool UAnim_SurvivalActionSystem::PlayActionMontage(UAnimMontage* Montage)
{
    if (!AnimInstance || !Montage)
    {
        return false;
    }
    
    float MontageLength = AnimInstance->Montage_Play(Montage, 1.0f);
    return MontageLength > 0.0f;
}

void UAnim_SurvivalActionSystem::StopActionMontage()
{
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.25f);
    }
}