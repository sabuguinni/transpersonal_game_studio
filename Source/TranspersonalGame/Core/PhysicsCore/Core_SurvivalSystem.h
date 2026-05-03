#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "Core_SurvivalSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health      UMETA(DisplayName = "Health"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Fear        UMETA(DisplayName = "Fear"),
    Temperature UMETA(DisplayName = "Temperature")
};

UENUM(BlueprintType)
enum class ECore_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Cold        UMETA(DisplayName = "Cold"),
    Hot         UMETA(DisplayName = "Hot"),
    Dying       UMETA(DisplayName = "Dying")
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "-20.0", ClampMax = "50.0"))
    float Temperature = 20.0f;

    FCore_SurvivalStats()
    {
        Health = 100.0f;
        Hunger = 100.0f;
        Thirst = 100.0f;
        Stamina = 100.0f;
        Fear = 0.0f;
        Temperature = 20.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_SurvivalConfig
{
    GENERATED_BODY()

    // Decay rates per second
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay Rates")
    float HungerDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay Rates")
    float ThirstDecayRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay Rates")
    float StaminaRecoveryRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay Rates")
    float FearDecayRate = 2.0f;

    // Critical thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalHealthThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalHungerThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalThirstThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxFearThreshold = 80.0f;

    FCore_SurvivalConfig()
    {
        HungerDecayRate = 0.5f;
        ThirstDecayRate = 0.8f;
        StaminaRecoveryRate = 15.0f;
        FearDecayRate = 2.0f;
        CriticalHealthThreshold = 20.0f;
        CriticalHungerThreshold = 10.0f;
        CriticalThirstThreshold = 5.0f;
        MaxFearThreshold = 80.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalStatChanged, ECore_SurvivalStat, StatType, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStateChanged, ECore_SurvivalState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_SurvivalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_SurvivalSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Current survival stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FCore_SurvivalStats CurrentStats;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FCore_SurvivalConfig Config;

    // Current survival state
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    ECore_SurvivalState CurrentSurvivalState;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Survival Events")
    FOnSurvivalStatChanged OnSurvivalStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival Events")
    FOnSurvivalStateChanged OnSurvivalStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival Events")
    FOnPlayerDeath OnPlayerDeath;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetTemperature(float NewTemperature);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatValue(ECore_SurvivalStat StatType) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsStatCritical(ECore_SurvivalStat StatType) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ResetAllStats();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage(float DamageAmount, bool bFromDinosaur = false);

private:
    // Internal timer handles
    FTimerHandle SurvivalTickTimer;

    // Internal methods
    void UpdateSurvivalStats(float DeltaTime);
    void UpdateSurvivalState();
    void ProcessStatDecay(float DeltaTime);
    void ProcessHealthEffects();
    void CheckForDeath();
    
    // Utility methods
    float ClampStat(float Value, float Min = 0.0f, float Max = 100.0f) const;
    void BroadcastStatChange(ECore_SurvivalStat StatType, float NewValue);
    void BroadcastStateChange(ECore_SurvivalState NewState);
};

#include "Core_SurvivalSystem.generated.h"