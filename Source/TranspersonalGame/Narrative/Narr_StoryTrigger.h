#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "Narr_StoryTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    CaveDiscovery,
    HuntingGround,
    TribalMemory,
    DangerWarning,
    SafeZone
};

USTRUCT(BlueprintType)
struct FNarr_StoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bRequiresPlayerInput;

    FNarr_StoryData()
    {
        StoryTitle = TEXT("Untitled Story");
        NarrativeText = TEXT("An ancient tale unfolds...");
        CharacterName = TEXT("Narrator");
        TriggerRadius = 200.0f;
        bOneTimeOnly = true;
        bRequiresPlayerInput = false;
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
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_StoryData StoryData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* NarrativeSound;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHasBeenTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastTriggerTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeAudio();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryData(const FNarr_StoryData& NewStoryData);

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool CanTrigger() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnStoryTriggered(const FNarr_StoryData& Story);

protected:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};