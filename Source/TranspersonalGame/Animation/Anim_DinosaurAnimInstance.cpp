#include "Anim_DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_DinosaurAnimInstance::UAnim_DinosaurAnimInstance()
{
    // Initialize default values
    DinosaurData.Speed = 0.0f;
    DinosaurData.Direction = 0.0f;
    DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
    DinosaurData.Species = EAnim_DinosaurSpecies::TRex;
    DinosaurData.bIsAggressive = false;
    DinosaurData.HealthPercent = 1.0f;
    DinosaurData.HungerLevel = 0.5f;
    DinosaurData.AlertLevel = 0.0f;

    bIsInAir = false;
    bIsMoving = false;
    bShouldPlayIdleVariation = false;

    IdleTimer = 0.0f;
    StateChangeTimer = 0.0f;
    LastAttackTime = 0.0f;
}

void UAnim_DinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get owner pawn and movement component
    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
        {
            MovementComponent = Character->GetCharacterMovement();
        }

        // Initialize species based on pawn class name
        FString PawnName = OwnerPawn->GetClass()->GetName();
        if (PawnName.Contains(TEXT("TRex")))
        {
            DinosaurData.Species = EAnim_DinosaurSpecies::TRex;
        }
        else if (PawnName.Contains(TEXT("Velociraptor")))
        {
            DinosaurData.Species = EAnim_DinosaurSpecies::Velociraptor;
        }
        else if (PawnName.Contains(TEXT("Triceratops")))
        {
            DinosaurData.Species = EAnim_DinosaurSpecies::Triceratops;
        }
        else if (PawnName.Contains(TEXT("Brachiosaurus")))
        {
            DinosaurData.Species = EAnim_DinosaurSpecies::Brachiosaurus;
        }
        else if (PawnName.Contains(TEXT("Ankylosaurus")))
        {
            DinosaurData.Species = EAnim_DinosaurSpecies::Ankylosaurus;
        }
        else if (PawnName.Contains(TEXT("Parasaurolophus")))
        {
            DinosaurData.Species = EAnim_DinosaurSpecies::Parasaurolophus;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("DinosaurAnimInstance initialized for species: %d"), (int32)DinosaurData.Species);
}

void UAnim_DinosaurAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerPawn)
    {
        return;
    }

    // Update timers
    IdleTimer += DeltaTimeX;
    StateChangeTimer += DeltaTimeX;

    // Update movement data
    UpdateMovementData();

    // Update state logic
    UpdateStateLogic();

    // Update species-specific behavior
    UpdateSpeciesSpecificBehavior();

    // Update idle variation logic
    bShouldPlayIdleVariation = (IdleTimer > 5.0f && DinosaurData.CurrentState == EAnim_DinosaurState::Idle);
    if (bShouldPlayIdleVariation)
    {
        IdleTimer = 0.0f;
    }
}

void UAnim_DinosaurAnimInstance::UpdateMovementData()
{
    if (!OwnerPawn)
    {
        return;
    }

    // Get velocity and calculate speed
    FVector Velocity = OwnerPawn->GetVelocity();
    DinosaurData.Speed = Velocity.Size();
    bIsMoving = DinosaurData.Speed > 10.0f;

    // Calculate movement direction relative to actor rotation
    if (bIsMoving)
    {
        FVector ForwardVector = OwnerPawn->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        DinosaurData.Direction = FVector::DotProduct(ForwardVector, VelocityNormalized);
    }
    else
    {
        DinosaurData.Direction = 0.0f;
    }

    // Check if in air (for flying species or jumping)
    if (MovementComponent)
    {
        bIsInAir = MovementComponent->IsFalling();
    }
}

void UAnim_DinosaurAnimInstance::UpdateStateLogic()
{
    EAnim_DinosaurState PreviousState = DinosaurData.CurrentState;

    // State machine logic
    switch (DinosaurData.CurrentState)
    {
        case EAnim_DinosaurState::Idle:
            if (bIsMoving)
            {
                if (DinosaurData.Speed > 300.0f)
                {
                    DinosaurData.CurrentState = EAnim_DinosaurState::Running;
                }
                else
                {
                    DinosaurData.CurrentState = EAnim_DinosaurState::Walking;
                }
            }
            else if (DinosaurData.AlertLevel > 0.7f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Alert;
            }
            else if (DinosaurData.HungerLevel > 0.8f && StateChangeTimer > 3.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Eating;
            }
            break;

        case EAnim_DinosaurState::Walking:
            if (!bIsMoving)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
            }
            else if (DinosaurData.Speed > 300.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Running;
            }
            else if (DinosaurData.bIsAggressive && StateChangeTimer > 2.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Attacking;
            }
            break;

        case EAnim_DinosaurState::Running:
            if (!bIsMoving)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
            }
            else if (DinosaurData.Speed < 200.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Walking;
            }
            break;

        case EAnim_DinosaurState::Alert:
            if (DinosaurData.AlertLevel < 0.3f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
            }
            else if (DinosaurData.bIsAggressive)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Attacking;
            }
            break;

        case EAnim_DinosaurState::Eating:
            if (StateChangeTimer > 5.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
                DinosaurData.HungerLevel = FMath::Max(0.0f, DinosaurData.HungerLevel - 0.3f);
            }
            break;

        case EAnim_DinosaurState::Attacking:
            if (StateChangeTimer > 3.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
                DinosaurData.bIsAggressive = false;
            }
            break;

        case EAnim_DinosaurState::Roaring:
            if (StateChangeTimer > 2.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
            }
            break;

        case EAnim_DinosaurState::Sleeping:
            if (DinosaurData.AlertLevel > 0.5f || StateChangeTimer > 10.0f)
            {
                DinosaurData.CurrentState = EAnim_DinosaurState::Idle;
            }
            break;

        case EAnim_DinosaurState::Dead:
            // Dead state is permanent
            break;
    }

    // Reset timer if state changed
    if (PreviousState != DinosaurData.CurrentState)
    {
        StateChangeTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Dinosaur state changed from %d to %d"), (int32)PreviousState, (int32)DinosaurData.CurrentState);
    }
}

