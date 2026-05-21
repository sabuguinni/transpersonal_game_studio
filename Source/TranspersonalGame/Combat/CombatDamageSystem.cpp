#include "CombatDamageSystem.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UCombatDamageSystem::UCombatDamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    ArmorValue = 0.0f;
    DamageReduction = 0.0f;
    bIsAlive = true;
    bIsKnockedDown = false;
    KnockdownRecoveryTime = 3.0f;
}

void UCombatDamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsAlive = true;
    bIsKnockedDown = false;
    
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(GetHealthPercentage());
    }
}

void UCombatDamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update knockdown status visualization or other per-frame combat logic
    if (bIsKnockedDown && GetOwner())
    {
        // Could add visual effects or physics updates here
    }
}

float UCombatDamageSystem::ApplyDamage(const FCombat_DamageInfo& DamageInfo, AActor* DamageSource)
{
    if (!bIsAlive)
    {
        return 0.0f;
    }
    
    float FinalDamage = CalculateFinalDamage(DamageInfo);
    
    // Apply damage
    CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
    
    // Broadcast damage event
    if (OnDamageTaken.IsBound())
    {
        OnDamageTaken.Broadcast(FinalDamage, DamageInfo.DamageType, DamageSource);
    }
    
    // Broadcast health change
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(GetHealthPercentage());
    }
    
    // Check for knockdown
    if (DamageInfo.bCanKnockdown && FinalDamage > 20.0f && !bIsKnockedDown)
    {
        ApplyKnockdown(KnockdownRecoveryTime, DamageInfo.KnockdownForce);
    }
    
    // Check for death
    if (CurrentHealth <= 0.0f && bIsAlive)
    {
        KillActor();
    }
    
    // Debug output
    if (GEngine && GetOwner())
    {
        FString DebugMsg = FString::Printf(TEXT("%s took %.1f %s damage (%.1f/%.1f HP)"), 
            *GetOwner()->GetName(), 
            FinalDamage,
            *UEnum::GetValueAsString(DamageInfo.DamageType),
            CurrentHealth, 
            MaxHealth);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, DebugMsg);
    }
    
    return FinalDamage;
}

void UCombatDamageSystem::ApplyHealing(float HealAmount)
{
    if (!bIsAlive || HealAmount <= 0.0f)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
    
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(GetHealthPercentage());
    }
    
    if (GEngine && GetOwner())
    {
        FString DebugMsg = FString::Printf(TEXT("%s healed %.1f HP (%.1f/%.1f)"), 
            *GetOwner()->GetName(), 
            CurrentHealth - OldHealth,
            CurrentHealth, 
            MaxHealth);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, DebugMsg);
    }
}

void UCombatDamageSystem::SetMaxHealth(float NewMaxHealth)
{
    if (NewMaxHealth <= 0.0f)
    {
        return;
    }
    
    float HealthPercentage = GetHealthPercentage();
    MaxHealth = NewMaxHealth;
    CurrentHealth = MaxHealth * HealthPercentage;
    
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(GetHealthPercentage());
    }
}

void UCombatDamageSystem::ResetHealth()
{
    CurrentHealth = MaxHealth;
    bIsAlive = true;
    bIsKnockedDown = false;
    
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(GetHealthPercentage());
    }
}

void UCombatDamageSystem::KillActor()
{
    if (!bIsAlive)
    {
        return;
    }
    
    bIsAlive = false;
    CurrentHealth = 0.0f;
    
    if (OnDeath.IsBound())
    {
        OnDeath.Broadcast();
    }
    
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(0.0f);
    }
    
    if (GEngine && GetOwner())
    {
        FString DebugMsg = FString::Printf(TEXT("%s has died"), *GetOwner()->GetName());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, DebugMsg);
    }
}

void UCombatDamageSystem::ApplyKnockdown(float Duration, float Force)
{
    if (bIsKnockedDown || !bIsAlive)
    {
        return;
    }
    
    bIsKnockedDown = true;
    KnockdownRecoveryTime = Duration;
    
    // Apply physics impulse if owner has physics
    if (GetOwner())
    {
        if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()))
        {
            if (RootComp->IsSimulatingPhysics())
            {
                FVector KnockdownDirection = FVector(0, 0, -1) + FVector(FMath::RandRange(-0.5f, 0.5f), FMath::RandRange(-0.5f, 0.5f), 0);
                KnockdownDirection.Normalize();
                RootComp->AddImpulse(KnockdownDirection * Force, NAME_None, true);
            }
        }
    }
    
    // Set recovery timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(KnockdownRecoveryTimer, this, &UCombatDamageSystem::HandleKnockdownRecovery, Duration, false);
    }
    
    if (OnKnockedDown.IsBound())
    {
        OnKnockedDown.Broadcast();
    }
    
    if (GEngine && GetOwner())
    {
        FString DebugMsg = FString::Printf(TEXT("%s knocked down for %.1f seconds"), *GetOwner()->GetName(), Duration);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, DebugMsg);
    }
}

void UCombatDamageSystem::RecoverFromKnockdown()
{
    if (!bIsKnockedDown)
    {
        return;
    }
    
    bIsKnockedDown = false;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(KnockdownRecoveryTimer);
    }
    
    if (OnRecovered.IsBound())
    {
        OnRecovered.Broadcast();
    }
    
    if (GEngine && GetOwner())
    {
        FString DebugMsg = FString::Printf(TEXT("%s recovered from knockdown"), *GetOwner()->GetName());
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, DebugMsg);
    }
}

void UCombatDamageSystem::HandleKnockdownRecovery()
{
    RecoverFromKnockdown();
}

float UCombatDamageSystem::CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo)
{
    float Damage = DamageInfo.BaseDamage;
    
    // Apply critical hit
    if (ShouldCriticalHit(DamageInfo.CriticalChance))
    {
        Damage *= DamageInfo.CriticalMultiplier;
        
        if (GEngine && GetOwner())
        {
            FString CritMsg = FString::Printf(TEXT("CRITICAL HIT on %s!"), *GetOwner()->GetName());
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, CritMsg);
        }
    }
    
    // Apply armor reduction
    float EffectiveArmor = FMath::Max(0.0f, ArmorValue - DamageInfo.ArmorPenetration);
    float ArmorReduction = EffectiveArmor / (EffectiveArmor + 100.0f); // Diminishing returns formula
    Damage *= (1.0f - ArmorReduction);
    
    // Apply additional damage reduction
    Damage *= (1.0f - FMath::Clamp(DamageReduction, 0.0f, 0.9f));
    
    return FMath::Max(1.0f, Damage); // Minimum 1 damage
}

bool UCombatDamageSystem::ShouldCriticalHit(float CritChance)
{
    return FMath::RandRange(0.0f, 1.0f) <= CritChance;
}