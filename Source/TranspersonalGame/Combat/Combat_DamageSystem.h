#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Combat_DamageSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_DamageType : uint8
{
    Physical    UMETA(DisplayName = "Physical"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Crush       UMETA(DisplayName = "Crush"),
    Environmental UMETA(DisplayName = "Environmental")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Amount = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* Instigator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ZeroVector;

    FCombat_DamageInfo()
    {
        Amount = 10.0f;
        DamageType = ECombat_DamageType::Physical;
        Instigator = nullptr;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Health and damage properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Resistance")
    float PhysicalResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Resistance")
    float BiteResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Resistance")
    float ClawResistance = 0.0f;

    // Damage application functions
    UFUNCTION(BlueprintCallable, Category = "Damage")
    void ApplyDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void ApplySimpleDamage(float DamageAmount, AActor* DamageInstigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsCriticalHealth() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, AActor*, Instigator);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageTaken OnDamageTaken;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeath OnDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealthPercentage);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChanged OnHealthChanged;

protected:
    // Internal damage calculation
    float CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo);

    // Death handling
    UFUNCTION()
    void HandleDeath();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float CriticalHealthThreshold = 0.25f;

private:
    bool bIsDead = false;
};

#include "Combat_DamageSystem.generated.h"