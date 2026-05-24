// SurvivalComponent.cpp
#include "Core/Survival/SurvivalComponent.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // Init to max on construction
    CurrentHealth  = MaxHealth;
    CurrentHunger  = MaxHunger;
    CurrentThirst  = MaxThirst;
    CurrentStamina = MaxStamina;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Broadcast initial values
    OnHealthChanged.Broadcast(CurrentHealth,  MaxHealth);
    OnHungerChanged.Broadcast(CurrentHunger,  MaxHunger);
    OnThirstChanged.Broadcast(CurrentThirst,  MaxThirst);
    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bIsAlive) return;
    DrainStats(DeltaTime);
    ApplyStarvationDamage(DeltaTime);
}

void USurvivalComponent::DrainStats(float DeltaTime)
{
    // Hunger
    float OldHunger = CurrentHunger;
    CurrentHunger = FMath::Clamp(CurrentHunger - HungerDrainRate * DeltaTime, 0.f, MaxHunger);
    if (!FMath::IsNearlyEqual(OldHunger, CurrentHunger))
        OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);

    // Thirst
    float OldThirst = CurrentThirst;
    CurrentThirst = FMath::Clamp(CurrentThirst - ThirstDrainRate * DeltaTime, 0.f, MaxThirst);
    if (!FMath::IsNearlyEqual(OldThirst, CurrentThirst))
        OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);

    // Stamina: regen when not running, drain when running
    float OldStamina = CurrentStamina;
    float StaminaDelta = bIsRunning
        ? -StaminaDrainRate * DeltaTime
        :  StaminaRegenRate * DeltaTime;
    CurrentStamina = FMath::Clamp(CurrentStamina + StaminaDelta, 0.f, MaxStamina);
    if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
        OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void USurvivalComponent::ApplyStarvationDamage(float DeltaTime)
{
    float Dmg = 0.f;
    if (CurrentHunger <= 0.f) Dmg += HungerDamageRate * DeltaTime;
    if (CurrentThirst <= 0.f) Dmg += ThirstDamageRate * DeltaTime;
    if (Dmg > 0.f) TakeDamage(Dmg);
}

void USurvivalComponent::Eat(float Amount)
{
    if (!bIsAlive) return;
    CurrentHunger = FMath::Clamp(CurrentHunger + Amount, 0.f, MaxHunger);
    OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
}

void USurvivalComponent::Drink(float Amount)
{
    if (!bIsAlive) return;
    CurrentThirst = FMath::Clamp(CurrentThirst + Amount, 0.f, MaxThirst);
    OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
}

void USurvivalComponent::TakeDamage(float DamageAmount)
{
    if (!bIsAlive) return;
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    if (CurrentHealth <= 0.f) Die();
}

void USurvivalComponent::Heal(float HealAmount)
{
    if (!bIsAlive) return;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void USurvivalComponent::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
    OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

bool USurvivalComponent::HasEnoughStamina(float Amount) const
{
    return CurrentStamina >= Amount;
}

void USurvivalComponent::Die()
{
    bIsAlive = false;
    OnPlayerDeath.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: Player has died."));
}
