#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DataTable.h"
#include "Quest_DialogueSystem.h"
#include "Quest_NPCActor.generated.h"

// Forward declarations
class UQuest_DialogueSystem;
class UUserWidget;

UENUM(BlueprintType)
enum class EQuest_NPCState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Talking UMETA(DisplayName = "Talking"),
    Busy UMETA(DisplayName = "Busy"),
    Hostile UMETA(DisplayName = "Hostile")
};

UENUM(BlueprintType)
enum class EQuest_NPCType : uint8
{
    Trader UMETA(DisplayName = "Trader"),
    QuestGiver UMETA(DisplayName = "Quest Giver"),
    Guard UMETA(DisplayName = "Guard"),
    Villager UMETA(DisplayName = "Villager"),
    Elder UMETA(DisplayName = "Elder"),
    Hunter UMETA(DisplayName = "Hunter")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    EQuest_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    int32 StartingDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    TArray<FString> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    float InteractionRange;

    FQuest_NPCData()
    {
        NPCName = TEXT("Unknown NPC");
        NPCType = EQuest_NPCType::Villager;
        Description = FText::FromString(TEXT("A mysterious figure"));
        StartingDialogueID = 0;
        InteractionRange = 300.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnNPCInteraction, AQuest_NPCActor*, NPC, APawn*, InteractingPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnNPCStateChanged, AQuest_NPCActor*, NPC, EQuest_NPCState, NewState);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_NPCActor : public ACharacter
{
    GENERATED_BODY()

public:
    AQuest_NPCActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Interaction system
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void StartInteraction(APawn* InteractingPawn);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool CanInteract(APawn* InteractingPawn) const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    float GetDistanceToPlayer() const;

    // State management
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void SetNPCState(EQuest_NPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    EQuest_NPCState GetNPCState() const { return CurrentState; }

    // Data access
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    FQuest_NPCData GetNPCData() const { return NPCData; }

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void SetNPCData(const FQuest_NPCData& NewData);

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool HasAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    TArray<FString> GetAvailableQuests() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest NPC Events")
    FQuest_OnNPCInteraction OnNPCInteraction;

    UPROPERTY(BlueprintAssignable, Category = "Quest NPC Events")
    FQuest_OnNPCStateChanged OnNPCStateChanged;

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC Components")
    UWidgetComponent* NameplateWidget;

    // NPC Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC Data")
    FQuest_NPCData NPCData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC State")
    EQuest_NPCState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC State")
    APawn* CurrentInteractingPawn;

    // Interaction settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC Settings")
    bool bShowNameplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC Settings")
    bool bAutoFacePlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC Settings")
    float NameplateDistance;

    // Dialogue integration
    UPROPERTY()
    UQuest_DialogueSystem* DialogueSystem;

private:
    // Interaction callbacks
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
                                       AActor* OtherActor, 
                                       UPrimitiveComponent* OtherComp, 
                                       int32 OtherBodyIndex, 
                                       bool bFromSweep, 
                                       const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
                                     AActor* OtherActor, 
                                     UPrimitiveComponent* OtherComp, 
                                     int32 OtherBodyIndex);

    // Helper functions
    void UpdateNameplateVisibility();
    void FacePlayer();
    bool IsPlayerCharacter(APawn* Pawn) const;

    // State tracking
    bool bPlayerInRange;
    float LastPlayerDistance;
    FTimerHandle FacePlayerTimerHandle;
};