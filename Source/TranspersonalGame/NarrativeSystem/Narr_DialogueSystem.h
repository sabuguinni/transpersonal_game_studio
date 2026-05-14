#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Information,
    Quest,
    Trade,
    Farewell
};

UENUM(BlueprintType)
enum class ENarr_CharacterType : uint8
{
    TribalElder,
    HuntLeader,
    Scout,
    Crafter,
    Shaman,
    Warrior
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_CharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        DialogueType = ENarr_DialogueType::Greeting;
        CharacterType = ENarr_CharacterType::TribalElder;
        AudioPath = TEXT("");
        Duration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bIsRepeatable = true;
        Priority = 0;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& SequenceID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueSequence GetDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogues(ENarr_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializePrehistoricDialogues();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueSequence> DialogueDatabase;

    UPROPERTY()
    bool bDialogueActive;

    UPROPERTY()
    FString CurrentSequenceID;

    UPROPERTY()
    int32 CurrentLineIndex;

    UPROPERTY()
    FTimerHandle DialogueTimerHandle;

    void PlayNextLine();
    void OnDialogueLineComplete();
    void CreateDefaultDialogues();
};