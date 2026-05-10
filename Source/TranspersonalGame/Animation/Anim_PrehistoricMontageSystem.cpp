#include "Anim_PrehistoricMontageSystem.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_PrehistoricMontageSystem::UAnim_PrehistoricMontageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    CurrentAction = EAnim_PrehistoricAction::None;
    bIsPlayingAction = false;
    CurrentActionTimeRemaining = 0.0f;
    CurrentSequenceIndex = -1;
    CurrentSequenceName = TEXT("");
    
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_PrehistoricMontageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindSkeletalMeshComponent();
    SetupDefaultMontageData();
    SetupDefaultSequences();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UAnim_PrehistoricMontageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateActionTimer(DeltaTime);
}

void UAnim_PrehistoricMontageSystem::FindSkeletalMeshComponent()
{
    if (GetOwner())
    {
        SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
            
            if (AnimInstance)
            {
                // Bind montage events
                AnimInstance->OnMontageEnded.AddDynamic(this, &UAnim_PrehistoricMontageSystem::OnMontageEnded);
                AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UAnim_PrehistoricMontageSystem::OnMontageBlendingOut);
                
                UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Found and bound to AnimInstance"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PrehistoricMontageSystem: SkeletalMeshComponent found but no AnimInstance"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PrehistoricMontageSystem: No SkeletalMeshComponent found on %s"), 
                   *GetOwner()->GetName());
        }
    }
}

void UAnim_PrehistoricMontageSystem::SetupDefaultMontageData()
{
    // Setup default montage data for prehistoric survival actions
    FAnim_PrehistoricMontageData CraftingData;
    CraftingData.ActionType = EAnim_PrehistoricAction::Crafting;
    CraftingData.PlayRate = 1.0f;
    CraftingData.BlendInTime = 0.3f;
    CraftingData.BlendOutTime = 0.3f;
    CraftingData.bLooping = true;
    CraftingData.Duration = 5.0f;
    CraftingData.Priority = 2;
    PrehistoricMontages.Add(EAnim_PrehistoricAction::Crafting, CraftingData);
    
    FAnim_PrehistoricMontageData GatheringData;
    GatheringData.ActionType = EAnim_PrehistoricAction::Gathering;
    GatheringData.PlayRate = 1.2f;
    GatheringData.BlendInTime = 0.25f;
    GatheringData.BlendOutTime = 0.25f;
    GatheringData.bLooping = false;
    GatheringData.Duration = 3.0f;
    GatheringData.Priority = 1;
    PrehistoricMontages.Add(EAnim_PrehistoricAction::Gathering, GatheringData);
    
    FAnim_PrehistoricMontageData HuntingData;
    HuntingData.ActionType = EAnim_PrehistoricAction::Hunting;
    HuntingData.PlayRate = 0.8f;
    HuntingData.BlendInTime = 0.2f;
    HuntingData.BlendOutTime = 0.4f;
    HuntingData.bLooping = false;
    HuntingData.Duration = 2.5f;
    HuntingData.Priority = 3;
    PrehistoricMontages.Add(EAnim_PrehistoricAction::Hunting, HuntingData);
    
    FAnim_PrehistoricMontageData ShelterData;
    ShelterData.ActionType = EAnim_PrehistoricAction::ShelterBuilding;
    ShelterData.PlayRate = 0.9f;
    ShelterData.BlendInTime = 0.4f;
    ShelterData.BlendOutTime = 0.4f;
    ShelterData.bLooping = true;
    ShelterData.Duration = 8.0f;
    ShelterData.Priority = 2;
    PrehistoricMontages.Add(EAnim_PrehistoricAction::ShelterBuilding, ShelterData);
    
    FAnim_PrehistoricMontageData FireData;
    FireData.ActionType = EAnim_PrehistoricAction::FireMaking;
    FireData.PlayRate = 1.1f;
    FireData.BlendInTime = 0.3f;
    FireData.BlendOutTime = 0.3f;
    FireData.bLooping = true;
    FireData.Duration = 6.0f;
    FireData.Priority = 2;
    PrehistoricMontages.Add(EAnim_PrehistoricAction::FireMaking, FireData);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Setup %d default montage configurations"), 
           PrehistoricMontages.Num());
}

