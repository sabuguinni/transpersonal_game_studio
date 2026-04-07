#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "ConsciousnessAnimationComponent.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Grounded        UMETA(DisplayName = "Grounded"),
    Focused         UMETA(DisplayName = "Focused"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Flowing         UMETA(DisplayName = "Flowing"),
    Awakened        UMETA(DisplayName = "Awakened")
};

UENUM(BlueprintType)
enum class EMeditationPose : uint8
{
    Lotus          UMETA(DisplayName = "Lotus"),
    Seiza          UMETA(DisplayName = "Seiza"),
    Standing       UMETA(DisplayName = "Standing"),
    Walking        UMETA(DisplayName = "Walking"),
    Lying          UMETA(DisplayName = "Lying")
};

USTRUCT(BlueprintType)
struct FConsciousnessAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AuraColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInMeditation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMeditationPose MeditationPose;

    FConsciousnessAnimationData()
    {
        CurrentState = EConsciousnessState::Grounded;
        StateIntensity = 0.0f;
        BreathingRate = 1.0f;
        EnergyLevel = 0.5f;
        AuraColor = FVector(1.0f, 1.0f, 1.0f);
        bIsInMeditation = false;
        MeditationPose = EMeditationPose::Lotus;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessAnimationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void SetConsciousnessState(EConsciousnessState NewState, float TransitionTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void StartMeditation(EMeditationPose Pose);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void StopMeditation();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void UpdateBreathingPattern(float Rate, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void TriggerEnergyPulse(float Intensity);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Consciousness Animation")
    FConsciousnessAnimationData GetAnimationData() const { return AnimationData; }

    UFUNCTION(BlueprintPure, Category = "Consciousness Animation")
    float GetStateTransitionProgress() const { return StateTransitionProgress; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FConsciousnessAnimationData AnimationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float StateTransitionDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float StateTransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    EConsciousnessState TargetState;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    EConsciousnessState PreviousState;

    // Breathing Animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float BreathingCycleTime;

    UPROPERTY(BlueprintReadOnly, Category = "Breathing")
    float CurrentBreathingPhase;

    // Energy Pulse System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy")
    float EnergyPulseDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Energy")
    float CurrentEnergyPulse;

private:
    void UpdateStateTransition(float DeltaTime);
    void UpdateBreathingCycle(float DeltaTime);
    void UpdateEnergyPulse(float DeltaTime);
    void ApplyConsciousnessEffects();
};