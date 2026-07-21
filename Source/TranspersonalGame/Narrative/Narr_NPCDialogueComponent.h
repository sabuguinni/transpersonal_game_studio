#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_NPCDialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredQuestID;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = "";
        DialogueText = "";
        NextDialogueID = "";
        bIsQuestRelated = false;
        RequiredQuestID = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Friendliness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Caution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Knowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> PreferredTopics;

    FNarr_NPCPersonality()
    {
        Friendliness = 0.5f;
        Caution = 0.5f;
        Knowledge = 0.5f;
        Aggression = 0.2f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_NPCDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_NPCDialogueComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FNarr_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    bool bCanInteract;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    int32 TimesSpokenTo;

public:
    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    bool CanStartDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void AdvanceDialogue(const FString& PlayerChoice);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void ResetDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void SetDialogueAvailable(bool bAvailable);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    TArray<FString> GetAvailableResponses();

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    float GetPersonalityTrait(const FString& TraitName);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void ModifyPersonalityTrait(const FString& TraitName, float Delta);

protected:
    UFUNCTION()
    void InitializeDialogue();

    UFUNCTION()
    FNarr_DialogueEntry* FindDialogueEntry(const FString& DialogueID);

    UFUNCTION()
    void UpdatePersonalityBasedOnInteraction();
};

#include "Narr_NPCDialogueComponent.generated.h"