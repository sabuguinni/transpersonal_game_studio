#include "PlayerMovementAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UPlayerMovementAnimController::UPlayerMovementAnimController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    MovementDeadZone = 10.0f;
    FearSpeedMultiplier = 1.5f;
    HighFearThreshold = 0.7f;
    LowStaminaThreshold = 0.3f;
    LowStaminaSpeedMultiplier = 0.7f;
    
    PreviousMovementState = EAnim_PlayerMovementState::Idle;
    StateChangeCooldown = 0.0f;
}

void UPlayerMovementAnimController::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-initialize with owner character if not already set
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
        if (OwnerCharacter)
        {
            InitializeWithCharacter(OwnerCharacter);
        }
    }
}

void UPlayerMovementAnimController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateAnimationState(DeltaTime);
    }
}

void UPlayerMovementAnimController::InitializeWithCharacter(ACharacter* InCharacter)
{
    if (!InCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerMovementAnimController: Cannot initialize with null character"));
        return;
    }
    
    OwnerCharacter = InCharacter;
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    MeshComponent = OwnerCharacter->GetMesh();
    
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerMovementAnimController: Character has no movement component"));
        return;
    }
    
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerMovementAnimController: Character has no mesh component"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PlayerMovementAnimController: Successfully initialized with character %s"), 
           *OwnerCharacter->GetName());
}

void UPlayerMovementAnimController::UpdateAnimationState(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update state change cooldown
    if (StateChangeCooldown > 0.0f)
    {
        StateChangeCooldown -= DeltaTime;
    }
    
    // Update movement data
    UpdateMovementData(DeltaTime);
}

void UPlayerMovementAnimController::UpdateMovementData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size2D();
    
    // Calculate movement direction
    MovementData.Direction = CalculateMovementDirection();
    
    // Check if character is moving
    MovementData.bIsMoving = MovementData.Speed > MovementDeadZone;
    
    // Check air state
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Check crouching state
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine movement state
    EAnim_PlayerMovementState NewState = DetermineMovementState();
    
    // Update state if cooldown has passed and state has changed
    if (StateChangeCooldown <= 0.0f && NewState != MovementData.MovementState)
    {
        PreviousMovementState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        StateChangeCooldown = StateChangeCooldownTime;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("PlayerMovementAnimController: State changed from %d to %d"), 
               (int32)PreviousMovementState, (int32)NewState);
    }
}

float UPlayerMovementAnimController::CalculateMovementDirection() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.Size2D() < MovementDeadZone)
    {
        return 0.0f;
    }
    
    // Get character forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Calculate angle between forward vector and velocity
    FVector VelocityNormalized = Velocity.GetSafeNormal2D();
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    // Convert to angle in degrees (-180 to 180)
    float Angle = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    
    return Angle;
}

EAnim_PlayerMovementState UPlayerMovementAnimController::DetermineMovementState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_PlayerMovementState::Idle;
    }
    
    // Check air states first
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_PlayerMovementState::Jumping;
        }
        else
        {
            return EAnim_PlayerMovementState::Falling;
        }
    }
    
    // Check crouching states
    if (MovementData.bIsCrouching)
    {
        if (MovementData.bIsMoving)
        {
            return EAnim_PlayerMovementState::Crawling;
        }
        else
        {
            return EAnim_PlayerMovementState::Crouching;
        }
    }
    
    // Check movement states
    if (!MovementData.bIsMoving)
    {
        return EAnim_PlayerMovementState::Idle;
    }
    
    // Apply fear and stamina modifiers to thresholds
    float ModifiedWalkThreshold = WalkSpeedThreshold;
    float ModifiedRunThreshold = RunSpeedThreshold;
    
    if (MovementData.FearLevel > HighFearThreshold)
    {
        ModifiedWalkThreshold *= FearSpeedMultiplier;
        ModifiedRunThreshold *= FearSpeedMultiplier;
    }
    
    if (MovementData.StaminaLevel < LowStaminaThreshold)
    {
        ModifiedWalkThreshold *= LowStaminaSpeedMultiplier;
        ModifiedRunThreshold *= LowStaminaSpeedMultiplier;
    }
    
    // Determine movement speed state
    if (MovementData.Speed >= ModifiedRunThreshold)
    {
        return EAnim_PlayerMovementState::Running;
    }
    else if (MovementData.Speed >= ModifiedWalkThreshold)
    {
        return EAnim_PlayerMovementState::Walking;
    }
    else
    {
        return EAnim_PlayerMovementState::Idle;
    }
}

void UPlayerMovementAnimController::SetFearLevel(float InFearLevel)
{
    MovementData.FearLevel = FMath::Clamp(InFearLevel, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("PlayerMovementAnimController: Fear level set to %f"), 
           MovementData.FearLevel);
}

void UPlayerMovementAnimController::SetStaminaLevel(float InStaminaLevel)
{
    MovementData.StaminaLevel = FMath::Clamp(InStaminaLevel, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("PlayerMovementAnimController: Stamina level set to %f"), 
           MovementData.StaminaLevel);
}

bool UPlayerMovementAnimController::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!MeshComponent || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerMovementAnimController: Cannot play montage - invalid mesh or montage"));
        return false;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerMovementAnimController: No animation instance found"));
        return false;
    }
    
    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate);
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("PlayerMovementAnimController: Playing montage %s for %f seconds"), 
               *Montage->GetName(), MontageLength);
        return true;
    }
    
    return false;
}

void UPlayerMovementAnimController::StopAnimationMontage(float BlendOutTime)
{
    if (!MeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    AnimInstance->Montage_Stop(BlendOutTime);
    UE_LOG(LogTemp, VeryVerbose, TEXT("PlayerMovementAnimController: Stopped current montage"));
}