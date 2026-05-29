#include "Anim_PrehistoricMontageSystem.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"

UAnim_PrehistoricMontageSystem::UAnim_PrehistoricMontageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30 FPS for performance
    
    // Initialize montage settings
    DefaultBlendInTime = 0.2f;
    DefaultBlendOutTime = 0.2f;
    DefaultPlayRate = 1.0f;
    
    // Initialize state
    CurrentMontage = nullptr;
    bIsMontageActive = false;
    bCanInterruptMontage = true;
    MontageInterruptionPriority = 0;
    
    // Initialize combat settings
    CombatMontageBlendTime = 0.15f;
    CombatChainWindow = 0.5f;
    
    // Initialize survival settings
    SurvivalActionBlendTime = 0.25f;
    SurvivalActionCooldown = 1.0f;
    
    // Initialize environmental settings
    EnvironmentalBlendTime = 0.3f;
    EnvironmentalReactionTime = 0.1f;
}

void UAnim_PrehistoricMontageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMesh = OwnerCharacter->GetMesh();
        OwnerAnimInstance = OwnerMesh ? OwnerMesh->GetAnimInstance() : nullptr;
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
    }
    
    // Initialize montage maps
    InitializeMontageLibrary();
    
    // Bind delegates
    if (OwnerAnimInstance)
    {
        OwnerAnimInstance->OnMontageEnded.AddDynamic(this, &UAnim_PrehistoricMontageSystem::OnMontageEnded);
        OwnerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UAnim_PrehistoricMontageSystem::OnMontageBlendingOut);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Prehistoric Montage System initialized for %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UAnim_PrehistoricMontageSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerAnimInstance || !OwnerCharacter)
        return;
    
    // Update montage state
    UpdateMontageState(DeltaTime);
    
    // Update combat chains
    UpdateCombatChains(DeltaTime);
    
    // Update survival action cooldowns
    UpdateSurvivalCooldowns(DeltaTime);
    
    // Check for environmental triggers
    CheckEnvironmentalTriggers(DeltaTime);
}

void UAnim_PrehistoricMontageSystem::InitializeMontageLibrary()
{
    // Initialize combat montages
    CombatMontages.Empty();
    CombatMontages.Add(EAnim_CombatAction::MeleeAttack, nullptr);
    CombatMontages.Add(EAnim_CombatAction::HeavyAttack, nullptr);
    CombatMontages.Add(EAnim_CombatAction::Block, nullptr);
    CombatMontages.Add(EAnim_CombatAction::Dodge, nullptr);
    CombatMontages.Add(EAnim_CombatAction::Parry, nullptr);
    
    // Initialize survival montages
    SurvivalMontages.Empty();
    SurvivalMontages.Add(EAnim_SurvivalAction::Crafting, nullptr);
    SurvivalMontages.Add(EAnim_SurvivalAction::Gathering, nullptr);
    SurvivalMontages.Add(EAnim_SurvivalAction::Hunting, nullptr);
    SurvivalMontages.Add(EAnim_SurvivalAction::Building, nullptr);
    SurvivalMontages.Add(EAnim_SurvivalAction::Cooking, nullptr);
    
    // Initialize environmental montages
    EnvironmentalMontages.Empty();
    EnvironmentalMontages.Add(EAnim_EnvironmentalReaction::ClimbingUp, nullptr);
    EnvironmentalMontages.Add(EAnim_EnvironmentalReaction::ClimbingDown, nullptr);
    EnvironmentalMontages.Add(EAnim_EnvironmentalReaction::Swimming, nullptr);
    EnvironmentalMontages.Add(EAnim_EnvironmentalReaction::Stumbling, nullptr);
    EnvironmentalMontages.Add(EAnim_EnvironmentalReaction::Balancing, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("Montage library initialized with %d combat, %d survival, %d environmental montages"),
           CombatMontages.Num(), SurvivalMontages.Num(), EnvironmentalMontages.Num());
}

