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
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Graze       UMETA(DisplayName = "Graze"),
    Rest        UMETA(DisplayName = "Rest"),
    GatherFood  UMETA(DisplayName = "GatherFood"),
    WarnPlayer  UMETA(DisplayName = "WarnPlayer")
};

UENUM(BlueprintType)
enum class ENPC_Species : uint8
{
    Human       UMETA(DisplayName = "Human"),
    TRex        UMETA(DisplayName = "TRex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastPlayerSighting;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bPlayerWasSpotted;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel;

    FNPC_BehaviorMemory()
        : LastKnownPlayerLocation(FVector::ZeroVector)
        , TimeSinceLastPlayerSighting(0.0f)
        , bPlayerWasSpotted(false)
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

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerDetected(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatNearby(AActor* Threat, float ThreatStrength);

    // --- Patrol ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AddPatrolWaypoint(FVector Waypoint);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    bool HasReachedCurrentWaypoint() const;

    // --- Species Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_Species Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ChaseSpeed;

    // --- Memory ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_BehaviorMemory BehaviorMemory;

    // --- Pack Behavior ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    int32 PackSize;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Pack")
    TArray<AActor*> PackMembers;

    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void RegisterPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void AlertPackToThreat(AActor* Threat);

    // --- Daily Routine ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float DawnActivityHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float DuskRestHour;

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float CurrentGameHour);

private:
    ENPC_BehaviorState CurrentState;
    ENPC_BehaviorState PreviousState;

    TArray<FVector> PatrolWaypoints;
    int32 CurrentWaypointIndex;
    float WaypointAcceptanceRadius;

    float StateTimer;

    void UpdateBehavior(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAlert(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateFlee(float DeltaTime);
    void UpdateGraze(float DeltaTime);

    void ApplySpeciesDefaults();
};
