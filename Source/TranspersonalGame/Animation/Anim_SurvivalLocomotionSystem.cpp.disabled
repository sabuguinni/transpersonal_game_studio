#include "Anim_SurvivalLocomotionSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_SurvivalLocomotionSystem::UAnim_SurvivalLocomotionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize movement thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 350.0f;
    SprintSpeedThreshold = 600.0f;
    MovementSmoothingSpeed = 5.0f;
    DirectionSmoothingSpeed = 8.0f;

    // Initialize survival modifiers
    InjuredSpeedMultiplier = 0.6f;
    ExhaustedSpeedMultiplier = 0.4f;
    FearMovementMultiplier = 1.3f;
    StaminaDepletionRate = 10.0f;
    HealthRegenerationRate = 2.0f;

    // Initialize internal state
    SmoothedSpeed = 0.0f;
    SmoothedDirection = 0.0f;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;

    // Initialize component references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;

    // Initialize animation assets to null
    LocomotionBlendSpace = nullptr;
    CrouchBlendSpace = nullptr;
    InjuredBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandingMontage = nullptr;
    ClimbingMontage = nullptr;
    SwimmingMontage = nullptr;
    CraftingMontage = nullptr;
    GatheringMontage = nullptr;
    HuntingMontage = nullptr;
    BuildingMontage = nullptr;
    EatingMontage = nullptr;
    DrinkingMontage = nullptr;
}

void UAnim_SurvivalLocomotionSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_SurvivalLocomotionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsInitialized)
    {
        UpdateMovementData(DeltaTime);
        UpdateEnvironmentalFactors();
    }
}

void UAnim_SurvivalLocomotionSystem::InitializeComponent()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalLocomotionSystem: Owner is not a Character"));
        return;
    }

    MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalLocomotionSystem: No CharacterMovementComponent found"));
        return;
    }

    MeshComponent = OwnerCharacter->GetMesh();
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalLocomotionSystem: No SkeletalMeshComponent found"));
        return;
    }

    bIsInitialized = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("SurvivalLocomotionSystem: Successfully initialized for %s"), *OwnerCharacter->GetName());
}

void UAnim_SurvivalLocomotionSystem::UpdateMovementData(float DeltaTime)
{
    if (!bIsInitialized)
    {
        return;
    }

    // Calculate basic movement values
    CalculateMovementValues();

    // Update movement and action states
    UpdateMovementState();
    UpdateActionState();

    // Apply survival modifiers
    ApplySurvivalModifiers();

    // Check environmental conditions
    CheckEnvironmentalConditions();

    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_SurvivalLocomotionSystem::CalculateMovementValues()
{
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    
    // Smooth speed for animation
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, CurrentSpeed, GetWorld()->GetDeltaSeconds(), MovementSmoothingSpeed);
    MovementData.Speed = SmoothedSpeed;

    // Calculate direction relative to character forward
    if (CurrentSpeed > 5.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        float CurrentDirection = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
        SmoothedDirection = FMath::FInterpTo(SmoothedDirection, CurrentDirection, GetWorld()->GetDeltaSeconds(), DirectionSmoothingSpeed);
        MovementData.Direction = SmoothedDirection;
    }
    else
    {
        MovementData.Direction = 0.0f;
    }

    // Calculate acceleration
    static float PreviousSpeed = 0.0f;
    MovementData.Acceleration = (CurrentSpeed - PreviousSpeed) / GetWorld()->GetDeltaSeconds();
    PreviousSpeed = CurrentSpeed;

    // Update basic movement flags
    MovementData.bIsMoving = CurrentSpeed > 5.0f;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_SurvivalLocomotionSystem::UpdateMovementState()
{
    EAnim_SurvivalMovementState NewState = DetermineMovementState();
    
    if (NewState != MovementData.MovementState)
    {
        SetMovementState(NewState);
    }
}

void UAnim_SurvivalLocomotionSystem::UpdateActionState()
{
    // Action state updates are handled externally through PlaySurvivalAction calls
    // This method can be used for automatic state transitions or timeouts
}

EAnim_SurvivalMovementState UAnim_SurvivalLocomotionSystem::DetermineMovementState() const
{
    // Check for special states first
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_SurvivalMovementState::Jumping;
        }
        else
        {
            return EAnim_SurvivalMovementState::Falling;
        }
    }

    if (MovementData.bIsInWater)
    {
        return EAnim_SurvivalMovementState::Swimming;
    }

    if (MovementData.bIsInjured)
    {
        return EAnim_SurvivalMovementState::Injured;
    }

    if (MovementData.bIsExhausted)
    {
        return EAnim_SurvivalMovementState::Exhausted;
    }

    // Determine movement state based on speed and crouch
    if (MovementData.bIsCrouching)
    {
        if (MovementData.Speed > 5.0f)
        {
            return EAnim_SurvivalMovementState::Sneaking;
        }
        else
        {
            return EAnim_SurvivalMovementState::Crouching;
        }
    }

    if (!MovementData.bIsMoving)
    {
        return EAnim_SurvivalMovementState::Idle;
    }

    // Speed-based movement states
    if (MovementData.Speed < WalkSpeedThreshold)
    {
        return EAnim_SurvivalMovementState::Walking;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        return EAnim_SurvivalMovementState::Running;
    }
    else
    {
        return EAnim_SurvivalMovementState::Sprinting;
    }
}

