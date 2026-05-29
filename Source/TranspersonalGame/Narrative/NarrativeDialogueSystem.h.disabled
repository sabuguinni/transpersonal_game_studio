#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

// Narrative dialogue entry for dynamic storytelling
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType TriggerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsDiscoveryDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEmergencyDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = "Narrator";
        DialogueText = "";
        AudioURL = "";
        TriggerBiome = EEng_BiomeType::Forest;
        bIsDiscoveryDialogue = false;
        bIsEmergencyDialogue = false;
        CooldownTime = 30.0f;
    }
};

// Narrative context for dynamic dialogue selection
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<EEng_DinosaurSpecies> NearbyDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bPlayerInDanger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bRecentDiscovery;

    FNarr_NarrativeContext()
    {
        CurrentBiome = EEng_BiomeType::Forest;
        CurrentTimeOfDay = EEng_TimeOfDay::Morning;
        CurrentWeather = EEng_WeatherType::Clear;
        ThreatLevel = EEng_ThreatLevel::Safe;
        PlayerHealthPercentage = 100.0f;
        bPlayerInDanger = false;
        bRecentDiscovery = false;
    }
};

/**
 * Dynamic Narrative and Dialogue System
 * Manages contextual storytelling, environmental narration, and survival guidance
 * Provides immersive audio feedback based on player actions and world state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueSystem();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, const FNarr_NarrativeContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualNarration(const FNarr_NarrativeContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEmergencyAlert(EEng_ThreatLevel ThreatLevel, const FString& ThreatDescription);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDiscoveryNarration(EEng_BiomeType Biome, const FString& DiscoveryType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeTrigger(ATriggerBox* TriggerBox, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FString& AudioURL, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsDialoguePlaying() const;

    // Dialogue database management
    UFUNCTION(BlueprintCallable, Category = "Database")
    void LoadDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Database")
    void AddDialogueEntry(const FNarr_DialogueEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Database")
    FNarr_DialogueEntry GetDialogueEntry(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Database")
    TArray<FNarr_DialogueEntry> GetContextualDialogues(const FNarr_NarrativeContext& Context) const;

protected:
    // Dialogue database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Database")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    // Current narrative state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    FNarr_NarrativeContext CurrentContext;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    float LastDialogueTime;

    // Audio management
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MinDialogueInterval;

    // Registered triggers
    UPROPERTY(BlueprintReadOnly, Category = "Triggers")
    TMap<ATriggerBox*, FString> RegisteredTriggers;

private:
    // Internal helper functions
    void InitializeDialogueDatabase();
    void CreateDefaultDialogues();
    FNarr_DialogueEntry SelectBestDialogue(const TArray<FNarr_DialogueEntry>& Candidates) const;
    bool CanPlayDialogue() const;
    void OnDialogueFinished();

    // Timer handles
    FTimerHandle DialogueCooldownTimer;
};