void UAnim_PrehistoricMontageSystem::SetupDefaultSequences()
{
    // Create fire-making sequence
    FAnim_ActionSequence FireMakingSequence;
    FireMakingSequence.Actions.Add(EAnim_PrehistoricAction::Gathering); // Gather tinder
    FireMakingSequence.Actions.Add(EAnim_PrehistoricAction::StoneKnapping); // Prepare flint
    FireMakingSequence.Actions.Add(EAnim_PrehistoricAction::FireMaking); // Make fire
    FireMakingSequence.ActionDelays = {1.0f, 2.0f, 1.5f};
    FireMakingSequence.bLoopSequence = false;
    FireMakingSequence.SequenceCooldown = 10.0f;
    ActionSequences.Add(TEXT("FireMaking"), FireMakingSequence);
    
    // Create tool crafting sequence
    FAnim_ActionSequence ToolCraftingSequence;
    ToolCraftingSequence.Actions.Add(EAnim_PrehistoricAction::Gathering); // Gather materials
    ToolCraftingSequence.Actions.Add(EAnim_PrehistoricAction::StoneKnapping); // Shape stone
    ToolCraftingSequence.Actions.Add(EAnim_PrehistoricAction::Crafting); // Assemble tool
    ToolCraftingSequence.ActionDelays = {2.0f, 3.0f, 2.5f};
    ToolCraftingSequence.bLoopSequence = false;
    ToolCraftingSequence.SequenceCooldown = 15.0f;
    ActionSequences.Add(TEXT("ToolCrafting"), ToolCraftingSequence);
    
    // Create shelter building sequence
    FAnim_ActionSequence ShelterSequence;
    ShelterSequence.Actions.Add(EAnim_PrehistoricAction::Gathering); // Gather branches
    ShelterSequence.Actions.Add(EAnim_PrehistoricAction::ToolCrafting); // Prepare tools
    ShelterSequence.Actions.Add(EAnim_PrehistoricAction::ShelterBuilding); // Build shelter
    ShelterSequence.ActionDelays = {3.0f, 2.0f, 5.0f};
    ShelterSequence.bLoopSequence = false;
    ShelterSequence.SequenceCooldown = 20.0f;
    ActionSequences.Add(TEXT("ShelterBuilding"), ShelterSequence);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Setup %d action sequences"), 
           ActionSequences.Num());
}

bool UAnim_PrehistoricMontageSystem::PlayPrehistoricAction(EAnim_PrehistoricAction ActionType, float CustomPlayRate, bool bForceRestart)
{
    if (!CanPlayAction(ActionType) && !bForceRestart)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricMontageSystem: Cannot play action %d - conditions not met"), 
               (int32)ActionType);
        return false;
    }
    
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("PrehistoricMontageSystem: No AnimInstance available"));
        return false;
    }
    
    // Find montage data
    FAnim_PrehistoricMontageData* MontageData = PrehistoricMontages.Find(ActionType);
    if (!MontageData)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricMontageSystem: No montage data for action %d"), 
               (int32)ActionType);
        return false;
    }
    
    // Stop current action if playing
    if (bIsPlayingAction)
    {
        StopCurrentAction(MontageData->BlendOutTime);
    }
    
    // Play the montage if available
    if (MontageData->Montage)
    {
        float PlayRate = CustomPlayRate > 0.0f ? CustomPlayRate : MontageData->PlayRate;
        
        float MontageLength = AnimInstance->Montage_Play(
            MontageData->Montage, 
            PlayRate, 
            EMontagePlayReturnType::MontageLength,
            0.0f,
            true
        );
        
        if (MontageLength > 0.0f)
        {
            // Set current state
            CurrentAction = ActionType;
            bIsPlayingAction = true;
            CurrentActionTimeRemaining = MontageData->Duration;
            
            // Setup timer for action completion
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    ActionTimerHandle,
                    this,
                    &UAnim_PrehistoricMontageSystem::OnActionCompleted_Internal,
                    MontageData->Duration,
                    false
                );
            }
            
            // Broadcast event
            OnActionStarted.Broadcast(ActionType, MontageData->Duration);
            
            UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Started action %d with duration %.2f"), 
                   (int32)ActionType, MontageData->Duration);
            
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PrehistoricMontageSystem: Failed to play montage for action %d"), 
                   (int32)ActionType);
        }
    }
    else
    {
        // Play action without montage (for testing)
        CurrentAction = ActionType;
        bIsPlayingAction = true;
        CurrentActionTimeRemaining = MontageData->Duration;
        
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                ActionTimerHandle,
                this,
                &UAnim_PrehistoricMontageSystem::OnActionCompleted_Internal,
                MontageData->Duration,
                false
            );
        }
        
        OnActionStarted.Broadcast(ActionType, MontageData->Duration);
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Started action %d without montage"), 
               (int32)ActionType);
        
        return true;
    }
    
    return false;
}

