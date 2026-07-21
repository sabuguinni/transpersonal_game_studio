#include "Anim_FacialAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

UAnim_FacialAnimationController::UAnim_FacialAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize default values
    CurrentExpression = EAnim_FacialExpression::Neutral;
    CurrentIntensity = EAnim_EmotionalIntensity::Moderate;
    CurrentExpressionWeight = 0.0f;
    ExpressionBlendSpeed = 2.0f;
    DefaultExpressionDuration = 2.0f;

    // Eye animation defaults
    CurrentEyeDirection = FVector::ForwardVector;
    BlinkTimer = 0.0f;
    BlinkInterval = 4.0f; // Blink every 4 seconds
    bIsBlinking = false;

    // Speech animation defaults
    bIsSpeaking = false;
    SpeechAnimationTimer = 0.0f;

    // Emotional sequence defaults
    CurrentSequenceIndex = 0;
    SequenceTimer = 0.0f;
    bPlayingEmotionalSequence = false;

    TargetMeshComponent = nullptr;
}

void UAnim_FacialAnimationController::BeginPlay()
{
    Super::BeginPlay();

    // Find the skeletal mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        TargetMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!TargetMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_FacialAnimationController: No SkeletalMeshComponent found on owner %s"), *Owner->GetName());
        }
    }

    // Initialize blend shape mappings
    InitializeBlendShapeMappings();

    // Set initial neutral expression
    SetFacialExpression(EAnim_FacialExpression::Neutral, EAnim_EmotionalIntensity::Subtle, 1.0f);
}

void UAnim_FacialAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!TargetMeshComponent)
    {
        return;
    }

    // Update all facial animation systems
    UpdateFacialAnimation(DeltaTime);
    UpdateEyeAnimation(DeltaTime);
    UpdateSpeechAnimation(DeltaTime);
    UpdateEmotionalSequence(DeltaTime);
}

void UAnim_FacialAnimationController::SetFacialExpression(EAnim_FacialExpression Expression, EAnim_EmotionalIntensity Intensity, float Duration)
{
    CurrentExpression = Expression;
    CurrentIntensity = Intensity;
    DefaultExpressionDuration = Duration;

    // Apply the expression immediately
    if (ExpressionBlendShapes.Contains(Expression))
    {
        const TArray<FAnim_FacialBlendTarget>& BlendTargets = ExpressionBlendShapes[Expression];
        float IntensityMultiplier = CalculateIntensityMultiplier(Intensity);
        ApplyBlendShapeWeights(BlendTargets, IntensityMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Set facial expression to %d with intensity %d"), 
           (int32)Expression, (int32)Intensity);
}

void UAnim_FacialAnimationController::BlendToExpression(EAnim_FacialExpression Expression, float BlendTime)
{
    // Store the target expression for gradual blending
    CurrentExpression = Expression;
    ExpressionBlendSpeed = 1.0f / BlendTime;

    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Blending to expression %d over %f seconds"), 
           (int32)Expression, BlendTime);
}

void UAnim_FacialAnimationController::SetBlendShapeWeight(const FString& BlendShapeName, float Weight)
{
    if (!TargetMeshComponent)
    {
        return;
    }

    // Clamp weight to valid range
    Weight = FMath::Clamp(Weight, 0.0f, 1.0f);

    // Apply the blend shape weight
    // Note: This would typically use UE5's morph target system
    // For now, we'll log the operation
    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Set blend shape '%s' weight to %f"), 
           *BlendShapeName, Weight);
}

void UAnim_FacialAnimationController::PlayEmotionalSequence(const TArray<FAnim_EmotionalState>& EmotionalSequence)
{
    if (EmotionalSequence.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_FacialAnimationController: Empty emotional sequence provided"));
        return;
    }

    CurrentEmotionalSequence = EmotionalSequence;
    CurrentSequenceIndex = 0;
    SequenceTimer = 0.0f;
    bPlayingEmotionalSequence = true;

    // Start with the first emotional state
    const FAnim_EmotionalState& FirstState = EmotionalSequence[0];
    SetFacialExpression(FirstState.PrimaryExpression, FirstState.Intensity, FirstState.Duration);

    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Started emotional sequence with %d states"), 
           EmotionalSequence.Num());
}

