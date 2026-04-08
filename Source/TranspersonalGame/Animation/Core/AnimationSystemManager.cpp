#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentMovementState = ECharacterMovementState::Idle;
    CurrentEmotionalState = EEmotionalState::Calm;
    TargetEmotionalState = EEmotionalState::Calm;
    EmotionalBlendTimer = 0.0f;
    MovementIntensity = 0.0f;
    ActiveAnimationCount = 0;
    
    // Initialize emotional weights
    EmotionalBlendWeights.Add(EEmotionalState::Calm, 1.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Alert, 0.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Fearful, 0.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Panicked, 0.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Exhausted, 0.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Injured, 0.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Confident, 0.0f);
    EmotionalBlendWeights.Add(EEmotionalState::Aggressive, 0.0f);
    
    CurrentTerrainLevel = ETerrainAdaptationLevel::None;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogAnimation, Log, TEXT("Animation System Manager initialized for %s"), 
           *GetOwner()->GetName());
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, 
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmotionalBlending(DeltaTime);
    UpdatePerformanceMetrics();
    
    if (bUseAnimationLOD)
    {
        OptimizeAnimationLOD();
    }
}

void UAnimationSystemManager::SetCharacterMovementState(ECharacterMovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        ECharacterMovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        
        // Update movement intensity based on state
        switch (NewState)
        {
            case ECharacterMovementState::Idle:
                MovementIntensity = 0.0f;
                break;
            case ECharacterMovementState::Walking:
                MovementIntensity = 0.3f;
                break;
            case ECharacterMovementState::Running:
                MovementIntensity = 0.8f;
                break;
            case ECharacterMovementState::Crouching:
                MovementIntensity = 0.1f;
                break;
            case ECharacterMovementState::Sneaking:
                MovementIntensity = 0.2f;
                break;
            case ECharacterMovementState::Climbing:
                MovementIntensity = 0.6f;
                break;
            case ECharacterMovementState::Swimming:
                MovementIntensity = 0.7f;
                break;
            case ECharacterMovementState::Falling:
                MovementIntensity = 1.0f;
                break;
            default:
                MovementIntensity = 0.5f;
                break;
        }
        
        UE_LOG(LogAnimation, Log, TEXT("Movement state changed from %d to %d, intensity: %f"), 
               (int32)PreviousState, (int32)NewState, MovementIntensity);
    }
}

void UAnimationSystemManager::SetEmotionalState(EEmotionalState NewState, float BlendTime)
{
    if (TargetEmotionalState != NewState)
    {
        TargetEmotionalState = NewState;
        EmotionalBlendTimer = 0.0f;
        EmotionalTransitionSpeed = 1.0f / FMath::Max(BlendTime, 0.1f);
        
        UE_LOG(LogAnimation, Log, TEXT("Emotional state transitioning to %d over %f seconds"), 
               (int32)NewState, BlendTime);
    }
}

void UAnimationSystemManager::UpdateTerrainAdaptation(ETerrainAdaptationLevel NewLevel)
{
    if (CurrentTerrainLevel != NewLevel)
    {
        CurrentTerrainLevel = NewLevel;
        
        // Adjust IK strength based on terrain complexity
        switch (NewLevel)
        {
            case ETerrainAdaptationLevel::None:
                TerrainAdaptationStrength = 0.0f;
                break;
            case ETerrainAdaptationLevel::Light:
                TerrainAdaptationStrength = 0.3f;
                break;
            case ETerrainAdaptationLevel::Medium:
                TerrainAdaptationStrength = 0.6f;
                break;
            case ETerrainAdaptationLevel::Heavy:
                TerrainAdaptationStrength = 0.9f;
                break;
            case ETerrainAdaptationLevel::Extreme:
                TerrainAdaptationStrength = 1.0f;
                break;
        }
        
        UE_LOG(LogAnimation, Log, TEXT("Terrain adaptation level changed to %d, strength: %f"), 
               (int32)NewLevel, TerrainAdaptationStrength);
    }
}

UPoseSearchDatabase* UAnimationSystemManager::GetCurrentMovementDatabase() const
{
    if (MovementDatabases.Contains(CurrentMovementState))
    {
        return MovementDatabases[CurrentMovementState].Get();
    }
    
    return nullptr;
}

float UAnimationSystemManager::GetEmotionalStateWeight(EEmotionalState State) const
{
    if (EmotionalBlendWeights.Contains(State))
    {
        return EmotionalBlendWeights[State];
    }
    
    return 0.0f;
}

bool UAnimationSystemManager::IsInFearState() const
{
    return (CurrentEmotionalState == EEmotionalState::Fearful || 
            CurrentEmotionalState == EEmotionalState::Panicked ||
            TargetEmotionalState == EEmotionalState::Fearful ||
            TargetEmotionalState == EEmotionalState::Panicked);
}

float UAnimationSystemManager::GetMovementIntensity() const
{
    return MovementIntensity;
}

void UAnimationSystemManager::UpdateEmotionalBlending(float DeltaTime)
{
    if (CurrentEmotionalState != TargetEmotionalState)
    {
        EmotionalBlendTimer += DeltaTime * EmotionalTransitionSpeed;
        float BlendAlpha = FMath::Clamp(EmotionalBlendTimer, 0.0f, 1.0f);
        
        if (BlendAlpha >= 1.0f)
        {
            // Transition complete
            CurrentEmotionalState = TargetEmotionalState;
            
            // Reset all weights
            for (auto& WeightPair : EmotionalBlendWeights)
            {
                WeightPair.Value = 0.0f;
            }
            
            // Set target state to full weight
            EmotionalBlendWeights[CurrentEmotionalState] = 1.0f;
        }
        else
        {
            // Blend between current and target states
            for (auto& WeightPair : EmotionalBlendWeights)
            {
                if (WeightPair.Key == CurrentEmotionalState)
                {
                    WeightPair.Value = 1.0f - BlendAlpha;
                }
                else if (WeightPair.Key == TargetEmotionalState)
                {
                    WeightPair.Value = BlendAlpha;
                }
                else
                {
                    WeightPair.Value = 0.0f;
                }
            }
        }
    }
}

void UAnimationSystemManager::UpdatePerformanceMetrics()
{
    // Count nearby animated actors for performance optimization
    NearbyAnimatedActors.Empty();
    
    if (UWorld* World = GetWorld())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner())
            {
                if (USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
                {
                    float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                    if (Distance <= LODDistanceThreshold)
                    {
                        NearbyAnimatedActors.Add(Actor);
                    }
                }
            }
        }
    }
    
    ActiveAnimationCount = NearbyAnimatedActors.Num();
}

void UAnimationSystemManager::OptimizeAnimationLOD()
{
    // Implement LOD optimization based on distance and performance
    if (ActiveAnimationCount > MaxSimultaneousAnimations)
    {
        // Sort by distance and disable animations for furthest actors
        NearbyAnimatedActors.Sort([this](const AActor& A, const AActor& B)
        {
            FVector OwnerLoc = GetOwner()->GetActorLocation();
            float DistA = FVector::DistSquared(OwnerLoc, A.GetActorLocation());
            float DistB = FVector::DistSquared(OwnerLoc, B.GetActorLocation());
            return DistA < DistB;
        });
        
        // Disable animations for actors beyond the limit
        for (int32 i = MaxSimultaneousAnimations; i < NearbyAnimatedActors.Num(); ++i)
        {
            if (USkeletalMeshComponent* SkelMesh = NearbyAnimatedActors[i]->FindComponentByClass<USkeletalMeshComponent>())
            {
                SkelMesh->bPauseAnims = true;
            }
        }
    }
}