#include "CombatDamageComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatDamageComponent::UCombatDamageComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    bCanRegenerate = false;
    RegenerationRate = 1.0f;
    
    BiteResistance = 0.0f;
    ClawResistance = 0.0f;
    CrushResistance = 0.0f;
    
    bIsInCombat = false;
    LastDamageTime = 0.0f;
    CombatTimeout = 10.0f;
}

void UCombatDamageComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsInCombat = false;
    LastDamageTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("CombatDamageComponent initialized for %s - Health: %.1f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), CurrentHealth);
}

void UCombatDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsAlive())
    {
        return;
    }
    
    // Update combat state
    UpdateCombatState();
    
    // Handle health regeneration
    if (bCanRegenerate && !bIsInCombat && CurrentHealth < MaxHealth)
    {
        float RegenAmount = RegenerationRate * DeltaTime;
        CurrentHealth = FMath::Min(CurrentHealth + RegenAmount, MaxHealth);
    }
}

void UCombatDamageComponent::TakeDamage(const FCombat_DamageInfo& DamageInfo)
{
    if (!IsAlive() || DamageInfo.Amount <= 0.0f)
    {
        return;
    }
    
    // Calculate resistance
    float Resistance = GetDamageResistance(DamageInfo.DamageType);
    float ActualDamage = DamageInfo.Amount * (1.0f - Resistance);
    
    // Apply damage
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
    LastDamageTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Update combat state
    bool bWasInCombat = bIsInCombat;
    bIsInCombat = true;
    
    if (!bWasInCombat)
    {
        OnCombatStateChanged(true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s took %.1f damage (%.1f after resistance) - Health: %.1f/%.1f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
           DamageInfo.Amount, ActualDamage, CurrentHealth, MaxHealth);
    
    // Trigger damage event
    OnDamageTaken(DamageInfo);
    
    // Check for death
    if (!IsAlive())
    {
        UE_LOG(LogTemp, Error, TEXT("%s has died!"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
        OnDeath();
    }
}

void UCombatDamageComponent::Heal(float HealAmount)
{
    if (!IsAlive() || HealAmount <= 0.0f)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Min(CurrentHealth + HealAmount, MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("%s healed for %.1f - Health: %.1f/%.1f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
           CurrentHealth - OldHealth, CurrentHealth, MaxHealth);
}

void UCombatDamageComponent::SetHealth(float NewHealth)
{
    CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    
    if (!IsAlive())
    {
        OnDeath();
    }
}

float UCombatDamageComponent::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool UCombatDamageComponent::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

bool UCombatDamageComponent::IsInCombat() const
{
    return bIsInCombat;
}

float UCombatDamageComponent::GetDamageResistance(ECombat_DamageType DamageType) const
{
    switch (DamageType)
    {
        case ECombat_DamageType::Bite:
            return BiteResistance;
        case ECombat_DamageType::Claw:
            return ClawResistance;
        case ECombat_DamageType::Crush:
            return CrushResistance;
        case ECombat_DamageType::Pierce:
            return 0.0f; // No specific resistance property
        case ECombat_DamageType::Blunt:
            return 0.0f; // No specific resistance property
        default:
            return 0.0f;
    }
}

void UCombatDamageComponent::UpdateCombatState()
{
    if (!bIsInCombat)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastDamageTime >= CombatTimeout)
    {
        bIsInCombat = false;
        OnCombatStateChanged(false);
        
        UE_LOG(LogTemp, Log, TEXT("%s exited combat state"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}