bool UAnim_PrehistoricMontageSystem::PlayCombatMontage(EAnim_CombatAction CombatAction, 
                                                      float PlayRate, float BlendInTime)
{
    if (!CanPlayMontage(5)) // Combat has high priority
        return false;
    
    UAnimMontage** MontagePtr = CombatMontages.Find(CombatAction);
    if (!MontagePtr || !*MontagePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat montage not found for action: %d"), (int32)CombatAction);
        return false;
    }
    
    UAnimMontage* MontageToPlay = *MontagePtr;
    
    // Stop current montage if necessary
    if (bIsMontageActive && CurrentMontage)
    {
        StopCurrentMontage(CombatMontageBlendTime);
    }
    
    // Play the montage
    float MontageLength = OwnerAnimInstance->Montage_Play(MontageToPlay, PlayRate, 
                                                         EMontagePlayReturnType::MontageLength, 
                                                         0.0f, true);
    
    if (MontageLength > 0.0f)
    {
        CurrentMontage = MontageToPlay;
        bIsMontageActive = true;
        bCanInterruptMontage = true;
        MontageInterruptionPriority = 5;
        CurrentMontageType = EAnim_MontageType::Combat;
        
        // Store combat chain data
        LastCombatAction = CombatAction;
        CombatChainTimer = CombatChainWindow;
        
        UE_LOG(LogTemp, Log, TEXT("Playing combat montage: %s (Length: %.2f)"), 
               *MontageToPlay->GetName(), MontageLength);
        
        return true;
    }
    
    return false;
}

bool UAnim_PrehistoricMontageSystem::PlaySurvivalMontage(EAnim_SurvivalAction SurvivalAction, 
                                                        float PlayRate, bool bCanBeInterrupted)
{
    if (!CanPlayMontage(3)) // Survival has medium priority
        return false;
    
    UAnimMontage** MontagePtr = SurvivalMontages.Find(SurvivalAction);
    if (!MontagePtr || !*MontagePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Survival montage not found for action: %d"), (int32)SurvivalAction);
        return false;
    }
    
    UAnimMontage* MontageToPlay = *MontagePtr;
    
    // Check cooldown
    if (IsActionOnCooldown(SurvivalAction))
    {
        UE_LOG(LogTemp, Log, TEXT("Survival action %d is on cooldown"), (int32)SurvivalAction);
        return false;
    }
    
    // Stop current montage if necessary
    if (bIsMontageActive && CurrentMontage)
    {
        StopCurrentMontage(SurvivalActionBlendTime);
    }
    
    // Play the montage
    float MontageLength = OwnerAnimInstance->Montage_Play(MontageToPlay, PlayRate, 
                                                         EMontagePlayReturnType::MontageLength, 
                                                         0.0f, true);
    
    if (MontageLength > 0.0f)
    {
        CurrentMontage = MontageToPlay;
        bIsMontageActive = true;
        bCanInterruptMontage = bCanBeInterrupted;
        MontageInterruptionPriority = 3;
        CurrentMontageType = EAnim_MontageType::Survival;
        
        // Set cooldown
        SurvivalActionCooldowns.Add(SurvivalAction, SurvivalActionCooldown);
        
        UE_LOG(LogTemp, Log, TEXT("Playing survival montage: %s (Length: %.2f)"), 
               *MontageToPlay->GetName(), MontageLength);
        
        return true;
    }
    
    return false;
}

