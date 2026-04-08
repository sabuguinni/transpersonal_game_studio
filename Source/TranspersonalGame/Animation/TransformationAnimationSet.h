#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "ConsciousnessAnimationController.h"
#include "TransformationAnimationSet.generated.h"

USTRUCT(BlueprintType)
struct FStateAnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Animations")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    class UAnimSequence* BreathingAdditive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy Flow")
    class UAnimSequence* EnergyFlowLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    class UAnimMontage* EnterStateMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    class UAnimMontage* ExitStateMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gestures")
    TArray<class UAnimMontage*> GestureMontages;

    FStateAnimationSet()
    {
        IdleAnimation = nullptr;
        MovementBlendSpace = nullptr;
        BreathingAdditive = nullptr;
        EnergyFlowLoop = nullptr;
        EnterStateMontage = nullptr;
        ExitStateMontage = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FTransformationSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    EConsciousnessState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    EConsciousnessState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    class UAnimMontage* TransformationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    float TransformationDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* TransformationVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* TransformationSound;

    FTransformationSequence()
    {
        FromState = EConsciousnessState::Neutral;
        ToState = EConsciousnessState::Neutral;
        TransformationMontage = nullptr;
        TransformationDuration = 2.0f;
        TransformationVFX = nullptr;
        TransformationSound = nullptr;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UTransformationAnimationSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UTransformationAnimationSet();

    // Animation sets for each consciousness state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Animations")
    TMap<EConsciousnessState, FStateAnimationSet> StateAnimations;

    // Transformation sequences between states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformations")
    TArray<FTransformationSequence> TransformationSequences;

    // Universal animations that work across all states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universal Animations")
    FStateAnimationSet UniversalAnimations;

    // Meditation specific animations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    class UAnimSequence* MeditationSitIdle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    class UAnimSequence* MeditationStandIdle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    class UAnimMontage* EnterMeditationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    class UAnimMontage* ExitMeditationMontage;

    // Shadow work animations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow Work")
    class UAnimSequence* ShadowConfrontationIdle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow Work")
    class UAnimMontage* ShadowIntegrationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow Work")
    class UAnimSequence* ShadowResistanceLoop;

    // Transcendent state animations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transcendent")
    class UAnimSequence* TranscendentFloatingIdle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transcendent")
    class UAnimSequence* TranscendentEnergyFlow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transcendent")
    class UAnimMontage* AscensionMontage;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Animation Set")
    FStateAnimationSet GetAnimationSetForState(EConsciousnessState State);

    UFUNCTION(BlueprintCallable, Category = "Animation Set")
    UAnimMontage* GetTransformationMontage(EConsciousnessState FromState, EConsciousnessState ToState);

    UFUNCTION(BlueprintCallable, Category = "Animation Set")
    float GetTransformationDuration(EConsciousnessState FromState, EConsciousnessState ToState);

    UFUNCTION(BlueprintCallable, Category = "Animation Set")
    TArray<UAnimMontage*> GetGesturesForState(EConsciousnessState State);

protected:
    // Helper function to initialize default animation sets
    void InitializeDefaultAnimationSets();
};