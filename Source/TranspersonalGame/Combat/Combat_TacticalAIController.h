#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - COMBAT & ENEMY AI AGENT #12
 * 
 * Advanced tactical AI controller that implements intelligent combat behaviors:
 * - Coordinated pack tactics with communication
 * - Dynamic threat assessment and target prioritization
 * - Adaptive positioning based on terrain and cover
 * - Emotional state management affecting combat decisions
 * - Learning from player behavior patterns
 * 
 * Design Philosophy:
 * "Combat is a conversation, not an equation" - every encounter tells a story
 * through intelligent enemy behavior that adapts and surprises.
 */

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol = 0          UMETA(DisplayName = "Patrol"),
    Alert = 1           UMETA(DisplayName = "Alert"),
    Hunting = 2         UMETA(DisplayName = "Hunting"),
    Combat = 3          UMETA(DisplayName = "Combat"),
    Flanking = 4        UMETA(DisplayName = "Flanking"),
    Retreating = 5      UMETA(DisplayName = "Retreating"),
    Regrouping = 6      UMETA(DisplayName = "Regrouping"),
    Coordinating = 7    UMETA(DisplayName = "Coordinating")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None = 0            UMETA(DisplayName = "None"),
    Low = 1             UMETA(DisplayName = "Low"),
    Medium = 2          UMETA(DisplayName = "Medium"),
    High = 3            UMETA(DisplayName = "High"),
    Critical = 4        UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ECombat_CommunicationType : uint8
{
    Alert = 0           UMETA(DisplayName = "Alert"),
    TargetSpotted = 1   UMETA(DisplayName = "Target Spotted"),
    RequestHelp = 2     UMETA(DisplayName = "Request Help"),
    FlankLeft = 3       UMETA(DisplayName = "Flank Left"),
    FlankRight = 4      UMETA(DisplayName = "Flank Right"),
    Retreat = 5         UMETA(DisplayName = "Retreat"),
    Regroup = 6         UMETA(DisplayName = "Regroup")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float TargetDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bHasLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float Caution = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float Confidence = 0.5f;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        ThreatLevel = ECombat_ThreatLevel::None;
        PrimaryTarget = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
        TargetDistance = 0.0f;
        bHasLineOfSight = false;
        Aggression = 0.5f;
        Caution = 0.5f;
        Confidence = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CommunicationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    ECombat_CommunicationType MessageType = ECombat_CommunicationType::Alert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float Urgency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float Timestamp = 0.0f;

    FCombat_CommunicationData()
    {
        MessageType = ECombat_CommunicationType::Alert;
        Location = FVector::ZeroVector;
        TargetActor = nullptr;
        Urgency = 0.5f;
        Timestamp = 0.0f;
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
    virtual void OnPossess(APawn* InPawn) override;

    // ═══════════════════════════════════════════════════════════════
    // CORE AI COMPONENTS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    // ═══════════════════════════════════════════════════════════════
    // TACTICAL DATA
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_CommunicationData> CommunicationQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<AActor*> KnownAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<AActor*> KnownEnemies;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR CONFIGURATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    class UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    class UBlackboard* DefaultBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    float CommunicationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    float UpdateFrequency = 0.1f;

    // ═══════════════════════════════════════════════════════════════
    // TACTICAL BEHAVIORS
    // ═══════════════════════════════════════════════════════════════

public:
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SendCommunication(ECombat_CommunicationType MessageType, FVector Location, AActor* Target = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ReceiveCommunication(const FCombat_CommunicationData& Message);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector FindOptimalPosition(FVector TargetLocation, float MinDistance, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldFlank(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CoordinateWithAllies();

protected:
    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION CALLBACKS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ═══════════════════════════════════════════════════════════════
    // INTERNAL METHODS
    // ═══════════════════════════════════════════════════════════════

    void InitializePerception();
    void InitializeBehaviorTree();
    void UpdateTacticalData(float DeltaTime);
    void ProcessCommunications();
    void UpdateEmotionalState(float DeltaTime);
    void AnalyzePlayerBehavior();

    // ═══════════════════════════════════════════════════════════════
    // TACTICAL CALCULATIONS
    // ═══════════════════════════════════════════════════════════════

    float CalculateThreatLevel(AActor* Target);
    FVector CalculateFlankingPosition(AActor* Target);
    bool HasCoverBetween(FVector Start, FVector End);
    TArray<AActor*> FindNearbyAllies(float Radius);
    bool IsPositionSafe(FVector Position);

private:
    float LastUpdateTime;
    float LastCommunicationTime;
    FTimerHandle TacticalUpdateTimer;
};