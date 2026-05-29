#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DataTable.h"
#include "SurvivalQuestNPC.generated.h"

// Forward declarations
class UQuestManager;
class UQuestInstance;

UENUM(BlueprintType)
enum class EQuest_NPCType : uint8
{
    Elder           UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Trader          UMETA(DisplayName = "Trader")
};

UENUM(BlueprintType)
enum class EQuest_NPCMood : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Worried         UMETA(DisplayName = "Worried"),
    Desperate       UMETA(DisplayName = "Desperate"),
    Grateful        UMETA(DisplayName = "Grateful"),
    Suspicious      UMETA(DisplayName = "Suspicious"),
    Hostile         UMETA(DisplayName = "Hostile")
};

USTRUCT(BlueprintType)
struct FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString GreetingText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestOfferText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestCompleteText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestFailedText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString FarewellText;

    FQuest_NPCDialogue()
    {
        GreetingText = TEXT("Hello, traveler.");
        QuestOfferText = TEXT("I have a task for you.");
        QuestCompleteText = TEXT("Well done! Here is your reward.");
        QuestFailedText = TEXT("Perhaps another time...");
        FarewellText = TEXT("Safe travels.");
    }
};

USTRUCT(BlueprintType)
struct FQuest_NPCStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Desperation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health;

    FQuest_NPCStats()
    {
        Trust = 50.0f;
        Fear = 30.0f;
        Desperation = 20.0f;
        Health = 100.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnPlayerInteract, class APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnQuestStatusChanged, int32, QuestID, bool, bCompleted);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASurvivalQuestNPC : public ACharacter
{
    GENERATED_BODY()

public:
    ASurvivalQuestNPC();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* DialogueWidget;

    // NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    EQuest_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    EQuest_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    FQuest_NPCDialogue DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    FQuest_NPCStats NPCStats;

    // Quest System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<int32> AvailableQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<int32> CompletedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 CurrentActiveQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bCanGiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bCanReceiveItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float InteractionRange;

    // Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bFleeFromDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FQuest_OnPlayerInteract OnPlayerInteract;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FQuest_OnQuestStatusChanged OnQuestStatusChanged;

public:
    // Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void StartInteraction(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool CanInteractWithPlayer(APlayerController* PlayerController) const;

    // Quest Functions
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool HasAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    TArray<int32> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool GiveQuestToPlayer(int32 QuestID, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool CompleteQuest(int32 QuestID, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void UpdateQuestProgress(int32 QuestID, int32 ObjectiveIndex, bool bCompleted);

    // Dialogue Functions
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    FString GetGreetingText() const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    FString GetQuestOfferText() const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    FString GetContextualDialogue() const;

    // Stats Functions
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void ModifyTrust(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void ModifyFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void ModifyDesperation(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void UpdateMoodBasedOnStats();

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void FleeFromThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void ReturnToPatrol();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool IsPlayerInRange(APlayerController* PlayerController) const;

protected:
    // Overlap Events
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Internal Functions
    void InitializeNPCDefaults();
    void SetupDialogueForNPCType();
    void SetupQuestsForNPCType();
    FString GenerateContextualDialogue() const;
    void HandlePlayerProximity(bool bPlayerNearby);

private:
    // State tracking
    bool bPlayerInRange;
    bool bCurrentlyInteracting;
    APlayerController* InteractingPlayer;
    FVector InitialLocation;
    float LastInteractionTime;
    float MoodUpdateTimer;

    // Constants
    static constexpr float MOOD_UPDATE_INTERVAL = 5.0f;
    static constexpr float INTERACTION_COOLDOWN = 2.0f;
    static constexpr float DEFAULT_INTERACTION_RANGE = 300.0f;
    static constexpr float DEFAULT_FLEE_DISTANCE = 1000.0f;
    static constexpr float DEFAULT_PATROL_RADIUS = 500.0f;
};