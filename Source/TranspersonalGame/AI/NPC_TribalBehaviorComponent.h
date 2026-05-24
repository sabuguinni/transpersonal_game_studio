#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_TribalBehaviorComponent.generated.h"

class AActor;
class APawn;

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"), 
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman")
};

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Socializing UMETA(DisplayName = "Socializing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Fighting    UMETA(DisplayName = "Fighting")
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenThreatTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> TribeMembers;

    FNPC_TribalMemory()
    {
        LastKnownThreatLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenThreatTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TribalBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core tribal properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float FleeRadius;

    // AI Memory and decision making
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    FNPC_TribalMemory TribalMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float MemoryRetentionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float DecisionUpdateInterval;

    // Behavioral parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression;

    // Current targets and goals
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FVector CurrentDestination;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsInDanger;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentFear;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    TArray<AActor*> NearbyTribeMembers;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void UpdateTribalBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void SetCurrentActivity(ENPC_TribalActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void DetectThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void ShareThreatInformation(const FVector& ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void FindNearbyTribeMembers();

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    FVector GetPatrolDestination();

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    bool ShouldFight();

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void UpdateMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void MakeDecision();

private:
    FTimerHandle DecisionTimerHandle;
    FTimerHandle MemoryUpdateHandle;

    void OnDecisionUpdate();
    void OnMemoryUpdate();
    
    float LastDecisionTime;
    float LastMemoryUpdate;
};