bool UAnim_PrehistoricMontageSystem::PlayEnvironmentalMontage(EAnim_EnvironmentalReaction Reaction, 
                                                             float PlayRate, float BlendInTime)
{
    if (!CanPlayMontage(2)) // Environmental has low priority
        return false;
    
    UAnimMontage** MontagePtr = EnvironmentalMontages.Find(Reaction);
    if (!MontagePtr || !*MontagePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Environmental montage not found for reaction: %d"), (int32)Reaction);
        return false;
    }
    
    UAnimMontage* MontageToPlay = *MontagePtr;
    
    // Environmental reactions can interrupt lower priority montages
    if (bIsMontageActive && CurrentMontage && MontageInterruptionPriority <= 2)
    {
        StopCurrentMontage(EnvironmentalBlendTime);
    }
    else if (bIsMontageActive && MontageInterruptionPriority > 2)
    {
        // Cannot interrupt higher priority montages
        return false;
    }
    
    // Play the montage
    float MontageLength = OwnerAnimInstance->Montage_Play(MontageToPlay, PlayRate, 
                                                         EMontagePlayReturnType::MontageLength, 
                                                         0.0f, true);
    
    if (MontageLength > 0.0f)
    {
        CurrentMontage = MontageToPlay;
        bIsMontageActive = true;
        bCanInterruptMontage = true;
        MontageInterruptionPriority = 2;
        CurrentMontageType = EAnim_MontageType::Environmental;
        
        UE_LOG(LogTemp, Log, TEXT("Playing environmental montage: %s (Length: %.2f)"), 
               *MontageToPlay->GetName(), MontageLength);
        
        return true;
    }
    
    return false;
}

bool UAnim_PrehistoricMontageSystem::CanPlayMontage(int32 RequiredPriority) const
{
    if (!OwnerAnimInstance || !OwnerCharacter)
        return false;
    
    // Check if we can interrupt current montage
    if (bIsMontageActive && CurrentMontage)
    {
        if (!bCanInterruptMontage || MontageInterruptionPriority > RequiredPriority)
            return false;
    }
    
    // Check if character is in a state that allows montages
    if (OwnerMovement && OwnerMovement->IsFalling())
    {
        // Only allow environmental reactions while falling
        return RequiredPriority <= 2;
    }
    
    return true;
}

void UAnim_PrehistoricMontageSystem::StopCurrentMontage(float BlendOutTime)
{
    if (!bIsMontageActive || !CurrentMontage || !OwnerAnimInstance)
        return;
    
    OwnerAnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
    
    UE_LOG(LogTemp, Log, TEXT("Stopping current montage: %s"), *CurrentMontage->GetName());
}

void UAnim_PrehistoricMontageSystem::UpdateMontageState(float DeltaTime)
{
    if (!bIsMontageActive || !CurrentMontage || !OwnerAnimInstance)
        return;
    
    // Check if montage is still playing
    if (!OwnerAnimInstance->Montage_IsPlaying(CurrentMontage))
    {
        bIsMontageActive = false;
        CurrentMontage = nullptr;
        MontageInterruptionPriority = 0;
        CurrentMontageType = EAnim_MontageType::None;
    }
}

void UAnim_PrehistoricMontageSystem::UpdateCombatChains(float DeltaTime)
{
    if (CombatChainTimer > 0.0f)
    {
        CombatChainTimer -= DeltaTime;
        if (CombatChainTimer <= 0.0f)
        {
            // Reset combat chain
            LastCombatAction = EAnim_CombatAction::None;
        }
    }
}

void UAnim_PrehistoricMontageSystem::UpdateSurvivalCooldowns(float DeltaTime)
{
    TArray<EAnim_SurvivalAction> ActionsToRemove;
    
    for (auto& Cooldown : SurvivalActionCooldowns)
    {
        Cooldown.Value -= DeltaTime;
        if (Cooldown.Value <= 0.0f)
        {
            ActionsToRemove.Add(Cooldown.Key);
        }
    }
    
    // Remove expired cooldowns
    for (EAnim_SurvivalAction Action : ActionsToRemove)
    {
        SurvivalActionCooldowns.Remove(Action);
    }
}

