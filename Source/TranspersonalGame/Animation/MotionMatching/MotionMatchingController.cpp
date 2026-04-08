#include "MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

UMotionMatchingController::UMotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the Animation System Manager component
    AnimationManager = GetOwner()->FindComponentByClass<UAnimationSystemManager>();
    
    if (!AnimationManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: No AnimationSystemManager found on %s"), 
               *GetOwner()->GetName());
    }
}

void UMotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateContextBasedOnGameplay();
    ProcessTerrainAdaptation();
    
    ContextTransitionTimer += DeltaTime;
}

void UMotionMatchingController::SetMotionMatchingContext(EMotionMatchingContext NewContext)
{
    if (CurrentContext != NewContext)
    {
        PreviousContext = CurrentContext;
        CurrentContext = NewContext;
        ContextTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Context Changed: %d -> %d"), 
               (int32)PreviousContext, (int32)CurrentContext);
    }
}

UPoseSearchDatabase* UMotionMatchingController::GetCurrentDatabase() const
{
    if (!AnimationManager)
    {
        return nullptr;
    }

    FMotionMatchingDatabaseSet* DatabaseSet = GetCurrentDatabaseSet();
    if (!DatabaseSet)
    {
        return nullptr;
    }

    switch (CurrentContext)
    {
        case EMotionMatchingContext::Locomotion:
            return DatabaseSet->LocomotionDB;
        case EMotionMatchingContext::Interaction:
            return DatabaseSet->InteractionDB;
        case EMotionMatchingContext::Combat:
            return DatabaseSet->CombatDB;
        case EMotionMatchingContext::Stealth:
            return DatabaseSet->StealthDB;
        case EMotionMatchingContext::Exploration:
            return DatabaseSet->ExplorationDB;
        case EMotionMatchingContext::Social:
            return DatabaseSet->SocialDB;
        default:
            return DatabaseSet->LocomotionDB;
    }
}

void UMotionMatchingController::UpdateTerrainAdaptation(const FTerrainAdaptationData& NewTerrainData)
{
    TerrainData = NewTerrainData;
    
    // Log significant terrain changes
    if (FMath::Abs(TerrainData.SlopeAngle) > 15.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Steep terrain detected: %f degrees"), TerrainData.SlopeAngle);
    }
    
    if (TerrainData.bIsUnstableGround)
    {
        UE_LOG(LogTemp, Log, TEXT("Unstable ground detected"));
    }
}

float UMotionMatchingController::GetContextualBlendTime() const
{
    float BaseBlendTime = 0.2f;
    
    // Adjust blend time based on context
    switch (CurrentContext)
    {
        case EMotionMatchingContext::Combat:
            BaseBlendTime = 0.1f; // Faster transitions for combat responsiveness
            break;
        case EMotionMatchingContext::Stealth:
            BaseBlendTime = 0.4f; // Slower, more deliberate transitions
            break;
        case EMotionMatchingContext::Social:
            BaseBlendTime = 0.3f; // Smooth, natural transitions
            break;
    }
    
    // Adjust for emotional state
    if (AnimationManager)
    {
        switch (AnimationManager->AnimationProfile.CurrentEmotionalState)
        {
            case EEmotionalState::Panicked:
                BaseBlendTime *= 0.7f; // Faster, more erratic transitions
                break;
            case EEmotionalState::Fearful:
                BaseBlendTime *= 1.2f; // More hesitant transitions
                break;
            case EEmotionalState::Calm:
                BaseBlendTime *= 1.1f; // Slightly smoother transitions
                break;
        }
    }
    
    // Adjust for terrain
    if (TerrainData.bIsUnstableGround)
    {
        BaseBlendTime *= 1.3f; // More careful transitions on unstable ground
    }
    
    return FMath::Clamp(BaseBlendTime, 0.05f, 1.0f);
}

