#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Narr_EnvironmentalStoryTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryTriggerType : uint8
{
    FossilDiscovery     UMETA(DisplayName = "Fossil Discovery"),
    PredatorWarning     UMETA(DisplayName = "Predator Warning"),
    MigrationRoute      UMETA(DisplayName = "Migration Route"),
    AbandonedCamp       UMETA(DisplayName = "Abandoned Camp"),
    SeismicActivity     UMETA(DisplayName = "Seismic Activity"),
    TribalRitual        UMETA(DisplayName = "Tribal Ritual Site"),
    DangerZone          UMETA(DisplayName = "Danger Zone"),
    SafeHaven           UMETA(DisplayName = "Safe Haven")
};

USTRUCT(BlueprintType)
struct FNarr_StoryTriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> VoicelineAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float DelayBeforeNarration;

    FNarr_StoryTriggerData()
    {
        StoryID = TEXT("");
        NarrativeText = FText::GetEmpty();
        TriggerType = ENarr_StoryTriggerType::FossilDiscovery;
        bOneTimeOnly = true;
        DelayBeforeNarration = 1.0f;
    }
};

/**
 * Environmental story trigger that plays contextual narration when player enters specific areas
 * Used for environmental storytelling, discovery moments, and atmospheric narrative
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_EnvironmentalStoryTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_EnvironmentalStoryTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // Story configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_StoryTriggerData StoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryTriggerData> AlternativeStories;

    // Audio system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    // Trigger behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bRequirePlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 MaxActivations;

    // Visual feedback
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowDebugInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor DebugColor;

private:
    UPROPERTY()
    bool bHasBeenTriggered;

    UPROPERTY()
    int32 ActivationCount;

    UPROPERTY()
    float LastActivationTime;

    UPROPERTY()
    FTimerHandle NarrationDelayTimer;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryNarration();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryData(const FNarr_StoryTriggerData& NewStoryData);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTrigger() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FNarr_StoryTriggerData GetCurrentStoryData() const { return StoryData; }

private:
    void PlayNarrationAudio();
    void HandleStoryProgression();
    FNarr_StoryTriggerData SelectStoryData() const;
};