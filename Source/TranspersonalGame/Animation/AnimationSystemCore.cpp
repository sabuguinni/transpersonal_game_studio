// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogAnimationSystem);

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize default values
    MovementSpeed = 0.0f;
    MovementDirection = FVector::ZeroVector;
    TurnRate = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    StaminaLevel = 1.0f;
    
    FootIKAlpha = 1.0f;
    HandIKAlpha = 1.0f;
    bEnableFootIK = true;
    bEnableHandIK = true;
    
    ConfidenceLevel = 0.7f;
    FatigueLevel = 0.0f;
    InjuryLevel = 0.0f;
    AgeModifier = 1.0f;
    
    TerrainRoughness = 0.0f;
    SlopeAngle = 0.0f;
    bIsOnUnstableSurface = false;
    
    LastMovementSpeed = 0.0f;
    LastMovementDirection = FVector::ZeroVector;
    BlendTimer = 0.0f;
    
    UpdateFrequency = 60.0f; // 60 FPS
    TimeSinceLastUpdate = 0.0f;
    
    AnimationLOD = 0;
    bHighQualityMode = true;
}

void UAnimationSystemCore::UpdateMotionMatchingQuery(float DeltaTime)
{
    if (!DefaultSchema)
    {
        UE_LOG(LogAnimationSystem, Warning, TEXT("No PoseSearchSchema assigned"));
        return;
    }

    // Update timing
    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate < (1.0f / UpdateFrequency))
    {
        return;
    }
    TimeSinceLastUpdate = 0.0f;

    // Calculate movement deltas for motion matching
    float SpeedDelta = FMath::Abs(MovementSpeed - LastMovementSpeed);
    float DirectionDelta = FVector::Dist(MovementDirection, LastMovementDirection);
    
    // Store current values for next frame
    LastMovementSpeed = MovementSpeed;
    LastMovementDirection = MovementDirection;
    
    // Apply character traits to motion matching
    float TraitModifiedSpeed = MovementSpeed;
    float PostureOffset = 0.0f;
    ApplyCharacterTraits(TraitModifiedSpeed, PostureOffset);
    
    // Log motion matching state
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Motion Matching Update - Speed: %f, Direction: %s"), 
           TraitModifiedSpeed, *MovementDirection.ToString());
}

UPoseSearchDatabase* UAnimationSystemCore::SelectActiveDatabase()
{
    // Select database based on current state
    if (bIsInAir)
    {
        return InteractionDatabase; // Use for aerial movements
    }
    else if (MovementSpeed > 300.0f) // Running threshold
    {
        return LocomotionDatabase;
    }
    else if (MovementSpeed > 50.0f) // Walking threshold
    {
        return LocomotionDatabase;
    }
    else
    {
        return LocomotionDatabase; // Idle animations
    }
}

void UAnimationSystemCore::BlendBetweenDatabases(float BlendWeight)
{
    // Smooth blending between different animation databases
    BlendTimer += BlendWeight;
    BlendTimer = FMath::Clamp(BlendTimer, 0.0f, 1.0f);
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Database blend weight: %f"), BlendTimer);
}

void UAnimationSystemCore::UpdateFootIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    if (!SkeletalMesh || !bEnableFootIK)
    {
        return;
    }

    // Get foot bone locations
    FVector LeftFootLocation = SkeletalMesh->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootLocation = SkeletalMesh->GetBoneLocation(TEXT("foot_r"));
    
    // Trace down from each foot to find ground
    FHitResult LeftHit, RightHit;
    FVector TraceStart, TraceEnd;
    
    // Left foot trace
    TraceStart = LeftFootLocation + FVector(0, 0, 50);
    TraceEnd = LeftFootLocation - FVector(0, 0, 100);
    
    if (SkeletalMesh->GetWorld()->LineTraceSingleByChannel(LeftHit, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        FVector TargetLocation = CalculateFootPlacement(LeftHit.Location, LeftHit.Normal);
        // Apply IK adjustment (would be done in Animation Blueprint)
        UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Left foot IK target: %s"), *TargetLocation.ToString());
    }
    
    // Right foot trace
    TraceStart = RightFootLocation + FVector(0, 0, 50);
    TraceEnd = RightFootLocation - FVector(0, 0, 100);
    
    if (SkeletalMesh->GetWorld()->LineTraceSingleByChannel(RightHit, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        FVector TargetLocation = CalculateFootPlacement(RightHit.Location, RightHit.Normal);
        // Apply IK adjustment (would be done in Animation Blueprint)
        UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Right foot IK target: %s"), *TargetLocation.ToString());
    }
}

void UAnimationSystemCore::UpdateHandIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    if (!SkeletalMesh || !bEnableHandIK)
    {
        return;
    }

    // Hand IK for tool interaction, wall touching, etc.
    // Implementation would depend on specific interaction context
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Updating hand IK"));
}

