#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "StorytellingSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryType : uint8
{
    Origin,
    Warning,
    Prophecy,
    Victory,
    Tragedy,
    Teaching,
    Legend
};

UENUM(BlueprintType)
enum class ENarr_StoryMood : uint8
{
    Hopeful,
    Fearful,
    Mysterious,
    Triumphant,
    Somber,
    Urgent
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryType StoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryMood Mood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsUnlocked;

    FNarr_StoryData()
    {
        StoryID = TEXT("");
        Title = TEXT("");
        Content = TEXT("");
        StoryType = ENarr_StoryType::Teaching;
        Mood = ENarr_StoryMood::Hopeful;
        AudioURL = TEXT("");
        Duration = 0.0f;
        CharacterName = TEXT("");
        bIsUnlocked = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StorytellerProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    FString StorytellerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    TArray<FString> KnownStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    TArray<ENarr_StoryType> PreferredTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyteller")
    bool bIsAvailable;

    FNarr_StorytellerProfile()
    {
        StorytellerID = TEXT("");
        Name = TEXT("");
        Role = TEXT("");
        TrustLevel = 0.5f;
        bIsAvailable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStorytellingSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story Management
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void RegisterStory(const FNarr_StoryData& StoryData);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData GetStoryByID(const FString& StoryID);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FNarr_StoryData> GetStoriesByType(ENarr_StoryType StoryType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FNarr_StoryData> GetUnlockedStories();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool UnlockStory(const FString& StoryID);

    // Storyteller Management
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void RegisterStoryteller(const FNarr_StorytellerProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StorytellerProfile GetStorytellerByID(const FString& StorytellerID);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FNarr_StorytellerProfile> GetAvailableStorytellers();

    // Story Playback
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlayStory(const FString& StoryID, AActor* ListenerActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StopCurrentStory();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool IsStoryPlaying() const;

    // Story Selection AI
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData SelectAppropriateStory(ENarr_StoryMood DesiredMood, const FString& StorytellerID = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData SelectRandomStory(ENarr_StoryType PreferredType = ENarr_StoryType::Teaching);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryStarted, const FNarr_StoryData&, Story);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryFinished, const FNarr_StoryData&, Story);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryUnlocked, const FNarr_StoryData&, Story);

    UPROPERTY(BlueprintAssignable, Category = "Storytelling")
    FOnStoryStarted OnStoryStarted;

    UPROPERTY(BlueprintAssignable, Category = "Storytelling")
    FOnStoryFinished OnStoryFinished;

    UPROPERTY(BlueprintAssignable, Category = "Storytelling")
    FOnStoryUnlocked OnStoryUnlocked;

protected:
    UPROPERTY()
    TMap<FString, FNarr_StoryData> Stories;

    UPROPERTY()
    TMap<FString, FNarr_StorytellerProfile> Storytellers;

    UPROPERTY()
    UAudioComponent* CurrentAudioComponent;

    UPROPERTY()
    FNarr_StoryData CurrentStory;

    UPROPERTY()
    bool bIsPlaying;

    // Internal methods
    void InitializeDefaultStories();
    void InitializeDefaultStorytellers();
    void OnAudioFinished();
    bool CheckUnlockConditions(const FNarr_StoryData& Story);
};