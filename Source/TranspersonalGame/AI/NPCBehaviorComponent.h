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
    Shelter     UMETA(DisplayName = "Shelter"),
    Sleep       UMETA(DisplayName = "Sleep"),
};

UENUM(BlueprintType)
enum class ENPC_Role : uint8
{
    TribeLeader UMETA(DisplayName = "TribeLeader"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Guard       UMETA(DisplayName = "Guard"),
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownThreatLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceThreat;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bHasSeenPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bHasSeenDinosaur;

    FNPC_Memory()
        : LastKnownThreatLocation(FVector::ZeroVector)
        , ThreatLevel(0.f)
        , TimeSinceThreat(0.f)
        , bHasSeenPlayer(false)
        , bHasSeenDinosaur(false)
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

    // Current behavior state
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior")
    ENPC_BehaviorState CurrentState;

    // NPC role in the tribe
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Role")
    ENPC_Role NPCRole;

    // NPC memory of threats and events
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_Memory Memory;

    // Patrol radius from home location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius;

    // Home base location (set on BeginPlay)
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    FVector HomeLocation;

    // Detection range for threats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float ThreatDetectionRange;

    // Fear threshold — above this, NPC flees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float FearFleeThreshold;

    // Current fear level (0-1)
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float FearLevel;

    // Stamina for running/fleeing (0-1)
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float Stamina;

    // Time of day awareness (0=midnight, 0.5=noon, 1=midnight)
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Schedule")
    float TimeOfDay;

    // Should NPC sleep at night?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    bool bSleepsAtNight;

    // Transition to a new behavior state
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    // Notify NPC of a threat at a location
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RegisterThreat(FVector ThreatLocation, float ThreatMagnitude, bool bIsDinosaur);

    // Clear threat memory over time
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void DecayMemory(float DeltaTime);

    // Get next patrol point within radius
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint() const;

    // Check if it's time to sleep (night schedule)
    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    bool ShouldSleep() const;

    // Get a flee direction away from threat
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetFleeDirection() const;

protected:
    void UpdateBehavior(float DeltaTime);
    void ScanForThreats();

private:
    float PatrolTimer;
    FVector CurrentPatrolTarget;
    bool bHasPatrolTarget;
};
