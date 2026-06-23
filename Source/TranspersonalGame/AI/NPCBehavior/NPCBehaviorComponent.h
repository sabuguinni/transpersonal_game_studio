#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Flee        UMETA(DisplayName = "Flee"),
    Interact    UMETA(DisplayName = "Interact"),
    Alert       UMETA(DisplayName = "Alert"),
    Seek        UMETA(DisplayName = "Seek")
};

USTRUCT(BlueprintType)
struct FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    float DisplayDuration;

    FNPC_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DisplayDuration(3.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FString EventTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel;

    FNPC_MemoryEntry()
        : EventTag(TEXT(""))
        , EventLocation(FVector::ZeroVector)
        , TimeStamp(0.0f)
        , ThreatLevel(0.0f)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State management
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // Dialogue
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    void TriggerDialogue(int32 LineIndex);

    UFUNCTION(BlueprintPure, Category = "NPC|Dialogue")
    FNPC_DialogueLine GetCurrentDialogueLine() const { return CurrentDialogueLine; }

    // Memory system
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemory(const FString& EventTag, const FVector& Location, float ThreatLevel);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOfThreat(float MinThreatLevel) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearOldMemories(float MaxAge);

    // Patrol
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AddPatrolPoint(const FVector& Point);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    // Threat response
    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void OnDinosaurDetected(const FVector& DinoLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void OnPlayerApproach(float Distance);

    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DialogueRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    TArray<FNPC_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

private:
    FNPC_DialogueLine CurrentDialogueLine;
    int32 CurrentPatrolIndex;
    float StateTimer;
    float MemoryDecayRate;

    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateFleeBehavior(float DeltaTime);
    void UpdateAlertBehavior(float DeltaTime);

    FVector FleeTarget;
};
