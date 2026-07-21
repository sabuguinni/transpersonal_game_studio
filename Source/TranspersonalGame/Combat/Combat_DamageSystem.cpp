#include "Combat_DamageSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    DamageResistance = 0.0f;
    bCanTakeDamage = true;
    InvulnerabilityDuration = 0.5f;
    LastDamageTime = -1.0f;
    
    // Initialize damage type resistances
    DamageTypeResistances.Add(EDamageType::Physical, 0.0f);
    DamageTypeResistances.Add(EDamageType::Fire, 0.0f);
    DamageTypeResistances.Add(EDamageType::Ice, 0.0f);
    DamageTypeResistances.Add(EDamageType::Poison, 0.0f);
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    LastDamageTime = -1.0f;
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update invulnerability state
    if (LastDamageTime >= 0.0f)
    {
        float TimeSinceLastDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;
        if (TimeSinceLastDamage >= InvulnerabilityDuration)
        {
            bCanTakeDamage = true;
        }
    }
}

void UCombat_DamageSystem::ApplyDamage(const FCombat_DamageData& DamageData)
{
    if (!bCanTakeDamage || CurrentHealth <= 0.0f)
    {
        return;
    }
    
    float FinalDamage = CalculateFinalDamage(DamageData.DamageAmount, DamageData.DamageType);
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    LastDamageTime = GetWorld()->GetTimeSeconds();
    bCanTakeDamage = false;
    
    // Fire damage taken event
    OnDamageTaken(DamageData);
    
    // Check for death
    if (CurrentHealth <= 0.0f)
    {
        HandleDeath(DamageData.DamageInstigator);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Damage Applied: %.1f (Final: %.1f) to %s. Health: %.1f/%.1f"), 
           DamageData.DamageAmount, FinalDamage, 
           *GetOwner()->GetName(), CurrentHealth, MaxHealth);
}

void UCombat_DamageSystem::ApplyDamageToTarget(AActor* Target, float Damage, EDamageType DamageType, AActor* Instigator)
{
    if (!Target)
    {
        return;
    }
    
    UCombat_DamageSystem* TargetDamageSystem = Target->FindComponentByClass<UCombat_DamageSystem>();
    if (TargetDamageSystem)
    {
        FCombat_DamageData DamageData;
        DamageData.DamageAmount = Damage;
        DamageData.DamageType = DamageType;
        DamageData.ImpactLocation = Target->GetActorLocation();
        DamageData.ImpactDirection = (Target->GetActorLocation() - Instigator->GetActorLocation()).GetSafeNormal();
        DamageData.DamageInstigator = Instigator;
        
        TargetDamageSystem->ApplyDamage(DamageData);
    }
}

void UCombat_DamageSystem::HandleDeath(AActor* Killer)
{
    UE_LOG(LogTemp, Warning, TEXT("%s has died! Killed by: %s"), 
           *GetOwner()->GetName(), 
           Killer ? *Killer->GetName() : TEXT("Unknown"));
    
    // Fire death event
    OnDeath(Killer);
    
    // Disable further damage
    bCanTakeDamage = false;
    
    // Optional: Destroy actor after delay or handle respawn
    if (GetOwner())
    {
        GetOwner()->SetActorEnableCollision(false);
        GetOwner()->SetActorHiddenInGame(true);
    }
}

float UCombat_DamageSystem::CalculateFinalDamage(float BaseDamage, EDamageType DamageType)
{
    float FinalDamage = BaseDamage;
    
    // Apply general damage resistance
    FinalDamage *= (1.0f - FMath::Clamp(DamageResistance, 0.0f, 0.95f));
    
    // Apply damage type specific resistance
    if (DamageTypeResistances.Contains(DamageType))
    {
        float TypeResistance = DamageTypeResistances[DamageType];
        FinalDamage *= (1.0f - FMath::Clamp(TypeResistance, 0.0f, 0.95f));
    }
    
    return FMath::Max(1.0f, FinalDamage); // Minimum 1 damage
}