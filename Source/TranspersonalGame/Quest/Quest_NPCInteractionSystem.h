#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Quest_NPCInteractionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_NPCRole : uint8
{
    None UMETA(DisplayName = "None"),
    QuestGiver UMETA(DisplayName = "Quest Giver"),
    Vendor UMETA(DisplayName = "Vendor"),
    Guide UMETA(DisplayName = "Guide"),
    Survivor UMETA(DisplayName = "Survivor"),
    Hunter UMETA(DisplayName = "Hunter"),
    Crafter UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class EQuest_NPCState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Talking UMETA(DisplayName = "Talking"),
    Working UMETA(DisplayName = "Working"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Hunting UMETA(DisplayName = "Hunting"),
    Crafting UMETA(DisplayName = "Crafting"),
    Gathering UMETA(DisplayName = "Gathering")
};

USTRUCT(BlueprintType)
struct FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    FQuest_NPCDialogue()
    {
        DialogueText = TEXT("Hello, survivor.");
        SpeakerName = TEXT("Unknown");
        DialogueDuration = 3.0f;
        bIsQuestRelated = false;
    }
};

USTRUCT(BlueprintType)
struct FQuest_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    EQuest_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FQuest_NPCDialogue> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    bool bCanGiveQuests;

    FQuest_NPCProfile()
    {
        NPCName = TEXT("Survivor");
        Role = EQuest_NPCRole::Survivor;
        Description = TEXT("A fellow survivor in this prehistoric world.");
        InteractionRange = 300.0f;
        bCanGiveQuests = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_NPCInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_NPCInteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // NPC Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FQuest_NPCProfile NPCProfile;

    // Current State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    EQuest_NPCState CurrentState;

    // Interaction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bIsInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionPrompt;

    // Quest Related
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<FString> AvailableQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    bool bHasActiveQuest;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool CanInteract() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void StartInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FQuest_NPCDialogue GetRandomDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SetNPCState(EQuest_NPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool HasAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void AddAvailableQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool IsQuestCompleted(const FString& QuestID) const;

private:
    // Internal state
    float LastInteractionTime;
    AActor* CurrentInteractingActor;
    bool bIsCurrentlyInteracting;

    // Helper functions
    void UpdateNPCBehavior(float DeltaTime);
    void CheckForNearbyPlayers();
    FString GetStateBasedDialogue() const;
};