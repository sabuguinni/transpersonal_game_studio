#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../Quest/Quest_DinosaurHuntSystem.h"
#include "../Core/TranspersonalCharacter.h"
#include "NarrativeQuestBridge.generated.h"

UENUM(BlueprintType)
enum class ENarr_QuestNarrativeType : uint8
{
    HuntIntroduction    UMETA(DisplayName = "Hunt Introduction"),
    HuntProgress        UMETA(DisplayName = "Hunt Progress"),
    HuntCompletion      UMETA(DisplayName = "Hunt Completion"),
    HuntFailure         UMETA(DisplayName = "Hunt Failure"),
    TerritoryWarning    UMETA(DisplayName = "Territory Warning"),
    SurvivalAdvice      UMETA(DisplayName = "Survival Advice")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    ENarr_QuestNarrativeType NarrativeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    float DialogueDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    bool bIsUrgent;

    FNarr_QuestNarrative()
    {
        NarrativeType = ENarr_QuestNarrativeType::HuntIntroduction;
        DialogueText = TEXT("");
        SpeakerName = TEXT("Narrator");
        DialogueDuration = 3.0f;
        bIsUrgent = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeQuestBridge : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeQuestBridge();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Bridge")
    TArray<FNarr_QuestNarrative> QuestNarratives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Bridge")
    class UQuest_DinosaurHuntSystem* HuntSystemRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Bridge")
    class ATranspersonalCharacter* PlayerCharacterRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Bridge")
    float NarrativeCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Bridge")
    bool bNarrativeSystemActive;

    FTimerHandle NarrativeTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    void InitializeQuestNarratives();

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    void TriggerHuntNarrative(ENarr_QuestNarrativeType NarrativeType, const FString& HuntTarget);

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    void CheckQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    void PlayNarrative(const FNarr_QuestNarrative& Narrative);

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    FNarr_QuestNarrative GetHuntIntroNarrative(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    FNarr_QuestNarrative GetHuntCompletionNarrative(const FString& DinosaurSpecies, int32 KillCount);

    UFUNCTION(BlueprintCallable, Category = "Quest Bridge")
    void SetHuntSystemReference(UQuest_DinosaurHuntSystem* HuntSystem);
};