void UAnim_FacialAnimationController::StopAllFacialAnimations()
{
    bPlayingEmotionalSequence = false;
    bIsSpeaking = false;
    CurrentEmotionalSequence.Empty();
    
    // Return to neutral expression
    BlendToNeutralExpression(1.0f);

    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Stopped all facial animations"));
}

void UAnim_FacialAnimationController::ExpressPain(float Intensity)
{
    EAnim_EmotionalIntensity IntensityLevel = EAnim_EmotionalIntensity::Moderate;
    if (Intensity > 0.8f) IntensityLevel = EAnim_EmotionalIntensity::Extreme;
    else if (Intensity > 0.6f) IntensityLevel = EAnim_EmotionalIntensity::Strong;
    else if (Intensity < 0.3f) IntensityLevel = EAnim_EmotionalIntensity::Subtle;

    SetFacialExpression(EAnim_FacialExpression::Pain, IntensityLevel, 1.5f);
}

void UAnim_FacialAnimationController::ExpressFear(float Intensity)
{
    EAnim_EmotionalIntensity IntensityLevel = EAnim_EmotionalIntensity::Strong;
    if (Intensity > 0.9f) IntensityLevel = EAnim_EmotionalIntensity::Extreme;
    else if (Intensity < 0.5f) IntensityLevel = EAnim_EmotionalIntensity::Moderate;
    else if (Intensity < 0.3f) IntensityLevel = EAnim_EmotionalIntensity::Subtle;

    SetFacialExpression(EAnim_FacialExpression::Fear, IntensityLevel, 2.0f);
}

void UAnim_FacialAnimationController::ExpressExhaustion(float Intensity)
{
    EAnim_EmotionalIntensity IntensityLevel = EAnim_EmotionalIntensity::Moderate;
    if (Intensity > 0.7f) IntensityLevel = EAnim_EmotionalIntensity::Strong;
    else if (Intensity < 0.4f) IntensityLevel = EAnim_EmotionalIntensity::Subtle;

    SetFacialExpression(EAnim_FacialExpression::Exhaustion, IntensityLevel, 3.0f);
}

void UAnim_FacialAnimationController::ExpressConcentration(float Intensity)
{
    EAnim_EmotionalIntensity IntensityLevel = EAnim_EmotionalIntensity::Moderate;
    if (Intensity > 0.6f) IntensityLevel = EAnim_EmotionalIntensity::Strong;
    else if (Intensity < 0.3f) IntensityLevel = EAnim_EmotionalIntensity::Subtle;

    SetFacialExpression(EAnim_FacialExpression::Concentration, IntensityLevel, 2.5f);
}

void UAnim_FacialAnimationController::ExpressRelief(float Intensity)
{
    EAnim_EmotionalIntensity IntensityLevel = EAnim_EmotionalIntensity::Moderate;
    if (Intensity > 0.6f) IntensityLevel = EAnim_EmotionalIntensity::Strong;
    else if (Intensity < 0.3f) IntensityLevel = EAnim_EmotionalIntensity::Subtle;

    SetFacialExpression(EAnim_FacialExpression::Happy, IntensityLevel, 2.0f);
}

void UAnim_FacialAnimationController::SetEyeDirection(FVector LookDirection)
{
    CurrentEyeDirection = LookDirection.GetSafeNormal();
    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Set eye direction to %s"), 
           *CurrentEyeDirection.ToString());
}

