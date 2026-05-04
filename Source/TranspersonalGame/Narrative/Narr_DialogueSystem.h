#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    FieldNarration     UMETA(DisplayName = "Field Narration"),
    DangerWarning      UMETA(DisplayName = "Danger Warning"),
    Discovery          UMETA(DisplayName = "Discovery"),
    ResourceAlert      UMETA(DisplayName = "Resource Alert"),
    TimeOfDay          UMETA(DisplayName = "Time of Day"),
    Tutorial           UMETA(DisplayName = "Tutorial")
};

UENUM(BlueprintType)
enum class ENarr_TriggerCondition : uint8
{
    PlayerProximity    UMETA(DisplayName = "Player Proximity"),
    TimeOfDay          UMETA(DisplayName = "Time of Day"),
    PlayerHealth       UMETA(DisplayName = "Player Health"),
    DinosaurPresence   UMETA(DisplayName = "Dinosaur Presence"),
    ResourceDiscovery  UMETA(DisplayName = "Resource Discovery")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        DialogueText = FText::FromString(TEXT(""));
        AudioFilePath = TEXT("");
        DialogueType = ENarr_DialogueType::FieldNarration;
        Duration = 5.0f;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueEntry DialogueData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    ENarr_TriggerCondition TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bCanRetrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float RetriggerCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHasTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastTriggerTime;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanTrigger() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueTriggered(const FNarr_DialogueEntry& Dialogue);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckTriggerConditions() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> ActiveDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DialogueStartTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FNarr_DialogueEntry& Dialogue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsPlayingDialogue() const { return bIsPlayingDialogue; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void QueueDialogue(const FNarr_DialogueEntry& Dialogue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProcessDialogueQueue();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueStarted(const FNarr_DialogueEntry& Dialogue);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueFinished(const FNarr_DialogueEntry& Dialogue);

protected:
    virtual void Tick(float DeltaTime);
    void UpdateCurrentDialogue(float DeltaTime);
};