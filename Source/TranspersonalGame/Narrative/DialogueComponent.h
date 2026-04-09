// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "Sound/DialogueWave.h"
#include "Sound/DialogueVoice.h"
#include "Components/AudioComponent.h"
#include "NarrativeSubsystem.h"
#include "DialogueComponent.generated.h"

class UNarrativeSubsystem;
class UWidgetComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueLineStarted, const FDialogueLine&, DialogueLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueLineCompleted, const FDialogueLine&, DialogueLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueSequenceCompleted, const FString&, SequenceID);

/**
 * Dialogue Component - Handles individual dialogue playback and subtitle display
 * Attached to actors that can speak (NPCs, environmental objects, or the player)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FDialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PauseDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResumeDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SkipCurrentLine();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Dialogue|Setup")
    void SetDefaultVoice(UDialogueVoice* Voice);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Setup")
    void SetVolumeMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Setup")
    void SetPitchMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Setup")
    void EnableSubtitles(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Setup")
    void SetSubtitleWidget(TSubclassOf<UUserWidget> WidgetClass);

    // Context and emotion
    UFUNCTION(BlueprintCallable, Category = "Dialogue|Context")
    void SetDialogueContext(EDialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Context")
    void SetEmotionalTone(ENarrativeTone Tone, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Context")
    void AddContextualTags(const TArray<FGameplayTag>& Tags);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Context")
    void RemoveContextualTags(const TArray<FGameplayTag>& Tags);

    // Query functions
    UFUNCTION(BlueprintPure, Category = "Dialogue|Query")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Query")
    bool IsDialoguePaused() const;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Query")
    FString GetCurrentSequenceID() const { return CurrentSequenceID; }

    UFUNCTION(BlueprintPure, Category = "Dialogue|Query")
    int32 GetCurrentLineIndex() const { return CurrentLineIndex; }

    UFUNCTION(BlueprintPure, Category = "Dialogue|Query")
    float GetDialogueProgress() const;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Query")
    FDialogueLine GetCurrentDialogueLine() const;

    // Advanced features
    UFUNCTION(BlueprintCallable, Category = "Dialogue|Advanced")
    void SetDialogueSpeed(float SpeedMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Advanced")
    void SetAutoAdvance(bool bEnable, float DelayBetweenLines = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Advanced")
    void SetInterruptible(bool bCanInterrupt);

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Advanced")
    void SetSpatialAudio(bool bEnable, float MaxDistance = 2000.0f);

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueLineStarted OnDialogueLineStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueLineCompleted OnDialogueLineCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueSequenceCompleted OnDialogueSequenceCompleted;

    // Configuration properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    TSoftObjectPtr<UDialogueVoice> DefaultDialogueVoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    bool bShowSubtitles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    bool bAutoAdvance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    float AutoAdvanceDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    bool bInterruptible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    bool bUseSpatialAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    float MaxAudioDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Settings")
    TSubclassOf<UUserWidget> SubtitleWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Context")
    EDialogueContext DefaultContext = EDialogueContext::Environmental;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Context")
    TArray<FGameplayTag> ContextualTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Context")
    ENarrativeTone DefaultTone = ENarrativeTone::Neutral;

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> SubtitleWidgetComponent;

    // Current state
    UPROPERTY()
    FString CurrentSequenceID;

    UPROPERTY()
    int32 CurrentLineIndex = 0;

    UPROPERTY()
    FNarrativeSequence CurrentSequence;

    UPROPERTY()
    FDialogueLine CurrentLine;

    UPROPERTY()
    bool bIsPlaying = false;

    UPROPERTY()
    bool bIsPaused = false;

    UPROPERTY()
    float CurrentLineStartTime = 0.0f;

    UPROPERTY()
    float DialogueSpeedMultiplier = 1.0f;

    UPROPERTY()
    ENarrativeTone CurrentEmotionalTone = ENarrativeTone::Neutral;

    UPROPERTY()
    float EmotionalIntensity = 1.0f;

    // Subsystem references
    UPROPERTY()
    TObjectPtr<UNarrativeSubsystem> NarrativeSubsystem;

    // Widget references
    UPROPERTY()
    TObjectPtr<UUserWidget> SubtitleWidget;

    // Timers
    FTimerHandle LineAdvanceTimer;
    FTimerHandle AutoAdvanceTimer;

    // Internal functions
    void InitializeComponents();
    void SetupAudioComponent();
    void SetupSubtitleWidget();
    void PlayNextLine();
    void OnAudioFinished();
    void OnLineCompleted();
    void OnSequenceCompleted();
    void UpdateSubtitles();
    void ApplyEmotionalModifiers();
    void ProcessContextualTags();

    // Audio processing
    void ConfigureDialogueWave(UDialogueWave* DialogueWave, const FDialogueLine& Line);
    void ApplyVoiceModifiers(const FDialogueLine& Line);
    void SetupSpatialAudio();

    // Subtitle management
    void ShowSubtitle(const FText& Text, float Duration);
    void HideSubtitle();
    void UpdateSubtitlePosition();

    // Context processing
    void ProcessEmotionalContext();
    void ProcessEnvironmentalContext();
    void ProcessSituationalContext();

private:
    // Internal state
    bool bComponentsInitialized = false;
    float LastUpdateTime = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;
};