void UAnim_PrehistoricMontageSystem::StopCurrentAction(float BlendOutTime)
{
    if (!bIsPlayingAction)
    {
        return;
    }
    
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(BlendOutTime);
    }
    
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ActionTimerHandle);
    }
    
    // Broadcast interruption
    OnActionInterrupted.Broadcast(CurrentAction);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Stopped action %d"), (int32)CurrentAction);
    
    // Reset state
    CurrentAction = EAnim_PrehistoricAction::None;
    bIsPlayingAction = false;
    CurrentActionTimeRemaining = 0.0f;
}

void UAnim_PrehistoricMontageSystem::StopAllActions()
{
    StopCurrentAction();
    StopActionSequence();
}

bool UAnim_PrehistoricMontageSystem::IsPlayingAction(EAnim_PrehistoricAction ActionType) const
{
    return bIsPlayingAction && CurrentAction == ActionType;
}

float UAnim_PrehistoricMontageSystem::GetCurrentActionProgress() const
{
    if (!bIsPlayingAction)
    {
        return 0.0f;
    }
    
    FAnim_PrehistoricMontageData* MontageData = PrehistoricMontages.Find(CurrentAction);
    if (!MontageData)
    {
        return 0.0f;
    }
    
    float ElapsedTime = MontageData->Duration - CurrentActionTimeRemaining;
    return FMath::Clamp(ElapsedTime / MontageData->Duration, 0.0f, 1.0f);
}

bool UAnim_PrehistoricMontageSystem::PlayActionSequence(const FString& SequenceName, bool bLoopSequence)
{
    FAnim_ActionSequence* Sequence = ActionSequences.Find(SequenceName);
    if (!Sequence || Sequence->Actions.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricMontageSystem: Action sequence '%s' not found or empty"), 
               *SequenceName);
        return false;
    }
    
    // Stop current sequence if playing
    StopActionSequence();
    
    // Start new sequence
    CurrentSequenceName = SequenceName;
    CurrentSequenceIndex = 0;
    
    // Update loop setting
    Sequence->bLoopSequence = bLoopSequence;
    
    // Play first action
    PlayNextSequenceAction();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Started action sequence '%s' with %d actions"), 
           *SequenceName, Sequence->Actions.Num());
    
    return true;
}

void UAnim_PrehistoricMontageSystem::StopActionSequence()
{
    if (CurrentSequenceName.IsEmpty())
    {
        return;
    }
    
    // Clear sequence timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SequenceTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Stopped action sequence '%s'"), 
           *CurrentSequenceName);
    
    // Reset sequence state
    CurrentSequenceName = TEXT("");
    CurrentSequenceIndex = -1;
}

void UAnim_PrehistoricMontageSystem::AddActionSequence(const FString& SequenceName, const FAnim_ActionSequence& Sequence)
{
    ActionSequences.Add(SequenceName, Sequence);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Added action sequence '%s' with %d actions"), 
           *SequenceName, Sequence.Actions.Num());
}

void UAnim_PrehistoricMontageSystem::RegisterPrehistoricMontage(EAnim_PrehistoricAction ActionType, UAnimMontage* Montage, float Duration, int32 Priority)
{
    FAnim_PrehistoricMontageData MontageData;
    MontageData.ActionType = ActionType;
    MontageData.Montage = Montage;
    MontageData.Duration = Duration;
    MontageData.Priority = Priority;
    
    PrehistoricMontages.Add(ActionType, MontageData);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Registered montage for action %d"), 
           (int32)ActionType);
}

UAnimMontage* UAnim_PrehistoricMontageSystem::GetMontageForAction(EAnim_PrehistoricAction ActionType) const
{
    const FAnim_PrehistoricMontageData* MontageData = PrehistoricMontages.Find(ActionType);
    return MontageData ? MontageData->Montage : nullptr;
}

void UAnim_PrehistoricMontageSystem::SetMontagePlayRate(EAnim_PrehistoricAction ActionType, float NewPlayRate)
{
    FAnim_PrehistoricMontageData* MontageData = PrehistoricMontages.Find(ActionType);
    if (MontageData)
    {
        MontageData->PlayRate = NewPlayRate;
    }
}

void UAnim_PrehistoricMontageSystem::InitializeDefaultMontages()
{
    SetupDefaultMontageData();
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Reinitialized default montages"));
}

void UAnim_PrehistoricMontageSystem::CreateDefaultActionSequences()
{
    SetupDefaultSequences();
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Recreated default action sequences"));
}

