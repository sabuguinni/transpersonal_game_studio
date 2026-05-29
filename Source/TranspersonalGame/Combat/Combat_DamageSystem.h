#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageAmount = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageInstigator = nullptr;
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(const FCombat_DamageData& DamageData);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToTarget(AActor* Target, float Damage, EDamageType DamageType, AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const { return CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnDamageTaken(const FCombat_DamageData& DamageData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnDeath(AActor* Killer);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TMap<EDamageType, float> DamageTypeResistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanTakeDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float InvulnerabilityDuration = 0.5f;

    float LastDamageTime = 0.0f;

private:
    void HandleDeath(AActor* Killer);
    float CalculateFinalDamage(float BaseDamage, EDamageType DamageType);
};