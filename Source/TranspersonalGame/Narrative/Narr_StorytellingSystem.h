#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Narr_StorytellingSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryType : uint8
{
    None            UMETA(DisplayName = "None"),
    Tribal_Legend   UMETA(DisplayName = "Tribal Legend"),
    Hunt_Tale       UMETA(DisplayName = "Hunt Tale"),
    Warning_Story   UMETA(DisplayName = "Warning Story"),
    Territory_Lore  UMETA(DisplayName = "Territory Lore"),
    Survival_Wisdom UMETA(DisplayName = "Survival Wisdom"),
    Beast_Knowledge UMETA(DisplayName = "Beast Knowledge")
};

UENUM(BlueprintType)
enum class ENarr_NarratorType : uint8
{
    None            UMETA(DisplayName = "None"),
    Tribal_Elder    UMETA(DisplayName = "Tribal Elder"),
    Scout_Warrior   UMETA(DisplayName = "Scout Warrior"),
    Hunter_Veteran  UMETA(DisplayName = "Hunter Veteran"),
    Tracker_Guide   UMETA(DisplayName = "Tracker Guide"),
    Crafter_Wise    UMETA(DisplayName = "Crafter Wise"),
    Territory_Guard UMETA(DisplayName = "Territory Guard")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryType StoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_NarratorType NarratorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float StoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsContextual;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    FNarr_StoryData()
    {
        StoryTitle = TEXT("");
        StoryText = TEXT("");
        StoryType = ENarr_StoryType::None;
        NarratorType = ENarr_NarratorType::None;
        AudioAssetPath = TEXT("");
        StoryDuration = 0.0f;
        bIsContextual = false;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_StorytellingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_StorytellingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FNarr_StoryData> AvailableStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FNarr_StoryData CurrentStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bIsPlayingStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StoryPlaybackTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    AActor* PlayerReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float ContextualTriggerDistance;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StartStory(const FNarr_StoryData& Story);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StopCurrentStory();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool IsStoryPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData GetRandomStoryByType(ENarr_StoryType StoryType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddStoryToLibrary(const FNarr_StoryData& NewStory);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void TriggerContextualStory(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void InitializeDefaultStories();

protected:
    UFUNCTION()
    void OnStoryComplete();

    UFUNCTION()
    void CheckContextualTriggers(FVector PlayerLocation);
};