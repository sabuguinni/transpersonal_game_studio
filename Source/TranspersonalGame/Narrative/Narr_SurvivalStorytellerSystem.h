#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_SurvivalStorytellerSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_SurvivalEventType : uint8
{
    None = 0,
    DinosaurSighting,
    WeatherChange,
    ResourceDiscovery,
    TerritorialDispute,
    HuntSuccess,
    HuntFailure,
    CampThreat,
    AllyEncounter,
    EnvironmentalDanger,
    SeasonalMigration
};

UENUM(BlueprintType)
enum class ENarr_StoryTone : uint8
{
    Neutral = 0,
    Tense,
    Hopeful,
    Desperate,
    Triumphant,
    Ominous,
    Cautious,
    Urgent
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    ENarr_SurvivalEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    ENarr_StoryTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    float EventDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    FVector EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Event")
    bool bRequiresPlayerAction;

    FNarr_SurvivalEvent()
    {
        EventType = ENarr_SurvivalEventType::None;
        EventTitle = TEXT("");
        EventDescription = TEXT("");
        Tone = ENarr_StoryTone::Neutral;
        EventDuration = 30.0f;
        EventLocation = FVector::ZeroVector;
        bRequiresPlayerAction = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StorytellerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller Config")
    float EventFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller Config")
    float TensionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller Config")
    int32 MaxActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller Config")
    bool bAdaptToPlayerActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller Config")
    float NarrativeRadius;

    FNarr_StorytellerConfig()
    {
        EventFrequency = 120.0f;
        TensionLevel = 0.5f;
        MaxActiveEvents = 3;
        bAdaptToPlayerActions = true;
        NarrativeRadius = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_SurvivalStorytellerSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_SurvivalStorytellerSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    FNarr_StorytellerConfig Config;

    UPROPERTY(BlueprintReadOnly, Category = "Storyteller")
    TArray<FNarr_SurvivalEvent> ActiveEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Storyteller")
    float CurrentTension;

    UPROPERTY(BlueprintReadOnly, Category = "Storyteller")
    float TimeSinceLastEvent;

    UFUNCTION(BlueprintCallable, Category = "Storyteller")
    void TriggerSurvivalEvent(ENarr_SurvivalEventType EventType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Storyteller")
    void CompleteEvent(int32 EventIndex);

    UFUNCTION(BlueprintCallable, Category = "Storyteller")
    FNarr_SurvivalEvent CreateRandomEvent();

    UFUNCTION(BlueprintCallable, Category = "Storyteller")
    void UpdateTensionLevel(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Storyteller")
    bool ShouldTriggerEvent() const;

    UFUNCTION(BlueprintCallable, Category = "Storyteller")
    FString GetEventNarration(const FNarr_SurvivalEvent& Event) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Storyteller")
    void OnSurvivalEventTriggered(const FNarr_SurvivalEvent& Event);

    UFUNCTION(BlueprintImplementableEvent, Category = "Storyteller")
    void OnEventCompleted(const FNarr_SurvivalEvent& Event);

private:
    void ProcessActiveEvents(float DeltaTime);
    void CleanupExpiredEvents();
    ENarr_SurvivalEventType SelectEventBasedOnContext();
    float CalculateEventProbability(ENarr_SurvivalEventType EventType) const;
};