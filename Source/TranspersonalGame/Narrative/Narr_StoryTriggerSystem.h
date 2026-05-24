#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Narr_StoryTriggerSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    LocationDiscovery    UMETA(DisplayName = "Location Discovery"),
    DangerWarning       UMETA(DisplayName = "Danger Warning"),
    SurvivalTip         UMETA(DisplayName = "Survival Tip"),
    EnvironmentLore     UMETA(DisplayName = "Environment Lore")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryTriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    FString TriggerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    float AudioDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    bool bCanRetrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    float RetriggerCooldown;

    FNarr_StoryTriggerData()
    {
        TriggerName = TEXT("Default Trigger");
        TriggerType = ENarr_TriggerType::LocationDiscovery;
        NarrativeText = TEXT("");
        AudioURL = TEXT("");
        AudioDuration = 0.0f;
        bCanRetrigger = false;
        RetriggerCooldown = 60.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryTriggerSystem : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_StoryTriggerSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    FNarr_StoryTriggerData TriggerData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    bool bIsTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Story Trigger")
    float LastTriggerTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    void TriggerStoryEvent(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    bool CanTrigger() const;

    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    void ResetTrigger();

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Trigger")
    void OnStoryTriggered(const FNarr_StoryTriggerData& Data);

protected:
    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

    void PlayNarrativeAudio();
    void LogTriggerEvent(const FString& EventType);
};