void UAnim_FacialAnimationController::TriggerBlink(float BlinkSpeed)
{
    if (!bIsBlinking)
    {
        bIsBlinking = true;
        BlinkTimer = 0.0f;
        
        // Reset blink timer based on speed
        BlinkInterval = 0.2f / BlinkSpeed; // Quick blink
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Triggered blink with speed %f"), BlinkSpeed);
    }
}

void UAnim_FacialAnimationController::SetBlinkRate(float BlinksPerMinute)
{
    BlinkInterval = 60.0f / FMath::Max(1.0f, BlinksPerMinute);
    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Set blink rate to %f blinks per minute"), BlinksPerMinute);
}

void UAnim_FacialAnimationController::StartSpeechAnimation(USoundWave* AudioClip)
{
    if (!AudioClip)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_FacialAnimationController: No audio clip provided for speech animation"));
        return;
    }

    bIsSpeaking = true;
    SpeechAnimationTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Started speech animation for audio clip %s"), 
           *AudioClip->GetName());
}

void UAnim_FacialAnimationController::StopSpeechAnimation()
{
    bIsSpeaking = false;
    SpeechAnimationTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Stopped speech animation"));
}

void UAnim_FacialAnimationController::SetMouthShape(const FString& Viseme, float Weight)
{
    Weight = FMath::Clamp(Weight, 0.0f, 1.0f);
    
    // Apply viseme blend shape
    SetBlendShapeWeight(Viseme, Weight);
    
    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Set mouth shape '%s' with weight %f"), 
           *Viseme, Weight);
}

void UAnim_FacialAnimationController::UpdateFacialAnimation(float DeltaTime)
{
    // Update current expression weight based on blend speed
    if (CurrentExpressionWeight < 1.0f)
    {
        CurrentExpressionWeight = FMath::Min(1.0f, CurrentExpressionWeight + (ExpressionBlendSpeed * DeltaTime));
    }
}

void UAnim_FacialAnimationController::UpdateEyeAnimation(float DeltaTime)
{
    BlinkTimer += DeltaTime;

    // Handle automatic blinking
    if (!bIsBlinking && BlinkTimer >= BlinkInterval)
    {
        TriggerBlink(1.0f);
    }

    // Handle blink animation
    if (bIsBlinking)
    {
        if (BlinkTimer >= BlinkInterval)
        {
            bIsBlinking = false;
            BlinkTimer = 0.0f;
        }
    }
}

void UAnim_FacialAnimationController::UpdateSpeechAnimation(float DeltaTime)
{
    if (bIsSpeaking)
    {
        SpeechAnimationTimer += DeltaTime;
        
        // Simple procedural mouth movement for speech
        float MouthMovement = FMath::Sin(SpeechAnimationTimer * 8.0f) * 0.5f + 0.5f;
        SetMouthShape(TEXT("Speech_Open"), MouthMovement);
    }
}

void UAnim_FacialAnimationController::UpdateEmotionalSequence(float DeltaTime)
{
    if (!bPlayingEmotionalSequence || CurrentEmotionalSequence.Num() == 0)
    {
        return;
    }

    SequenceTimer += DeltaTime;

    // Check if current emotional state duration has elapsed
    if (CurrentSequenceIndex < CurrentEmotionalSequence.Num())
    {
        const FAnim_EmotionalState& CurrentState = CurrentEmotionalSequence[CurrentSequenceIndex];
        
        if (SequenceTimer >= CurrentState.Duration)
        {
            CurrentSequenceIndex++;
            SequenceTimer = 0.0f;

            // Move to next emotional state or end sequence
            if (CurrentSequenceIndex < CurrentEmotionalSequence.Num())
            {
                const FAnim_EmotionalState& NextState = CurrentEmotionalSequence[CurrentSequenceIndex];
                SetFacialExpression(NextState.PrimaryExpression, NextState.Intensity, NextState.Duration);
            }
            else
            {
                // Sequence complete
                bPlayingEmotionalSequence = false;
                BlendToNeutralExpression(1.0f);
                UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Emotional sequence completed"));
            }
        }
    }
}