void UAnim_SurvivalLocomotionSystem::ApplySurvivalModifiers()
{
    // Update survival stats over time
    if (MovementData.MovementState == EAnim_SurvivalMovementState::Sprinting)
    {
        ModifyStamina(-StaminaDepletionRate * GetWorld()->GetDeltaSeconds());
    }
    else if (MovementData.MovementState == EAnim_SurvivalMovementState::Idle)
    {
        ModifyStamina(HealthRegenerationRate * GetWorld()->GetDeltaSeconds());
    }

    // Check for exhaustion
    if (MovementData.StaminaLevel <= 10.0f)
    {
        MovementData.bIsExhausted = true;
    }
    else if (MovementData.StaminaLevel >= 30.0f)
    {
        MovementData.bIsExhausted = false;
    }

    // Apply speed modifiers based on survival state
    float SpeedMultiplier = GetMovementSpeedMultiplier();
    if (MovementComponent && SpeedMultiplier != 1.0f)
    {
        // This would need to be applied to the character's movement component
        // Implementation depends on how the character movement is set up
    }
}

void UAnim_SurvivalLocomotionSystem::CheckEnvironmentalConditions()
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Perform line trace to check ground conditions
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calculate ground slope
        FVector GroundNormal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
        MovementData.GroundSlope = SlopeAngle;
        MovementData.bIsOnUnevenTerrain = SlopeAngle > 15.0f;
    }

    // Check for water (simplified - would need proper water volume detection)
    // This is a placeholder implementation
    MovementData.bIsInWater = false;
}

