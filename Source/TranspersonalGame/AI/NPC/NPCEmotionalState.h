#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "NPCEmotionalState.generated.h"

UENUM(BlueprintType)
enum class ENPC_EmotionType : uint8
{
    Neutral,
    Happy,
    Sad,
    Angry,
    Fearful,
    Excited,
    Anxious,
    Calm,
    Aggressive,
    Friendly
};

UENUM(BlueprintType)
enum class ENPC_MoodModifier : uint8
{
    Hungry,
    Tired,
    Injured,
    Threatened,
    Safe,
    Comfortable,
    Social,
    Isolated
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Happiness = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Stress = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Energy = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sociability = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Caution = 50.0f;

    FNPC_EmotionalProfile()
    {
        Happiness = 50.0f;
        Stress = 30.0f;
        Energy = 70.0f;
        Sociability = 60.0f;
        Aggression = 20.0f;
        Curiosity = 40.0f;
        Caution = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_EmotionType TriggerEmotion = ENPC_EmotionType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp = 0.0f;

    FNPC_EmotionalEvent()
    {
        TriggerEmotion = ENPC_EmotionType::Neutral;
        Intensity = 1.0f;
        Duration = 5.0f;
        EventDescription = TEXT("Unknown Event");
        TimeStamp = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_EmotionalState : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_EmotionalState();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    FNPC_EmotionalProfile BasePersonality;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    FNPC_EmotionalProfile CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    ENPC_EmotionType DominantEmotion = ENPC_EmotionType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float EmotionalStability = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float EmotionalRecoveryRate = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    TArray<FNPC_EmotionalEvent> RecentEmotionalEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    TArray<ENPC_MoodModifier> ActiveMoodModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    int32 MaxEmotionalEventHistory = 10;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void TriggerEmotionalEvent(ENPC_EmotionType EmotionType, float Intensity, float Duration, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void AddMoodModifier(ENPC_MoodModifier Modifier);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void RemoveMoodModifier(ENPC_MoodModifier Modifier);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    float GetEmotionalIntensity(ENPC_EmotionType EmotionType) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    bool IsInEmotionalState(ENPC_EmotionType EmotionType, float MinIntensity = 0.5f) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    ENPC_EmotionType GetDominantEmotion() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    float GetOverallMood() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    bool ShouldReactToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    float GetSocialInteractionModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void ResetEmotionalState();

private:
    void UpdateEmotionalState(float DeltaTime);
    void ProcessEmotionalEvents(float DeltaTime);
    void ApplyMoodModifiers();
    void CalculateDominantEmotion();
    void CleanupOldEmotionalEvents();
    
    float GetEmotionalValue(ENPC_EmotionType EmotionType, const FNPC_EmotionalProfile& Profile) const;
    void SetEmotionalValue(ENPC_EmotionType EmotionType, float Value, FNPC_EmotionalProfile& Profile);
};