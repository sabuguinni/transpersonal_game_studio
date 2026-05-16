#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DamageEvents.h"
#include "SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, AActor*, DamagedActor, float, DamageAmount, AActor*, DamageSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorDied, AActor*, DeadActor, AActor*, Killer);

UENUM(BlueprintType)
enum class ECombat_DamageType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Spear           UMETA(DisplayName = "Spear"),
    Club            UMETA(DisplayName = "Club"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Fall            UMETA(DisplayName = "Fall")
};

USTRUCT(BlueprintType)
struct FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float ArmorPenetration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCanKnockdown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float KnockdownForce = 0.0f;

    FCombat_DamageInfo()
    {
        BaseDamage = 10.0f;
        DamageType = ECombat_DamageType::None;
        ArmorPenetration = 0.0f;
        bCanKnockdown = false;
        KnockdownForce = 0.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    float ArmorValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    float DamageReduction = 0.0f;

    // Damage dealing
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float DealDamage(AActor* Target, const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage(const FCombat_DamageInfo& DamageInfo, AActor* DamageSource);

    // Health management
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void HealDamage(float HealAmount);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAtFullHealth() const;

    // Combat state
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanTakeDamage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetInvulnerable(bool bInvulnerable, float Duration = 0.0f);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageDealt OnDamageDealt;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnActorDied OnActorDied;

protected:
    // Internal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsInvulnerable = false;

    UPROPERTY()
    FTimerHandle InvulnerabilityTimer;

    // Damage calculation
    float CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const;
    
    // Death handling
    void HandleDeath(AActor* Killer);

    // Invulnerability
    void ClearInvulnerability();

private:
    bool bIsDead = false;
};