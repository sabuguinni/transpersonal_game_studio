#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Narr_StoryProgression.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribalContact   UMETA(DisplayName = "Tribal Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaChallenge  UMETA(DisplayName = "Alpha Challenge"),
    Mastery         UMETA(DisplayName = "Mastery")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CompletionReward;

    FNarr_StoryEvent()
    {
        EventID = TEXT("DefaultEvent");
        EventTitle = FText::FromString(TEXT("Unknown Event"));
        EventDescription = FText::FromString(TEXT("No description"));
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
        CompletionReward = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase IntroductionPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RelatedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsMet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 RelationshipLevel;

    FNarr_CharacterArc()
    {
        CharacterName = TEXT("Unknown");
        IntroductionPhase = ENarr_StoryPhase::Awakening;
        bIsMet = false;
        RelationshipLevel = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_StoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_StoryComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_CharacterArc> CharacterArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float StoryProgressPercentage;

public:
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceToPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CanAccessEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void IntroduceCharacter(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdateRelationship(const FString& CharacterName, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Story")
    float CalculateStoryProgress();
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY()
    ENarr_StoryPhase GlobalStoryPhase;

    UPROPERTY()
    TArray<FString> GlobalCompletedEvents;

    UPROPERTY()
    TMap<FString, int32> GlobalCharacterRelationships;

public:
    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetGlobalPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetGlobalPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterGlobalEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventGloballyCompleted(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdateGlobalRelationship(const FString& CharacterName, int32 NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetGlobalRelationship(const FString& CharacterName);
};