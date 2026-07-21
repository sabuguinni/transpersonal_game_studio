#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "CombatDamageComponent.generated.h"

UENUM(BlueprintType)
enum class ECombat_DamageType : uint8
{
    None        UMETA(DisplayName = "None"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Crush       UMETA(DisplayName = "Crush"),
    Pierce      UMETA(DisplayName = "Pierce"),
    Blunt       UMETA(DisplayName = "Blunt")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Amount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* Instigator = nullptr;

    FCombat_DamageInfo()
    {
        Amount = 0.0f;
        DamageType = ECombat_DamageType::None;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
        Instigator = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatDamageComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatDamageComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Health System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bCanRegenerate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
    float RegenerationRate = 1.0f;

    // Damage Resistance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BiteResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClawResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CrushResistance = 0.0f;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastDamageTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatTimeout = 10.0f;

    // Damage Functions
    UFUNCTION(BlueprintCallable, Category = "Damage")
    void TakeDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat() const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
    void OnDamageTaken(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnCombatStateChanged(bool bNewCombatState);

private:
    float GetDamageResistance(ECombat_DamageType DamageType) const;
    void UpdateCombatState();
};