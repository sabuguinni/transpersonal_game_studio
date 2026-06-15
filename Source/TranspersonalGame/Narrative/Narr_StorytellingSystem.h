#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_StorytellingSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryType : uint8
{
    DinosaurEncounter,
    TribalWisdom,
    HuntingStory,
    SurvivalTip,
    TerrainWarning,
    WeatherLore
};

USTRUCT(BlueprintType)
struct FNarr_StoryData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText StoryText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryType StoryType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    float StoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> RequiredTags;

    FNarr_StoryData()
    {
        StoryTitle = TEXT("Untitled Story");
        StoryText = FText::FromString(TEXT("A tale of survival..."));
        StoryType = ENarr_StoryType::TribalWisdom;
        AudioAssetPath = TEXT("");
        StoryDuration = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_StorytellingContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector PlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> NearbyDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bIsNearFire;

    FNarr_StorytellingContext()
    {
        PlayerLocation = FVector::ZeroVector;
        TimeOfDay = 12.0f;
        CurrentBiome = TEXT("Forest");
        bIsNearFire = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StorytellingSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StorytellingSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void TriggerContextualStory(const FNarr_StorytellingContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData GetRandomStoryByType(ENarr_StoryType StoryType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FNarr_StoryData> GetStoriesForContext(const FNarr_StorytellingContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlayStoryAudio(const FNarr_StoryData& Story);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool IsStoryPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StopCurrentStory();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    class UDataTable* StoryDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingStory;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_StoryData CurrentStory;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float StoryStartTime;

private:
    void LoadStoryData();
    bool DoesStoryMatchContext(const FNarr_StoryData& Story, const FNarr_StorytellingContext& Context);
    void OnStoryFinished();

    UPROPERTY()
    TArray<FNarr_StoryData> CachedStories;
};