#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_NPCDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EmotionalWeight;

    FQuest_DialogueNode()
    {
        DialogueText = TEXT("");
        QuestID = TEXT("");
        bIsQuestGiver = false;
        EmotionalWeight = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Helpfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Wisdom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Fearfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Trustworthiness;

    FQuest_NPCPersonality()
    {
        Aggressiveness = 0.5f;
        Helpfulness = 0.7f;
        Wisdom = 0.6f;
        Fearfulness = 0.4f;
        Trustworthiness = 0.8f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_NPCDialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_NPCDialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    TArray<FQuest_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FQuest_NPCPersonality NPCPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    ENPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    float RelationshipLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    bool bCanGiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    TArray<FString> AvailableQuestIDs;

public:
    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    FQuest_DialogueNode GetDialogueForPlayer(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void ProcessPlayerResponse(const FString& Response, class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    bool CanOfferQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void UpdateRelationship(float DeltaRelationship);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    FString GenerateContextualDialogue(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void InitializeNPCRole(ENPCRole Role);

protected:
    void SetupDialogueForRole();
    FString GetRoleBasedGreeting() const;
    FString GetPersonalityModifiedDialogue(const FString& BaseDialogue) const;
};