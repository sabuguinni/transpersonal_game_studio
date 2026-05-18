#include "Combat_DamageSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    PhysicalResistance = 0.0f;
    BiteResistance = 0.0f;
    ClawResistance = 0.0f;
    CriticalHealthThreshold = 0.25f;
    bIsDead = false;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsDead = false;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Combat Damage System initialized for %s - Health: %.1f"), 
            *GetOwner()->GetName(), CurrentHealth));
    }
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Tick is disabled by default, only enabled when needed for special effects
}

void UCombat_DamageSystem::ApplyDamage(const FCombat_DamageInfo& DamageInfo)
{
    if (bIsDead || DamageInfo.Amount <= 0.0f)
    {
        return;
    }

    float FinalDamage = CalculateFinalDamage(DamageInfo);
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    
    // Broadcast damage event
    OnDamageTaken.Broadcast(FinalDamage, DamageInfo.Instigator);
    OnHealthChanged.Broadcast(GetHealthPercentage());
    
    if (GEngine)
    {
        FString InstigatorName = DamageInfo.Instigator ? DamageInfo.Instigator->GetName() : TEXT("Unknown");
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("%s took %.1f %s damage from %s - Health: %.1f/%.1f"), 
            *GetOwner()->GetName(), FinalDamage, 
            *UEnum::GetValueAsString(DamageInfo.DamageType),
            *InstigatorName, CurrentHealth, MaxHealth));
    }
    
    // Check for death
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        HandleDeath();
    }
}

void UCombat_DamageSystem::ApplySimpleDamage(float DamageAmount, AActor* DamageInstigator)
{
    FCombat_DamageInfo DamageInfo;
    DamageInfo.Amount = DamageAmount;
    DamageInfo.DamageType = ECombat_DamageType::Physical;
    DamageInfo.Instigator = DamageInstigator;
    
    ApplyDamage(DamageInfo);
}

void UCombat_DamageSystem::Heal(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.0f)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    
    OnHealthChanged.Broadcast(GetHealthPercentage());
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
            FString::Printf(TEXT("%s healed for %.1f - Health: %.1f/%.1f"), 
            *GetOwner()->GetName(), HealAmount, CurrentHealth, MaxHealth));
    }
}

void UCombat_DamageSystem::SetHealth(float NewHealth)
{
    CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(GetHealthPercentage());
    
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        HandleDeath();
    }
    else if (bIsDead && CurrentHealth > 0.0f)
    {
        bIsDead = false;
    }
}

float UCombat_DamageSystem::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool UCombat_DamageSystem::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

bool UCombat_DamageSystem::IsCriticalHealth() const
{
    return GetHealthPercentage() <= CriticalHealthThreshold;
}

float UCombat_DamageSystem::CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo)
{
    float BaseDamage = DamageInfo.Amount;
    float Resistance = 0.0f;
    
    // Apply damage type specific resistance
    switch (DamageInfo.DamageType)
    {
        case ECombat_DamageType::Physical:
            Resistance = PhysicalResistance;
            break;
        case ECombat_DamageType::Bite:
            Resistance = BiteResistance;
            break;
        case ECombat_DamageType::Claw:
            Resistance = ClawResistance;
            break;
        case ECombat_DamageType::Crush:
            Resistance = PhysicalResistance * 1.5f; // Crush is harder to resist
            break;
        case ECombat_DamageType::Environmental:
            Resistance = PhysicalResistance * 0.5f; // Environmental damage bypasses some resistance
            break;
        default:
            Resistance = PhysicalResistance;
            break;
    }
    
    // Calculate final damage with resistance
    float DamageMultiplier = FMath::Max(0.0f, 1.0f - (Resistance / 100.0f));
    return BaseDamage * DamageMultiplier;
}

void UCombat_DamageSystem::HandleDeath()
{
    if (bIsDead)
    {
        return;
    }
    
    bIsDead = true;
    CurrentHealth = 0.0f;
    
    OnDeath.Broadcast();
    OnHealthChanged.Broadcast(0.0f);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            FString::Printf(TEXT("%s has died!"), *GetOwner()->GetName()));
    }
    
    // Disable tick component to save performance
    SetComponentTickEnabled(false);
}