void UAnim_DinosaurAnimInstance::UpdateSpeciesSpecificBehavior()
{
    // Species-specific behavior modifications
    switch (DinosaurData.Species)
    {
        case EAnim_DinosaurSpecies::TRex:
            // T-Rex is more aggressive and has slower movements but higher damage
            if (DinosaurData.AlertLevel > 0.4f)
            {
                DinosaurData.bIsAggressive = true;
            }
            break;

        case EAnim_DinosaurSpecies::Velociraptor:
            // Velociraptors are pack hunters, more alert and agile
            DinosaurData.AlertLevel = FMath::Min(1.0f, DinosaurData.AlertLevel + 0.01f);
            break;

        case EAnim_DinosaurSpecies::Triceratops:
            // Triceratops are defensive herbivores
            if (DinosaurData.AlertLevel > 0.6f)
            {
                DinosaurData.bIsAggressive = true; // Defensive aggression
            }
            break;

        case EAnim_DinosaurSpecies::Brachiosaurus:
            // Brachiosaurus are peaceful giants, slow to anger
            DinosaurData.AlertLevel = FMath::Max(0.0f, DinosaurData.AlertLevel - 0.005f);
            DinosaurData.HungerLevel = FMath::Min(1.0f, DinosaurData.HungerLevel + 0.02f);
            break;

        case EAnim_DinosaurSpecies::Ankylosaurus:
            // Ankylosaurus are heavily armored, defensive
            if (DinosaurData.HealthPercent < 0.5f)
            {
                DinosaurData.bIsAggressive = true;
            }
            break;

        case EAnim_DinosaurSpecies::Parasaurolophus:
            // Parasaurolophus are social herbivores, communicate through roaring
            if (StateChangeTimer > 8.0f && DinosaurData.CurrentState == EAnim_DinosaurState::Idle)
            {
                SetDinosaurState(EAnim_DinosaurState::Roaring);
            }
            break;
    }
}

void UAnim_DinosaurAnimInstance::PlayAttackAnimation()
{
    if (AttackMontage && CanPlayMontage())
    {
        Montage_Play(AttackMontage);
        SetDinosaurState(EAnim_DinosaurState::Attacking);
        LastAttackTime = GetWorld()->GetTimeSeconds();
        UE_LOG(LogTemp, Log, TEXT("Playing attack animation for dinosaur"));
    }
}

void UAnim_DinosaurAnimInstance::PlayRoarAnimation()
{
    if (RoarMontage && CanPlayMontage())
    {
        Montage_Play(RoarMontage);
        SetDinosaurState(EAnim_DinosaurState::Roaring);
        UE_LOG(LogTemp, Log, TEXT("Playing roar animation for dinosaur"));
    }
}

void UAnim_DinosaurAnimInstance::PlayEatAnimation()
{
    if (EatMontage && CanPlayMontage())
    {
        Montage_Play(EatMontage);
        SetDinosaurState(EAnim_DinosaurState::Eating);
        UE_LOG(LogTemp, Log, TEXT("Playing eat animation for dinosaur"));
    }
}

void UAnim_DinosaurAnimInstance::PlayDeathAnimation()
{
    if (DeathMontage && CanPlayMontage())
    {
        Montage_Play(DeathMontage);
        SetDinosaurState(EAnim_DinosaurState::Dead);
        DinosaurData.HealthPercent = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Playing death animation for dinosaur"));
    }
}

void UAnim_DinosaurAnimInstance::SetDinosaurState(EAnim_DinosaurState NewState)
{
    if (DinosaurData.CurrentState != NewState)
    {
        DinosaurData.CurrentState = NewState;
        StateChangeTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Dinosaur state manually set to: %d"), (int32)NewState);
    }
}

void UAnim_DinosaurAnimInstance::SetDinosaurSpecies(EAnim_DinosaurSpecies NewSpecies)
{
    DinosaurData.Species = NewSpecies;
    UE_LOG(LogTemp, Log, TEXT("Dinosaur species set to: %d"), (int32)NewSpecies);
}

void UAnim_DinosaurAnimInstance::SetAggressionLevel(float AggressionLevel)
{
    DinosaurData.bIsAggressive = AggressionLevel > 0.5f;
    DinosaurData.AlertLevel = FMath::Clamp(AggressionLevel, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Dinosaur aggression level set to: %f"), AggressionLevel);
}