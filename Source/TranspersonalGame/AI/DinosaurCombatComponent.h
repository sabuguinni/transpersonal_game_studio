#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatComponent.generated.h"

// Forward declarations
class ACharacter;
class UCapsuleComponent;
class UCharacterMovementComponent;

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_AttackPhase : uint8
{
    None        UMETA(DisplayName = "None"),
    WindUp      UMETA(DisplayName = "WindUp"),
    Strike      UMETA(DisplayName = "Strike"),
    Recovery    UMETA(DisplayName = "Recovery"),
    Cooldown    UMETA(DisplayName = "Cooldown")
};

UENUM(BlueprintType)
enum class ECombat_DinoAttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
    Charge      UMETA(DisplayName = "Charge"),
    Claw        UMETA(DisplayName = "Claw"),
    Stomp       UMETA(DisplayName = "Stomp")
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinoAttackType AttackType = ECombat_DinoAttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackAngleDegrees = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WindUpTime = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StrikeTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RecoveryTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CooldownTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 800.0f;

    FCombat_AttackProfile(){}
};

USTRUCT(BlueprintType)
struct FCombat_HitResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bHit = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float DamageDealt = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* HitActor = nullptr;

    FCombat_HitResult(){}
};

// ============================================================
// UDinosaurCombatComponent — main combat component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Attack API ----

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool TryInitiateAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CancelAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAttacking() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsOnCooldown() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AttackPhase GetCurrentPhase() const { return CurrentPhase; }

    // ---- Configuration ----

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAttackProfile(const FCombat_AttackProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTRexDefaults();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetRaptorDefaults();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetBrachiosaurusDefaults();

    // ---- Hit Detection ----

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_HitResult PerformSweepAttack();

    // ---- Events ----

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackHit(const FCombat_HitResult& HitResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackMiss();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackPhaseChanged(ECombat_AttackPhase NewPhase);

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_AttackProfile AttackProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bDebugDrawAttackSweep = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 TotalHitsDealt = 0;

private:
    ECombat_AttackPhase CurrentPhase = ECombat_AttackPhase::None;
    float PhaseTimer = 0.0f;

    void AdvanceAttackPhase(float DeltaTime);
    void ExecuteStrike();
    void ApplyDamageToTarget(AActor* Target, const FVector& HitLoc);
};
