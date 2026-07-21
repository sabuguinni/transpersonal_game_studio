#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,      // Player starts alone, learns survival basics
    FirstContact,   // Encounters other survivors, forms alliances
    TribalGrowth,   // Builds community, establishes territory
    GreatHunt,      // Major dinosaur threats, epic confrontations
    Legacy          // End game, establishing permanent settlement
};

USTRUCT(BlueprintType)
struct FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExperienceReward;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = TEXT("");
        EventDescription = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
        ExperienceReward = 0;
    }
};

USTRUCT(BlueprintType)
struct FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlive;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("");
        CharacterRole = TEXT("");
        BackgroundStory = TEXT("");
        TrustLevel = 0.0f;
        bIsAlive = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FString& EventID) const;

    // Character management
    UFUNCTION(BlueprintCallable, Category = "Characters")
    void RegisterCharacter(const FNarr_CharacterProfile& Character);

    UFUNCTION(BlueprintCallable, Category = "Characters")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Characters")
    void UpdateCharacterTrust(const FString& CharacterName, float TrustDelta);

    // Narrative context
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentNarrativeContext() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogueOptions(const FString& CharacterName) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characters")
    TArray<FNarr_CharacterProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    TArray<FString> NarrativeContexts;

private:
    void InitializeStoryEvents();
    void InitializeCharacterProfiles();
    bool CheckEventPrerequisites(const FNarr_StoryEvent& Event) const;
};