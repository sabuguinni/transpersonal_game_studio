#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Quest_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_DialogueType : uint8
{
    QuestGive       UMETA(DisplayName = "Quest Give"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    QuestProgress   UMETA(DisplayName = "Quest Progress"),
    Greeting        UMETA(DisplayName = "Greeting"),
    Trade           UMETA(DisplayName = "Trade"),
    Information     UMETA(DisplayName = "Information"),
    Warning         UMETA(DisplayName = "Warning"),
    Farewell        UMETA(DisplayName = "Farewell")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuest_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    FQuest_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DialogueType = EQuest_DialogueType::Information;
        AudioPath = TEXT("");
        DisplayDuration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FQuest_DialogueSequence()
    {
        SequenceID = TEXT("default");
        NextSequenceID = TEXT("");
        bIsRepeatable = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, FQuest_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float InteractionRange;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool StartDialogue(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void NextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FQuest_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddDialogueSequence(const FString& SequenceID, const FQuest_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void SetupHuntQuestDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void SetupCollectQuestDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void SetupExploreQuestDialogue();

private:
    void InitializeDefaultDialogues();
};