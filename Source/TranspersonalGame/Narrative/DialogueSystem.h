#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Urgent      UMETA(DisplayName = "Urgent"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Informative UMETA(DisplayName = "Informative"),
    Desperate   UMETA(DisplayName = "Desperate")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , Tone(ENarr_DialogueTone::Informative)
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bTriggeredOnce;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bTriggeredOnce(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName ActiveSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerSequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsSequenceActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndSequence();

protected:
    virtual void BeginPlay() override;

private:
    bool bSequenceActive;
    int32 ActiveSequenceIndex;
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName DialogueSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bTriggerOnce;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bHasTriggered;
    APawn* CachedPlayer;
};
