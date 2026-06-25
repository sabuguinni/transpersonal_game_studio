#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/NPCBehaviorComponent.h"
#include "CombatAIComponent.generated.h"

// ============================================================
// Combat AI Component — Agent #12
// Tactical combat intelligence for dinosaurs and enemies.
// Attaches to SkeletalMeshActors representing dinos in MinPlayableMap.
// Depends on UNPCBehaviorComponent for state machine.
// ============================================================

UENUM(BlueprintType)
enum class ECombat_ThreatResponse : uint8
{
    None        UMETA(DisplayName = "None"),
    Investigate UMETA(DisplayName = "Investigate"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Charge      UMETA(DisplayName = "Charge"),
    Flank       UMETA(DisplayName = "Flank"),
    Retreat     UMETA(DisplayName = "Retreat"),
    PackCall    UMETA(DisplayName = "PackCall")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Charge      UMETA(DisplayName = "Charge"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Headbutt    UMETA(DisplayName = "Headbutt")
};

USTRUCT(BlueprintType)
struct FCombat_AttackRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_AttackType AttackType = ECombat_AttackType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float Damage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float Range = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float Cooldown = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastUsedTime = -999.0f;
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* MemberActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_ThreatResponse AssignedRole = ECombat_ThreatResponse::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsActive = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Core Combat Parameters ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float AggressionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float ChargeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float StalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float RetreatHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    bool bIsPackPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float PackCallRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float PackDamageMultiplier = 1.5f;

    // ── Attack Repertoire ───────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attacks")
    TArray<FCombat_AttackRecord> AttackRepertoire;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Attacks")
    ECombat_AttackType LastAttackType = ECombat_AttackType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Attacks")
    float LastAttackTime = -999.0f;

    // ── Threat Response ─────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Threat")
    ECombat_ThreatResponse CurrentThreatResponse = ECombat_ThreatResponse::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threat")
    float ThreatDecayRate = 0.1f;

    // ── Pack Coordination ───────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    int32 PackSize = 0;

    // ── Tactical State ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Tactical")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Tactical")
    float DistanceToTarget = 99999.0f;

    // ── Blueprint-Callable Functions ────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatProfile(bool bPackPredator, float AggrRadius, float ChargeSpd, float RetreatHP);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddAttack(ECombat_AttackType Type, float Damage, float Range, float Cooldown);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnTargetDetected(AActor* Target, float InitialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteBestAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssignPackRoles();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatResponse EvaluateThreatResponse(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetBestAttackDamage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanExecuteAttack(ECombat_AttackType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ApplyPackDamageBonus(float& OutDamage) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPackLeader(bool bLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel(float Delta);

private:
    float TickAccumulator = 0.0f;
    static constexpr float CombatTickInterval = 0.1f;

    void UpdateCombatTick(float DeltaTime);
    void UpdateDistanceToTarget();
    void SelectThreatResponse(float CurrentHealth, float MaxHealth);
    FCombat_AttackRecord* FindBestAvailableAttack();
};
