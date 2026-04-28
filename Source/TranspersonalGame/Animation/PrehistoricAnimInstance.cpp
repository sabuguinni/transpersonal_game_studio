#include "PrehistoricAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UPrehistoricAnimInstance::UPrehistoricAnimInstance()
{
    // Initialize default values
    Speed = 0.0f;
    Direction = 0.0f;
    LeanAmount = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsMoving = false;
    
    // Survival states
    bIsHunting = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsResting = false;
    
    // Combat states
    bIsInCombat = false;
    bIsBlocking = false;
    bIsAttacking = false;
    bIsDodging = false;
    
    // Animation settings
    IdleBreakChance = 0.1f;
    IdleBreakMinInterval = 5.0f;
    LookAroundChance = 0.05f;
    
    // Internal state
    LastIdleBreakTime = 0.0f;
    LastLookAroundTime = 0.0f;
    bWasMoving = false;
    
    MovementState = EAnim_MovementState::Idle;
}

void UPrehistoricAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (!OwningCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAnimInstance: Failed to get owning character"));
        return;
    }
    
    // Get movement component
    CharacterMovement = OwningCharacter->GetCharacterMovement();
    if (!CharacterMovement)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAnimInstance: Failed to get character movement component"));
        return;
    }
    
    // Get animation controller
    AnimationController = OwningCharacter->FindComponentByClass<UPrimitiveAnimationController>();
    if (!AnimationController)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAnimInstance: No PrimitiveAnimationController found on character"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAnimInstance: Initialized successfully"));
}

void UPrehistoricAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update movement variables
    UpdateMovementVariables();
    
    // Update survival and combat states
    UpdateSurvivalStates();
    UpdateCombatStates();
    
    // Check for idle animations
    CheckIdleAnimations();
}

void UPrehistoricAnimInstance::UpdateMovementVariables()
{
    if (!CharacterMovement)
    {
        return;
    }
    
    // Get data from animation controller if available
    if (AnimationController)
    {
        MovementState = AnimationController->GetCurrentMovementState();
        BlendParams = AnimationController->GetBlendParameters();
        
        Speed = BlendParams.Speed;
        Direction = BlendParams.Direction;
        LeanAmount = BlendParams.LeanAmount;
        bIsInAir = BlendParams.bIsInAir;
        bIsCrouching = BlendParams.bIsCrouching;
    }
    else
    {
        // Fallback calculation if no animation controller
        FVector Velocity = CharacterMovement->Velocity;
        Velocity.Z = 0.0f;
        Speed = Velocity.Size();
        
        bIsInAir = CharacterMovement->IsFalling();
        bIsCrouching = CharacterMovement->IsCrouching();
        
        // Simple direction calculation
        if (Speed > 10.0f)
        {
            FVector Forward = OwningCharacter->GetActorForwardVector();
            Forward.Z = 0.0f;
            Forward.Normalize();
            
            Velocity.Normalize();
            float DotProduct = FVector::DotProduct(Forward, Velocity);
            float CrossProduct = FVector::CrossProduct(Forward, Velocity).Z;
            Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        }
        else
        {
            Direction = 0.0f;
        }
    }
    
    // Update movement flag
    bWasMoving = bIsMoving;
    bIsMoving = Speed > 50.0f;
}

void UPrehistoricAnimInstance::UpdateSurvivalStates()
{
    // These would be updated based on gameplay systems
    // For now, we'll leave them as placeholders that can be set by other systems
    
    // Example: Check if character is near crafting station
    // bIsCrafting = CheckNearCraftingStation();
    
    // Example: Check if character is in hunting mode
    // bIsHunting = CheckHuntingMode();
    
    // Example: Check if character is gathering resources
    // bIsGathering = CheckGatheringMode();
    
    // Example: Check if character is resting
    // bIsResting = CheckRestingMode();
}

void UPrehistoricAnimInstance::UpdateCombatStates()
{
    // These would be updated based on combat system
    // For now, we'll leave them as placeholders
    
    // Example: Check if character is in combat
    // bIsInCombat = CheckCombatState();
    
    // Example: Check if character is blocking
    // bIsBlocking = CheckBlockingState();
    
    // Example: Check if character is attacking
    // bIsAttacking = CheckAttackingState();
    
    // Example: Check if character is dodging
    // bIsDodging = CheckDodgingState();
}

void UPrehistoricAnimInstance::CheckIdleAnimations()
{
    if (!IsIdleState() || bIsMoving)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check for idle break animations
    if (ShouldPlayIdleBreak())
    {
        LastIdleBreakTime = CurrentTime;
        // Trigger idle break animation
        UE_LOG(LogTemp, Log, TEXT("Triggering idle break animation"));
    }
    
    // Check for look around animations
    if (ShouldLookAround())
    {
        LastLookAroundTime = CurrentTime;
        // Trigger look around animation
        UE_LOG(LogTemp, Log, TEXT("Triggering look around animation"));
    }
}

bool UPrehistoricAnimInstance::ShouldPlayIdleBreak() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastBreak = CurrentTime - LastIdleBreakTime;
    
    return TimeSinceLastBreak > IdleBreakMinInterval && 
           FMath::RandRange(0.0f, 1.0f) < IdleBreakChance;
}

bool UPrehistoricAnimInstance::ShouldLookAround() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    return FMath::RandRange(0.0f, 1.0f) < LookAroundChance;
}

// Animation event functions
void UPrehistoricAnimInstance::OnJumpStart()
{
    UE_LOG(LogTemp, Log, TEXT("Animation Event: Jump Start"));
}

void UPrehistoricAnimInstance::OnLanded()
{
    UE_LOG(LogTemp, Log, TEXT("Animation Event: Landed"));
}

void UPrehistoricAnimInstance::OnCombatStart()
{
    bIsInCombat = true;
    UE_LOG(LogTemp, Log, TEXT("Animation Event: Combat Start"));
}

void UPrehistoricAnimInstance::OnCombatEnd()
{
    bIsInCombat = false;
    bIsBlocking = false;
    bIsAttacking = false;
    bIsDodging = false;
    UE_LOG(LogTemp, Log, TEXT("Animation Event: Combat End"));
}

void UPrehistoricAnimInstance::OnSurvivalActionStart(ESurvivalAction Action)
{
    // Reset all survival states
    bIsHunting = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsResting = false;
    
    // Set the appropriate state
    switch (Action)
    {
        case ESurvivalAction::Hunting:
            bIsHunting = true;
            break;
        case ESurvivalAction::Gathering:
            bIsGathering = true;
            break;
        case ESurvivalAction::Crafting:
            bIsCrafting = true;
            break;
        case ESurvivalAction::Resting:
            bIsResting = true;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation Event: Survival Action Start - %d"), (int32)Action);
}

void UPrehistoricAnimInstance::OnSurvivalActionEnd()
{
    bIsHunting = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsResting = false;
    
    UE_LOG(LogTemp, Log, TEXT("Animation Event: Survival Action End"));
}