#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "Combat_PlayerDamageSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDamaged, float, DamageAmount, AActor*, DamageSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float DamageAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    AActor* DamageSource = nullptr;

    UPROPERTY(BlueprintReadOnly)
    float Timestamp = 0.0f;

    FCombat_DamageInstance()
    {
        DamageAmount = 0.0f;
        HitLocation = FVector::ZeroVector;
        DamageSource = nullptr;
        Timestamp = 0.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PlayerDamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PlayerDamageSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Damage Processing
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void ProcessDamage(float DamageAmount, const FVector& HitLocation, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void ApplyInstantDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void ApplyDamageOverTime(float DamagePerSecond, float Duration, AActor* DamageSource);

    // Health Management
    UFUNCTION(BlueprintCallable, Category = "Combat Health")
    void HealPlayer(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat Health")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintPure, Category = "Combat Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Combat Health")
    bool IsPlayerAlive() const { return CurrentHealth > 0.0f; }

    // Damage Resistance
    UFUNCTION(BlueprintCallable, Category = "Combat Resistance")
    void SetDamageResistance(float ResistancePercentage);

    UFUNCTION(BlueprintCallable, Category = "Combat Resistance")
    void AddTemporaryResistance(float ResistancePercentage, float Duration);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnPlayerDamaged OnPlayerDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnPlayerDeath OnPlayerDeath;

protected:
    // Health Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Health")
    float CurrentHealth = 100.0f;

    // Damage Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Damage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseDamageResistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Damage")
    float CurrentDamageResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Damage")
    float InvulnerabilityDuration = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Damage")
    float LastDamageTime = 0.0f;

    // Damage Over Time
    UPROPERTY(BlueprintReadOnly, Category = "Combat DOT")
    float DOTDamagePerSecond = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat DOT")
    float DOTRemainingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat DOT")
    AActor* DOTSource = nullptr;

    // Temporary Resistance
    UPROPERTY(BlueprintReadOnly, Category = "Combat Resistance")
    float TemporaryResistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Resistance")
    float TemporaryResistanceEndTime = 0.0f;

    // Damage History
    UPROPERTY(BlueprintReadOnly, Category = "Combat History")
    TArray<FCombat_DamageInstance> RecentDamageHistory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat History")
    int32 MaxDamageHistoryEntries = 10;

private:
    void UpdateDamageOverTime(float DeltaTime);
    void UpdateTemporaryResistance(float DeltaTime);
    void AddDamageToHistory(const FCombat_DamageInstance& DamageInstance);
    float CalculateFinalDamage(float BaseDamage) const;
    void TriggerPlayerDeath();
};