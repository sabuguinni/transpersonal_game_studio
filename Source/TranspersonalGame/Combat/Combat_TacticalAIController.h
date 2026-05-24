#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

class UCombat_ThreatAssessmentSystem;
class ANPC_DinosaurBase;

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol        UMETA(DisplayName = "Patrol"),
    Hunt          UMETA(DisplayName = "Hunt"),
    Engage        UMETA(DisplayName = "Engage"),
    Flank         UMETA(DisplayName = "Flank"),
    Retreat       UMETA(DisplayName = "Retreat"),
    Regroup       UMETA(DisplayName = "Regroup"),
    Ambush        UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None          UMETA(DisplayName = "None"),
    Line          UMETA(DisplayName = "Line"),
    Wedge         UMETA(DisplayName = "Wedge"),
    Circle        UMETA(DisplayName = "Circle"),
    Pincer        UMETA(DisplayName = "Pincer"),
    Ambush        UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_Formation Formation = ECombat_Formation::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        Formation = ECombat_Formation::None;
        EngagementRange = 1500.0f;
        FlankingDistance = 800.0f;
        RetreatThreshold = 0.3f;
        PackSize = 3;
        bIsPackLeader = false;
        PrimaryTarget = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombat_ThreatAssessmentSystem* ThreatAssessment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboard* CombatBlackboard;

public:
    // Tactical AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_TacticalState GetTacticalState() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetFormation(ECombat_Formation NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPrimaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetPrimaryTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetPackMembers() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPackLeader(bool bLeader);

    // Combat Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OrderRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegroupPack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetupAmbush(const FVector& AmbushLocation);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal tactical functions
    void UpdateTacticalState(float DeltaTime);
    void ProcessThreatAssessment();
    void CoordinatePackBehavior();
    void UpdateBlackboardValues();
    FVector CalculateFlankingPosition(AActor* Target);
    bool ShouldRetreat() const;
    void BroadcastTacticalCommand(const FString& Command);

private:
    float StateUpdateTimer;
    float ThreatAssessmentTimer;
    float PackCoordinationTimer;
    
    static const float STATE_UPDATE_INTERVAL;
    static const float THREAT_ASSESSMENT_INTERVAL;
    static const float PACK_COORDINATION_INTERVAL;
};