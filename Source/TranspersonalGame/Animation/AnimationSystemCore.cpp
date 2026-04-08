#include "AnimationSystemCore.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "IKRigDefinition.h"
#include "Engine/World.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    CurrentMovementState = ECharacterMovementState::Idle;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    TerrainAngle = 0.0f;
    TerrainRoughness = 0.0f;
    DomesticationLevel = 0.0f;
}

void UAnimationSystemCore::InitializeMotionMatchingSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Animation System: Initializing Motion Matching System"));
    
    // Load default motion matching databases
    if (!HumanLocomotionDatabase)
    {
        HumanLocomotionDatabase = LoadObject<UPoseSearchDatabase>(nullptr, 
            TEXT("/Game/Animation/MotionMatching/DB_HumanLocomotion"));
    }
    
    if (!DinosaurBehaviorDatabase)
    {
        DinosaurBehaviorDatabase = LoadObject<UPoseSearchDatabase>(nullptr, 
            TEXT("/Game/Animation/MotionMatching/DB_DinosaurBehavior"));
    }
    
    if (!DefaultMotionSchema)
    {
        DefaultMotionSchema = LoadObject<UPoseSearchSchema>(nullptr, 
            TEXT("/Game/Animation/MotionMatching/Schema_Default"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Motion Matching System Initialized"));
}

void UAnimationSystemCore::SetCharacterMovementState(ECharacterMovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        ECharacterMovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Animation System: Character movement state changed from %d to %d"), 
            (int32)PreviousState, (int32)NewState);
        
        // Trigger animation blend based on personality
        ApplyMovementStateTransition(PreviousState, NewState);
    }
}

void UAnimationSystemCore::SetDinosaurBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Animation System: Dinosaur behavior state changed from %d to %d"), 
            (int32)PreviousState, (int32)NewState);
        
        // Handle domestication state transitions
        if (NewState == EDinosaurBehaviorState::Domesticated_Calm || 
            NewState == EDinosaurBehaviorState::Domesticated_Playful)
        {
            ApplyDomesticationAnimationBlend();
        }
    }
}

void UAnimationSystemCore::ApplyAnimationPersonality(const FAnimationPersonality& Personality)
{
    CurrentPersonality = Personality;
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Applied personality - Nervousness: %f, Confidence: %f"), 
        Personality.Nervousness, Personality.Confidence);
    
    // Modify animation parameters based on personality
    ModifyAnimationTimings();
    AdjustMovementVariations();
    ApplyPhysicalCharacteristics();
}

void UAnimationSystemCore::UpdateTerrainAdaptation(float SurfaceAngle, float SurfaceRoughness)
{
    TerrainAngle = SurfaceAngle;
    TerrainRoughness = SurfaceRoughness;
    
    // Trigger IK adjustments for foot placement
    if (HumanIKRig)
    {
        ApplyTerrainIKAdjustments();
    }
}

void UAnimationSystemCore::UpdateDomesticationLevel(float DomesticationProgress)
{
    float PreviousLevel = DomesticationLevel;
    DomesticationLevel = FMath::Clamp(DomesticationProgress, 0.0f, 1.0f);
    
    if (FMath::Abs(DomesticationLevel - PreviousLevel) > 0.1f)
    {
        UE_LOG(LogTemp, Log, TEXT("Animation System: Domestication level updated to %f"), DomesticationLevel);
        ApplyDomesticationAnimationChanges();
    }
}

void UAnimationSystemCore::ApplyMovementStateTransition(ECharacterMovementState From, ECharacterMovementState To)
{
    // Calculate blend time based on personality and state transition
    float BlendTime = CalculatePersonalityBlendTime(From, To);
    
    // Apply nervous fidgeting for high nervousness characters
    if (CurrentPersonality.Nervousness > 0.7f && To == ECharacterMovementState::Idle)
    {
        TriggerNervousFidgeting();
    }
    
    // Apply confident posture adjustments
    if (CurrentPersonality.Confidence > 0.7f)
    {
        ApplyConfidentPosture();
    }
}

float UAnimationSystemCore::CalculatePersonalityBlendTime(ECharacterMovementState From, ECharacterMovementState To)
{
    float BaseBlendTime = 0.3f;
    
    // Nervous characters have quicker, more abrupt transitions
    if (CurrentPersonality.Nervousness > 0.6f)
    {
        BaseBlendTime *= 0.7f;
    }
    
    // Confident characters have smoother, more deliberate transitions
    if (CurrentPersonality.Confidence > 0.6f)
    {
        BaseBlendTime *= 1.3f;
    }
    
    return BaseBlendTime;
}

