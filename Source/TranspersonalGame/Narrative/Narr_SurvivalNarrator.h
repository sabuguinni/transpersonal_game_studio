#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Narr_SurvivalNarrator.generated.h"

UENUM(BlueprintType)
enum class ENarr_SurvivalEvent : uint8
{
    None = 0,
    FirstHunt,
    PackEncounter,
    WaterFound,
    ShelterBuilt,
    FireDiscovered,
    TerritoryMarked,
    AlphaChallenge,
    StormSurvived
};

USTRUCT(BlueprintType)
struct FNarr_SurvivalMoment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ENarr_SurvivalEvent EventType = ENarr_SurvivalEvent::None;

    UPROPERTY(BlueprintReadOnly)
    FString NarrativeText;

    UPROPERTY(BlueprintReadOnly)
    float Intensity = 1.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsTriggered = false;

    FNarr_SurvivalMoment()
    {
        EventType = ENarr_SurvivalEvent::None;
        NarrativeText = TEXT("");
        Intensity = 1.0f;
        bIsTriggered = false;
    }
};

UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_SurvivalNarrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_SurvivalNarrator();

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void TriggerSurvivalEvent(ENarr_SurvivalEvent EventType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    FString GetCurrentNarrative() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    bool HasActiveNarrative() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void ClearCurrentNarrative();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Narrative")
    FNarr_SurvivalMoment CurrentMoment;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Narrative")
    TArray<FNarr_SurvivalMoment> EventHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Narrative")
    float NarrativeDuration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Narrative")
    bool bAutoTriggerEvents = true;

private:
    void InitializeSurvivalEvents();
    FString GenerateNarrativeForEvent(ENarr_SurvivalEvent EventType, float Intensity);
    
    UPROPERTY()
    TMap<ENarr_SurvivalEvent, FString> EventNarratives;

    FTimerHandle NarrativeClearTimer;
};