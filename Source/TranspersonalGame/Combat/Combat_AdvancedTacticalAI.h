#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_AdvancedTacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol          UMETA(DisplayName = "Patrol"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Engage          UMETA(DisplayName = "Engage"),
    Flank           UMETA(DisplayName = "Flank"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Coordinate      UMETA(DisplayName = "Coordinate"),
    Ambush          UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    None            UMETA(DisplayName = "None"),
    Line            UMETA(DisplayName = "Line"),
    Circle          UMETA(DisplayName = "Circle"),
    Wedge           UMETA(DisplayName = "Wedge"),
    Pincer          UMETA(DisplayName = "Pincer"),
    Ambush          UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHostile = false;

    FCombat_TacticalTarget()
    {
        TargetActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsHostile = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_FormationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_FormationType FormationType = ECombat_FormationType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FormationCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FormationRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxUnits = 6;

    FCombat_FormationData()
    {
        FormationType = ECombat_FormationType::None;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 1000.0f;
        MaxUnits = 6;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AdvancedTacticalAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AdvancedTacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* PerceptionComponent;

    // Tactical State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_TacticalState CurrentTacticalState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalTarget PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalTarget> KnownTargets;

    // Formation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FCombat_FormationData CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<ACombat_AdvancedTacticalAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ACombat_AdvancedTacticalAI* PackLeader = nullptr;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float HearingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float FlankingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float CoordinationRadius = 1500.0f;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* TacticalBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBlackboardAsset* TacticalBlackboard;

public:
    // Tactical AI Methods
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdatePrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AddKnownTarget(AActor* Target, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FCombat_TacticalTarget GetBestTarget() const;

    // Formation Methods
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormation(ECombat_FormationType FormationType, FVector Center);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void JoinPack(ACombat_AdvancedTacticalAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetFormationPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void UpdatePackCoordination();

    // Tactical Maneuvers
    UFUNCTION(BlueprintCallable, Category = "Tactical Maneuvers")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Maneuvers")
    void ExecutePincerMovement(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Maneuvers")
    void ExecuteAmbushTactic(FVector AmbushLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical Maneuvers")
    void ExecuteRetreatManeuver();

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastThreatAlert(AActor* Threat, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceiveThreatAlert(AActor* Threat, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void RequestAssistance(AActor* Target);

protected:
    // Internal Methods
    void InitializePerception();
    void UpdateTacticalAssessment();
    void CalculateFormationPositions();
    bool IsInFormationPosition() const;
    float CalculateThreatLevel(AActor* Target) const;
    void CleanupOldTargets();

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
    float LastTacticalUpdate = 0.0f;
    float TacticalUpdateInterval = 0.5f;
    int32 FormationIndex = -1;
};