// TranscendentalAnimInstance.h
// Animation Blueprint base class for transcendental character animations

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SpiritualAnimationComponent.h"
#include "TranscendentalAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FTranscendentalAnimationState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFloating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMeditating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EnergyFlowDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AuraIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LevitationHeight;

    FTranscendentalAnimationState()
    {
        ConsciousnessLevel = EConsciousnessState::Unconscious;
        SpiritualIntensity = 0.0f;
        bIsFloating = false;
        bIsMeditating = false;
        bIsInTransition = false;
        EnergyFlowDirection = FVector::ZeroVector;
        BreathingRate = 1.0f;
        AuraIntensity = 0.0f;
        LevitationHeight = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranscendentalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranscendentalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Animation state variables accessible from Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendental Animation")
    FTranscendentalAnimationState AnimationState;

    // Pose blending weights
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pose Blending")
    float MeditationPoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pose Blending")
    float FloatingPoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pose Blending")
    float TranscendentPoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pose Blending")
    float EnergyFlowPoseWeight;

    // Breathing animation variables
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Breathing Animation")
    float BreathingCycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Breathing Animation")
    float ChestExpansion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Breathing Animation")
    float DiaphragmMovement;

    // Floating animation variables
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floating Animation")
    float FloatingBobSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floating Animation")
    float FloatingBobAmount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floating Animation")
    FVector FloatingOffset;

    // Energy flow animation variables
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Energy Animation")
    float EnergyPulseRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Energy Animation")
    float EnergyIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Energy Animation")
    FRotator EnergyFlowRotation;

    // Consciousness transition variables
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consciousness Transition")
    float ConsciousnessTransitionAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consciousness Transition")
    float PreviousStateWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consciousness Transition")
    float NextStateWeight;

    // Animation curves for smooth transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* BreathingCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* FloatingCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* EnergyPulseCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* ConsciousnessTransitionCurve;

protected:
    // Reference to spiritual animation component
    UPROPERTY()
    class USpiritualAnimationComponent* SpiritualAnimComponent;

    // Internal timing variables
    UPROPERTY()
    float InternalTimer;

    UPROPERTY()
    float BreathingTimer;

    UPROPERTY()
    float FloatingTimer;

    UPROPERTY()
    float EnergyTimer;

private:
    // Update functions for different animation systems
    void UpdateBreathingAnimation(float DeltaTime);
    void UpdateFloatingAnimation(float DeltaTime);
    void UpdateEnergyFlowAnimation(float DeltaTime);
    void UpdateConsciousnessTransition(float DeltaTime);
    void UpdatePoseWeights(float DeltaTime);

    // Helper functions
    float CalculateBreathingIntensity(float Time) const;
    FVector CalculateFloatingOffset(float Time) const;
    float CalculateEnergyPulse(float Time) const;
    float GetConsciousnessStateWeight(EConsciousnessState State) const;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Transcendental Animation")
    void SetSpiritualIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Animation")
    void TriggerConsciousnessTransition(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Animation")
    void StartMeditationSequence(float BreathingRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Animation")
    void StartFloatingSequence(float Height = 100.0f, float Speed = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Animation")
    void StartEnergyFlowSequence(FVector FlowDirection, float Intensity = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Transcendental Animation")
    bool IsInTranscendentState() const;

    UFUNCTION(BlueprintPure, Category = "Transcendental Animation")
    float GetSpiritualElevation() const;
};