void UAnim_PrehistoricMontageSystem::CheckEnvironmentalTriggers(float DeltaTime)
{
    if (!OwnerCharacter || !OwnerMovement)
        return;
    
    // Check for environmental reactions based on movement state
    if (OwnerMovement->IsFalling() && CurrentMontageType != EAnim_MontageType::Environmental)
    {
        // Trigger falling reaction if falling for more than 0.5 seconds
        // This would be handled by the movement system calling PlayEnvironmentalMontage
    }
    
    // Check for stumbling on uneven terrain
    // This would be triggered by the IK foot placement system
    
    // Check for swimming state
    if (OwnerMovement->IsSwimming() && CurrentMontageType != EAnim_MontageType::Environmental)
    {
        PlayEnvironmentalMontage(EAnim_EnvironmentalReaction::Swimming);
    }
}

bool UAnim_PrehistoricMontageSystem::IsActionOnCooldown(EAnim_SurvivalAction Action) const
{
    const float* CooldownPtr = SurvivalActionCooldowns.Find(Action);
    return CooldownPtr && *CooldownPtr > 0.0f;
}

void UAnim_PrehistoricMontageSystem::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        bIsMontageActive = false;
        CurrentMontage = nullptr;
        MontageInterruptionPriority = 0;
        CurrentMontageType = EAnim_MontageType::None;
        
        UE_LOG(LogTemp, Log, TEXT("Montage ended: %s (Interrupted: %s)"), 
               *Montage->GetName(), bInterrupted ? TEXT("Yes") : TEXT("No"));
        
        // Broadcast montage ended event
        OnMontageCompleted.Broadcast(CurrentMontageType, bInterrupted);
    }
}

void UAnim_PrehistoricMontageSystem::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("Montage blending out: %s"), *Montage->GetName());
    }
}

bool UAnim_PrehistoricMontageSystem::CanChainCombatAction(EAnim_CombatAction NextAction) const
{
    if (CombatChainTimer <= 0.0f || LastCombatAction == EAnim_CombatAction::None)
        return false;
    
    // Define valid combat chains
    switch (LastCombatAction)
    {
        case EAnim_CombatAction::MeleeAttack:
            return NextAction == EAnim_CombatAction::MeleeAttack || 
                   NextAction == EAnim_CombatAction::HeavyAttack;
        
        case EAnim_CombatAction::HeavyAttack:
            return NextAction == EAnim_CombatAction::MeleeAttack;
        
        case EAnim_CombatAction::Block:
            return NextAction == EAnim_CombatAction::Parry || 
                   NextAction == EAnim_CombatAction::MeleeAttack;
        
        default:
            return false;
    }
}

void UAnim_PrehistoricMontageSystem::SetMontageAsset(EAnim_CombatAction Action, UAnimMontage* Montage)
{
    if (Montage)
    {
        CombatMontages.Add(Action, Montage);
        UE_LOG(LogTemp, Log, TEXT("Set combat montage for action %d: %s"), 
               (int32)Action, *Montage->GetName());
    }
}

void UAnim_PrehistoricMontageSystem::SetMontageAsset(EAnim_SurvivalAction Action, UAnimMontage* Montage)
{
    if (Montage)
    {
        SurvivalMontages.Add(Action, Montage);
        UE_LOG(LogTemp, Log, TEXT("Set survival montage for action %d: %s"), 
               (int32)Action, *Montage->GetName());
    }
}

void UAnim_PrehistoricMontageSystem::SetMontageAsset(EAnim_EnvironmentalReaction Reaction, UAnimMontage* Montage)
{
    if (Montage)
    {
        EnvironmentalMontages.Add(Reaction, Montage);
        UE_LOG(LogTemp, Log, TEXT("Set environmental montage for reaction %d: %s"), 
               (int32)Reaction, *Montage->GetName());
    }
}

float UAnim_PrehistoricMontageSystem::GetMontagePlaybackPosition() const
{
    if (bIsMontageActive && CurrentMontage && OwnerAnimInstance)
    {
        return OwnerAnimInstance->Montage_GetPosition(CurrentMontage);
    }
    return 0.0f;
}

float UAnim_PrehistoricMontageSystem::GetMontagePlaybackLength() const
{
    if (CurrentMontage)
    {
        return CurrentMontage->GetPlayLength();
    }
    return 0.0f;
}