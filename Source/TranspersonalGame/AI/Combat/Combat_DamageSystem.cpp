#include "Combat_DamageSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default health stats
    HealthStats.MaxHealth = 100.0f;
    HealthStats.CurrentHealth = 100.0f;
    HealthStats.HealthRegenRate = 1.0f;
    HealthStats.DamageResistance = 0.0f;
    HealthStats.bIsInvulnerable = false;
    HealthStats.LastDamageTime = 0.0f;

    // Initialize damage type resistances
    DamageTypeResistances.Add(EDamageType::Physical, 0.0f);
    DamageTypeResistances.Add(EDamageType::Fire, 0.0f);
    DamageTypeResistances.Add(EDamageType::Ice, 0.0f);
    DamageTypeResistances.Add(EDamageType::Poison, 0.0f);
    DamageTypeResistances.Add(EDamageType::Bleed, 0.0f);

    // Initialize visual settings
    bShowDamageNumbers = true;
    DamageNumberDisplayTime = 2.0f;
    CriticalHitColor = FLinearColor::Red;
    NormalHitColor = FLinearColor::White;

    bIsDead = false;
    LastRegenTime = 0.0f;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();

    // Ensure current health doesn't exceed max health
    HealthStats.CurrentHealth = FMath::Min(HealthStats.CurrentHealth, HealthStats.MaxHealth);
    
    // Initialize last damage time
    HealthStats.LastDamageTime = GetWorld()->GetTimeSeconds();
    LastRegenTime = GetWorld()->GetTimeSeconds();
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsDead && HealthStats.HealthRegenRate > 0.0f)
    {
        ProcessHealthRegeneration(DeltaTime);
    }

    // Clean up expired damage number actors
    for (int32 i = DamageNumberActors.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(DamageNumberActors[i]))
        {
            DamageNumberActors.RemoveAt(i);
        }
    }
}

bool UCombat_DamageSystem::ApplyDamage(const FCombat_DamageInfo& DamageInfo)
{
    if (bIsDead || HealthStats.bIsInvulnerable || DamageInfo.DamageAmount <= 0.0f)
    {
        return false;
    }

    // Calculate actual damage after resistances
    float ActualDamage = CalculateActualDamage(DamageInfo);
    
    if (ActualDamage <= 0.0f)
    {
        return false;
    }

    // Apply damage to health
    HealthStats.CurrentHealth = FMath::Max(0.0f, HealthStats.CurrentHealth - ActualDamage);
    HealthStats.LastDamageTime = GetWorld()->GetTimeSeconds();

    // Show damage numbers if enabled
    if (bShowDamageNumbers)
    {
        ShowDamageNumber(DamageInfo, ActualDamage);
    }

    // Broadcast damage taken event
    OnDamageTaken.Broadcast(ActualDamage, DamageInfo);
    OnHealthChanged.Broadcast(GetHealthPercentage());

    // Check for death
    if (HealthStats.CurrentHealth <= 0.0f && !bIsDead)
    {
        TriggerDeathSequence();
    }

    return true;
}

void UCombat_DamageSystem::HealDamage(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.0f)
    {
        return;
    }

    float OldHealth = HealthStats.CurrentHealth;
    HealthStats.CurrentHealth = FMath::Min(HealthStats.MaxHealth, HealthStats.CurrentHealth + HealAmount);

    if (HealthStats.CurrentHealth != OldHealth)
    {
        OnHealthChanged.Broadcast(GetHealthPercentage());
    }
}

void UCombat_DamageSystem::SetMaxHealth(float NewMaxHealth)
{
    if (NewMaxHealth <= 0.0f)
    {
        return;
    }

    float HealthPercentage = GetHealthPercentage();
    HealthStats.MaxHealth = NewMaxHealth;
    HealthStats.CurrentHealth = HealthStats.MaxHealth * HealthPercentage;

    OnHealthChanged.Broadcast(GetHealthPercentage());
}

void UCombat_DamageSystem::SetInvulnerable(bool bNewInvulnerable)
{
    HealthStats.bIsInvulnerable = bNewInvulnerable;
}

float UCombat_DamageSystem::GetHealthPercentage() const
{
    if (HealthStats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return HealthStats.CurrentHealth / HealthStats.MaxHealth;
}

bool UCombat_DamageSystem::IsAlive() const
{
    return !bIsDead && HealthStats.CurrentHealth > 0.0f;
}

bool UCombat_DamageSystem::IsAtFullHealth() const
{
    return HealthStats.CurrentHealth >= HealthStats.MaxHealth;
}

float UCombat_DamageSystem::CalculateActualDamage(const FCombat_DamageInfo& DamageInfo) const
{
    float BaseDamage = DamageInfo.DamageAmount;
    
    // Apply critical hit multiplier
    if (DamageInfo.bIsCriticalHit)
    {
        BaseDamage *= 2.0f;
    }

    // Apply general damage resistance
    BaseDamage *= (1.0f - HealthStats.DamageResistance);

    // Apply damage type specific resistance
    float TypeResistance = GetDamageTypeResistance(DamageInfo.DamageType);
    BaseDamage *= (1.0f - TypeResistance);

    return FMath::Max(0.0f, BaseDamage);
}

void UCombat_DamageSystem::ProcessHealthRegeneration(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastDamage = CurrentTime - HealthStats.LastDamageTime;
    
    // Only regenerate if enough time has passed since last damage (3 seconds)
    if (TimeSinceLastDamage >= 3.0f && HealthStats.CurrentHealth < HealthStats.MaxHealth)
    {
        float RegenAmount = HealthStats.HealthRegenRate * DeltaTime;
        HealDamage(RegenAmount);
    }
}

void UCombat_DamageSystem::TriggerDeathSequence()
{
    bIsDead = true;
    HealthStats.CurrentHealth = 0.0f;
    
    // Broadcast death event
    OnDeath.Broadcast();

    // Disable collision and movement for the owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Disable collision
        Owner->SetActorEnableCollision(false);
        
        // If it's a character, disable movement
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            Character->DisableInput(nullptr);
        }
    }
}

void UCombat_DamageSystem::ShowDamageNumber(const FCombat_DamageInfo& DamageInfo, float ActualDamage)
{
    if (!GetWorld())
    {
        return;
    }

    // Create a simple static mesh actor to display damage number
    FVector SpawnLocation = DamageInfo.ImpactLocation;
    if (SpawnLocation.IsZero() && GetOwner())
    {
        SpawnLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 100);
    }

    AActor* DamageNumberActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    if (DamageNumberActor)
    {
        DamageNumberActor->SetLifeSpan(DamageNumberDisplayTime);
        DamageNumberActors.Add(DamageNumberActor);

        // Set color based on critical hit
        FLinearColor DisplayColor = DamageInfo.bIsCriticalHit ? CriticalHitColor : NormalHitColor;
        
        // Log damage for debugging
        UE_LOG(LogTemp, Log, TEXT("Damage: %.1f %s"), ActualDamage, DamageInfo.bIsCriticalHit ? TEXT("CRITICAL!") : TEXT(""));
    }
}

float UCombat_DamageSystem::GetDamageTypeResistance(EDamageType DamageType) const
{
    if (const float* Resistance = DamageTypeResistances.Find(DamageType))
    {
        return *Resistance;
    }
    return 0.0f;
}