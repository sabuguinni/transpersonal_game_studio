#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_SurvivalNPCController.generated.h"

UENUM(BlueprintType)
enum class EQuest_NPCRole : uint8
{
    TutorialGuide    UMETA(DisplayName = "Tutorial Guide"),
    HuntingExpert    UMETA(DisplayName = "Hunting Expert"),
    SurvivalMentor   UMETA(DisplayName = "Survival Mentor"),
    CraftingMaster   UMETA(DisplayName = "Crafting Master"),
    ScoutLeader      UMETA(DisplayName = "Scout Leader")
};

UENUM(BlueprintType)
enum class EQuest_NPCMood : uint8
{
    Friendly         UMETA(DisplayName = "Friendly"),
    Cautious         UMETA(DisplayName = "Cautious"),
    Urgent           UMETA(DisplayName = "Urgent"),
    Desperate        UMETA(DisplayName = "Desperate"),
    Grateful         UMETA(DisplayName = "Grateful")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bStartsQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FQuest_DialogueOption()
    {
        OptionText = TEXT("Talk");
        ResponseText = TEXT("Hello, survivor.");
        bStartsQuest = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EQuest_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EQuest_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FQuest_DialogueOption> AvailableDialogue;

    FQuest_NPCPersonality()
    {
        Role = EQuest_NPCRole::TutorialGuide;
        CurrentMood = EQuest_NPCMood::Friendly;
        NPCName = TEXT("Unnamed Survivor");
        BackgroundStory = TEXT("A fellow survivor in this prehistoric world.");
    }
};

/**
 * Controller for survival NPCs that provide quests and guidance
 * Manages NPC behavior, dialogue, and quest distribution
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalNPCController : public AAIController
{
    GENERATED_BODY()

public:
    AQuest_SurvivalNPCController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Setup")
    FQuest_NPCPersonality NPCPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Setup")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Setup")
    bool bCanMoveAround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Setup")
    float MovementRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    class APawn* PlayerInRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    bool bIsInConversation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    FVector HomeLocation;

public:
    // Core NPC functions
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void StartConversation(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool CanInteract(APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FString GetGreeting() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    TArray<FQuest_DialogueOption> GetAvailableDialogue() const;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void GiveQuestToPlayer(const FString& QuestID, APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool HasQuestForPlayer(APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void UpdateQuestDialogue();

    // NPC behavior
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetNPCMood(EQuest_NPCMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void PatrolAroundHome();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReturnHome();

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "NPC Info")
    FString GetNPCRoleString() const;

    UFUNCTION(BlueprintPure, Category = "NPC Info")
    FString GetNPCMoodString() const;

private:
    void CheckForNearbyPlayer();
    void HandleMovement(float DeltaTime);
    FString GenerateContextualDialogue() const;
    
    float LastPlayerCheckTime;
    float PatrolTimer;
    FVector CurrentPatrolTarget;
    bool bMovingToPatrolTarget;
};