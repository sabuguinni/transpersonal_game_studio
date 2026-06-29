#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UENUM(BlueprintType)
enum class ECombat_DinoAIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Alert           UMETA(DisplayName = "Alert"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Charge          UMETA(DisplayName = "Charge"),
    Attack          UMETA(DisplayName = "Attack"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Flee            UMETA(DisplayName = "Flee"),
    PackCoordinate  UMETA(DisplayName = "PackCoordinate")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpeciesType : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_DinoPerceptionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float LoseSightRadius = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float PeripheralVisionAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float HearingRange = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float DamageDetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    bool bCanDetectMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float MovementDetectionThreshold = 150.0f;
};

USTRUCT(BlueprintType)
struct FCombat_DinoThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    TWeakObjectPtr<AActor> ThreatActor;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    bool bIsCurrentTarget = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // === STATE MACHINE ===
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetAIState(ECombat_DinoAIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|AI")
    ECombat_DinoAIState GetCurrentAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void EvaluateThreatLevel();

    // === THREAT SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void RegisterThreat(AActor* ThreatActor, float InitialScore);

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void UpdateThreatScore(AActor* ThreatActor, float ScoreDelta);

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void ClearThreat(AActor* ThreatActor);

    // === PACK COORDINATION ===
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SignalPackMembers(ECombat_DinoAIState SignalState, AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackSignal(ECombat_DinoAIState SignalState, AActor* TargetActor, ADinosaurCombatAIController* Sender);

    UFUNCTION(BlueprintPure, Category = "Combat|Pack")
    bool IsPackLeader() const { return bIsPackLeader; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetPackLeader(bool bLeader) { bIsPackLeader = bLeader; }

    // === ATTACK EXECUTION ===
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ExecuteChargeAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ExecuteFlankingManeuver(AActor* Target, FVector FlankPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool CanAttackTarget(AActor* Target) const;

    // === PERCEPTION CALLBACKS ===
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // === CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpeciesType SpeciesType = ECombat_DinoSpeciesType::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoPerceptionData PerceptionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float RetreatHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float PackSignalRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|BehaviorTree")
    UBehaviorTree* BehaviorTreeAsset = nullptr;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AI", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoAIState CurrentAIState = ECombat_DinoAIState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AI", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_DinoThreatEntry> ThreatList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AI", meta = (AllowPrivateAccess = "true"))
    bool bIsPackLeader = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AI", meta = (AllowPrivateAccess = "true"))
    float LastAttackTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AI", meta = (AllowPrivateAccess = "true"))
    bool bIsCharging = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat|Perception", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent = nullptr;

private:
    void InitializePerception();
    void UpdateStateMachine(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandleAlertState(float DeltaTime);
    void HandleStalkState(float DeltaTime);
    void HandleChargeState(float DeltaTime);
    void HandleAttackState(float DeltaTime);
    void HandleRetreatState(float DeltaTime);
    float CalculateThreatScore(AActor* Actor) const;
    bool IsTargetInAttackRange(AActor* Target) const;
    FVector GetFlankPosition(AActor* Target, int32 FlankIndex) const;
};
