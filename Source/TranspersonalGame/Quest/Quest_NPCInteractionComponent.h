#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Core/SharedTypes.h"
#include "Quest_NPCInteractionComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bStartsQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCompletesQuest;

    FQuest_DialogueOption()
    {
        OptionText = TEXT("");
        ResponseText = TEXT("");
        QuestID = TEXT("");
        bStartsQuest = false;
        bCompletesQuest = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString GreetingText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueOption> DialogueOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString FarewellText;

    FQuest_NPCDialogue()
    {
        GreetingText = TEXT("Greetings, survivor.");
        FarewellText = TEXT("Stay safe out there.");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_NPCInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_NPCInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void StartInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FQuest_NPCDialogue GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool CanInteract(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SetNPCType(EQuest_NPCType NewNPCType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EQuest_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FQuest_NPCDialogue CurrentDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsInteracting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    AActor* CurrentInteractingActor;

    // Quest Manager Reference
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    class AQuest_QuestManager* QuestManager;

    // Dialogue Setup
    void InitializeDialogue();
    void SetupHuntMasterDialogue();
    void SetupGathererDialogue();
    void SetupTraderDialogue();
    void SetupElderDialogue();

    // Quest Integration
    void HandleQuestStart(const FString& QuestID);
    void HandleQuestComplete(const FString& QuestID);
    void UpdateDialogueBasedOnQuests();

    // Utility
    float GetDistanceToActor(AActor* OtherActor);
    void FindQuestManager();
};