#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"
#include "Audio_GameplayFeedbackSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_GameplayEvent : uint8
{
    PlayerDamaged       UMETA(DisplayName = "Player Damaged"),
    DinosaurNearby      UMETA(DisplayName = "Dinosaur Nearby"),
    ItemCrafted         UMETA(DisplayName = "Item Crafted"),
    FoodConsumed        UMETA(DisplayName = "Food Consumed"),
    ShelterEntered      UMETA(DisplayName = "Shelter Entered"),
    DangerZoneEntered   UMETA(DisplayName = "Danger Zone Entered"),
    SafeZoneEntered     UMETA(DisplayName = "Safe Zone Entered"),
    NightFalling        UMETA(DisplayName = "Night Falling"),
    DayBreaking         UMETA(DisplayName = "Day Breaking")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FeedbackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    EAudio_GameplayEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    TSoftObjectPtr<USoundCue> FeedbackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    bool bShouldLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float FadeInTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float FadeOutTime = 0.5f;

    FAudio_FeedbackData()
    {
        EventType = EAudio_GameplayEvent::PlayerDamaged;
        Volume = 1.0f;
        Pitch = 1.0f;
        bShouldLoop = false;
        FadeInTime = 0.5f;
        FadeOutTime = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_GameplayFeedbackSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_GameplayFeedbackSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    TArray<FAudio_FeedbackData> FeedbackEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float GlobalVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxSimultaneousSounds = 5.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<class UAudioComponent*> AudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    TArray<EAudio_GameplayEvent> ActiveEvents;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerGameplayFeedback(EAudio_GameplayEvent EventType);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void StopGameplayFeedback(EAudio_GameplayEvent EventType);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void StopAllFeedback();

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void SetGlobalVolume(float NewVolume);

    UFUNCTION(BlueprintPure, Category = "Audio Feedback")
    bool IsEventActive(EAudio_GameplayEvent EventType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio Setup")
    void InitializeDefaultFeedbackEvents();

private:
    void InitializeAudioComponents();
    UAudioComponent* GetAvailableAudioComponent();
    FAudio_FeedbackData* GetFeedbackData(EAudio_GameplayEvent EventType);
    void PlayFeedbackSound(const FAudio_FeedbackData& FeedbackData, UAudioComponent* AudioComp);
};