#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ConsciousnessAnimationComponent.h"
#include "TranspersonalAnimInstance.generated.h"

UCLASS()
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness Animation")
    EConsciousnessState CurrentConsciousnessState;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness Animation")
    float StateTransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness Animation")
    float EnergyLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness Animation")
    float LuminosityIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness Animation")
    FVector AuraScale;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    // Spiritual Animation States
    UPROPERTY(BlueprintReadOnly, Category = "Spiritual States")
    bool bIsMeditating;

    UPROPERTY(BlueprintReadOnly, Category = "Spiritual States")
    bool bIsTransforming;

    UPROPERTY(BlueprintReadOnly, Category = "Spiritual States")
    bool bIsChannelingEnergy;

    UPROPERTY(BlueprintReadOnly, Category = "Spiritual States")
    bool bIsInTrance;

    // Animation Blend Values
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float DormantBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float AwakeningBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float ExpandingBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float TranscendentBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float UnityBlend;

    // Pose Offsets
    UPROPERTY(BlueprintReadOnly, Category = "Pose Offsets")
    float SpineOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Pose Offsets")
    float HeadOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Pose Offsets")
    float ArmOffset;

private:
    // References
    UPROPERTY()
    class APawn* OwningPawn;

    UPROPERTY()
    class UConsciousnessAnimationComponent* ConsciousnessComponent;

    // Helper functions
    void UpdateMovementProperties();
    void UpdateConsciousnessProperties();
    void UpdateSpiritualStates();
    void UpdateAnimationBlends();
    void UpdatePoseOffsets();
    
    float CalculateBlendWeight(EConsciousnessState TargetState, float TransitionProgress);
};