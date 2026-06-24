#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "NarrDialogueTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialoguePriority : uint8
{
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    Proximity   UMETA(DisplayName = "Proximity"),
    Event       UMETA(DisplayName = "Event"),
    TimeOfDay   UMETA(DisplayName = "TimeOfDay"),
    QuestState  UMETA(DisplayName = "QuestState")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasVoiceLine;

    FNarr_DialogueLine()
        : SpeakerID(TEXT(""))
        , DisplayDuration(4.0f)
        , bHasVoiceLine(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarrDialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarrDialogueTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPlayerEnterTrigger(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialoguePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bFireOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggerID;

private:
    bool bHasFired;
};
