#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "ConsciousnessAnimationComponent.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Awakening       UMETA(DisplayName = "Awakening"), 
    Expanding       UMETA(DisplayName = "Expanding"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity")
};

UENUM(BlueprintType)
enum class ETransformationType : uint8
{
    Materialization     UMETA(DisplayName = "Materialization"),
    Dematerialization   UMETA(DisplayName = "Dematerialization"),
    EnergyShift         UMETA(DisplayName = "Energy Shift"),
    ConsciousnessExpansion UMETA(DisplayName = "Consciousness Expansion")
};

USTRUCT(BlueprintType)
struct FConsciousnessAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTransitionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AuraScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LuminosityIntensity;

    FConsciousnessAnimationData()
    {
        CurrentState = EConsciousnessState::Dormant;
        StateTransitionProgress = 0.0f;
        EnergyLevel = 0.5f;
        AuraScale = FVector(1.0f, 1.0f, 1.0f);
        LuminosityIntensity = 0.0f;
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
    void SetConsciousnessState(EConsciousnessState NewState, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void TriggerTransformation(ETransformationType TransformType, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void UpdateEnergyLevel(float NewEnergyLevel);

    UFUNCTION(BlueprintPure, Category = "Consciousness Animation")
    FConsciousnessAnimationData GetAnimationData() const { return AnimationData; }

    // Aura and Energy Effects
    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void PulseAura(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void StartEnergyFlow(FVector Direction, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Animation")
    void StopEnergyFlow();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FConsciousnessAnimationData AnimationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BaseAnimationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float TransformationIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableAutoStateTransitions;

    // State Transition
    EConsciousnessState TargetState;
    float StateTransitionDuration;
    float StateTransitionTimer;
    bool bIsTransitioning;

    // Transformation Effects
    ETransformationType CurrentTransformation;
    float TransformationDuration;
    float TransformationTimer;
    bool bIsTransforming;

    // Aura Pulse
    float AuraPulseIntensity;
    float AuraPulseDuration;
    float AuraPulseTimer;
    bool bIsPulsing;

    // Energy Flow
    FVector EnergyFlowDirection;
    float EnergyFlowSpeed;
    bool bEnergyFlowing;

    void UpdateStateTransition(float DeltaTime);
    void UpdateTransformation(float DeltaTime);
    void UpdateAuraPulse(float DeltaTime);
    void UpdateEnergyFlow(float DeltaTime);
    
    float CalculateStateBlendValue(EConsciousnessState FromState, EConsciousnessState ToState, float Progress);
};