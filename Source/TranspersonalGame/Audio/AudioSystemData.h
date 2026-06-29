#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AudioSystemData.generated.h"

// ============================================================
// AudioSystemData.h — Agent #16 Audio Agent
// Cycle: PROD_CYCLE_AUTO_20260629_011
// Quest ambient audio + dialogue cue data structures
// ============================================================

UENUM(BlueprintType)
enum class EAudio_QuestZone : uint8
{
    None        UMETA(DisplayName = "None"),
    Hunt        UMETA(DisplayName = "Hunt Zone"),
    Gather      UMETA(DisplayName = "Gather Zone"),
    Defend      UMETA(DisplayName = "Defend Zone"),
};

UENUM(BlueprintType)
enum class EAudio_DialogueSpeaker : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    HunterBrek  UMETA(DisplayName = "Hunter Brek"),
    Player      UMETA(DisplayName = "Player"),
};

USTRUCT(BlueprintType)
struct FAudio_DialogueCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    EAudio_DialogueSpeaker Speaker = EAudio_DialogueSpeaker::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DisplayDuration = 4.0f;

    FAudio_DialogueCue() {}
    FAudio_DialogueCue(EAudio_DialogueSpeaker InSpeaker, const FString& InText, const FString& InURL, float InDuration = 4.0f)
        : Speaker(InSpeaker), DialogueText(InText), AudioURL(InURL), DisplayDuration(InDuration)
    {}
};

USTRUCT(BlueprintType)
struct FAudio_QuestAudioSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Quest")
    EAudio_QuestZone QuestZone = EAudio_QuestZone::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Quest")
    TArray<FAudio_DialogueCue> BriefingLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Quest")
    TArray<FAudio_DialogueCue> ProgressLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Quest")
    TArray<FAudio_DialogueCue> CompletionLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Quest")
    TArray<FAudio_DialogueCue> FailureLines;

    // Freesound ambient IDs for this zone (river, camp, jungle)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Quest")
    TArray<int32> FreesoundAmbientIDs;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_QuestAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_QuestAudioManager();

    // All quest audio sets indexed by zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_QuestAudioSet> QuestAudioSets;

    // Get audio set for a given quest zone
    UFUNCTION(BlueprintCallable, Category = "Audio")
    FAudio_QuestAudioSet GetQuestAudioSet(EAudio_QuestZone Zone) const;

    // Get briefing lines for a zone
    UFUNCTION(BlueprintCallable, Category = "Audio")
    TArray<FAudio_DialogueCue> GetBriefingLines(EAudio_QuestZone Zone) const;

    // Get progress lines for a zone
    UFUNCTION(BlueprintCallable, Category = "Audio")
    TArray<FAudio_DialogueCue> GetProgressLines(EAudio_QuestZone Zone) const;

    // Get completion lines for a zone
    UFUNCTION(BlueprintCallable, Category = "Audio")
    TArray<FAudio_DialogueCue> GetCompletionLines(EAudio_QuestZone Zone) const;

    // Get failure lines for a zone
    UFUNCTION(BlueprintCallable, Category = "Audio")
    TArray<FAudio_DialogueCue> GetFailureLines(EAudio_QuestZone Zone) const;

private:
    void InitializeQuestAudio();
};
