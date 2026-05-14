#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresQuestCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredQuestID;

    FNarr_DialogueNode()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioFilePath = TEXT("");
        bRequiresQuestCompletion = false;
        RequiredQuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("");
        CharacterName = TEXT("");
        TribalRole = ENarr_TribalRole::Survivor;
        WorldLocation = FVector::ZeroVector;
        bIsAlive = true;
        TrustLevel = 50.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& CharacterID, const FString& InitialDialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool SelectDialogueResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueNode GetCurrentDialogueNode() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    // Character management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacter(const FNarr_CharacterProfile& Character);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateCharacterTrust(const FString& CharacterID, float TrustDelta);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetNearbyCharacters(const FVector& PlayerLocation, float SearchRadius) const;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasStoryEventOccurred(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryVariable(const FString& VariableName, const FString& Value);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetStoryVariable(const FString& VariableName) const;

protected:
    // Dialogue data
    UPROPERTY()
    TMap<FString, FNarr_DialogueNode> DialogueDatabase;

    UPROPERTY()
    TMap<FString, FNarr_CharacterProfile> CharacterDatabase;

    UPROPERTY()
    FNarr_DialogueNode CurrentDialogue;

    UPROPERTY()
    FString CurrentCharacterID;

    UPROPERTY()
    bool bDialogueActive;

    // Story tracking
    UPROPERTY()
    TSet<FString> CompletedStoryEvents;

    UPROPERTY()
    TMap<FString, FString> StoryVariables;

    // Internal methods
    void LoadDialogueData();
    void InitializeTribalCharacters();
    bool ValidateDialogueTransition(const FString& FromDialogueID, const FString& ToDialogueID) const;
};