FVector UAnimationSystemCore::CalculateFootPlacement(const FVector& FootLocation, const FVector& Normal)
{
    // Calculate optimal foot placement based on surface normal and character traits
    FVector AdjustedLocation = FootLocation;
    
    // Adjust for terrain roughness
    if (TerrainRoughness > 0.1f)
    {
        float RandomOffset = FMath::RandRange(-TerrainRoughness * 5.0f, TerrainRoughness * 5.0f);
        AdjustedLocation.Z += RandomOffset;
    }
    
    // Adjust for slope
    if (FMath::Abs(SlopeAngle) > 5.0f)
    {
        FVector SlopeAdjustment = Normal * (SlopeAngle * 0.1f);
        AdjustedLocation += SlopeAdjustment;
    }
    
    return AdjustedLocation;
}

void UAnimationSystemCore::ApplyCharacterTraits(float& AnimationSpeed, float& PostureOffset)
{
    // Confidence affects posture and stride
    PostureOffset = (ConfidenceLevel - 0.5f) * 10.0f; // -5 to +5 degrees
    
    // Fatigue affects speed and precision
    float FatigueSpeedModifier = 1.0f - (FatigueLevel * 0.3f);
    AnimationSpeed *= FatigueSpeedModifier;
    
    // Age affects overall movement speed
    AnimationSpeed *= AgeModifier;
    
    // Injury creates asymmetry
    if (InjuryLevel > 0.1f)
    {
        // Add slight randomness to simulate limping
        float AsymmetryFactor = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * InjuryLevel * 0.1f;
        AnimationSpeed += AsymmetryFactor;
    }
}

void UAnimationSystemCore::UpdateFatigueLevel(float DeltaTime, float MovementIntensity)
{
    // Increase fatigue based on movement intensity
    float FatigueIncrease = MovementIntensity * DeltaTime * 0.01f;
    FatigueLevel = FMath::Clamp(FatigueLevel + FatigueIncrease, 0.0f, 1.0f);
    
    // Natural recovery when not moving intensely
    if (MovementIntensity < 0.3f)
    {
        float Recovery = DeltaTime * 0.05f; // Slow recovery
        FatigueLevel = FMath::Clamp(FatigueLevel - Recovery, 0.0f, 1.0f);
    }
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Fatigue level: %f"), FatigueLevel);
}

void UAnimationSystemCore::ApplyInjuryEffects(FVector& MovementVector)
{
    if (InjuryLevel <= 0.1f)
    {
        return;
    }
    
    // Create asymmetric movement patterns
    float LimpFactor = FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * InjuryLevel;
    MovementVector.Y += LimpFactor * 10.0f; // Side-to-side compensation
    
    // Reduce overall movement efficiency
    MovementVector *= (1.0f - InjuryLevel * 0.2f);
}

void UAnimationSystemCore::AdaptToTerrain(const FVector& SurfaceNormal, float SurfaceRoughness)
{
    TerrainRoughness = SurfaceRoughness;
    
    // Calculate slope angle from normal
    FVector UpVector = FVector::UpVector;
    SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, UpVector)));
    
    // Determine if surface is unstable
    bIsOnUnstableSurface = (SurfaceRoughness > 0.5f) || (FMath::Abs(SlopeAngle) > 30.0f);
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Terrain adaptation - Roughness: %f, Slope: %f"), 
           TerrainRoughness, SlopeAngle);
}

void UAnimationSystemCore::UpdateEnvironmentalFactors(const FVector& Location)
{
    // This would integrate with the environment system to get surface properties
    // For now, we'll simulate basic terrain detection
    
    if (GetWorld())
    {
        FHitResult Hit;
        FVector TraceStart = Location + FVector(0, 0, 100);
        FVector TraceEnd = Location - FVector(0, 0, 200);
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            // Simulate surface roughness based on hit result
            float SimulatedRoughness = FMath::RandRange(0.0f, 0.3f);
            AdaptToTerrain(Hit.Normal, SimulatedRoughness);
        }
    }
}

