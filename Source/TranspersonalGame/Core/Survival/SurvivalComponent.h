// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival stats: health, hunger, thirst, stamina, temperature, fear
// CYCLE: PROD_CYCLE_AUTO_20260626_001

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "SurvivalComponent.generated.h"

// ============================================================
// Survival stats snapshot — returned by GetCurrentStats()
// ============================================================
USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Temperature = 37.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsStarving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsDehydrated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsDead = false;
};

// ============================================================
// Biome type enum — drives survival drain rates
// ============================================================
UENUM(BlueprintType)
enum class ECore_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Jungle      UMETA(DisplayName = "Jungle"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Coastal     UMETA(DisplayName = "Coastal"),
};

// ============================================================
// Delegates
// ============================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

// ============================================================
// USurvivalComponent
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Survival Actions ----
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrainStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetBiome(ECore_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalStats GetCurrentStats() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool CanSprint() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnHungerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnThirstChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnFearChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnDeath OnDeath;

    // ---- Stats (exposed for Blueprint HUD) ----
    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Health;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Hunger;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival|Stats")
    float MaxHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Thirst;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival|Stats")
    float MaxThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Stamina;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival|Stats")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Temperature;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Fear;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival|Stats")
    float MaxFear;

    // ---- Drain rates (editable per biome) ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate;

    // ---- Status flags ----
    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    bool bIsStarving;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    bool bIsDehydrated;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    bool bIsFrozen;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    bool bIsOverheating;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Biome")
    ECore_BiomeType CurrentBiome;
};