void UAnim_PrehistoricMontageSystem::TestPrehistoricActions()
{
    UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Testing prehistoric actions..."));
    
    // Test each action type
    TArray<EAnim_PrehistoricAction> TestActions = {
        EAnim_PrehistoricAction::Gathering,
        EAnim_PrehistoricAction::Crafting,
        EAnim_PrehistoricAction::FireMaking
    };
    
    for (EAnim_PrehistoricAction Action : TestActions)
    {
        UE_LOG(LogTemp, Log, TEXT("  - Testing action %d"), (int32)Action);
        
        if (PrehistoricMontages.Contains(Action))
        {
            UE_LOG(LogTemp, Log, TEXT("    ✓ Montage data available"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("    ✗ No montage data"));
        }
    }
}

void UAnim_PrehistoricMontageSystem::OnActionCompleted_Internal()
{
    if (bIsPlayingAction)
    {
        EAnim_PrehistoricAction CompletedAction = CurrentAction;
        
        // Reset state
        CurrentAction = EAnim_PrehistoricAction::None;
        bIsPlayingAction = false;
        CurrentActionTimeRemaining = 0.0f;
        
        // Broadcast completion
        OnActionCompleted.Broadcast(CompletedAction);
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Action %d completed"), 
               (int32)CompletedAction);
        
        // Continue sequence if playing
        if (!CurrentSequenceName.IsEmpty())
        {
            OnSequenceStepCompleted();
        }
    }
}

void UAnim_PrehistoricMontageSystem::OnSequenceStepCompleted()
{
    if (CurrentSequenceName.IsEmpty())
    {
        return;
    }
    
    FAnim_ActionSequence* Sequence = ActionSequences.Find(CurrentSequenceName);
    if (!Sequence)
    {
        StopActionSequence();
        return;
    }
    
    // Move to next action
    CurrentSequenceIndex++;
    
    if (CurrentSequenceIndex >= Sequence->Actions.Num())
    {
        // Sequence completed
        if (Sequence->bLoopSequence)
        {
            // Restart sequence after cooldown
            CurrentSequenceIndex = 0;
            
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    SequenceTimerHandle,
                    this,
                    &UAnim_PrehistoricMontageSystem::PlayNextSequenceAction,
                    Sequence->SequenceCooldown,
                    false
                );
            }
        }
        else
        {
            // End sequence
            StopActionSequence();
        }
    }
    else
    {
        // Play next action after delay
        float Delay = 0.5f; // Default delay
        if (Sequence->ActionDelays.IsValidIndex(CurrentSequenceIndex))
        {
            Delay = Sequence->ActionDelays[CurrentSequenceIndex];
        }
        
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                SequenceTimerHandle,
                this,
                &UAnim_PrehistoricMontageSystem::PlayNextSequenceAction,
                Delay,
                false
            );
        }
    }
}

void UAnim_PrehistoricMontageSystem::PlayNextSequenceAction()
{
    if (CurrentSequenceName.IsEmpty())
    {
        return;
    }
    
    FAnim_ActionSequence* Sequence = ActionSequences.Find(CurrentSequenceName);
    if (!Sequence || !Sequence->Actions.IsValidIndex(CurrentSequenceIndex))
    {
        StopActionSequence();
        return;
    }
    
    EAnim_PrehistoricAction NextAction = Sequence->Actions[CurrentSequenceIndex];
    PlayPrehistoricAction(NextAction);
}

bool UAnim_PrehistoricMontageSystem::CanPlayAction(EAnim_PrehistoricAction ActionType) const
{
    if (ActionType == EAnim_PrehistoricAction::None)
    {
        return false;
    }
    
    // Check if we have data for this action
    if (!PrehistoricMontages.Contains(ActionType))
    {
        return false;
    }
    
    // Check if already playing same action
    if (bIsPlayingAction && CurrentAction == ActionType)
    {
        return false;
    }
    
    return true;
}

void UAnim_PrehistoricMontageSystem::UpdateActionTimer(float DeltaTime)
{
    if (bIsPlayingAction && CurrentActionTimeRemaining > 0.0f)
    {
        CurrentActionTimeRemaining -= DeltaTime;
        
        if (CurrentActionTimeRemaining <= 0.0f)
        {
            CurrentActionTimeRemaining = 0.0f;
        }
    }
}

void UAnim_PrehistoricMontageSystem::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bInterrupted && bIsPlayingAction)
    {
        OnActionInterrupted.Broadcast(CurrentAction);
        UE_LOG(LogTemp, Log, TEXT("PrehistoricMontageSystem: Montage interrupted for action %d"), 
               (int32)CurrentAction);
    }
}

void UAnim_PrehistoricMontageSystem::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    // Handle montage blending out if needed
    UE_LOG(LogTemp, VeryVerbose, TEXT("PrehistoricMontageSystem: Montage blending out"));
}