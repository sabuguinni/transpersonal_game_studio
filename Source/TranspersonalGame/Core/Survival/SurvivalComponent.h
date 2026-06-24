// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Survival stats component: hunger, thirst, stamina, fear, temperature
// Attach to TranspersonalCharacter for full prehistoric survival mechanics

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Starving    UMETA(DisplayName = "Starving"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Dehydrated  UMETA(DisplayName = "Dehydrated"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Panicking   UMETA(DisplayName = "Panicking"),
    Hypothermic UMETA(DisplayName = "Hypothermic"),
    Hyperthermic UMETA(DisplayName = "Hyperthermic"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ECore_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Distant     UMETA(DisplayName = "Distant"),
    Nearby      UMETA(DisplayName = "Nearby"),
    Immediate   UMETA(DisplayName = "Immediate"),
    Attacking   UMETA(DisplayName = "Attacking")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCore_SurvivalSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Temperature = 37.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    ECore_SurvivalStatus Status = ECore_SurvivalStatus::Healthy;
};

// ─── Delegates ───────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStatusChanged, ECore_SurvivalStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatCritical, float, StatValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

// ─── USurvivalComponent ───────────────────────────────────────────────────────

UCLASS(ClassGroup = (Survival), meta = (BlueprintSpawnableComponent), DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Core Stats (read-only from Blueprint) ─────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Health;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Hunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Thirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Stamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Fear;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Temperature;

    // ── Config (editable in editor) ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float MaxHunger = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float MaxThirst = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float MaxStamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float MaxFear = 100.f;

    /** Hunger drain per second (real-time) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;

    /** Thirst drain per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaSprintDrain = 10.f;

    /** Stamina recovery per second while idle/walking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 5.f;

    /** Fear decay per second when no threat */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.f;

    /** Health drain per second when starving */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamageRate = 1.f;

    /** Health drain per second when dehydrated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamageRate = 1.5f;

    /** Ideal body temperature (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Temperature")
    float IdealTemperature = 37.f;

    /** Ambient temperature influence per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Temperature")
    float TemperatureAdaptRate = 0.1f;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    ECore_SurvivalStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    ECore_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsAlive;

    // ── Actions (callable from Blueprint/C++) ─────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void Rest(float RestValue);

    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void TakeDamage_Survival(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void SetThreatLevel(ECore_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival|Actions")
    void SetAmbientTemperature(float AmbientCelsius);

    // ── Queries ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    FCore_SurvivalSnapshot GetSnapshot() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    bool CanSprint() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    bool IsStarving() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    bool IsDehydrated() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    bool IsPanicking() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Query")
    float GetStaminaPercent() const;

    // ── Delegates ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnSurvivalStatusChanged OnStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatCritical OnHealthCritical;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatCritical OnStaminaCritical;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnCharacterDeath OnCharacterDeath;

private:
    // ── Internal helpers ───────────────────────────────────────────────────
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickFear(float DeltaTime);
    void TickTemperature(float DeltaTime);
    void TickHealthDecay(float DeltaTime);
    void UpdateStatus();
    void Die();

    float AmbientTemperature = 28.f;
    bool bDeathFired = false;

    // Broadcast threshold trackers
    bool bHealthCriticalFired = false;
    bool bStaminaCriticalFired = false;
};
