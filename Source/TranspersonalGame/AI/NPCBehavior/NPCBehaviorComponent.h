#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Interact    UMETA(DisplayName = "Interact"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    ENPC_BehaviorState TriggerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    float Cooldown = 30.0f;
};

USTRUCT(BlueprintType)
struct FNPC_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
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

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // Threat detection
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateThreatLevel(ENPC_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_ThreatLevel GetThreatLevel() const { return CurrentThreat; }

    // Patrol
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AddWaypoint(FVector Location, float WaitTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void ClearWaypoints();

    // Dialogue
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    void AddDialogueLine(FString Text, ENPC_BehaviorState TriggerState, float Cooldown = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    FString GetCurrentDialogue() const;

    // Detection
    UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
    bool DetectThreatInRadius(float Radius);

    UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
    void AlertNearbyNPCs(float AlertRadius = 2000.0f);

    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    TArray<FNPC_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_WaypointData> Waypoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior")
    ENPC_ThreatLevel CurrentThreat;

private:
    int32 CurrentWaypointIndex = 0;
    float StateTimer = 0.0f;
    float DialogueCooldownTimer = 0.0f;
    FString ActiveDialogueLine;

    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickIdle(float DeltaTime);
    void SelectDialogueForState();
};
