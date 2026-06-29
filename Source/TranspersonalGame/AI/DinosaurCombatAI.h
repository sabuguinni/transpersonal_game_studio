#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class ECombat_DinoAIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Alert           UMETA(DisplayName = "Alert"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    PackHunting     UMETA(DisplayName = "PackHunting"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Charge          UMETA(DisplayName = "Charge"),
    TailSwipe       UMETA(DisplayName = "TailSwipe"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Pounce          UMETA(DisplayName = "Pounce"),
    Gore            UMETA(DisplayName = "Gore")
};

UENUM(BlueprintType)
enum class ECombat_ThreatResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Warn            UMETA(DisplayName = "Warn"),
    Engage          UMETA(DisplayName = "Engage"),
    Flee            UMETA(DisplayName = "Flee"),
    CallPack        UMETA(DisplayName = "CallPack")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = true;

    FCombat_AttackData() {}
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AggroRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FearThreshold = 0.3f;

    FCombat_DinoStats() {}
};

USTRUCT(BlueprintType)
struct FCombat_PackInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlpha = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCoordinationRadius = 1500.0f;

    FCombat_PackInfo() {}
};

USTRUCT(BlueprintType)
struct FCombat_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsActive = true;

    FCombat_ThreatMemory() {}
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_StateChangedDelegate,
    ECombat_DinoAIState, OldState, ECombat_DinoAIState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_AttackLandedDelegate,
    AActor*, Target, float, DamageDealt);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_PackCallDelegate,
    FVector, ThreatLocation);

// ============================================================
// COMPONENT
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attacks")
    TArray<FCombat_AttackData> AttackRepertoire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FCombat_PackInfo PackInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    bool bIsTerritorial = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float AmbushChance = 0.0f;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
              meta = (AllowPrivateAccess = "true"))
    ECombat_DinoAIState CurrentState = ECombat_DinoAIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
              meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
              meta = (AllowPrivateAccess = "true"))
    float RageLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
              meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatMemory> ThreatMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
              meta = (AllowPrivateAccess = "true"))
    float LastAttackTime = 0.0f;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_StateChangedDelegate OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_AttackLandedDelegate OnAttackLanded;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_PackCallDelegate OnPackCall;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAIState(ECombat_DinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore, bool bIsPlayer);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target, ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastPackCall(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_ThreatResponse EvaluateThreatResponse(float ThreatScore) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_AttackData GetBestAttackForRange(float DistanceToTarget) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInAttackCooldown() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpeciesDefaults(ECombat_DinoSpecies InSpecies);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TickAccumulator = 0.0f;
    static constexpr float AITickInterval = 0.1f; // 10Hz

    void TickCombatAI(float DeltaTime);
    void TickThreatMemoryDecay(float DeltaTime);
    void UpdateRageDecay(float DeltaTime);
    void SelectNewTarget();
    bool CanAttackNow() const;
};