void UAnim_SurvivalLocomotionSystem::SetMovementState(EAnim_SurvivalMovementState NewState)
{
    if (MovementData.MovementState != NewState)
    {
        EAnim_SurvivalMovementState PreviousState = MovementData.MovementState;
        MovementData.MovementState = NewState;

        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_SurvivalLocomotionSystem::SetActionState(EAnim_SurvivalActionState NewState)
{
    if (MovementData.ActionState != NewState)
    {
        EAnim_SurvivalActionState PreviousState = MovementData.ActionState;
        MovementData.ActionState = NewState;

        UE_LOG(LogTemp, Log, TEXT("Action state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_SurvivalLocomotionSystem::PlaySurvivalAction(EAnim_SurvivalActionState ActionType)
{
    if (!CanPerformAction())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot perform action - character is busy or incapacitated"));
        return;
    }

    UAnimMontage* MontageToPlay = GetMontageForAction(ActionType);
    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("No montage found for action type %d"), (int32)ActionType);
        return;
    }

    if (MeshComponent && MeshComponent->GetAnimInstance())
    {
        float MontageLength = MeshComponent->GetAnimInstance()->Montage_Play(MontageToPlay);
        if (MontageLength > 0.0f)
        {
            SetActionState(ActionType);
            UE_LOG(LogTemp, Log, TEXT("Playing survival action montage for action type %d"), (int32)ActionType);
        }
    }
}

void UAnim_SurvivalLocomotionSystem::StopSurvivalAction()
{
    if (MeshComponent && MeshComponent->GetAnimInstance())
    {
        MeshComponent->GetAnimInstance()->Montage_Stop(0.3f);
        SetActionState(EAnim_SurvivalActionState::None);
    }
}

UAnimMontage* UAnim_SurvivalLocomotionSystem::GetMontageForAction(EAnim_SurvivalActionState ActionType) const
{
    switch (ActionType)
    {
        case EAnim_SurvivalActionState::Crafting:
            return CraftingMontage;
        case EAnim_SurvivalActionState::Gathering:
            return GatheringMontage;
        case EAnim_SurvivalActionState::Hunting:
            return HuntingMontage;
        case EAnim_SurvivalActionState::Building:
            return BuildingMontage;
        case EAnim_SurvivalActionState::Eating:
            return EatingMontage;
        case EAnim_SurvivalActionState::Drinking:
            return DrinkingMontage;
        case EAnim_SurvivalActionState::Climbing:
            return ClimbingMontage;
        case EAnim_SurvivalActionState::Fighting:
            return nullptr; // Combat montages handled separately
        default:
            return nullptr;
    }
}

void UAnim_SurvivalLocomotionSystem::ApplyInjury(float InjuryLevel)
{
    MovementData.HealthLevel = FMath::Clamp(MovementData.HealthLevel - InjuryLevel, 0.0f, 100.0f);
    MovementData.bIsInjured = MovementData.HealthLevel < 50.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Applied injury: %f, Health now: %f"), InjuryLevel, MovementData.HealthLevel);
}

void UAnim_SurvivalLocomotionSystem::ModifyStamina(float StaminaChange)
{
    MovementData.StaminaLevel = FMath::Clamp(MovementData.StaminaLevel + StaminaChange, 0.0f, 100.0f);
}

void UAnim_SurvivalLocomotionSystem::ModifyFear(float FearChange)
{
    MovementData.FearLevel = FMath::Clamp(MovementData.FearLevel + FearChange, 0.0f, 100.0f);
}

void UAnim_SurvivalLocomotionSystem::UpdateEnvironmentalFactors()
{
    CheckEnvironmentalConditions();
}

bool UAnim_SurvivalLocomotionSystem::CanPerformAction() const
{
    // Cannot perform actions if already performing one
    if (MovementData.ActionState != EAnim_SurvivalActionState::None)
    {
        return false;
    }

    // Cannot perform actions while in air
    if (MovementData.bIsInAir)
    {
        return false;
    }

    // Cannot perform actions while severely injured or exhausted
    if (MovementData.HealthLevel < 20.0f || MovementData.StaminaLevel < 10.0f)
    {
        return false;
    }

    return true;
}

float UAnim_SurvivalLocomotionSystem::GetMovementSpeedMultiplier() const
{
    float Multiplier = 1.0f;

    if (MovementData.bIsInjured)
    {
        Multiplier *= InjuredSpeedMultiplier;
    }

    if (MovementData.bIsExhausted)
    {
        Multiplier *= ExhaustedSpeedMultiplier;
    }

    if (MovementData.FearLevel > 50.0f)
    {
        Multiplier *= FearMovementMultiplier;
    }

    return Multiplier;
}