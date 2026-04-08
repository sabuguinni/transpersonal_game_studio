#include "AnimationSystemManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    CurrentFearLevel = BaseFearLevel;
    LastMovementTime = 0.0f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    
    // Initialize animation data
    CurrentAnimationData.MovementState = ECharacterMovementState::Idle;
    CurrentAnimationData.Speed = 0.0f;
    CurrentAnimationData.Direction = 0.0f;
    CurrentAnimationData.FearLevel = BaseFearLevel;
    CurrentAnimationData.Exhaustion = 0.0f;
    CurrentAnimationData.CurrentTerrain = ETerrainType::Flat;
    CurrentAnimationData.bIsCarryingWeight = false;
    CurrentAnimationData.bIsInjured = false;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Validate required components
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (!Character->GetMesh())
        {
            UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: Character has no skeletal mesh component"));
        }
        
        if (!Character->GetCharacterMovement())
        {
            UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: Character has no movement component"));
        }
    }
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update fear level based on environment
    UpdateFearLevel(DeltaTime);
    
    // Analyze terrain for IK adjustments
    AnalyzeTerrainBelowFeet();
    
    // Update character movement data
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            CurrentAnimationData.Speed = MovementComp->Velocity.Size();
            
            // Determine movement state based on speed and input
            if (CurrentAnimationData.Speed < 10.0f)
            {
                CurrentAnimationData.MovementState = ECharacterMovementState::Idle;
            }
            else if (CurrentAnimationData.Speed < 200.0f)
            {
                CurrentAnimationData.MovementState = ECharacterMovementState::Walking;
            }
            else
            {
                CurrentAnimationData.MovementState = ECharacterMovementState::Running;
            }
            
            // Calculate movement direction
            FVector Forward = Character->GetActorForwardVector();
            FVector VelocityNorm = MovementComp->Velocity.GetSafeNormal();
            CurrentAnimationData.Direction = FVector::DotProduct(Forward, VelocityNorm);
        }
    }
    
    // Update animation data with current fear level
    CurrentAnimationData.FearLevel = CurrentFearLevel;
}

void UAnimationSystemManager::UpdateMovementState(ECharacterMovementState NewState)
{
    if (CurrentAnimationData.MovementState != NewState)
    {
        CurrentAnimationData.MovementState = NewState;
        LastMovementTime = GetWorld()->GetTimeSeconds();
        
        // Adjust fear based on movement state
        switch (NewState)
        {
            case ECharacterMovementState::Running:
                SetFearLevel(FMath::Min(CurrentFearLevel + 0.3f, MaxFearLevel));
                break;
            case ECharacterMovementState::Sneaking:
                SetFearLevel(FMath::Min(CurrentFearLevel + 0.1f, MaxFearLevel));
                break;
            case ECharacterMovementState::Frightened:
                SetFearLevel(MaxFearLevel);
                break;
            default:
                break;
        }
    }
}

void UAnimationSystemManager::SetFearLevel(float NewFearLevel)
{
    CurrentFearLevel = FMath::Clamp(NewFearLevel, 0.0f, MaxFearLevel);
    CurrentAnimationData.FearLevel = CurrentFearLevel;
}

void UAnimationSystemManager::UpdateTerrainType(ETerrainType NewTerrain)
{
    CurrentAnimationData.CurrentTerrain = NewTerrain;
    
    // Adjust IK intensity based on terrain
    switch (NewTerrain)
    {
        case ETerrainType::Rocky:
        case ETerrainType::Uneven:
            FootIKIntensity = 1.5f;
            break;
        case ETerrainType::Muddy:
            FootIKIntensity = 1.2f;
            break;
        case ETerrainType::Flat:
            FootIKIntensity = 1.0f;
            break;
        default:
            FootIKIntensity = 1.1f;
            break;
    }
}

FVector UAnimationSystemManager::GetFootIKOffset(bool bIsLeftFoot)
{
    return bIsLeftFoot ? LeftFootIKOffset : RightFootIKOffset;
}

float UAnimationSystemManager::GetCurrentBlendTime() const
{
    // Dynamic blend time based on fear level and movement state
    float BaseBlendTime = 0.2f;
    
    // Faster blending when frightened
    if (CurrentAnimationData.MovementState == ECharacterMovementState::Frightened)
    {
        BaseBlendTime *= 0.5f;
    }
    
    // Slower blending when exhausted
    BaseBlendTime *= (1.0f + CurrentAnimationData.Exhaustion * 0.5f);
    
    return FMath::Clamp(BaseBlendTime, 0.1f, 1.0f);
}

void UAnimationSystemManager::UpdateFearLevel(float DeltaTime)
{
    // Natural fear decay over time
    if (CurrentFearLevel > BaseFearLevel)
    {
        CurrentFearLevel = FMath::Max(BaseFearLevel, CurrentFearLevel - (FearDecayRate * DeltaTime));
    }
    
    // Environmental fear factors would be handled by other systems
    // This is just the baseline decay
}

void UAnimationSystemManager::AnalyzeTerrainBelowFeet()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetMesh())
    {
        return;
    }
    
    // Get foot bone locations
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    FVector LeftFootLocation = Mesh->GetSocketLocation(TEXT("foot_l"));
    FVector RightFootLocation = Mesh->GetSocketLocation(TEXT("foot_r"));
    
    // Perform traces for each foot
    LeftFootIKOffset = PerformFootTrace(LeftFootLocation, true);
    RightFootIKOffset = PerformFootTrace(RightFootLocation, false);
}

FVector UAnimationSystemManager::PerformFootTrace(FVector FootLocation, bool bIsLeftFoot)
{
    FHitResult HitResult;
    FVector StartLocation = FootLocation + FVector(0, 0, 20);
    FVector EndLocation = FootLocation - FVector(0, 0, FootIKTraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        float DistanceFromGround = (FootLocation.Z - HitResult.Location.Z);
        FVector TargetOffset = FVector(0, 0, -DistanceFromGround) * FootIKIntensity;
        
        // Smooth interpolation
        FVector CurrentOffset = bIsLeftFoot ? LeftFootIKOffset : RightFootIKOffset;
        return FMath::VInterpTo(CurrentOffset, TargetOffset, GetWorld()->GetDeltaSeconds(), FootIKInterpSpeed);
    }
    
    return FVector::ZeroVector;
}