float UAnimationSystemCore::CalculateBlendWeight(const FString& AnimationType, float CurrentState)
{
    // Calculate smooth blend weights for different animation types
    float BlendWeight = 0.0f;
    
    if (AnimationType == TEXT("Idle"))
    {
        BlendWeight = (MovementSpeed < 50.0f) ? 1.0f : 0.0f;
    }
    else if (AnimationType == TEXT("Walk"))
    {
        BlendWeight = (MovementSpeed >= 50.0f && MovementSpeed < 300.0f) ? 1.0f : 0.0f;
    }
    else if (AnimationType == TEXT("Run"))
    {
        BlendWeight = (MovementSpeed >= 300.0f) ? 1.0f : 0.0f;
    }
    
    // Apply smooth transitions
    float BlendSpeed = 5.0f;
    return FMath::FInterpTo(CurrentState, BlendWeight, GetWorld()->GetDeltaSeconds(), BlendSpeed);
}

void UAnimationSystemCore::SmoothTransition(float& CurrentValue, float TargetValue, float DeltaTime, float BlendSpeed)
{
    CurrentValue = FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, BlendSpeed);
}

void UAnimationSystemCore::DrawDebugAnimationState(UWorld* World)
{
    if (!World || !bHighQualityMode)
    {
        return;
    }
    
    // Draw debug information in viewport
    FVector DebugLocation = FVector(0, 0, 200);
    
    // Movement state
    FString DebugText = FString::Printf(TEXT("Speed: %.1f\nDirection: %s\nFatigue: %.2f\nConfidence: %.2f"), 
                                       MovementSpeed, *MovementDirection.ToString(), FatigueLevel, ConfidenceLevel);
    
    DrawDebugString(World, DebugLocation, DebugText, nullptr, FColor::White, 0.0f);
    
    // IK state
    if (bEnableFootIK)
    {
        DrawDebugSphere(World, DebugLocation + FVector(100, 0, 0), 5.0f, 8, FColor::Green);
    }
}

void UAnimationSystemCore::LogAnimationMetrics()
{
    UE_LOG(LogAnimationSystem, Log, TEXT("=== Animation System Metrics ==="));
    UE_LOG(LogAnimationSystem, Log, TEXT("Movement Speed: %f"), MovementSpeed);
    UE_LOG(LogAnimationSystem, Log, TEXT("Fatigue Level: %f"), FatigueLevel);
    UE_LOG(LogAnimationSystem, Log, TEXT("Confidence Level: %f"), ConfidenceLevel);
    UE_LOG(LogAnimationSystem, Log, TEXT("Terrain Roughness: %f"), TerrainRoughness);
    UE_LOG(LogAnimationSystem, Log, TEXT("Slope Angle: %f"), SlopeAngle);
    UE_LOG(LogAnimationSystem, Log, TEXT("Animation LOD: %d"), AnimationLOD);
}

// UCharacterAnimInstance Implementation

UCharacterAnimInstance::UCharacterAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    LeanAmount = 0.0f;
    
    CurrentDatabase = nullptr;
    DatabaseBlendWeight = 0.0f;
    
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    
    ConfidenceModifier = 1.0f;
    FatigueModifier = 1.0f;
    InjuryModifier = 1.0f;
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character references
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
    
    // Create animation system if not assigned
    if (!AnimationSystem)
    {
        AnimationSystem = NewObject<UAnimationSystemCore>(this);
    }
    
    UE_LOG(LogAnimationSystem, Log, TEXT("Character Animation Instance initialized for %s"), 
           OwningCharacter ? *OwningCharacter->GetName() : TEXT("Unknown"));
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwningCharacter || !MovementComponent || !AnimationSystem)
    {
        return;
    }
    
    // Update all animation components
    UpdateMovementValues();
    UpdateMotionMatching(DeltaTime);
    UpdateIKValues(DeltaTime);
    UpdateCharacterTraits(DeltaTime);
}

