#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/TriggerVolume.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "NarrativeTypes.h"
#include "SurvivalNarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalTip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FString TipText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FString AudioFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ENarr_SurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    bool bIsTriggered;

    FNarr_SurvivalTip()
    {
        TipText = TEXT("");
        AudioFile = TEXT("");
        Context = ENarr_SurvivalContext::General;
        Priority = 1.0f;
        bIsTriggered = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_SurvivalTip SurvivalTip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_NarrativeTrigger()
    {
        Location = FVector::ZeroVector;
        TriggerRadius = 300.0f;
        bIsOneTime = true;
        bHasBeenTriggered = false;
    }
};

/**
 * Manages survival narrative and contextual audio tips
 * Provides immersive storytelling through environmental triggers
 */
UCLASS()
class TRANSPERSONALGAME_API USurvivalNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    USurvivalNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Narrative management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeSurvivalTips();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeTrigger(const FNarr_NarrativeTrigger& Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckPlayerProximity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerSurvivalTip(const FNarr_SurvivalTip& Tip);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeAudio(const FString& AudioFile);

    // Context-based tips
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualTip(ENarr_SurvivalContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_SurvivalTip GetRandomTipForContext(ENarr_SurvivalContext Context);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrativePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarrative();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_SurvivalTip> SurvivalTips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_NarrativeTrigger> NarrativeTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float NarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bNarrativeEnabled;

private:
    void LoadSurvivalTipDatabase();
    void SetupDefaultTriggers();
    FNarr_SurvivalTip CreateSurvivalTip(const FString& Text, const FString& Audio, ENarr_SurvivalContext Context, float Priority = 1.0f);
    
    FTimerHandle TriggerCheckTimer;
    bool bIsInitialized;
};