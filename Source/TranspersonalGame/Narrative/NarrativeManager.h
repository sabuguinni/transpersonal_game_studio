#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_NarrativeEvent : uint8
{
    PlayerSpawn = 0,
    FirstDinosaurSighting,
    ResourceDiscovery,
    DangerEncounter,
    SurvivalMilestone,
    AreaTransition,
    TimeOfDay,
    WeatherChange
};

USTRUCT(BlueprintType)
struct FNarr_NarrativeTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarrativeEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioClipPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneTimeEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_NarrativeTrigger()
    {
        EventType = ENarr_NarrativeEvent::PlayerSpawn;
        AudioClipPath = TEXT("");
        NarrativeText = FText::FromString(TEXT(""));
        TriggerRadius = 500.0f;
        bIsOneTimeEvent = true;
        bHasBeenTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_NarrativeTrigger> NarrativeTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    class UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float NarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bNarrativeEnabled;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeTrigger(const FNarr_NarrativeTrigger& NewTrigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeEnabled(bool bEnabled);

protected:
    UFUNCTION()
    void CheckProximityTriggers(const FVector& PlayerLocation);

    UFUNCTION()
    void OnNarrativeAudioFinished();

private:
    float LastProximityCheck;
    const float ProximityCheckInterval = 1.0f;
};