void UAnimationSystemCore::ModifyAnimationTimings()
{
    // Apply speed variations based on personality
    float SpeedModifier = 1.0f;
    
    if (CurrentPersonality.Nervousness > 0.6f)
    {
        SpeedModifier *= (1.0f + CurrentPersonality.Nervousness * 0.3f); // Faster, more jittery
    }
    
    if (CurrentPersonality.Confidence > 0.6f)
    {
        SpeedModifier *= (1.0f - CurrentPersonality.Confidence * 0.2f); // Slower, more deliberate
    }
    
    CurrentPersonality.AnimationSpeedVariation = SpeedModifier;
}

void UAnimationSystemCore::AdjustMovementVariations()
{
    // Create subtle movement variations that make each character unique
    float MovementVariation = FMath::RandRange(0.95f, 1.05f);
    
    // Apply personality-based movement modifications
    if (CurrentPersonality.Aggressiveness > 0.7f)
    {
        MovementVariation *= 1.1f; // More forceful movements
    }
    
    if (CurrentPersonality.SocialTendency > 0.7f)
    {
        // More expressive, open body language
        ApplyOpenBodyLanguage();
    }
    else if (CurrentPersonality.SocialTendency < 0.3f)
    {
        // More closed, defensive body language
        ApplyClosedBodyLanguage();
    }
}

void UAnimationSystemCore::ApplyPhysicalCharacteristics()
{
    // Apply limp or injury effects
    if (CurrentPersonality.LimpSeverity > 0.0f)
    {
        ApplyLimpAnimation();
    }
    
    // Apply unique movement signature based on identifier
    ApplyUniqueMovementSignature();
}

void UAnimationSystemCore::ApplyTerrainIKAdjustments()
{
    // Calculate foot IK adjustments based on terrain
    float IKAdjustmentStrength = FMath::Clamp(TerrainRoughness * 2.0f, 0.0f, 1.0f);
    
    // Apply slope compensation
    float SlopeCompensation = FMath::Sin(FMath::DegreesToRadians(TerrainAngle)) * 0.5f;
    
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Applying terrain IK - Roughness: %f, Angle: %f"), 
        TerrainRoughness, TerrainAngle);
}

void UAnimationSystemCore::ApplyDomesticationAnimationBlend()
{
    // Blend from wild to domesticated animations
    float WildWeight = 1.0f - DomesticationLevel;
    float DomesticatedWeight = DomesticationLevel;
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Applying domestication blend - Wild: %f, Domestic: %f"), 
        WildWeight, DomesticatedWeight);
}

void UAnimationSystemCore::ApplyDomesticationAnimationChanges()
{
    // Gradually change animation sets as domestication progresses
    if (DomesticationLevel > 0.5f)
    {
        // Start showing more trusting animations
        EnableTrustingAnimations();
    }
    
    if (DomesticationLevel > 0.8f)
    {
        // Enable playful interactions
        EnablePlayfulAnimations();
    }
}

// Helper methods for animation application
void UAnimationSystemCore::TriggerNervousFidgeting()
{
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Triggering nervous fidgeting"));
}

void UAnimationSystemCore::ApplyConfidentPosture()
{
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Applying confident posture"));
}

void UAnimationSystemCore::ApplyOpenBodyLanguage()
{
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Applying open body language"));
}

void UAnimationSystemCore::ApplyClosedBodyLanguage()
{
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Applying closed body language"));
}

void UAnimationSystemCore::ApplyLimpAnimation()
{
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Applying limp animation with severity %f"), 
        CurrentPersonality.LimpSeverity);
}

void UAnimationSystemCore::ApplyUniqueMovementSignature()
{
    UE_LOG(LogTemp, Verbose, TEXT("Animation System: Applying unique movement signature for %s"), 
        *CurrentPersonality.UniqueIdentifier);
}

void UAnimationSystemCore::EnableTrustingAnimations()
{
    UE_LOG(LogTemp, Log, TEXT("Animation System: Enabling trusting animations"));
}

void UAnimationSystemCore::EnablePlayfulAnimations()
{
    UE_LOG(LogTemp, Log, TEXT("Animation System: Enabling playful animations"));
}