#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCEmotionalState.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Anger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Happiness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Stress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Energy = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PersonalityTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Bravery = 0.5f; // How likely to face danger

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f; // How much they seek social interaction

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.3f; // How quickly they turn to violence

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f; // Problem solving ability

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Loyalty = 0.7f; // How much they value relationships

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Adaptability = 0.5f; // How well they handle change
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_EmotionalStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_EmotionalStateComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    FNPC_EmotionalState CurrentEmotions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNPC_PersonalityTraits PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EmotionalDecayRate = 0.1f; // How fast emotions return to baseline

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float StressThreshold = 0.7f; // When NPC becomes stressed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PanicThreshold = 0.9f; // When NPC panics

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void ModifyEmotion(ENPC_EmotionType EmotionType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    float GetEmotionLevel(ENPC_EmotionType EmotionType);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    ENPC_EmotionType GetDominantEmotion();

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    bool IsStressed();

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    bool IsPanicked();

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void ReactToEvent(ENPC_EventType EventType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityTrait(ENPC_PersonalityTrait TraitType);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    ENPC_BehaviorType GetEmotionalBehaviorModifier();

private:
    void UpdateEmotionalDecay(float DeltaTime);
    void ProcessEmotionalInteractions();
    float CalculateEmotionalResponse(ENPC_EventType EventType, float BaseIntensity);
};