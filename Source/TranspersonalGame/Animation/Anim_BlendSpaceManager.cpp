#include "Anim_BlendSpaceManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_BlendSpaceManager::UAnim_BlendSpaceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentSpeed = 0.0f;
    CurrentDirection = 0.0f;
    bIsMoving = false;
    bIsRunning = false;
    PreviousSpeed = 0.0f;
    SpeedChangeRate = 5.0f;
    OwnerCharacter = nullptr;
}

void UAnim_BlendSpaceManager::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("BlendSpaceManager: Owner is not a Character"));
        return;
    }

    LoadBlendSpaces();
}

void UAnim_BlendSpaceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter)
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    FVector CurrentVelocity = MovementComp->Velocity;
    float MaxSpeed = MovementComp->MaxWalkSpeed;

    UpdateBlendSpaceValues(CurrentVelocity, MaxSpeed);
}

void UAnim_BlendSpaceManager::UpdateBlendSpaceValues(const FVector& Velocity, float MaxSpeed)
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Calculate speed and direction
    CalculateMovementValues(Velocity);

    // Determine movement states
    bIsMoving = CurrentSpeed > 10.0f;
    bIsRunning = CurrentSpeed > BlendSpaceConfig.MaxWalkSpeed;

    // Update blend space parameters
    UpdateBlendSpaceParameters();
}

void UAnim_BlendSpaceManager::CalculateMovementValues(const FVector& Velocity)
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Calculate current speed
    float TargetSpeed = Velocity.Size();
    CurrentSpeed = FMath::FInterpTo(PreviousSpeed, TargetSpeed, GetWorld()->GetDeltaSeconds(), SpeedChangeRate);
    PreviousSpeed = CurrentSpeed;

    // Calculate direction relative to character forward
    if (CurrentSpeed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        CurrentDirection = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
    else
    {
        CurrentDirection = 0.0f;
    }
}

void UAnim_BlendSpaceManager::UpdateBlendSpaceParameters()
{
    if (!OwnerCharacter)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    // Set animation blueprint variables
    // These would typically be accessed through the animation blueprint
    // For now, we'll log the values for debugging
    UE_LOG(LogTemp, Log, TEXT("BlendSpace Values - Speed: %f, Direction: %f, Moving: %s, Running: %s"), 
           CurrentSpeed, CurrentDirection, bIsMoving ? TEXT("true") : TEXT("false"), bIsRunning ? TEXT("true") : TEXT("false"));
}

void UAnim_BlendSpaceManager::SetBlendSpaceConfig(const FAnim_BlendSpaceConfig& NewConfig)
{
    BlendSpaceConfig = NewConfig;
    LoadBlendSpaces();
}

float UAnim_BlendSpaceManager::GetNormalizedSpeed() const
{
    if (BlendSpaceConfig.MaxRunSpeed <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(CurrentSpeed / BlendSpaceConfig.MaxRunSpeed, 0.0f, 1.0f);
}

FVector2D UAnim_BlendSpaceManager::GetBlendSpaceInput() const
{
    float NormalizedSpeed = GetNormalizedSpeed();
    float DirectionRadians = FMath::DegreesToRadians(CurrentDirection);
    
    float X = NormalizedSpeed * FMath::Cos(DirectionRadians);
    float Y = NormalizedSpeed * FMath::Sin(DirectionRadians);
    
    return FVector2D(X, Y);
}

void UAnim_BlendSpaceManager::LoadBlendSpaces()
{
    // Load blend space assets
    if (BlendSpaceConfig.LocomotionBlendSpace.IsValid())
    {
        UBlendSpace* LoadedBlendSpace = BlendSpaceConfig.LocomotionBlendSpace.LoadSynchronous();
        if (LoadedBlendSpace)
        {
            UE_LOG(LogTemp, Log, TEXT("BlendSpaceManager: Loaded locomotion blend space"));
        }
    }

    if (BlendSpaceConfig.SpeedBlendSpace.IsValid())
    {
        UBlendSpace1D* LoadedBlendSpace1D = BlendSpaceConfig.SpeedBlendSpace.LoadSynchronous();
        if (LoadedBlendSpace1D)
        {
            UE_LOG(LogTemp, Log, TEXT("BlendSpaceManager: Loaded speed blend space"));
        }
    }
}