#include "Anim_TribalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_TribalAnimInstance::UAnim_TribalAnimInstance()
{
    CurrentState = EAnim_TribalState::Idle;
    PreviousState = EAnim_TribalState::Idle;
    
    OwnerCharacter = nullptr;
    CharacterMovement = nullptr;
    
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    CombatBlend = 0.0f;
    SurvivalActivityBlend = 0.0f;
    
    bIsGathering = false;
    bIsCrafting = false;
    bIsHunting = false;
    GatheringProgress = 0.0f;
    CraftingProgress = 0.0f;
    
    HealthPercentage = 100.0f;
    bIsInjured = false;
    bIsFatigued = false;
    FatigueLevel = 0.0f;
    
    StateTransitionTime = 0.2f;
    BlendUpdateRate = 5.0f;
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    MovementStopThreshold = 10.0f;
    
    TimeSinceLastStateChange = 0.0f;
    LastUpdateTime = 0.0f;
}

void UAnim_TribalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("TribalAnimInstance: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAnimInstance: Failed to get owner character"));
    }
}

void UAnim_TribalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TimeSinceLastStateChange += DeltaTimeX;
    
    // Update all data systems
    UpdateMovementData();
    UpdateCombatData();
    UpdateSurvivalData();
    UpdateHealthData();
    UpdateStateTransitions();
    UpdateAnimationBlends();
    
    LastUpdateTime = CurrentTime;
}

void UAnim_TribalAnimInstance::UpdateMovementData()
{
    if (!CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    MovementData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (MovementData.Speed > MovementStopThreshold)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Check if moving backwards
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsMoving = MovementData.Speed > MovementStopThreshold;
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
}

void UAnim_TribalAnimInstance::UpdateCombatData()
{
    // This would be updated by the combat system
    // For now, maintain current values
    
    // Update attack speed based on weapon type
    switch (CombatData.CurrentWeapon)
    {
        case EAnim_TribalWeapon::Spear:
            CombatData.AttackSpeed = 1.2f;
            break;
        case EAnim_TribalWeapon::Club:
            CombatData.AttackSpeed = 0.8f;
            break;
        case EAnim_TribalWeapon::Bow:
            CombatData.AttackSpeed = 1.5f;
            break;
        case EAnim_TribalWeapon::StoneAxe:
            CombatData.AttackSpeed = 0.6f;
            break;
        case EAnim_TribalWeapon::Knife:
            CombatData.AttackSpeed = 2.0f;
            break;
        default:
            CombatData.AttackSpeed = 1.0f;
            break;
    }
}

void UAnim_TribalAnimInstance::UpdateSurvivalData()
{
    // Update survival activity progress
    if (bIsGathering)
    {
        GatheringProgress += GetDeltaSeconds() * 0.5f; // 2 seconds to complete
        if (GatheringProgress >= 1.0f)
        {
            GatheringProgress = 1.0f;
            bIsGathering = false;
        }
    }
    
    if (bIsCrafting)
    {
        CraftingProgress += GetDeltaSeconds() * 0.2f; // 5 seconds to complete
        if (CraftingProgress >= 1.0f)
        {
            CraftingProgress = 1.0f;
            bIsCrafting = false;
        }
    }
}

void UAnim_TribalAnimInstance::UpdateHealthData()
{
    // This would be updated by the health system
    // For now, determine injured state based on health
    bIsInjured = HealthPercentage < 50.0f;
    
    // Update fatigue based on movement
    if (MovementData.bIsMoving && MovementData.Speed > RunSpeedThreshold)
    {
        FatigueLevel += GetDeltaSeconds() * 0.1f; // Gain fatigue while running
    }
    else if (!MovementData.bIsMoving)
    {
        FatigueLevel -= GetDeltaSeconds() * 0.05f; // Recover fatigue while idle
    }
    
    FatigueLevel = FMath::Clamp(FatigueLevel, 0.0f, 1.0f);
    bIsFatigued = FatigueLevel > 0.7f;
}

void UAnim_TribalAnimInstance::UpdateStateTransitions()
{
    EAnim_TribalState NewState = CurrentState;
    
    // Determine new state based on current conditions
    if (HealthPercentage <= 0.0f)
    {
        NewState = EAnim_TribalState::Dead;
    }
    else if (bIsInjured && MovementData.Speed < MovementStopThreshold)
    {
        NewState = EAnim_TribalState::Injured;
    }
    else if (CombatData.bIsAttacking || CombatData.bIsBlocking)
    {
        NewState = EAnim_TribalState::Fighting;
    }
    else if (bIsGathering)
    {
        NewState = EAnim_TribalState::Gathering;
    }
    else if (bIsCrafting)
    {
        NewState = EAnim_TribalState::Crafting;
    }
    else if (bIsHunting)
    {
        NewState = EAnim_TribalState::Hunting;
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_TribalState::Crouching;
    }
    else if (MovementData.Speed > RunSpeedThreshold)
    {
        NewState = EAnim_TribalState::Running;
    }
    else if (MovementData.Speed > WalkSpeedThreshold)
    {
        NewState = EAnim_TribalState::Walking;
    }
    else
    {
        NewState = EAnim_TribalState::Idle;
    }
    
    // Apply state transition if valid
    if (NewState != CurrentState && CanTransitionTo(NewState))
    {
        SetTribalState(NewState);
    }
}

void UAnim_TribalAnimInstance::UpdateAnimationBlends()
{
    float DeltaTime = GetDeltaSeconds();
    float BlendSpeed = BlendUpdateRate * DeltaTime;
    
    // Update idle to walk blend
    float TargetIdleWalkBlend = (MovementData.Speed > MovementStopThreshold) ? 1.0f : 0.0f;
    IdleToWalkBlend = FMath::FInterpTo(IdleToWalkBlend, TargetIdleWalkBlend, DeltaTime, BlendSpeed);
    
    // Update walk to run blend
    float TargetWalkRunBlend = 0.0f;
    if (MovementData.Speed > WalkSpeedThreshold)
    {
        TargetWalkRunBlend = FMath::Clamp((MovementData.Speed - WalkSpeedThreshold) / (RunSpeedThreshold - WalkSpeedThreshold), 0.0f, 1.0f);
    }
    WalkToRunBlend = FMath::FInterpTo(WalkToRunBlend, TargetWalkRunBlend, DeltaTime, BlendSpeed);
    
    // Update combat blend
    float TargetCombatBlend = (CombatData.bIsAttacking || CombatData.bIsBlocking || CombatData.bIsAiming) ? 1.0f : 0.0f;
    CombatBlend = FMath::FInterpTo(CombatBlend, TargetCombatBlend, DeltaTime, BlendSpeed);
    
    // Update survival activity blend
    float TargetSurvivalBlend = (bIsGathering || bIsCrafting || bIsHunting) ? 1.0f : 0.0f;
    SurvivalActivityBlend = FMath::FInterpTo(SurvivalActivityBlend, TargetSurvivalBlend, DeltaTime, BlendSpeed);
}

bool UAnim_TribalAnimInstance::CanTransitionTo(EAnim_TribalState NewState) const
{
    // Dead state can only be entered, not exited
    if (CurrentState == EAnim_TribalState::Dead)
    {
        return false;
    }
    
    // Some states require minimum time before transition
    if (TimeSinceLastStateChange < StateTransitionTime)
    {
        // Allow immediate transitions to critical states
        if (NewState == EAnim_TribalState::Dead || NewState == EAnim_TribalState::Fighting)
        {
            return true;
        }
        return false;
    }
    
    return true;
}

void UAnim_TribalAnimInstance::HandleStateTransition(EAnim_TribalState NewState)
{
    PreviousState = CurrentState;
    CurrentState = NewState;
    TimeSinceLastStateChange = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("TribalAnimInstance: State transition from %d to %d"), 
           (int32)PreviousState, (int32)CurrentState);
}

