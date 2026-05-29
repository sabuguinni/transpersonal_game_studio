#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeEventSystem.generated.h"

class UNarr_DialogueComponent;
class ANarr_StoryTrigger;

UENUM(BlueprintType)
enum class ENarr_EventType : uint8
{
    FirstDinosaurSighting,
    FirstKill,
    FirstCraft,
    FirstFire,
    FirstShelter,
    DangerWarning,
    TutorialPrompt,
    Discovery,
    CombatStart,
    CombatEnd,
    QuestComplete,
    AreaEnter,
    AreaExit,
    ResourceFound,
    WeatherChange,
    DayNightTransition
};

UENUM(BlueprintType)
enum class ENarr_NarratorType : uint8
{
    InternalThought,    // Player's inner voice
    AncestralWisdom,    // Ancestral knowledge
    EnvironmentalCue,   // Environmental storytelling
    TribalElder,        // Wise elder voice
    CombatInstructor,   // Combat guidance
    SurvivalGuide       // Survival tips
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    ENarr_EventType EventType;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    FString EventDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    FVector EventLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    AActor* TriggeringActor;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    float EventIntensity;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    bool bIsFirstTime;

    FNarr_EventData()
    {
        EventType = ENarr_EventType::Discovery;
        EventDescription = TEXT("");
        EventLocation = FVector::ZeroVector;
        TriggeringActor = nullptr;
        EventIntensity = 1.0f;
        bIsFirstTime = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    ENarr_NarratorType NarratorType;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    FString NarrativeText;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    FString AudioFilePath;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    float DelayBeforePlay;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    float AudioVolume;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    bool bShowSubtitles;

    FNarr_NarrativeResponse()
    {
        NarratorType = ENarr_NarratorType::InternalThought;
        NarrativeText = TEXT("");
        AudioFilePath = TEXT("");
        DelayBeforePlay = 0.0f;
        AudioVolume = 1.0f;
        bShowSubtitles = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEvent, const FNarr_EventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeResponse, const FNarr_EventData&, EventData, const FNarr_NarrativeResponse&, Response);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeEventSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeEventSystem();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Event triggering
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_EventType EventType, const FVector& Location, AActor* TriggeringActor = nullptr, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerCustomEvent(const FString& EventDescription, const FVector& Location, AActor* TriggeringActor = nullptr);

    // Event history
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasEventOccurred(ENarr_EventType EventType) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetEventCount(ENarr_EventType EventType) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkEventAsOccurred(ENarr_EventType EventType);

    // Narrative response system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeResponse(const FNarr_NarrativeResponse& Response, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_NarrativeResponse GenerateResponseForEvent(const FNarr_EventData& EventData);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarration();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrationPlaying() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetSubtitlesEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetMasterVolume(float Volume);

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeEvent OnNarrativeEvent;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeResponse OnNarrativeResponse;

protected:
    // Event tracking
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<ENarr_EventType, int32> EventCounts;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_EventData> EventHistory;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    UAudioComponent* NarrationAudioComponent;

    // Configuration
    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    bool bNarrativeEnabled;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    bool bSubtitlesEnabled;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    float MasterVolume;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    float MaxNarrationDistance;

    // Response templates
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<ENarr_EventType, TArray<FNarr_NarrativeResponse>> ResponseTemplates;

private:
    void InitializeResponseTemplates();
    void ProcessEventQueue();
    FNarr_NarrativeResponse SelectBestResponse(const FNarr_EventData& EventData);
    void PlayAudioAtLocation(const FString& AudioPath, const FVector& Location, float Volume);

    FTimerHandle EventProcessingTimer;
    TArray<FNarr_EventData> PendingEvents;
    bool bIsProcessingEvents;
};