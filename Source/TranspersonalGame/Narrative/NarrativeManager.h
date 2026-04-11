#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EmotionIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetFlags;

    FDialogueEntry()
    {
        CharacterName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        VoiceAssetPath = TEXT("");
        EmotionIntensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeFlag
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString FlagName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsSet = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Value = 0.0f;

    FNarrativeFlag()
    {
        FlagName = TEXT("");
        bIsSet = false;
        Value = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FString&, CharacterName, const FText&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeStateChanged, const FString&, StateName);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool TriggerDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadDialogueTable(UDataTable* DialogueTable);

    // Narrative Flags
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeFlag(const FString& FlagName, bool bValue, float NumericValue = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetNarrativeFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetNarrativeFlagValue(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CheckNarrativeConditions(const TArray<FString>& RequiredFlags) const;

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryState(const FString& NewState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentStoryState() const { return CurrentStoryState; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeStateChanged OnNarrativeStateChanged;

protected:
    UPROPERTY()
    UDataTable* DialogueDataTable;

    UPROPERTY()
    TMap<FString, FNarrativeFlag> NarrativeFlags;

    UPROPERTY()
    FString CurrentStoryState;

    UPROPERTY()
    TArray<FString> StoryHistory;

    // Internal methods
    FDialogueEntry* FindDialogueEntry(const FString& DialogueID);
    void ProcessDialogueFlags(const FDialogueEntry& Entry);
    void InitializeDefaultFlags();
};