void UAnim_FacialAnimationController::InitializeBlendShapeMappings()
{
    // Initialize blend shape mappings for each facial expression
    // These would typically map to actual morph targets in the skeletal mesh

    // Neutral expression (baseline)
    TArray<FAnim_FacialBlendTarget> NeutralTargets;
    ExpressionBlendShapes.Add(EAnim_FacialExpression::Neutral, NeutralTargets);

    // Happy expression
    TArray<FAnim_FacialBlendTarget> HappyTargets;
    FAnim_FacialBlendTarget SmileTarget;
    SmileTarget.BlendShapeName = TEXT("Smile");
    SmileTarget.TargetWeight = 0.8f;
    SmileTarget.BlendSpeed = 2.0f;
    HappyTargets.Add(SmileTarget);
    ExpressionBlendShapes.Add(EAnim_FacialExpression::Happy, HappyTargets);

    // Fear expression
    TArray<FAnim_FacialBlendTarget> FearTargets;
    FAnim_FacialBlendTarget EyesWideTarget;
    EyesWideTarget.BlendShapeName = TEXT("EyesWide");
    EyesWideTarget.TargetWeight = 0.9f;
    EyesWideTarget.BlendSpeed = 3.0f;
    FearTargets.Add(EyesWideTarget);
    
    FAnim_FacialBlendTarget MouthOpenTarget;
    MouthOpenTarget.BlendShapeName = TEXT("MouthOpen");
    MouthOpenTarget.TargetWeight = 0.4f;
    MouthOpenTarget.BlendSpeed = 2.5f;
    FearTargets.Add(MouthOpenTarget);
    ExpressionBlendShapes.Add(EAnim_FacialExpression::Fear, FearTargets);

    // Pain expression
    TArray<FAnim_FacialBlendTarget> PainTargets;
    FAnim_FacialBlendTarget BrowFurrowTarget;
    BrowFurrowTarget.BlendShapeName = TEXT("BrowFurrow");
    BrowFurrowTarget.TargetWeight = 0.7f;
    BrowFurrowTarget.BlendSpeed = 1.5f;
    PainTargets.Add(BrowFurrowTarget);
    
    FAnim_FacialBlendTarget EyesSquintTarget;
    EyesSquintTarget.BlendShapeName = TEXT("EyesSquint");
    EyesSquintTarget.TargetWeight = 0.6f;
    EyesSquintTarget.BlendSpeed = 1.5f;
    PainTargets.Add(EyesSquintTarget);
    ExpressionBlendShapes.Add(EAnim_FacialExpression::Pain, PainTargets);

    UE_LOG(LogTemp, Log, TEXT("UAnim_FacialAnimationController: Initialized blend shape mappings"));
}

void UAnim_FacialAnimationController::ApplyBlendShapeWeights(const TArray<FAnim_FacialBlendTarget>& BlendTargets, float GlobalWeight)
{
    for (const FAnim_FacialBlendTarget& Target : BlendTargets)
    {
        float FinalWeight = Target.TargetWeight * GlobalWeight;
        SetBlendShapeWeight(Target.BlendShapeName, FinalWeight);
    }
}

void UAnim_FacialAnimationController::BlendToNeutralExpression(float BlendTime)
{
    BlendToExpression(EAnim_FacialExpression::Neutral, BlendTime);
}

float UAnim_FacialAnimationController::CalculateIntensityMultiplier(EAnim_EmotionalIntensity Intensity) const
{
    switch (Intensity)
    {
        case EAnim_EmotionalIntensity::Subtle:
            return 0.3f;
        case EAnim_EmotionalIntensity::Moderate:
            return 0.6f;
        case EAnim_EmotionalIntensity::Strong:
            return 0.9f;
        case EAnim_EmotionalIntensity::Extreme:
            return 1.0f;
        default:
            return 0.6f;
    }
}