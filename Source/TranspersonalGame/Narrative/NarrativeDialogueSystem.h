#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Scout       UMETA(DisplayName = "Tribe Scout"),
    Elder       UMETA(DisplayName = "Tribe Elder"),
    Crafter     UMETA(DisplayName = "Crafting Master"),
    Leader      UMETA(DisplayName = "Tribe Leader"),
    Unknown     UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity   UMETA(DisplayName = "Proximity"),
    QuestStart  UMETA(DisplayName = "Quest Start"),
    QuestEnd    UMETA(DisplayName = "Quest End"),
    Danger      UMETA(DisplayName = "Danger Nearby"),
    Resource    UMETA(DisplayName = "Resource Found")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    FNarr_DialogueLine()
        : LineText(TEXT(""))
        , Speaker(ENarr_DialogueSpeaker::Unknown)
        , DisplayDuration(5.0f)
        , AudioURL(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCanRepeat;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , TriggerType(ENarr_DialogueTriggerType::Proximity)
        , bHasPlayed(false)
        , bCanRepeat(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueTriggerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueTriggerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTrigger() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

protected:
    virtual void BeginPlay() override;

private:
    bool bTriggered;
};

UCLASS(ClassGroup = (Narrative), BlueprintType)
class TRANSPERSONALGAME_API ANarr_DialogueZoneActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    UNarr_DialogueTriggerComponent* DialogueTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker AssignedSpeaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> DialogueLines;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterZone();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetNextLine();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    int32 CurrentLineIndex;
    float LineTimer;
    bool bIsPlaying;
};
