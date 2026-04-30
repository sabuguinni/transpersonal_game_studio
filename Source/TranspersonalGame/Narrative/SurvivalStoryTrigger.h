#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "NarrativeTypes.h"
#include "SurvivalStoryTrigger.generated.h"

class UDialogueComponent;
class USoundCue;

UENUM(BlueprintType)
enum class ENarr_StoryTriggerType : uint8
{
    WaterSource     UMETA(DisplayName = "Water Source Discovery"),
    DangerZone      UMETA(DisplayName = "Predator Territory Warning"),
    SafeZone        UMETA(DisplayName = "Safe Haven Found"),
    HuntingGrounds  UMETA(DisplayName = "Hunting Grounds Alert"),
    FirstContact    UMETA(DisplayName = "First Dinosaur Encounter"),
    NightFall       UMETA(DisplayName = "Night Survival Warning")
};

USTRUCT(BlueprintType)
struct FNarr_StoryMoment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString TriggerMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TSoftObjectPtr<USoundCue> VoiceOverCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float DelayBeforeMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bOnlyTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryTriggerType TriggerType;

    FNarr_StoryMoment()
    {
        TriggerMessage = TEXT("A story moment occurs...");
        DelayBeforeMessage = 1.0f;
        bOnlyTriggerOnce = true;
        TriggerType = ENarr_StoryTriggerType::SafeZone;
    }
};

/**
 * Trigger box that creates narrative moments during survival gameplay
 * Provides contextual storytelling based on player location and situation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASurvivalStoryTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ASurvivalStoryTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPlayerEnterTrigger(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnPlayerExitTrigger(AActor* OverlappedActor, AActor* OtherActor);

    // Story configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_StoryMoment StoryMoment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> RandomMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bUseRandomMessages;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* VoiceOverComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AmbientAudioComponent;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHasTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 TriggerCount;

    // Timer for delayed messages
    FTimerHandle MessageDelayTimer;

private:
    UFUNCTION()
    void TriggerStoryMoment();

    UFUNCTION()
    void PlayVoiceOver();

    bool IsPlayerCharacter(AActor* Actor) const;
    FString GetContextualMessage() const;
    void LogStoryTrigger() const;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryMoment(const FNarr_StoryMoment& NewStoryMoment);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTrigger() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    ENarr_StoryTriggerType GetTriggerType() const { return StoryMoment.TriggerType; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    int32 GetTriggerCount() const { return TriggerCount; }
};