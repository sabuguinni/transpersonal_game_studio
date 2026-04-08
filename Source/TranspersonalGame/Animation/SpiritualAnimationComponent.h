// SpiritualAnimationComponent.h
// Animation component for spiritual and transcendental character movements

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "SpiritualAnimationComponent.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Unconscious     UMETA(DisplayName = "Unconscious"),
    Awakening       UMETA(DisplayName = "Awakening"),
    Aware           UMETA(DisplayName = "Aware"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity")
};

UENUM(BlueprintType)
enum class ESpiritualMovementType : uint8
{
    Floating        UMETA(DisplayName = "Floating"),
    Meditation      UMETA(DisplayName = "Meditation"),
    EnergyFlow      UMETA(DisplayName = "Energy Flow"),
    Manifestation   UMETA(DisplayName = "Manifestation"),
    Dissolution     UMETA(DisplayName = "Dissolution")
};

USTRUCT(BlueprintType)
struct FConsciousnessAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EnergyFlowDirection;

    FConsciousnessAnimationData()
    {
        State = EConsciousnessState::Unconscious;
        IntensityLevel = 0.0f;
        TransitionSpeed = 1.0f;
        EnergyFlowDirection = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USpiritualAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USpiritualAnimationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core animation functions
    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void SetConsciousnessState(EConsciousnessState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void PlaySpiritualMovement(ESpiritualMovementType MovementType, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void StartEnergyFlowAnimation(FVector FlowDirection, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void TriggerTranscendentMoment(float Duration = 3.0f);

    // Meditation and breathing animations
    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void StartMeditationPose(float BreathingRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void StopMeditationPose();

    // Floating and levitation
    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void StartFloatingAnimation(float Height = 100.0f, float Speed = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void StopFloatingAnimation();

    // Aura and energy field animations
    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void PulseAuraAnimation(float PulseRate = 2.0f, float MaxIntensity = 1.5f);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Animation")
    void ExpandConsciousnessField(float ExpansionRadius = 500.0f, float Duration = 4.0f);

protected:
    // Animation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Animation")
    FConsciousnessAnimationData CurrentAnimationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Animation")
    class UAnimMontage* MeditationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Animation")
    class UAnimMontage* TranscendenceMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Animation")
    class UAnimMontage* EnergyFlowMontage;

    // Animation curves for smooth transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* ConsciousnessTransitionCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* BreathingCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* FloatingCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Curves")
    class UCurveFloat* AuraPulseCurve;

    // Internal state
    UPROPERTY()
    bool bIsInTransition;

    UPROPERTY()
    bool bIsMeditating;

    UPROPERTY()
    bool bIsFloating;

    UPROPERTY()
    float CurrentFloatingHeight;

    UPROPERTY()
    float AnimationTimer;

    UPROPERTY()
    EConsciousnessState TargetState;

    UPROPERTY()
    EConsciousnessState PreviousState;

private:
    // Internal animation update functions
    void UpdateConsciousnessTransition(float DeltaTime);
    void UpdateFloatingMovement(float DeltaTime);
    void UpdateMeditationBreathing(float DeltaTime);
    void UpdateEnergyFlow(float DeltaTime);
    void UpdateAuraPulse(float DeltaTime);

    // Helper functions
    float GetStateTransitionProgress() const;
    FVector CalculateFloatingOffset(float DeltaTime) const;
    float CalculateBreathingIntensity(float DeltaTime) const;
};