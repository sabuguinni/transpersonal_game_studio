#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    SmoothedDirection = FVector2D::ZeroVector;
    SmoothedSpeed = 0.0f;
    LastUpdateTime = 0.0f;
    
    // Set default configuration
    Config = FAnim_BlendSpaceConfig();
}

void UAnim_BlendSpaceController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize blend spaces
    InitializeBlendSpaces();
    
    // Validate owner is a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        UE_LOG(LogTemp, Log, TEXT("BlendSpaceController initialized for character: %s"), *Character->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BlendSpaceController attached to non-character actor"));
    }
}

void UAnim_BlendSpaceController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            // Get current movement data
            FVector Velocity = MovementComp->Velocity;
            float CurrentSpeed = Velocity.Size2D();
            FVector2D MovementDir = FVector2D::ZeroVector;
            
            if (CurrentSpeed > 0.1f)
            {
                FVector ForwardVector = Character->GetActorForwardVector();
                FVector RightVector = Character->GetActorRightVector();
                
                // Calculate movement direction relative to character
                float ForwardDot = FVector::DotProduct(Velocity.GetSafeNormal(), ForwardVector);
                float RightDot = FVector::DotProduct(Velocity.GetSafeNormal(), RightVector);
                
                MovementDir = FVector2D(ForwardDot, RightDot);
            }
            
            // Update blend space values
            UpdateBlendSpaceValues(CurrentSpeed, MovementDir);
            
            // Update smoothed values
            UpdateSmoothedValues(DeltaTime, CurrentSpeed, MovementDir);
            
            // Update air state
            BlendSpaceData.bIsInAir = MovementComp->IsFalling();
        }
    }
}

void UAnim_BlendSpaceController::InitializeBlendSpaces()
{
    // Create default blend spaces if none are assigned
    if (!BlendSpaceData.MovementBlendSpace)
    {
        CreateDefaultBlendSpaces();
    }
    
    ValidateBlendSpaces();
}

void UAnim_BlendSpaceController::UpdateBlendSpaceValues(float Speed, const FVector2D& Direction)
{
    BlendSpaceData.CurrentSpeed = Speed;
    BlendSpaceData.MovementDirection = Direction;
    
    // Normalize speed for blend space sampling
    float NormalizedSpeed = GetNormalizedSpeed();
    
    // Update last update time
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_BlendSpaceController::SetMovementBlendSpace(UBlendSpace* NewBlendSpace)
{
    BlendSpaceData.MovementBlendSpace = NewBlendSpace;
    
    if (NewBlendSpace)
    {
        UE_LOG(LogTemp, Log, TEXT("Movement BlendSpace set: %s"), *NewBlendSpace->GetName());
    }
}

void UAnim_BlendSpaceController::SetSpeedBlendSpace(UBlendSpace1D* NewBlendSpace)
{
    BlendSpaceData.SpeedBlendSpace = NewBlendSpace;
    
    if (NewBlendSpace)
    {
        UE_LOG(LogTemp, Log, TEXT("Speed BlendSpace set: %s"), *NewBlendSpace->GetName());
    }
}

float UAnim_BlendSpaceController::GetNormalizedSpeed() const
{
    if (Config.MaxRunSpeed > 0.0f)
    {
        return FMath::Clamp(BlendSpaceData.CurrentSpeed / Config.MaxRunSpeed, 0.0f, 1.0f);
    }
    
    return 0.0f;
}

void UAnim_BlendSpaceController::SetBlendSpaceConfig(const FAnim_BlendSpaceConfig& NewConfig)
{
    Config = NewConfig;
    
    UE_LOG(LogTemp, Log, TEXT("BlendSpace configuration updated - MaxWalk: %f, MaxRun: %f"), 
           Config.MaxWalkSpeed, Config.MaxRunSpeed);
}

UAnimSequence* UAnim_BlendSpaceController::SampleBlendSpaceAnimation(float Speed, const FVector2D& Direction)
{
    if (!BlendSpaceData.MovementBlendSpace)
    {
        return nullptr;
    }
    
    // Sample the blend space at the given parameters
    FBlendSampleData SampleData;
    TArray<FBlendSampleData> BlendSamples;
    
    // This would typically be done through the animation blueprint
    // For now, return nullptr as this requires more complex blend space sampling
    return nullptr;
}

void UAnim_BlendSpaceController::CreateDefaultBlendSpaces()
{
    // Create default movement blend space
    BlendSpaceData.MovementBlendSpace = CreateMovementBlendSpace();
    BlendSpaceData.SpeedBlendSpace = CreateSpeedBlendSpace();
    
    UE_LOG(LogTemp, Log, TEXT("Default blend spaces created"));
}

void UAnim_BlendSpaceController::UpdateSmoothedValues(float DeltaTime, float TargetSpeed, const FVector2D& TargetDirection)
{
    // Smooth speed changes
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, TargetSpeed, DeltaTime, Config.BlendSpaceSmoothing);
    
    // Smooth direction changes
    SmoothedDirection = FMath::Vector2DInterpTo(SmoothedDirection, TargetDirection, DeltaTime, Config.BlendSpaceSmoothing);
    
    // Apply direction change threshold
    if (FVector2D::Distance(SmoothedDirection, TargetDirection) < Config.DirectionChangeThreshold)
    {
        SmoothedDirection = TargetDirection;
    }
}

void UAnim_BlendSpaceController::ValidateBlendSpaces()
{
    if (!BlendSpaceData.MovementBlendSpace)
    {
        UE_LOG(LogTemp, Warning, TEXT("Movement BlendSpace is null"));
    }
    
    if (!BlendSpaceData.SpeedBlendSpace)
    {
        UE_LOG(LogTemp, Warning, TEXT("Speed BlendSpace is null"));
    }
}

UBlendSpace* UAnim_BlendSpaceController::CreateMovementBlendSpace()
{
    // In a real implementation, this would create a blend space asset
    // For now, return nullptr as asset creation requires editor-only code
    UE_LOG(LogTemp, Log, TEXT("Movement BlendSpace creation requested - requires asset creation"));
    return nullptr;
}

UBlendSpace1D* UAnim_BlendSpaceController::CreateSpeedBlendSpace()
{
    // In a real implementation, this would create a 1D blend space asset
    // For now, return nullptr as asset creation requires editor-only code
    UE_LOG(LogTemp, Log, TEXT("Speed BlendSpace creation requested - requires asset creation"));
    return nullptr;
}