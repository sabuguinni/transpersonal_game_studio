#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Scouting UMETA(DisplayName = "Scouting"),
    Engaging UMETA(DisplayName = "Engaging"),
    Flanking UMETA(DisplayName = "Flanking"),
    Ambushing UMETA(DisplayName = "Ambushing"),
    Retreating UMETA(DisplayName = "Retreating"),
    Coordinating UMETA(DisplayName = "Coordinating"),
    Defending UMETA(DisplayName = "Defending")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AdvantageScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float LastUsedTime;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        AdvantageScore = 0.0f;
        bIsOccupied = false;
        LastUsedTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EngagementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TimeLastSeen;

    FCombat_EngagementData()
    {
        Target = nullptr;
        ThreatLevel = 0.0f;
        EngagementRange = 1000.0f;
        LastKnownPosition = FVector::ZeroVector;
        TimeLastSeen = 0.0f;
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

    // Tactical AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // Tactical State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalPosition> TacticalPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_EngagementData CurrentEngagement;

    // AI Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Parameters")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Parameters")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Parameters")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Parameters")
    float RetreatHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Parameters")
    float CoordinationRadius;

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void EvaluateTacticalSituation();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SelectTacticalPosition();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void InitiateAmbush();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CoordinateWithAllies();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteTacticalRetreat();

    // Combat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngage(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector FindOptimalAttackPosition(AActor* Target);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    // Internal tactical processing
    void UpdateTacticalState(float DeltaTime);
    void ProcessEngagementLogic();
    void UpdateTacticalPositions();
    bool IsPositionSafe(const FVector& Position);
    float CalculatePositionAdvantage(const FVector& Position, AActor* Target);
    TArray<ACombat_TacticalAIController*> GetNearbyAllies();
};