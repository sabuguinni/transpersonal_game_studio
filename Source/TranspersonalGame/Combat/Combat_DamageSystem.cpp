#include "Combat_DamageSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second for status effects
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    HealthRegenRate = 1.0f;
    HealthRegenDelay = 5.0f;
    
    bIsBleeding = false;
    bIsPoisoned = false;
    bIsOnFire = false;
    bIsDead = false;
    
    LastDamageTime = 0.0f;
    BleedingTimer = 0.0f;
    BleedingDPS = 0.0f;
    PoisonTimer = 0.0f;
    PoisonDPS = 0.0f;
    BurningTimer = 0.0f;
    BurningDPS = 0.0f;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    LastDamageTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem initialized for %s with %f max health"), 
           *GetOwner()->GetName(), MaxHealth);
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDead)
    {
        return;
    }
    
    ProcessStatusEffects(DeltaTime);
    ProcessHealthRegeneration(DeltaTime);
}

float UCombat_DamageSystem::TakeDamage(const FCombat_DamageInfo& DamageInfo)
{
    if (bIsDead)
    {
        return 0.0f;
    }
    
    float FinalDamage = CalculateFinalDamage(DamageInfo);
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    LastDamageTime = GetWorld()->GetTimeSeconds();
    
    // Broadcast events
    OnDamageTaken.Broadcast(FinalDamage, DamageInfo);
    OnHealthChanged.Broadcast(CurrentHealth);
    
    // Check for death
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        Die();
    }
    
    // Apply status effects based on damage type
    switch (DamageInfo.DamageType)
    {
        case EDamageType::Bleeding:
            ApplyBleeding(3.0f, FinalDamage * 0.1f);
            break;
        case EDamageType::Poison:
            ApplyPoison(5.0f, FinalDamage * 0.05f);
            break;
        case EDamageType::Fire:
            ApplyBurning(4.0f, FinalDamage * 0.15f);
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s took %f damage from %s. Health: %f/%f"), 
           *GetOwner()->GetName(), FinalDamage, 
           DamageInfo.DamageSource ? *DamageInfo.DamageSource->GetName() : TEXT("Unknown"),
           CurrentHealth, MaxHealth);
    
    return FinalDamage;
}

void UCombat_DamageSystem::DealDamageToTarget(AActor* Target, const FCombat_DamageInfo& DamageInfo)
{
    if (!Target)
    {
        return;
    }
    
    UCombat_DamageSystem* TargetDamageSystem = Target->FindComponentByClass<UCombat_DamageSystem>();
    if (TargetDamageSystem)
    {
        float DamageDealt = TargetDamageSystem->TakeDamage(DamageInfo);
        OnDamageDealt.Broadcast(Target, DamageInfo);
        
        UE_LOG(LogTemp, Log, TEXT("%s dealt %f damage to %s"), 
               *GetOwner()->GetName(), DamageDealt, *Target->GetName());
    }
}

void UCombat_DamageSystem::Heal(float HealAmount)
{
    if (bIsDead)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    
    if (CurrentHealth != OldHealth)
    {
        OnHealthChanged.Broadcast(CurrentHealth);
        
        UE_LOG(LogTemp, Log, TEXT("%s healed for %f. Health: %f/%f"), 
               *GetOwner()->GetName(), HealAmount, CurrentHealth, MaxHealth);
    }
}

void UCombat_DamageSystem::SetMaxHealth(float NewMaxHealth)
{
    float HealthRatio = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 1.0f;
    MaxHealth = FMath::Max(1.0f, NewMaxHealth);
    CurrentHealth = MaxHealth * HealthRatio;
    
    OnHealthChanged.Broadcast(CurrentHealth);
}

float UCombat_DamageSystem::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

bool UCombat_DamageSystem::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

void UCombat_DamageSystem::ApplyBleeding(float Duration, float DamagePerSecond)
{
    bIsBleeding = true;
    BleedingTimer = Duration;
    BleedingDPS = DamagePerSecond;
    
    UE_LOG(LogTemp, Warning, TEXT("%s is bleeding! %f DPS for %f seconds"), 
           *GetOwner()->GetName(), DamagePerSecond, Duration);
}

void UCombat_DamageSystem::ApplyPoison(float Duration, float DamagePerSecond)
{
    bIsPoisoned = true;
    PoisonTimer = Duration;
    PoisonDPS = DamagePerSecond;
    
    UE_LOG(LogTemp, Warning, TEXT("%s is poisoned! %f DPS for %f seconds"), 
           *GetOwner()->GetName(), DamagePerSecond, Duration);
}

void UCombat_DamageSystem::ApplyBurning(float Duration, float DamagePerSecond)
{
    bIsOnFire = true;
    BurningTimer = Duration;
    BurningDPS = DamagePerSecond;
    
    UE_LOG(LogTemp, Warning, TEXT("%s is burning! %f DPS for %f seconds"), 
           *GetOwner()->GetName(), DamagePerSecond, Duration);
}

void UCombat_DamageSystem::ClearAllStatusEffects()
{
    bIsBleeding = false;
    bIsPoisoned = false;
    bIsOnFire = false;
    BleedingTimer = 0.0f;
    PoisonTimer = 0.0f;
    BurningTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("%s cleared all status effects"), *GetOwner()->GetName());
}

