#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Narr_TribalStorytellingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StorySegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarratorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredSurvivalSkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsLegendaryTale;

    FNarr_StorySegment()
    {
        StoryTitle = TEXT("Untold Tale");
        NarratorName = TEXT("Unknown Elder");
        StoryContent = TEXT("");
        EmotionalIntensity = 0.5f;
        bIsLegendaryTale = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TribalCharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 SurvivalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> Specializations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FNarr_StorySegment> KnownStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsTribalElder;

    FNarr_TribalCharacterProfile()
    {
        CharacterName = TEXT("Unknown Survivor");
        Age = 25;
        SurvivalExperience = 5;
        PersonalityTraits = TEXT("Cautious, Practical");
        TrustLevel = 0.5f;
        bIsTribalElder = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_TribalStorytellingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_TribalStorytellingComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FNarr_StorySegment> AvailableStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FNarr_TribalCharacterProfile CharacterProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bCanTellStories;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StartStorytelling();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void EndStorytelling();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StorySegment GetRandomStory();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool CanPlayerHearStory(AActor* PlayerActor);

protected:
    virtual void BeginPlay() override;

private:
    bool bIsCurrentlyTellingStory;
    float LastStoryTime;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_TribalStorytellingSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_TribalStorytellingSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    TArray<FNarr_TribalCharacterProfile> TribalMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    FVector CampfireLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    float GatheringRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    bool bStorytellingActive;

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void InitializeTribalCharacters();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void StartTribalGathering();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void EndTribalGathering();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FNarr_TribalCharacterProfile GetElderKava();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FNarr_TribalCharacterProfile GetScoutThane();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FNarr_TribalCharacterProfile GetCraftMasterNira();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void CreateTribalCharacters();
    void UpdateStorytellingState();
    float LastGatheringTime;
};