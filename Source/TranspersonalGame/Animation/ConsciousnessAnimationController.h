#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "ConsciousnessAnimationController.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Meditative      UMETA(DisplayName = "Meditative"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Shadow          UMETA(DisplayName = "Shadow"),
    Integrated      UMETA(DisplayName = "Integrated"),
    Transforming    UMETA(DisplayName = "Transforming")
};

UENUM(BlueprintType)
enum class EEmotionalIntensity : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Peak            UMETA(DisplayName = "Peak")
};

USTRUCT(BlueprintType)
struct FConsciousnessAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AuraOpacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EnergyFlowDirection;

    FConsciousnessAnimationData()
    {
        CurrentState = EConsciousnessState::Neutral;
        Intensity = EEmotionalIntensity::Low;
        TransitionSpeed = 1.0f;
        AuraOpacity = 0.5f;
        EnergyFlowDirection = FVector::ZeroVector;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UConsciousnessAnimationController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UConsciousnessAnimationController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void SetConsciousnessState(EConsciousnessState NewState, float TransitionDuration = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void SetEmotionalIntensity(EEmotionalIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void TriggerTransformation(EConsciousnessState TargetState);

    // Breathing and Energy Flow
    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void StartBreathingCycle(float InhaleTime = 4.0f, float ExhaleTime = 6.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void SetEnergyFlowDirection(FVector Direction);

protected:
    // Animation Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FConsciousnessAnimationData AnimationData;

    // Movement Properties
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    // Consciousness Properties
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    float ConsciousnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    float MeditationDepth;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    float ShadowIntegration;

    // Breathing Animation
    UPROPERTY(BlueprintReadOnly, Category = "Breathing")
    float BreathingCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Breathing")
    float BreathingAmplitude;

    UPROPERTY(BlueprintReadOnly, Category = "Breathing")
    bool bIsBreathing;

    // Energy Flow
    UPROPERTY(BlueprintReadOnly, Category = "Energy")
    float EnergyFlowIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Energy")
    FVector EnergyDirection;

    // Aura and Visual Effects
    UPROPERTY(BlueprintReadOnly, Category = "Visual Effects")
    float AuraIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Visual Effects")
    FLinearColor AuraColor;

    UPROPERTY(BlueprintReadOnly, Category = "Visual Effects")
    float ParticleEmissionRate;

private:
    // Internal state tracking
    EConsciousnessState PreviousState;
    float StateTransitionTimer;
    float StateTransitionDuration;
    
    // Breathing cycle tracking
    float BreathingTimer;
    float InhaleDuration;
    float ExhaleDuration;
    bool bInhaling;

    // Animation curve references
    UPROPERTY()
    class UCurveFloat* BreathingCurve;

    UPROPERTY()
    class UCurveFloat* EnergyFlowCurve;

    UPROPERTY()
    class UCurveFloat* TransitionCurve;

    // Helper functions
    void UpdateBreathingAnimation(float DeltaTime);
    void UpdateEnergyFlow(float DeltaTime);
    void UpdateStateTransition(float DeltaTime);
    void UpdateAuraEffects();
    FLinearColor GetStateColor(EConsciousnessState State);
    float GetStateIntensity(EConsciousnessState State, EEmotionalIntensity Intensity);
};