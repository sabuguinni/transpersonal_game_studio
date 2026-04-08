#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    CurrentConsciousnessState = EConsciousnessState::Dormant;
    StateTransitionProgress = 0.0f;
    EnergyLevel = 0.5f;
    LuminosityIntensity = 0.0f;
    AuraScale = FVector(1.0f, 1.0f, 1.0f);
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsMoving = false;
    
    bIsMeditating = false;
    bIsTransforming = false;
    bIsChannelingEnergy = false;
    bIsInTrance = false;
    
    DormantBlend = 1.0f;
    AwakeningBlend = 0.0f;
    ExpandingBlend = 0.0f;
    TranscendentBlend = 0.0f;
    UnityBlend = 0.0f;
    
    SpineOffset = 0.0f;
    HeadOffset = 0.0f;
    ArmOffset = 0.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        ConsciousnessComponent = OwningPawn->FindComponentByClass<UConsciousnessAnimationComponent>();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningPawn) return;
    
    UpdateMovementProperties();
    UpdateConsciousnessProperties();
    UpdateSpiritualStates();
    UpdateAnimationBlends();
    UpdatePoseOffsets();
}

void UTranspersonalAnimInstance::UpdateMovementProperties()
{
    if (ACharacter* Character = Cast<ACharacter>(OwningPawn))
    {
        FVector Velocity = Character->GetVelocity();
        Speed = Velocity.Size2D();
        bIsMoving = Speed > 3.0f;
        
        if (Speed > 0.0f)
        {
            FVector Forward = Character->GetActorForwardVector();
            FVector VelocityNormalized = Velocity.GetSafeNormal2D();
            Direction = FMath::Atan2(FVector::CrossProduct(Forward, VelocityNormalized).Z, 
                                   FVector::DotProduct(Forward, VelocityNormalized));
        }
        
        if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
        {
            bIsInAir = MovementComponent->IsFalling();
        }
    }
}

void UTranspersonalAnimInstance::UpdateConsciousnessProperties()
{
    if (ConsciousnessComponent)
    {
        FConsciousnessAnimationData AnimData = ConsciousnessComponent->GetAnimationData();
        
        CurrentConsciousnessState = AnimData.CurrentState;
        StateTransitionProgress = AnimData.StateTransitionProgress;
        EnergyLevel = AnimData.EnergyLevel;
        LuminosityIntensity = AnimData.LuminosityIntensity;
        AuraScale = AnimData.AuraScale;
    }
}

void UTranspersonalAnimInstance::UpdateSpiritualStates()
{
    // Update spiritual states based on consciousness level and movement
    bIsMeditating = !bIsMoving && EnergyLevel > 0.7f;
    bIsTransforming = StateTransitionProgress > 0.1f;
    bIsChannelingEnergy = LuminosityIntensity > 0.5f && !bIsMoving;
    bIsInTrance = CurrentConsciousnessState == EConsciousnessState::Transcendent || 
                  CurrentConsciousnessState == EConsciousnessState::Unity;
}

void UTranspersonalAnimInstance::UpdateAnimationBlends()
{
    // Reset all blends
    DormantBlend = 0.0f;
    AwakeningBlend = 0.0f;
    ExpandingBlend = 0.0f;
    TranscendentBlend = 0.0f;
    UnityBlend = 0.0f;
    
    // Set primary blend based on current state
    switch (CurrentConsciousnessState)
    {
        case EConsciousnessState::Dormant:
            DormantBlend = 1.0f - StateTransitionProgress;
            break;
        case EConsciousnessState::Awakening:
            AwakeningBlend = 1.0f - StateTransitionProgress;
            break;
        case EConsciousnessState::Expanding:
            ExpandingBlend = 1.0f - StateTransitionProgress;
            break;
        case EConsciousnessState::Transcendent:
            TranscendentBlend = 1.0f - StateTransitionProgress;
            break;
        case EConsciousnessState::Unity:
            UnityBlend = 1.0f - StateTransitionProgress;
            break;
    }
    
    // Add transition blend if transitioning
    if (StateTransitionProgress > 0.0f)
    {
        // This would need to know the target state to blend properly
        // For now, we'll use a simple approach
        float TransitionBlend = StateTransitionProgress;
        
        switch (CurrentConsciousnessState)
        {
            case EConsciousnessState::Dormant:
                AwakeningBlend = FMath::Max(AwakeningBlend, TransitionBlend);
                break;
            case EConsciousnessState::Awakening:
                ExpandingBlend = FMath::Max(ExpandingBlend, TransitionBlend);
                break;
            case EConsciousnessState::Expanding:
                TranscendentBlend = FMath::Max(TranscendentBlend, TransitionBlend);
                break;
            case EConsciousnessState::Transcendent:
                UnityBlend = FMath::Max(UnityBlend, TransitionBlend);
                break;
        }
    }
}

void UTranspersonalAnimInstance::UpdatePoseOffsets()
{
    // Calculate pose offsets based on consciousness state and energy level
    float BaseIntensity = EnergyLevel * LuminosityIntensity;
    
    // Spine straightens and elongates with higher consciousness
    SpineOffset = BaseIntensity * 0.1f; // Subtle spine extension
    
    // Head tilts slightly upward in higher states
    HeadOffset = BaseIntensity * 0.05f;
    
    // Arms become more open and relaxed
    ArmOffset = BaseIntensity * 0.2f;
    
    // Add subtle breathing effect based on consciousness state
    float BreathingCycle = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f);
    float BreathingIntensity = 0.02f + (BaseIntensity * 0.03f);
    
    SpineOffset += BreathingCycle * BreathingIntensity;
    
    // Special pose adjustments for specific states
    switch (CurrentConsciousnessState)
    {
        case EConsciousnessState::Dormant:
            // Slightly hunched, less open posture
            SpineOffset -= 0.05f;
            ArmOffset -= 0.1f;
            break;
            
        case EConsciousnessState::Awakening:
            // Beginning to straighten
            HeadOffset += 0.02f;
            break;
            
        case EConsciousnessState::Expanding:
            // More open, expansive posture
            ArmOffset += 0.1f;
            break;
            
        case EConsciousnessState::Transcendent:
            // Elevated, ethereal posture
            SpineOffset += 0.05f;
            HeadOffset += 0.03f;
            break;
            
        case EConsciousnessState::Unity:
            // Perfect balance and alignment
            SpineOffset += 0.08f;
            HeadOffset += 0.05f;
            ArmOffset += 0.15f;
            break;
    }
}

float UTranspersonalAnimInstance::CalculateBlendWeight(EConsciousnessState TargetState, float TransitionProgress)
{
    if (CurrentConsciousnessState == TargetState)
    {
        return 1.0f - TransitionProgress;
    }
    
    // Check if this is the target state for transition
    int32 CurrentStateValue = (int32)CurrentConsciousnessState;
    int32 TargetStateValue = (int32)TargetState;
    
    if (FMath::Abs(CurrentStateValue - TargetStateValue) == 1)
    {
        return TransitionProgress;
    }
    
    return 0.0f;
}