void UCharacterAnimInstance::UpdateMovementValues()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate movement values
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    
    if (Speed > 0.1f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        Direction = FMath::Atan2(FVector::DotProduct(Velocity, OwningCharacter->GetActorRightVector()), 
                                FVector::DotProduct(Velocity, ForwardVector));
        Direction = FMath::RadiansToDegrees(Direction);
    }
    
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate lean amount for turns
    float TurnRate = OwningCharacter->GetInputAxisValue(TEXT("Turn"));
    LeanAmount = FMath::FInterpTo(LeanAmount, TurnRate * 15.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
    
    // Update animation system
    AnimationSystem->MovementSpeed = Speed;
    AnimationSystem->MovementDirection = Velocity.GetSafeNormal();
    AnimationSystem->TurnRate = TurnRate;
    AnimationSystem->bIsInAir = bIsInAir;
    AnimationSystem->bIsCrouching = bIsCrouching;
}

void UCharacterAnimInstance::UpdateMotionMatching(float DeltaTime)
{
    if (!AnimationSystem)
    {
        return;
    }
    
    // Update motion matching query
    AnimationSystem->UpdateMotionMatchingQuery(DeltaTime);
    
    // Select active database
    CurrentDatabase = AnimationSystem->SelectActiveDatabase();
    
    // Update blend weight
    DatabaseBlendWeight = AnimationSystem->CalculateBlendWeight(TEXT("Current"), DatabaseBlendWeight);
}

void UCharacterAnimInstance::UpdateIKValues(float DeltaTime)
{
    if (!AnimationSystem || !OwningCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = OwningCharacter->GetMesh();
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Update foot IK
    AnimationSystem->UpdateFootIK(SkeletalMesh, DeltaTime);
    
    // Update hand IK
    AnimationSystem->UpdateHandIK(SkeletalMesh, DeltaTime);
}

void UCharacterAnimInstance::UpdateCharacterTraits(float DeltaTime)
{
    if (!AnimationSystem)
    {
        return;
    }
    
    // Update fatigue based on movement intensity
    float MovementIntensity = Speed / 600.0f; // Normalize to 0-1
    AnimationSystem->UpdateFatigueLevel(DeltaTime, MovementIntensity);
    
    // Apply character traits
    float AnimSpeed = 1.0f;
    float PostureOffset = 0.0f;
    AnimationSystem->ApplyCharacterTraits(AnimSpeed, PostureOffset);
    
    // Store modifiers for Blueprint access
    ConfidenceModifier = AnimationSystem->ConfidenceLevel;
    FatigueModifier = 1.0f - AnimationSystem->FatigueLevel;
    InjuryModifier = 1.0f - AnimationSystem->InjuryLevel;
}

// UProtagonistAnimInstance Implementation

UProtagonistAnimInstance::UProtagonistAnimInstance()
{
    bIsObservingDinosaur = false;
    StressLevel = 0.0f;
    CuriosityLevel = 0.7f;
    bIsHoldingTool = false;
    bIsCrafting = false;
}

void UProtagonistAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    UpdateProtagonistBehaviors(DeltaTime);
    UpdateStressResponse(DeltaTime);
    UpdateScientificBehaviors(DeltaTime);
}

void UProtagonistAnimInstance::UpdateProtagonistBehaviors(float DeltaTime)
{
    // Update protagonist-specific animation states
    // This would integrate with gameplay systems to determine observation state, etc.
    
    UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Protagonist behaviors - Stress: %f, Curiosity: %f"), 
           StressLevel, CuriosityLevel);
}

void UProtagonistAnimInstance::UpdateStressResponse(float DeltaTime)
{
    // Stress affects breathing rate, fidgeting, posture
    // Would integrate with threat detection system
    
    if (bIsObservingDinosaur)
    {
        StressLevel = FMath::FInterpTo(StressLevel, 0.8f, DeltaTime, 2.0f);
    }
    else
    {
        StressLevel = FMath::FInterpTo(StressLevel, 0.2f, DeltaTime, 1.0f);
    }
}

void UProtagonistAnimInstance::UpdateScientificBehaviors(float DeltaTime)
{
    // Update scientific behavior states
    // This would trigger specific animation montages for note-taking, tool use, etc.
    
    if (bIsCrafting && ToolUseMontage)
    {
        // Trigger crafting animation
        UE_LOG(LogAnimationSystem, VeryVerbose, TEXT("Triggering tool use animation"));
    }
}