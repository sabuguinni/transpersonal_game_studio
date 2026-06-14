#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_StorytellingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 TribeReputationRequired;

    FNarr_StoryData()
    {
        StoryTitle = TEXT("");
        StoryText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        TribeReputationRequired = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CampfireSession
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire")
    TArray<FNarr_StoryData> AvailableStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire")
    int32 CurrentStoryIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire")
    float SessionStartTime;

    FNarr_CampfireSession()
    {
        CurrentStoryIndex = 0;
        bIsActive = false;
        SessionStartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StorytellingSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StorytellingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core storytelling components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* CampfireMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* FireEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionRadius;

    // Story data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FNarr_StoryData> TribalStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FNarr_CampfireSession CurrentSession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    int32 MaxParticipants;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StartStorytellingSession();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void EndStorytellingSession();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlayNextStory();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool CanPlayerJoinSession(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddPlayerToSession(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void RemovePlayerFromSession(class APawn* Player);

    // Story management
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void LoadTribalStories();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData GetRandomStory();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FNarr_StoryData> GetStoriesForReputation(int32 ReputationLevel);

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlayStoryAudio(const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StopCurrentAudio();

private:
    // Internal tracking
    TArray<class APawn*> SessionParticipants;
    float CurrentStoryStartTime;
    bool bIsPlayingStory;

    void InitializeStoryDatabase();
    void UpdateSessionState();
    bool IsPlayerInRange(class APawn* Player);
};