float UCombat_DamageSystem::CalculateDamageReduction(EDamageType DamageType) const
{
    switch (DamageType)
    {
        case EDamageType::Physical:
            return ResistanceData.PhysicalResistance;
        case EDamageType::Fire:
            return ResistanceData.FireResistance;
        case EDamageType::Poison:
            return ResistanceData.PoisonResistance;
        case EDamageType::Bleeding:
            return ResistanceData.BleedingResistance;
        default:
            return 0.0f;
    }
}

void UCombat_DamageSystem::ModifyResistance(EDamageType DamageType, float ResistanceChange)
{
    switch (DamageType)
    {
        case EDamageType::Physical:
            ResistanceData.PhysicalResistance = FMath::Clamp(ResistanceData.PhysicalResistance + ResistanceChange, 0.0f, 0.95f);
            break;
        case EDamageType::Fire:
            ResistanceData.FireResistance = FMath::Clamp(ResistanceData.FireResistance + ResistanceChange, 0.0f, 0.95f);
            break;
        case EDamageType::Poison:
            ResistanceData.PoisonResistance = FMath::Clamp(ResistanceData.PoisonResistance + ResistanceChange, 0.0f, 0.95f);
            break;
        case EDamageType::Bleeding:
            ResistanceData.BleedingResistance = FMath::Clamp(ResistanceData.BleedingResistance + ResistanceChange, 0.0f, 0.95f);
            break;
    }
}

void UCombat_DamageSystem::ProcessStatusEffects(float DeltaTime)
{
    // Process bleeding
    if (bIsBleeding && BleedingTimer > 0.0f)
    {
        BleedingTimer -= DeltaTime;
        
        FCombat_DamageInfo BleedDamage;
        BleedDamage.BaseDamage = BleedingDPS * DeltaTime;
        BleedDamage.DamageType = EDamageType::Bleeding;
        BleedDamage.DamageSource = GetOwner();
        
        TakeDamage(BleedDamage);
        
        if (BleedingTimer <= 0.0f)
        {
            bIsBleeding = false;
            UE_LOG(LogTemp, Log, TEXT("%s stopped bleeding"), *GetOwner()->GetName());
        }
    }
    
    // Process poison
    if (bIsPoisoned && PoisonTimer > 0.0f)
    {
        PoisonTimer -= DeltaTime;
        
        FCombat_DamageInfo PoisonDamage;
        PoisonDamage.BaseDamage = PoisonDPS * DeltaTime;
        PoisonDamage.DamageType = EDamageType::Poison;
        PoisonDamage.DamageSource = GetOwner();
        
        TakeDamage(PoisonDamage);
        
        if (PoisonTimer <= 0.0f)
        {
            bIsPoisoned = false;
            UE_LOG(LogTemp, Log, TEXT("%s recovered from poison"), *GetOwner()->GetName());
        }
    }
    
    // Process burning
    if (bIsOnFire && BurningTimer > 0.0f)
    {
        BurningTimer -= DeltaTime;
        
        FCombat_DamageInfo BurnDamage;
        BurnDamage.BaseDamage = BurningDPS * DeltaTime;
        BurnDamage.DamageType = EDamageType::Fire;
        BurnDamage.DamageSource = GetOwner();
        
        TakeDamage(BurnDamage);
        
        if (BurningTimer <= 0.0f)
        {
            bIsOnFire = false;
            UE_LOG(LogTemp, Log, TEXT("%s stopped burning"), *GetOwner()->GetName());
        }
    }
}

void UCombat_DamageSystem::ProcessHealthRegeneration(float DeltaTime)
{
    if (CurrentHealth < MaxHealth && HealthRegenRate > 0.0f)
    {
        float TimeSinceLastDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;
        
        if (TimeSinceLastDamage >= HealthRegenDelay)
        {
            Heal(HealthRegenRate * DeltaTime);
        }
    }
}

void UCombat_DamageSystem::Die()
{
    if (bIsDead)
    {
        return;
    }
    
    bIsDead = true;
    CurrentHealth = 0.0f;
    
    // Clear all status effects on death
    ClearAllStatusEffects();
    
    // Broadcast death event
    OnDeath.Broadcast();
    
    UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetOwner()->GetName());
    
    // Disable collision and physics for the actor
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        OwnerPawn->SetActorEnableCollision(false);
        
        if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(OwnerPawn->GetRootComponent()))
        {
            RootComp->SetSimulatePhysics(false);
        }
    }
}

float UCombat_DamageSystem::CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const
{
    float BaseDamage = DamageInfo.BaseDamage;
    
    // Apply critical hit multiplier
    if (DamageInfo.bIsCriticalHit || ShouldBeCriticalHit(DamageInfo))
    {
        BaseDamage *= DamageInfo.CriticalMultiplier;
    }
    
    // Apply resistance
    float Resistance = CalculateDamageReduction(DamageInfo.DamageType);
    float FinalDamage = BaseDamage * (1.0f - Resistance);
    
    return FMath::Max(0.0f, FinalDamage);
}

bool UCombat_DamageSystem::ShouldBeCriticalHit(const FCombat_DamageInfo& DamageInfo) const
{
    // 5% base critical hit chance
    return FMath::RandRange(0.0f, 1.0f) <= 0.05f;
}