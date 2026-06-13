#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "Narr_StoryTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    OpeningNarrative    UMETA(DisplayName = "Opening Narrative"),
    DinosaurEncounter   UMETA(DisplayName = "Dinosaur Encounter"),
    LocationDiscovery   UMETA(DisplayName = "Location Discovery"),
    QuestProgression    UMETA(DisplayName = "Quest Progression"),
    DangerWarning       UMETA(DisplayName = "Danger Warning"),
    LoreReveal          UMETA(DisplayName = "Lore Reveal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_TriggerType TriggerType;

    FNarr_NarrativeEvent()
    {
        EventName = TEXT("");
        NarrativeText = TEXT("");
        AudioFilePath = TEXT("");
        DisplayDuration = 5.0f;
        bPlayOnce = true;
        TriggerType = ENarr_TriggerType::OpeningNarrative;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_StoryTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NarrativeEvent NarrativeEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bRequiresLineOfSight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CooldownTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* NarrativeAudio;

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(class APawn* TriggeringPawn);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnNarrativeTriggered(const FString& EventName, const FString& NarrativeText);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnNarrativeCompleted();

protected:
    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

private:
    bool bHasTriggered;
    float LastTriggerTime;
    
    void InitializeDefaultNarratives();
    bool CheckLineOfSight(APawn* PlayerPawn);
    void PlayNarrativeAudio();
};