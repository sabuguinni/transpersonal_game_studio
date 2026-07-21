#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_FacialAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_FacialExpression : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Happy           UMETA(DisplayName = "Happy"),
    Sad             UMETA(DisplayName = "Sad"),
    Angry           UMETA(DisplayName = "Angry"),
    Fear            UMETA(DisplayName = "Fear"),
    Surprise        UMETA(DisplayName = "Surprise"),
    Disgust         UMETA(DisplayName = "Disgust"),
    Pain            UMETA(DisplayName = "Pain"),
    Concentration   UMETA(DisplayName = "Concentration"),
    Exhaustion      UMETA(DisplayName = "Exhaustion")
};

UENUM(BlueprintType)
enum class EAnim_EmotionalIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Strong      UMETA(DisplayName = "Strong"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FacialBlendTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Animation")
    FString BlendShapeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TargetWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Animation", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float BlendSpeed;

    FAnim_FacialBlendTarget()
    {
        BlendShapeName = TEXT("");
        TargetWeight = 0.0f;
        BlendSpeed = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EAnim_FacialExpression PrimaryExpression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EAnim_EmotionalIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ExpressionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    bool bShouldBlendOut;

    FAnim_EmotionalState()
    {
        PrimaryExpression = EAnim_FacialExpression::Neutral;
        Intensity = EAnim_EmotionalIntensity::Moderate;
        ExpressionWeight = 1.0f;
        Duration = 2.0f;
        bShouldBlendOut = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_FacialAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_FacialAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core facial animation functions
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetFacialExpression(EAnim_FacialExpression Expression, EAnim_EmotionalIntensity Intensity = EAnim_EmotionalIntensity::Moderate, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void BlendToExpression(EAnim_FacialExpression Expression, float BlendTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetBlendShapeWeight(const FString& BlendShapeName, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void PlayEmotionalSequence(const TArray<FAnim_EmotionalState>& EmotionalSequence);

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StopAllFacialAnimations();

    // Survival-specific expressions
    UFUNCTION(BlueprintCallable, Category = "Survival Expressions")
    void ExpressPain(float Intensity = 0.7f);

    UFUNCTION(BlueprintCallable, Category = "Survival Expressions")
    void ExpressFear(float Intensity = 0.8f);

    UFUNCTION(BlueprintCallable, Category = "Survival Expressions")
    void ExpressExhaustion(float Intensity = 0.6f);

    UFUNCTION(BlueprintCallable, Category = "Survival Expressions")
    void ExpressConcentration(float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Survival Expressions")
    void ExpressRelief(float Intensity = 0.4f);

    // Eye and blink control
    UFUNCTION(BlueprintCallable, Category = "Eye Animation")
    void SetEyeDirection(FVector LookDirection);

    UFUNCTION(BlueprintCallable, Category = "Eye Animation")
    void TriggerBlink(float BlinkSpeed = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Eye Animation")
    void SetBlinkRate(float BlinksPerMinute = 15.0f);

    // Mouth and speech
    UFUNCTION(BlueprintCallable, Category = "Speech Animation")
    void StartSpeechAnimation(class USoundWave* AudioClip);

    UFUNCTION(BlueprintCallable, Category = "Speech Animation")
    void StopSpeechAnimation();

    UFUNCTION(BlueprintCallable, Category = "Speech Animation")
    void SetMouthShape(const FString& Viseme, float Weight = 1.0f);

protected:
    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* TargetMeshComponent;

    // Current facial state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial State")
    EAnim_FacialExpression CurrentExpression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial State")
    EAnim_EmotionalIntensity CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial State")
    float CurrentExpressionWeight;

    // Blend shape mappings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Shapes")
    TMap<EAnim_FacialExpression, TArray<FAnim_FacialBlendTarget>> ExpressionBlendShapes;

    // Animation timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Timing")
    float ExpressionBlendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Timing")
    float DefaultExpressionDuration;

    // Eye animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye Animation")
    FVector CurrentEyeDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye Animation")
    float BlinkTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye Animation")
    float BlinkInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eye Animation")
    bool bIsBlinking;

    // Speech animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speech Animation")
    bool bIsSpeaking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speech Animation")
    float SpeechAnimationTimer;

    // Emotional sequence playback
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Sequence")
    TArray<FAnim_EmotionalState> CurrentEmotionalSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Sequence")
    int32 CurrentSequenceIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Sequence")
    float SequenceTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Sequence")
    bool bPlayingEmotionalSequence;

private:
    // Internal helper functions
    void UpdateFacialAnimation(float DeltaTime);
    void UpdateEyeAnimation(float DeltaTime);
    void UpdateSpeechAnimation(float DeltaTime);
    void UpdateEmotionalSequence(float DeltaTime);
    void InitializeBlendShapeMappings();
    void ApplyBlendShapeWeights(const TArray<FAnim_FacialBlendTarget>& BlendTargets, float GlobalWeight = 1.0f);
    void BlendToNeutralExpression(float BlendTime = 1.0f);
    float CalculateIntensityMultiplier(EAnim_EmotionalIntensity Intensity) const;
};