bool UMotionMatchingController::ShouldUseIKFootPlacement() const
{
    // Always use IK for foot placement except in specific contexts
    if (CurrentContext == EMotionMatchingContext::Combat)
    {
        return false; // Disable IK during combat for performance
    }
    
    // Use IK more aggressively on uneven terrain
    return TerrainData.TerrainRoughness > 0.1f || FMath::Abs(TerrainData.SlopeAngle) > 5.0f;
}

FVector UMotionMatchingController::GetTrajectoryPrediction(float TimeAhead) const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector CurrentVelocity = MovementComp->Velocity;
            FVector CurrentLocation = Character->GetActorLocation();
            
            // Simple linear prediction - could be enhanced with acceleration
            FVector PredictedLocation = CurrentLocation + (CurrentVelocity * TimeAhead);
            
            // Adjust prediction based on emotional state
            if (AnimationManager)
            {
                float PredictionModifier = 1.0f;
                
                switch (AnimationManager->AnimationProfile.CurrentEmotionalState)
                {
                    case EEmotionalState::Fearful:
                        PredictionModifier = 0.8f; // Less predictable movement
                        break;
                    case EEmotionalState::Panicked:
                        PredictionModifier = 0.6f; // Erratic movement
                        break;
                    case EEmotionalState::Alert:
                        PredictionModifier = 1.2f; // More deliberate movement
                        break;
                }
                
                PredictedLocation = FMath::Lerp(CurrentLocation, PredictedLocation, PredictionModifier);
            }
            
            return PredictedLocation;
        }
    }
    
    return FVector::ZeroVector;
}

void UMotionMatchingController::UpdateContextBasedOnGameplay()
{
    if (!AnimationManager)
    {
        return;
    }
    
    // Auto-detect context based on gameplay state
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    float CurrentSpeed = MovementComp->Velocity.Size();
    EMotionMatchingContext NewContext = CurrentContext;
    
    // Context switching logic
    if (CurrentSpeed < 50.0f)
    {
        // Standing still or moving very slowly
        if (AnimationManager->AnimationProfile.CurrentEmotionalState == EEmotionalState::Fearful ||
            AnimationManager->AnimationProfile.CurrentEmotionalState == EEmotionalState::Alert)
        {
            NewContext = EMotionMatchingContext::Stealth;
        }
        else
        {
            NewContext = EMotionMatchingContext::Exploration;
        }
    }
    else if (CurrentSpeed > 300.0f)
    {
        // Fast movement - likely fleeing or urgent
        NewContext = EMotionMatchingContext::Locomotion;
    }
    else
    {
        // Normal movement speed
        NewContext = EMotionMatchingContext::Locomotion;
    }
    
    // Apply context change with some hysteresis to prevent rapid switching
    if (NewContext != CurrentContext && ContextTransitionTimer > 1.0f)
    {
        SetMotionMatchingContext(NewContext);
    }
}

void UMotionMatchingController::ProcessTerrainAdaptation()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    // Perform ground trace to get terrain information
    FVector StartLocation = Character->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        FTerrainAdaptationData NewTerrainData;
        NewTerrainData.SurfaceNormal = HitResult.Normal;
        
        // Calculate slope angle
        float DotProduct = FVector::DotProduct(HitResult.Normal, FVector::UpVector);
        NewTerrainData.SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Estimate terrain roughness based on normal variation
        // This is simplified - in a real implementation you might sample multiple points
        NewTerrainData.TerrainRoughness = FMath::Clamp(1.0f - DotProduct, 0.0f, 1.0f);
        
        // Check for unstable ground (could be based on material type, physics, etc.)
        NewTerrainData.bIsUnstableGround = NewTerrainData.SlopeAngle > 30.0f;
        
        UpdateTerrainAdaptation(NewTerrainData);
    }
}

FMotionMatchingDatabaseSet* UMotionMatchingController::GetCurrentDatabaseSet() const
{
    if (!AnimationManager)
    {
        return nullptr;
    }
    
    ECharacterArchetype CurrentArchetype = AnimationManager->AnimationProfile.Archetype;
    return const_cast<FMotionMatchingDatabaseSet*>(DatabasesByArchetype.Find(CurrentArchetype));
}