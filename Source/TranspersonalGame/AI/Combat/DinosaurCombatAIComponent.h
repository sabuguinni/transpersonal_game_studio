#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatAIComponent.generated.h"

// ============================================================
// Combat AI enums — prefixed Combat_ to avoid project-wide conflicts
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoThreatResponse : uint8
{
    None            UMETA(DisplayName = "None"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Warn            UMETA(DisplayName = "Warn"),
    Charge          UMETA(DisplayName = "Charge"),
    Flank           UMETA(DisplayName = "Flank"),
    Retreat         UMETA(DisplayName = "Retreat"),
    PackCall        UMETA(DisplayName = "PackCall")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpeciesType : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Generic         UMETA(DisplayName = "Generic")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Stomp           UMETA(DisplayName = "Stomp"),
    TailSwipe       UMETA(DisplayName = "TailSwipe"),
    Charge          UMETA(DisplayName = "Charge"),
    Claw            UMETA(DisplayName = "Claw"),
    Headbutt        UMETA(DisplayName = "Headbutt"),
    Roar            UMETA(DisplayName = "Roar")
};

// ============================================================
// Combat AI structs
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float BaseDamage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float StaminaCost = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bCanInterrupt = true;
};

USTRUCT(BlueprintType)
struct FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    ECombat_DinoThreatResponse CurrentResponse = ECombat_DinoThreatResponse::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    bool bPackCallSent = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    int32 PackMembersNearby = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Tactical")
    float CurrentStamina = 100.0f;
};

USTRUCT(BlueprintType)
struct FCombat_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpeciesType Species = ECombat_DinoSpeciesType::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AggressionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PackCallRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsAmbushPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MoveSpeedIdle = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MoveSpeedCharge = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    TArray<FCombat_AttackProfile> AttackProfiles;
};

// ============================================================
// Main Combat AI Component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Species Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_SpeciesTraits SpeciesTraits;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_TacticalState TacticalState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackExecuted(ECombat_AttackType AttackType, float DamageDealt, AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnThreatDetected(AActor* ThreatActor, float ThreatDistance);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnPackCallReceived(FVector PackLeaderLocation);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnFleeTriggered(FVector FleeDirection);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnDeath();

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoThreatResponse GetCurrentThreatResponse() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPackLeader(bool bIsLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReceivePackCall(FVector LeaderLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_AttackProfile GetBestAttackForRange(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ForceEngageTarget(AActor* Target);

private:
    // Internal tick helpers
    void UpdateThreatDetection(float DeltaTime);
    void UpdateCombatResponse(float DeltaTime);
    void ExecuteAttack(const FCombat_AttackProfile& Attack);
    void BroadcastPackCall();
    void EvaluateFlee();

    float ThreatScanTimer = 0.0f;
    static constexpr float ThreatScanInterval = 0.25f;

    float AttackCooldownRemaining = 0.0f;
    float StaminaRegenTimer = 0.0f;

    bool bIsDead = false;
};
