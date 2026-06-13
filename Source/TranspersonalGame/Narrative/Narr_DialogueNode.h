#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueNode.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Statement       UMETA(DisplayName = "Statement"),
    Question        UMETA(DisplayName = "Question"),
    Choice          UMETA(DisplayName = "Choice"),
    Conditional     UMETA(DisplayName = "Conditional"),
    Action          UMETA(DisplayName = "Action")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    Player          UMETA(DisplayName = "Player"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Narrator        UMETA(DisplayName = "Narrator"),
    Warning         UMETA(DisplayName = "Warning System")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ConsequenceActions;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::GetEmpty();
        NextNodeID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SpeakerType Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioClipPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> CompletionActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueNode()
    {
        NodeID = TEXT("");
        Speaker = ENarr_SpeakerType::Narrator;
        DialogueType = ENarr_DialogueType::Statement;
        DialogueText = FText::GetEmpty();
        AudioClipPath = TEXT("");
        NextNodeID = TEXT("");
        DisplayDuration = 3.0f;
        bIsRepeatable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueNode : public UObject
{
    GENERATED_BODY()

public:
    UNarr_DialogueNode();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanExecute(const TMap<FString, int32>& GameState) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> GetValidChoices(const TMap<FString, int32>& GameState) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ExecuteCompletionActions();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetSpeakerName() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueNode DialogueData;

protected:
    bool CheckConditions(const TArray<FString>& Conditions, const TMap<FString, int32>& GameState) const;
    FString ParseCondition(const FString& Condition, const TMap<FString, int32>& GameState) const;
};