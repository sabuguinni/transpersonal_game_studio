#include "AnimationSystemManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionMatchingComponent.h"
#include "FootIKComponent.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    LocomotionDatabase = nullptr;
    CombatDatabase = nullptr;
    InteractionDatabase = nullptr;

    bEnableFootIK = true;
    bEnableHandIK = true;
    FootIKRange = 50.0f;
    HandIKRange = 30.0f;

    bIsInCombat = false;
    bIsClimbing = false;
    bIsSwimming = false;
    bIsCrouching = false;

    ConfidenceLevel = 0.5f;
    AgilityLevel = 0.5f;
    StrengthLevel = 0.5f;

    BlendInTime = 0.2f;
    BlendOutTime = 0.2f;

    LastVelocity = FVector::ZeroVector;
    LastPosition = FVector::ZeroVector;
    LastUpdateTime = 0.0f;

    MotionMatchingComponent = nullptr;
    FootIKComponent = nullptr;
    SkeletalMeshComponent = nullptr;
    CharacterMovementComponent = nullptr;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeAnimationSystem();
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateAnimationState(DeltaTime);
}

void UAnimationSystemManager::InitializeAnimationSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Initializing animation system..."));

    // Cache component references
    CacheComponentReferences();

    // Create Motion Matching component if not exists
    if (!MotionMatchingComponent && GetOwner())
    {
        MotionMatchingComponent = NewObject<UMotionMatchingComponent>(GetOwner());
        if (MotionMatchingComponent)
        {
            GetOwner()->AddInstanceComponent(MotionMatchingComponent);
            MotionMatchingComponent->RegisterComponent();
            UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Created Motion Matching component"));
        }
    }

    // Create Foot IK component if not exists
    if (!FootIKComponent && GetOwner())
    {
        FootIKComponent = NewObject<UFootIKComponent>(GetOwner());
        if (FootIKComponent)
        {
            GetOwner()->AddInstanceComponent(FootIKComponent);
            FootIKComponent->RegisterComponent();
            UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Created Foot IK component"));
        }
    }

    // Validate animation databases
    ValidateAnimationDatabases();

    // Initialize position tracking
    if (GetOwner())
    {
        LastPosition = GetOwner()->GetActorLocation();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }

    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Animation system initialized successfully"));
}

void UAnimationSystemManager::UpdateAnimationState(float DeltaTime)
{
    if (!GetOwner() || !SkeletalMeshComponent)
    {
        return;
    }

    // Update movement data
    UpdateMovementData(DeltaTime);

    // Update IK systems
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }

    if (bEnableHandIK)
    {
        UpdateHandIK(DeltaTime);
    }

    // Update Motion Matching component
    if (MotionMatchingComponent)
    {
        // Motion Matching will be handled by the component itself
        // We just provide the necessary data
    }
}

void UAnimationSystemManager::SetMotionMatchingDatabase(UPoseSearchDatabase* Database)
{
    if (Database)
    {
        LocomotionDatabase = Database;
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Motion Matching database set: %s"), *Database->GetName());
    }
}

FVector UAnimationSystemManager::GetDesiredVelocity() const
{
    if (CharacterMovementComponent)
    {
        return CharacterMovementComponent->GetLastInputVector() * CharacterMovementComponent->GetMaxSpeed();
    }
    return FVector::ZeroVector;
}

float UAnimationSystemManager::GetMovementSpeed() const
{
    if (CharacterMovementComponent)
    {
        return CharacterMovementComponent->Velocity.Size();
    }
    return 0.0f;
}

FVector UAnimationSystemManager::GetMovementDirection() const
{
    if (CharacterMovementComponent)
    {
        FVector Velocity = CharacterMovementComponent->Velocity;
        Velocity.Z = 0.0f; // Ignore vertical movement for direction
        return Velocity.GetSafeNormal();
    }
    return FVector::ForwardVector;
}

void UAnimationSystemManager::UpdateFootIK(float DeltaTime)
{
    if (!FootIKComponent || !bEnableFootIK)
    {
        return;
    }

    // Foot IK will be handled by the FootIKComponent
    // This is where we would update foot placement based on terrain
}

void UAnimationSystemManager::UpdateHandIK(float DeltaTime)
{
    if (!bEnableHandIK)
    {
        return;
    }

    // Hand IK for interaction with environment
    // This would handle reaching for objects, climbing, etc.
}

FVector UAnimationSystemManager::GetFootIKOffset(bool bIsLeftFoot) const
{
    if (FootIKComponent)
    {
        return FootIKComponent->GetFootIKOffset(bIsLeftFoot);
    }
    return FVector::ZeroVector;
}

