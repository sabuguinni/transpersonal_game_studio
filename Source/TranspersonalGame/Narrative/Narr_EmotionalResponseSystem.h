#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Narr_EmotionalResponseSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_EmotionalState : uint8
{
    Calm,
    Alert,
    Fear,
    Anger,
    Curiosity,
    Sadness,
    Relief,
    Excitement
};

USTRUCT(BlueprintType)
struct FNarr_EmotionalResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENarr_EmotionalState EmotionalState = ENarr_EmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString DialogueResponse;

    FNarr_EmotionalResponse()
    {
        EmotionalState = ENarr_EmotionalState::Calm;
        Intensity = 0.5f;
        Duration = 5.0f;
        TriggerEvent = TEXT("");
        DialogueResponse = TEXT("");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_EmotionalResponseSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_EmotionalResponseSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    ENarr_EmotionalState CurrentEmotionalState = ENarr_EmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float CurrentIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float EmotionalDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    TArray<FNarr_EmotionalResponse> EmotionalResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    bool bIsEmotionallyActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float EmotionalThreshold = 0.3f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void TriggerEmotionalResponse(const FString& EventName, ENarr_EmotionalState NewState, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    ENarr_EmotionalState GetCurrentEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    float GetCurrentIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    FString GetEmotionalDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void SetEmotionalState(ENarr_EmotionalState NewState, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ResetEmotionalState();

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    bool IsEmotionallyActive() const;

private:
    float EmotionalTimer = 0.0f;
    void UpdateEmotionalDecay(float DeltaTime);
    FNarr_EmotionalResponse* FindEmotionalResponse(const FString& EventName);
};