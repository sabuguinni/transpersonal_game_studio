#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPC_AdvancedDecisionSystem.generated.h"

// Forward declarations
class ANPCBaseActor;
class ADinosaurBase;

UENUM(BlueprintType)
enum class ENPC_DecisionType : uint8
{
    None = 0,
    Patrol = 1,
    Hunt = 2,
    Gather = 3,
    Defend = 4,
    Flee = 5,
    Rest = 6,
    Socialize = 7,
    Craft = 8,
    Explore = 9
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4
};

USTRUCT(BlueprintType)
struct FNPC_DecisionContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    ENPC_DecisionType CurrentDecision;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float DecisionConfidence;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float DecisionDuration;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    AActor* TargetActor;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    ENPC_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    TArray<AActor*> NearbyThreats;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    TArray<AActor*> NearbyAllies;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float Hunger;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float Fatigue;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float Fear;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float Curiosity;

    FNPC_DecisionContext()
    {
        CurrentDecision = ENPC_DecisionType::None;
        DecisionConfidence = 0.0f;
        DecisionDuration = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        ThreatLevel = ENPC_ThreatLevel::None;
        Hunger = 0.5f;
        Fatigue = 0.0f;
        Fear = 0.0f;
        Curiosity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DecisionWeight
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    ENPC_DecisionType DecisionType;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float Weight;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    float Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Decision")
    bool bIsAvailable;

    FNPC_DecisionWeight()
    {
        DecisionType = ENPC_DecisionType::None;
        Weight = 0.0f;
        Priority = 0.0f;
        bIsAvailable = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AdvancedDecisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AdvancedDecisionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Decision making functions
    UFUNCTION(BlueprintCallable, Category = "Decision System")
    ENPC_DecisionType MakeDecision();

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    void UpdateDecisionContext();

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    void ExecuteDecision(ENPC_DecisionType Decision);

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    float CalculateDecisionWeight(ENPC_DecisionType DecisionType);

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    void AddThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    ENPC_ThreatLevel AssessThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    bool ShouldFleeFromThreat();

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    FVector FindSafeLocation();

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    AActor* FindNearestResource();

    UFUNCTION(BlueprintCallable, Category = "Decision System")
    void SetEmotionalState(float NewFear, float NewCuriosity);

protected:
    // Decision context and state
    UPROPERTY(BlueprintReadOnly, Category = "Decision System")
    FNPC_DecisionContext DecisionContext;

    UPROPERTY(BlueprintReadOnly, Category = "Decision System")
    TArray<FNPC_DecisionWeight> DecisionWeights;

    UPROPERTY(BlueprintReadWrite, Category = "Decision System")
    float DecisionUpdateInterval;

    UPROPERTY(BlueprintReadWrite, Category = "Decision System")
    float LastDecisionTime;

    UPROPERTY(BlueprintReadWrite, Category = "Decision System")
    float ThreatDetectionRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Decision System")
    float AllyDetectionRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Decision System")
    float ResourceDetectionRadius;

    // Decision weights
    UPROPERTY(BlueprintReadWrite, Category = "Decision Weights")
    float HungerInfluence;

    UPROPERTY(BlueprintReadWrite, Category = "Decision Weights")
    float FatigueInfluence;

    UPROPERTY(BlueprintReadWrite, Category = "Decision Weights")
    float FearInfluence;

    UPROPERTY(BlueprintReadWrite, Category = "Decision Weights")
    float CuriosityInfluence;

    UPROPERTY(BlueprintReadWrite, Category = "Decision Weights")
    float ThreatInfluence;

    // Helper functions
    void InitializeDecisionWeights();
    void UpdateEmotionalState(float DeltaTime);
    void ScanForThreats();
    void ScanForAllies();
    float CalculateThreatDistance(AActor* ThreatActor);
    bool IsActorThreat(AActor* Actor);
    bool IsActorAlly(AActor* Actor);
};