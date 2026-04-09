// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NarrativeTypes.generated.h"

UENUM(BlueprintType)
enum class ENarrativeEventType : uint8
{
    Discovery,
    Survival,
    Domestication,
    Threat,
    Memory,
    Revelation,
    Connection
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Wonder,
    Fear,
    Hope,
    Desperation,
    Awe,
    Loneliness,
    Determination,
    Peace
};

UENUM(BlueprintType)
enum class EDialogueContext : uint8
{
    FirstEncounter,
    Friendly,
    Hostile,
    Neutral,
    Fearful,
    Curious,
    Protective
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeEvent : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarrativeEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EEmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTagContainer GrantedTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float EmotionalImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsRepeatable;

    FNarrativeEvent()
    {
        EventType = ENarrativeEventType::Discovery;
        EmotionalTone = EEmotionalTone::Wonder;
        EmotionalImpact = 1.0f;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEmotionalTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceActorDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTagContainer RequiredConditions;

    FDialogueLine()
    {
        Context = EDialogueContext::Neutral;
        Tone = EEmotionalTone::Wonder;
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarrativeEventType BeatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    EEmotionalTone EmotionalArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> TriggeredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float StoryWeight;

    FStoryBeat()
    {
        BeatType = ENarrativeEventType::Discovery;
        EmotionalArc = EEmotionalTone::Wonder;
        StoryWeight = 1.0f;
    }
};