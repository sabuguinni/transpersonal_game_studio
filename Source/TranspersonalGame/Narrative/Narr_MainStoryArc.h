#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Narr_MainStoryArc.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryAct : uint8
{
    Act1_Arrival        UMETA(DisplayName = "Act 1: Arrival"),
    Act2_Discovery      UMETA(DisplayName = "Act 2: Discovery"),
    Act3_Conflict       UMETA(DisplayName = "Act 3: Conflict"),
    Act4_Resolution     UMETA(DisplayName = "Act 4: Resolution"),
    Act5_Legacy         UMETA(DisplayName = "Act 5: Legacy"),
    None                UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class ENarr_CharacterArchetype : uint8
{
    Survivor            UMETA(DisplayName = "Survivor"),
    Hunter              UMETA(DisplayName = "Hunter"),
    Gatherer            UMETA(DisplayName = "Gatherer"),
    Scout               UMETA(DisplayName = "Scout"),
    Crafter             UMETA(DisplayName = "Crafter"),
    Healer              UMETA(DisplayName = "Healer"),
    Elder               UMETA(DisplayName = "Elder"),
    Leader              UMETA(DisplayName = "Leader")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString BeatName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    ENarr_StoryAct Act;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FString> UnlockedContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    float CompletionPercentage;

    FNarr_StoryBeat()
    {
        BeatName = TEXT("");
        Act = ENarr_StoryAct::None;
        Description = TEXT("");
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> SkillSpecialties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CurrentMotivation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsAvailable;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("");
        Archetype = ENarr_CharacterArchetype::Survivor;
        BackgroundStory = TEXT("");
        CurrentMotivation = TEXT("");
        TrustLevel = 0.0f;
        bIsAvailable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_MainStoryArc : public AActor
{
    GENERATED_BODY()

public:
    ANarr_MainStoryArc();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Story Progression System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    ENarr_StoryAct CurrentAct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    float OverallProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    bool bStoryCompleted;

    // Character Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
    TArray<FNarr_CharacterProfile> MainCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
    FString PlayerCharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
    ENarr_CharacterArchetype PlayerArchetype;

    // Narrative Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Config")
    bool bEnableVoiceActing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Config")
    bool bEnableSubtitles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Config")
    float DialogueSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Config")
    bool bAutoAdvanceDialogue;

    // Story Progression Functions
    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void AdvanceToNextAct();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void CompleteStoryBeat(const FString& BeatName);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool IsStoryBeatCompleted(const FString& BeatName) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    float GetActProgress(ENarr_StoryAct Act) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetAvailableStoryBeats() const;

    // Character Management Functions
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void AddCharacter(const FNarr_CharacterProfile& Character);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void UpdateCharacterTrust(const FString& CharacterName, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    TArray<FString> GetAvailableCharacters() const;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SetCharacterAvailability(const FString& CharacterName, bool bAvailable);

    // Narrative Events
    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void TriggerStoryEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void UpdatePlayerArchetype(ENarr_CharacterArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void RecalculateStoryProgress();

private:
    void InitializeStoryBeats();
    void InitializeMainCharacters();
    void UpdateStoryProgression();
    bool CheckStoryBeatConditions(const FNarr_StoryBeat& Beat) const;
    void UnlockStoryContent(const FNarr_StoryBeat& Beat);
};