#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerSphere.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_NPCRole : uint8
{
    SurvivalTeacher     UMETA(DisplayName = "Survival Teacher"),
    HuntingExpert       UMETA(DisplayName = "Hunting Expert"),
    ResourceExpert      UMETA(DisplayName = "Resource Expert"),
    TerritoryScout      UMETA(DisplayName = "Territory Scout"),
    ToolMaker           UMETA(DisplayName = "Tool Maker"),
    TribalElder         UMETA(DisplayName = "Tribal Elder")
};

UENUM(BlueprintType)
enum class EQuest_DialogueState : uint8
{
    Available           UMETA(DisplayName = "Available"),
    InProgress          UMETA(DisplayName = "In Progress"),
    ReadyToComplete     UMETA(DisplayName = "Ready To Complete"),
    Completed           UMETA(DisplayName = "Completed"),
    Locked              UMETA(DisplayName = "Locked")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FQuest_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_NPCRole RequiredNPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> ObjectiveList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_DialogueLine> IntroDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_DialogueLine> CompletionDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_DialogueState CurrentState;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("");
        Description = TEXT("");
        RequiredNPCRole = EQuest_NPCRole::SurvivalTeacher;
        ExperienceReward = 0;
        CurrentState = EQuest_DialogueState::Available;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalNPC : public APawn
{
    GENERATED_BODY()

public:
    AQuest_SurvivalNPC();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NPCMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    EQuest_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    TArray<FQuest_SurvivalMission> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    FQuest_SurvivalMission CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueLine> GreetingDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsInDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueRange;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartDialogue(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CanStartMission(const FQuest_SurvivalMission& Mission) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_DialogueLine GetNextDialogueLine();

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    void InitializeNPCData();
    void SetupMissionsForRole();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalDialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalDialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FQuest_DialogueLine> CurrentDialogueChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentDialogueIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    AQuest_SurvivalNPC* CurrentNPC;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StartDialogueChain(const TArray<FQuest_DialogueLine>& DialogueLines, AQuest_SurvivalNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void EndDialogueChain();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FQuest_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    float GetDialogueProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogueAudio(const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterDialogueCompletion(const FString& MissionName);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DialogueTrigger : public ATriggerSphere
{
    GENERATED_BODY()

public:
    AQuest_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    FString TriggerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    EQuest_NPCRole RequiredNPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    TArray<FQuest_DialogueLine> TriggerDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    bool bOneTimeUse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Data")
    bool bHasBeenTriggered;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TriggerDialogue(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CanTrigger() const;

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

#include "Quest_SurvivalDialogueSystem.generated.h"