FVector UAnimationSystemManager::GetHandIKOffset(bool bIsLeftHand) const
{
    // Hand IK implementation would go here
    // For now, return zero offset
    return FVector::ZeroVector;
}

void UAnimationSystemManager::SetCombatMode(bool bInCombat)
{
    if (bIsInCombat != bInCombat)
    {
        bIsInCombat = bInCombat;
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Combat mode set to %s"), bInCombat ? TEXT("true") : TEXT("false"));

        // Switch to combat database if available
        if (bInCombat && CombatDatabase)
        {
            SetMotionMatchingDatabase(CombatDatabase);
        }
        else if (!bInCombat && LocomotionDatabase)
        {
            SetMotionMatchingDatabase(LocomotionDatabase);
        }
    }
}

void UAnimationSystemManager::SetClimbingMode(bool bClimbing)
{
    bIsClimbing = bClimbing;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Climbing mode set to %s"), bClimbing ? TEXT("true") : TEXT("false"));
}

void UAnimationSystemManager::SetSwimmingMode(bool bSwimming)
{
    bIsSwimming = bSwimming;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Swimming mode set to %s"), bSwimming ? TEXT("true") : TEXT("false"));
}

void UAnimationSystemManager::SetCrouchingMode(bool bCrouching)
{
    bIsCrouching = bCrouching;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Crouching mode set to %s"), bCrouching ? TEXT("true") : TEXT("false"));
}

void UAnimationSystemManager::SetCharacterPersonality(float Confidence, float Agility, float Strength)
{
    ConfidenceLevel = FMath::Clamp(Confidence, 0.0f, 1.0f);
    AgilityLevel = FMath::Clamp(Agility, 0.0f, 1.0f);
    StrengthLevel = FMath::Clamp(Strength, 0.0f, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Character personality set - Confidence: %.2f, Agility: %.2f, Strength: %.2f"), 
           ConfidenceLevel, AgilityLevel, StrengthLevel);
}

float UAnimationSystemManager::GetPersonalityBlendWeight(const FString& AnimationType) const
{
    return CalculatePersonalityInfluence(AnimationType);
}

UAnimInstance* UAnimationSystemManager::GetAnimInstance() const
{
    if (SkeletalMeshComponent)
    {
        return SkeletalMeshComponent->GetAnimInstance();
    }
    return nullptr;
}

USkeletalMeshComponent* UAnimationSystemManager::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComponent;
}

UCharacterMovementComponent* UAnimationSystemManager::GetCharacterMovement() const
{
    return CharacterMovementComponent;
}

void UAnimationSystemManager::CacheComponentReferences()
{
    if (GetOwner())
    {
        SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        CharacterMovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();

        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: No SkeletalMeshComponent found on owner!"));
        }

        if (!CharacterMovementComponent)
        {
            UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: No CharacterMovementComponent found on owner!"));
        }
    }
}

void UAnimationSystemManager::UpdateMovementData(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }

    FVector CurrentPosition = GetOwner()->GetActorLocation();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Calculate velocity from position change
    if (CurrentTime > LastUpdateTime)
    {
        FVector PositionDelta = CurrentPosition - LastPosition;
        float TimeDelta = CurrentTime - LastUpdateTime;
        LastVelocity = PositionDelta / TimeDelta;
    }

    LastPosition = CurrentPosition;
    LastUpdateTime = CurrentTime;
}

float UAnimationSystemManager::CalculatePersonalityInfluence(const FString& AnimationType) const
{
    // Calculate how personality traits affect different animation types
    float BlendWeight = 0.5f; // Default neutral

    if (AnimationType == "Confident")
    {
        BlendWeight = ConfidenceLevel;
    }
    else if (AnimationType == "Agile")
    {
        BlendWeight = AgilityLevel;
    }
    else if (AnimationType == "Strong")
    {
        BlendWeight = StrengthLevel;
    }
    else if (AnimationType == "Timid")
    {
        BlendWeight = 1.0f - ConfidenceLevel;
    }
    else if (AnimationType == "Clumsy")
    {
        BlendWeight = 1.0f - AgilityLevel;
    }
    else if (AnimationType == "Weak")
    {
        BlendWeight = 1.0f - StrengthLevel;
    }

    return FMath::Clamp(BlendWeight, 0.0f, 1.0f);
}

void UAnimationSystemManager::ValidateAnimationDatabases()
{
    if (!LocomotionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: No locomotion database assigned!"));
    }

    if (!CombatDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: No combat database assigned!"));
    }

    if (!InteractionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: No interaction database assigned!"));
    }
}