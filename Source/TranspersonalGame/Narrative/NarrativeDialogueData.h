#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueData.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260620_007
// All 8 quest zone voice briefs defined and linked to TTS URLs.
// ============================================================

UENUM(BlueprintType)
enum class ENarr_QuestZoneID : uint8
{
    WaterHole       UMETA(DisplayName = "Hunt: Water Hole"),
    ForestEdge      UMETA(DisplayName = "Gather: Forest Edge"),
    RockShelter     UMETA(DisplayName = "Defend: Rock Shelter"),
    Clearing        UMETA(DisplayName = "Explore: Clearing"),
    Hilltop         UMETA(DisplayName = "Survive: Hilltop"),
    RiverBank       UMETA(DisplayName = "Gather: River Bank"),
    Cave            UMETA(DisplayName = "Explore: Cave"),
    HuntingGround   UMETA(DisplayName = "Hunt: Hunting Ground")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString VoiceAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestZoneID LinkedQuestZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float EstimatedDurationSeconds;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("ElderNPC"))
        , DialogueText(TEXT(""))
        , VoiceAudioURL(TEXT(""))
        , LinkedQuestZone(ENarr_QuestZoneID::WaterHole)
        , EstimatedDurationSeconds(10.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestVoiceBriefTable
{
    GENERATED_BODY()

    // All 8 quest zone voice briefs — populated from ElevenLabs TTS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|VoiceBriefs")
    TArray<FNarr_DialogueLine> AllBriefs;

    FNarr_QuestVoiceBriefTable() {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    // All 8 quest zone voice briefs — keyed by ENarr_QuestZoneID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<uint8, FNarr_DialogueLine> QuestZoneDialogueMap;

    // Retrieve dialogue line for a given quest zone
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetDialogueForZone(ENarr_QuestZoneID ZoneID) const;

    // Play dialogue line (logs URL for audio system to pick up)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueForZone(ENarr_QuestZoneID ZoneID);

    // Populate all 8 quest zone briefs from TTS asset URLs
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void InitialiseAllQuestBriefs();

protected:
    virtual void BeginPlay() override;

private:
    void RegisterBrief(ENarr_QuestZoneID Zone, const FString& Speaker,
                       const FString& Text, const FString& AudioURL, float Duration);
};
