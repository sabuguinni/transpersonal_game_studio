#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Combat_DamageSystem.generated.h"

// Estrutura para informação de dano
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Amount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* Instigator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ZeroVector;

    FCombat_DamageInfo()
    {
        Amount = 0.0f;
        DamageType = EDamageType::Physical;
        Instigator = nullptr;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ZeroVector;
    }
};

// Estrutura para resistências de dano
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageResistance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float PhysicalResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float FireResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float ColdResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float PoisonResistance = 0.0f;

    FCombat_DamageResistance()
    {
        PhysicalResistance = 0.0f;
        FireResistance = 0.0f;
        ColdResistance = 0.0f;
        PoisonResistance = 0.0f;
    }
};

// Delegate para eventos de dano
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, const FCombat_DamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

/**
 * Sistema de dano e saúde para combate
 * Gere health, resistências e eventos de dano
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DamageSystem();

protected:
    virtual void BeginPlay() override;

public:
    // === PROPRIEDADES DE SAÚDE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bCanRegenerate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (EditCondition = "bCanRegenerate"))
    float RegenerationRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (EditCondition = "bCanRegenerate"))
    float RegenerationDelay = 5.0f;

    // === RESISTÊNCIAS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    FCombat_DamageResistance DamageResistances;

    // === CONFIGURAÇÕES DE COMBATE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInvulnerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanTakeFriendlyFire = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FString> ImmunityTags;

    // === EVENTOS ===
    
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnDeath OnDeath;

    // === MÉTODOS PÚBLICOS ===
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool TakeDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Kill();

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const { return CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercentage() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanTakeDamageFrom(AActor* DamageInstigator) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddImmunity(const FString& Tag);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveImmunity(const FString& Tag);

private:
    // === PROPRIEDADES PRIVADAS ===
    
    FTimerHandle RegenerationTimerHandle;
    float LastDamageTime = 0.0f;

    // === MÉTODOS PRIVADOS ===
    
    float CalculateDamageReduction(EDamageType DamageType) const;
    void StartRegeneration();
    void StopRegeneration();
    void RegenerateHealth();
    void BroadcastHealthChanged();
    void HandleDeath();
};