void UAnim_TribalAnimInstance::SetTribalState(EAnim_TribalState NewState)
{
    if (CanTransitionTo(NewState))
    {
        HandleStateTransition(NewState);
    }
}

void UAnim_TribalAnimInstance::SetWeapon(EAnim_TribalWeapon NewWeapon)
{
    CombatData.CurrentWeapon = NewWeapon;
    UE_LOG(LogTemp, Log, TEXT("TribalAnimInstance: Weapon changed to %d"), (int32)NewWeapon);
}

void UAnim_TribalAnimInstance::TriggerAttackAnimation()
{
    CombatData.bIsAttacking = true;
    SetTribalState(EAnim_TribalState::Fighting);
    
    // Reset attack flag after a delay (would be handled by animation notify)
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { CombatData.bIsAttacking = false; },
        1.0f / CombatData.AttackSpeed,
        false
    );
}

void UAnim_TribalAnimInstance::TriggerGatherAnimation()
{
    bIsGathering = true;
    GatheringProgress = 0.0f;
    SetTribalState(EAnim_TribalState::Gathering);
}

void UAnim_TribalAnimInstance::TriggerCraftAnimation()
{
    bIsCrafting = true;
    CraftingProgress = 0.0f;
    SetTribalState(EAnim_TribalState::Crafting);
}

bool UAnim_TribalAnimInstance::ShouldPlayIdleAnimation() const
{
    return CurrentState == EAnim_TribalState::Idle && !MovementData.bIsMoving;
}

bool UAnim_TribalAnimInstance::ShouldPlayMovementAnimation() const
{
    return (CurrentState == EAnim_TribalState::Walking || CurrentState == EAnim_TribalState::Running) && MovementData.bIsMoving;
}

bool UAnim_TribalAnimInstance::ShouldPlayCombatAnimation() const
{
    return CurrentState == EAnim_TribalState::Fighting;
}

bool UAnim_TribalAnimInstance::ShouldPlaySurvivalAnimation() const
{
    return CurrentState == EAnim_TribalState::Gathering || CurrentState == EAnim_TribalState::Crafting || CurrentState == EAnim_TribalState::Hunting;
}