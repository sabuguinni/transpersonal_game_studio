#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Narr_StoryTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    FirstDinosaur     UMETA(DisplayName = "First Dinosaur Encounter"),
    WaterSource       UMETA(DisplayName = "Water Source Discovery"),
    NightFall         UMETA(DisplayName = "Night Fall Warning"),
    PredatorNear      UMETA(DisplayName = "Predator Nearby"),
    SafeZone          UMETA(DisplayName = "Safe Zone Entry"),
    CraftingHint      UMETA(DisplayName = "Crafting Tutorial"),
    DangerZone        UMETA(DisplayName = "Danger Zone Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    USoundCue* VoiceOverCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bRequiresPlayerInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bOnlyTriggerOnce;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        NarrativeText = FText::GetEmpty();
        VoiceOverCue = nullptr;
        DisplayDuration = 5.0f;
        bRequiresPlayerInput = false;
        bOnlyTriggerOnce = true;
    }
};

/**
 * Story trigger that activates narrative events when player enters specific areas
 * Designed for survival game storytelling through environmental triggers
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_StoryTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                      UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

public:
    // Trigger configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    FNarr_StoryEvent StoryEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    bool bHasBeenTriggered;

    // Audio component for voice-over
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    // Survival context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    bool bIncreaseFearOnTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float FearIncreaseAmount;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    void TriggerStoryEvent();

    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    void SetTriggerActive(bool bActive);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Trigger")
    void OnStoryEventTriggered(const FNarr_StoryEvent& Event);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Trigger")
    void OnPlayerEnterTrigger();

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Trigger")
    void OnPlayerExitTrigger();

private:
    void PlayVoiceOver();
    void UpdatePlayerFear();
};