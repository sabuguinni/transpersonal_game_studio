#include "AnimationSystemArchitecture.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UAnimationStateManager::UAnimationStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UAnimationStateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default state
    CurrentState.EmotionalState = ECharacterEmotionalState::Calm;
    CurrentState.MovementContext = EMovementContext::Normal;
    CurrentState.StressLevel = 0.0f;
    CurrentState.FatigueLevel = 0.0f;
    CurrentState.bIsBeingHunted = false;
    CurrentState.bCanSeeThreats = false;
}

void UAnimationStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateStressDecay(DeltaTime);
    UpdateFatigueAccumulation(DeltaTime);
    
    // Auto-adjust emotional state based on stress levels
    if (CurrentState.StressLevel > 0.8f)
    {
        UpdateEmotionalState(ECharacterEmotionalState::Terrified);
    }
    else if (CurrentState.StressLevel > 0.6f)
    {
        UpdateEmotionalState(ECharacterEmotionalState::Afraid);
    }
    else if (CurrentState.StressLevel > 0.3f)
    {
        UpdateEmotionalState(ECharacterEmotionalState::Nervous);
    }
    else if (CurrentState.FatigueLevel > 0.7f)
    {
        UpdateEmotionalState(ECharacterEmotionalState::Exhausted);
    }
    else
    {
        UpdateEmotionalState(ECharacterEmotionalState::Calm);
    }
}

void UAnimationStateManager::UpdateEmotionalState(ECharacterEmotionalState NewState)
{
    if (CurrentState.EmotionalState != NewState)
    {
        CurrentState.EmotionalState = NewState;
        
        // Broadcast state change to animation blueprint
        if (AActor* Owner = GetOwner())
        {
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
                {
                    if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
                    {
                        // Trigger animation blueprint event
                        // This will be handled in the Animation Blueprint
                    }
                }
            }
        }
    }
}

void UAnimationStateManager::UpdateMovementContext(EMovementContext NewContext)
{
    CurrentState.MovementContext = NewContext;
}

void UAnimationStateManager::UpdateStressLevel(float NewStressLevel)
{
    CurrentState.StressLevel = FMath::Clamp(NewStressLevel, 0.0f, 1.0f);
}

void UAnimationStateManager::OnThreatDetected(const FVector& ThreatLocation)
{
    CurrentState.bIsBeingHunted = true;
    CurrentState.bCanSeeThreats = true;
    CurrentState.LastKnownThreatDirection = ThreatLocation - GetOwner()->GetActorLocation();
    CurrentState.LastKnownThreatDirection.Normalize();
    
    // Immediate stress spike
    UpdateStressLevel(FMath::Min(CurrentState.StressLevel + 0.4f, 1.0f));
}

void UAnimationStateManager::OnThreatLost()
{
    CurrentState.bCanSeeThreats = false;
    // Keep bIsBeingHunted true for a while - stress will decay naturally
}

void UAnimationStateManager::UpdateStressDecay(float DeltaTime)
{
    if (!CurrentState.bIsBeingHunted && CurrentState.StressLevel > 0.0f)
    {
        CurrentState.StressLevel = FMath::Max(0.0f, CurrentState.StressLevel - (StressDecayRate * DeltaTime));
    }
}

void UAnimationStateManager::UpdateFatigueAccumulation(float DeltaTime)
{
    // Fatigue accumulates during movement and stress
    if (GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            float MovementSpeed = Character->GetVelocity().Size();
            if (MovementSpeed > 100.0f) // Character is moving
            {
                float FatigueRate = FatigueAccumulationRate;
                
                // Increase fatigue rate when stressed or running
                if (CurrentState.StressLevel > 0.5f)
                {
                    FatigueRate *= (1.0f + CurrentState.StressLevel);
                }
                
                if (MovementSpeed > 400.0f) // Running
                {
                    FatigueRate *= 2.0f;
                }
                
                CurrentState.FatigueLevel = FMath::Min(1.0f, CurrentState.FatigueLevel + (FatigueRate * DeltaTime));
            }
            else
            {
                // Recover fatigue when resting
                CurrentState.FatigueLevel = FMath::Max(0.0f, CurrentState.FatigueLevel - (0.2f * DeltaTime));
            }
        }
    }
}