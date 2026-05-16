#include "Combat_DamageSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    ArmorValue = 0.0f;
    DamageReduction = 0.0f;
    bIsInvulnerable = false;
    bIsDead = false;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsDead = false;
    bIsInvulnerable = false;
    
    UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem initialized for %s with %f health"), 
           *GetOwner()->GetName(), MaxHealth);
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Health regeneration could be added here if needed
}

float UCombat_DamageSystem::DealDamage(AActor* Target, const FCombat_DamageInfo& DamageInfo)
{
    if (!Target || !IsValid(Target))
    {
        return 0.0f;
    }
    
    UCombat_DamageSystem* TargetDamageSystem = Target->FindComponentByClass<UCombat_DamageSystem>();
    if (!TargetDamageSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target %s has no damage system component"), *Target->GetName());
        return 0.0f;
    }
    
    float DamageDealt = TargetDamageSystem->TakeDamage(DamageInfo, GetOwner());
    
    if (DamageDealt > 0.0f)
    {
        OnDamageDealt.Broadcast(Target, DamageDealt, GetOwner());
        UE_LOG(LogTemp, Log, TEXT("%s dealt %f damage to %s"), 
               *GetOwner()->GetName(), DamageDealt, *Target->GetName());
    }
    
    return DamageDealt;
}

float UCombat_DamageSystem::TakeDamage(const FCombat_DamageInfo& DamageInfo, AActor* DamageSource)
{
    if (!CanTakeDamage() || bIsDead)
    {
        return 0.0f;
    }
    
    float FinalDamage = CalculateFinalDamage(DamageInfo);
    
    CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("%s took %f damage (from %f base), health now %f/%f"), 
           *GetOwner()->GetName(), FinalDamage, DamageInfo.BaseDamage, CurrentHealth, MaxHealth);
    
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        HandleDeath(DamageSource);
    }
    
    return FinalDamage;
}

float UCombat_DamageSystem::CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const
{
    float Damage = DamageInfo.BaseDamage;
    
    // Apply armor reduction
    float EffectiveArmor = FMath::Max(0.0f, ArmorValue - DamageInfo.ArmorPenetration);
    float ArmorReduction = EffectiveArmor / (EffectiveArmor + 100.0f);
    Damage *= (1.0f - ArmorReduction);
    
    // Apply general damage reduction
    Damage *= (1.0f - DamageReduction);
    
    // Damage type modifiers could be added here
    switch (DamageInfo.DamageType)
    {
        case ECombat_DamageType::Bite:
            // Bite damage might be more effective against soft targets
            break;
        case ECombat_DamageType::Claw:
            // Claw damage might have bleeding effects
            break;
        case ECombat_DamageType::Stomp:
            // Stomp damage might ignore some armor
            Damage *= 1.2f;
            break;
        default:
            break;
    }
    
    return FMath::Max(0.0f, Damage);
}

void UCombat_DamageSystem::SetHealth(float NewHealth)
{
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    
    if (OldHealth > 0.0f && CurrentHealth <= 0.0f && !bIsDead)
    {
        HandleDeath(nullptr);
    }
    else if (OldHealth <= 0.0f && CurrentHealth > 0.0f)
    {
        // Resurrection
        bIsDead = false;
    }
}

void UCombat_DamageSystem::HealDamage(float HealAmount)
{
    if (bIsDead)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("%s healed for %f, health now %f/%f"), 
           *GetOwner()->GetName(), CurrentHealth - OldHealth, CurrentHealth, MaxHealth);
}

float UCombat_DamageSystem::GetHealthPercentage() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return CurrentHealth / MaxHealth;
}

bool UCombat_DamageSystem::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

bool UCombat_DamageSystem::IsAtFullHealth() const
{
    return CurrentHealth >= MaxHealth;
}

bool UCombat_DamageSystem::CanTakeDamage() const
{
    return !bIsInvulnerable && !bIsDead;
}

void UCombat_DamageSystem::SetInvulnerable(bool bInvulnerable, float Duration)
{
    bIsInvulnerable = bInvulnerable;
    
    if (bInvulnerable && Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(InvulnerabilityTimer, this, 
                                               &UCombat_DamageSystem::ClearInvulnerability, 
                                               Duration, false);
    }
    else if (!bInvulnerable)
    {
        GetWorld()->GetTimerManager().ClearTimer(InvulnerabilityTimer);
    }
}

void UCombat_DamageSystem::HandleDeath(AActor* Killer)
{
    if (bIsDead)
    {
        return;
    }
    
    bIsDead = true;
    CurrentHealth = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("%s has died"), *GetOwner()->GetName());
    
    OnActorDied.Broadcast(GetOwner(), Killer);
    
    // Additional death handling could be added here
    // - Disable collision
    // - Play death animation
    // - Drop items
    // - Award experience
}

void UCombat_DamageSystem::ClearInvulnerability()
{
    bIsInvulnerable = false;
    UE_LOG(LogTemp, Log, TEXT("%s is no longer invulnerable"), *GetOwner()->GetName());
}