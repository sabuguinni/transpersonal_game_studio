#include "Combat_WeaponSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

UCombat_WeaponSystem::UCombat_WeaponSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    
    // Initialize default weapon stats
    WeaponStats = FCombat_WeaponStats();
    CurrentWeaponType = ECombat_WeaponType::None;
    CurrentWeaponState = ECombat_WeaponState::Idle;
    
    LastAttackTime = 0.0f;
    bIsAttacking = false;
    AttackCooldown = 1.0f;
}

void UCombat_WeaponSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Attach weapon mesh to owner if possible
    if (AActor* Owner = GetOwner())
    {
        if (USceneComponent* RootComp = Owner->GetRootComponent())
        {
            WeaponMesh->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
    
    // Initialize with no weapon
    UnequipWeapon();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_WeaponSystem: BeginPlay completed for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_WeaponSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update attack state
    if (bIsAttacking)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastAttackTime >= AttackCooldown)
        {
            bIsAttacking = false;
            CurrentWeaponState = ECombat_WeaponState::Idle;
        }
    }
    
    // Check for broken weapon
    if (IsWeaponEquipped() && IsWeaponBroken() && CurrentWeaponState != ECombat_WeaponState::Broken)
    {
        CurrentWeaponState = ECombat_WeaponState::Broken;
        UE_LOG(LogTemp, Warning, TEXT("Weapon broken: %s"), 
               *UEnum::GetValueAsString(CurrentWeaponType));
    }
}

bool UCombat_WeaponSystem::EquipWeapon(ECombat_WeaponType WeaponType)
{
    if (WeaponType == ECombat_WeaponType::None)
    {
        return UnequipWeapon();
    }
    
    // Set weapon type and initialize stats
    CurrentWeaponType = WeaponType;
    InitializeWeaponStats(WeaponType);
    UpdateWeaponMesh();
    
    CurrentWeaponState = ECombat_WeaponState::Idle;
    
    UE_LOG(LogTemp, Log, TEXT("Equipped weapon: %s"), 
           *UEnum::GetValueAsString(CurrentWeaponType));
    
    return true;
}

bool UCombat_WeaponSystem::UnequipWeapon()
{
    CurrentWeaponType = ECombat_WeaponType::None;
    CurrentWeaponState = ECombat_WeaponState::Idle;
    
    if (WeaponMesh)
    {
        WeaponMesh->SetVisibility(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weapon unequipped"));
    return true;
}

bool UCombat_WeaponSystem::PerformAttack(FVector TargetLocation)
{
    if (!CanAttack())
    {
        return false;
    }
    
    // Start attack
    bIsAttacking = true;
    LastAttackTime = GetWorld()->GetTimeSeconds();
    CurrentWeaponState = ECombat_WeaponState::Attacking;
    
    // Calculate damage based on weapon and durability
    float DamageAmount = GetCurrentDamage();
    
    // Take durability damage
    TakeDurabilityDamage(5.0f);
    
    // Play attack sound
    PlayAttackSound();
    
    // Spawn hit effect at target location
    SpawnHitEffect(TargetLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Attack performed with %s, damage: %.1f"), 
           *UEnum::GetValueAsString(CurrentWeaponType), DamageAmount);
    
    return true;
}

bool UCombat_WeaponSystem::StartBlocking()
{
    if (!IsWeaponEquipped() || !WeaponStats.bCanBlock || IsWeaponBroken())
    {
        return false;
    }
    
    CurrentWeaponState = ECombat_WeaponState::Blocking;
    UE_LOG(LogTemp, Log, TEXT("Started blocking with %s"), 
           *UEnum::GetValueAsString(CurrentWeaponType));
    
    return true;
}

bool UCombat_WeaponSystem::StopBlocking()
{
    if (CurrentWeaponState == ECombat_WeaponState::Blocking)
    {
        CurrentWeaponState = ECombat_WeaponState::Idle;
        UE_LOG(LogTemp, Log, TEXT("Stopped blocking"));
        return true;
    }
    
    return false;
}

float UCombat_WeaponSystem::GetCurrentDamage() const
{
    if (!IsWeaponEquipped() || IsWeaponBroken())
    {
        return 5.0f; // Bare hands damage
    }
    
    // Damage scales with durability
    float DurabilityMultiplier = FMath::Clamp(WeaponStats.Durability / WeaponStats.MaxDurability, 0.2f, 1.0f);
    return WeaponStats.BaseDamage * DurabilityMultiplier;
}

bool UCombat_WeaponSystem::CanAttack() const
{
    if (bIsAttacking)
    {
        return false;
    }
    
    if (CurrentWeaponState == ECombat_WeaponState::Blocking)
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void UCombat_WeaponSystem::TakeDurabilityDamage(float Damage)
{
    if (!IsWeaponEquipped())
    {
        return;
    }
    
    WeaponStats.Durability = FMath::Max(0.0f, WeaponStats.Durability - Damage);
    
    if (WeaponStats.Durability <= 0.0f)
    {
        CurrentWeaponState = ECombat_WeaponState::Broken;
        UE_LOG(LogTemp, Warning, TEXT("Weapon %s is now broken!"), 
               *UEnum::GetValueAsString(CurrentWeaponType));
    }
}

bool UCombat_WeaponSystem::RepairWeapon(float RepairAmount)
{
    if (!IsWeaponEquipped())
    {
        return false;
    }
    
    float OldDurability = WeaponStats.Durability;
    WeaponStats.Durability = FMath::Min(WeaponStats.MaxDurability, WeaponStats.Durability + RepairAmount);
    
    if (OldDurability <= 0.0f && WeaponStats.Durability > 0.0f)
    {
        CurrentWeaponState = ECombat_WeaponState::Idle;
        UE_LOG(LogTemp, Log, TEXT("Weapon %s repaired and functional again"), 
               *UEnum::GetValueAsString(CurrentWeaponType));
    }
    
    return true;
}

float UCombat_WeaponSystem::GetDurabilityPercentage() const
{
    if (!IsWeaponEquipped() || WeaponStats.MaxDurability <= 0.0f)
    {
        return 0.0f;
    }
    
    return (WeaponStats.Durability / WeaponStats.MaxDurability) * 100.0f;
}

void UCombat_WeaponSystem::InitializeWeaponStats(ECombat_WeaponType WeaponType)
{
    // Initialize weapon-specific stats
    switch (WeaponType)
    {
        case ECombat_WeaponType::StoneSpear:
            WeaponStats.BaseDamage = 25.0f;
            WeaponStats.AttackRange = 200.0f;
            WeaponStats.AttackSpeed = 0.8f;
            WeaponStats.Durability = 80.0f;
            WeaponStats.MaxDurability = 80.0f;
            WeaponStats.Weight = 3.0f;
            WeaponStats.bCanBlock = true;
            WeaponStats.bIsRanged = false;
            AttackCooldown = 1.2f;
            break;
            
        case ECombat_WeaponType::WoodenClub:
            WeaponStats.BaseDamage = 20.0f;
            WeaponStats.AttackRange = 120.0f;
            WeaponStats.AttackSpeed = 1.2f;
            WeaponStats.Durability = 60.0f;
            WeaponStats.MaxDurability = 60.0f;
            WeaponStats.Weight = 2.5f;
            WeaponStats.bCanBlock = true;
            WeaponStats.bIsRanged = false;
            AttackCooldown = 0.8f;
            break;
            
        case ECombat_WeaponType::StoneAxe:
            WeaponStats.BaseDamage = 30.0f;
            WeaponStats.AttackRange = 130.0f;
            WeaponStats.AttackSpeed = 0.7f;
            WeaponStats.Durability = 100.0f;
            WeaponStats.MaxDurability = 100.0f;
            WeaponStats.Weight = 4.0f;
            WeaponStats.bCanBlock = false;
            WeaponStats.bIsRanged = false;
            AttackCooldown = 1.5f;
            break;
            
        case ECombat_WeaponType::BoneKnife:
            WeaponStats.BaseDamage = 15.0f;
            WeaponStats.AttackRange = 80.0f;
            WeaponStats.AttackSpeed = 1.8f;
            WeaponStats.Durability = 40.0f;
            WeaponStats.MaxDurability = 40.0f;
            WeaponStats.Weight = 1.0f;
            WeaponStats.bCanBlock = false;
            WeaponStats.bIsRanged = false;
            AttackCooldown = 0.5f;
            break;
            
        case ECombat_WeaponType::Sling:
            WeaponStats.BaseDamage = 12.0f;
            WeaponStats.AttackRange = 800.0f;
            WeaponStats.AttackSpeed = 0.9f;
            WeaponStats.Durability = 150.0f;
            WeaponStats.MaxDurability = 150.0f;
            WeaponStats.Weight = 0.5f;
            WeaponStats.bCanBlock = false;
            WeaponStats.bIsRanged = true;
            AttackCooldown = 2.0f;
            break;
            
        case ECombat_WeaponType::Torch:
            WeaponStats.BaseDamage = 10.0f;
            WeaponStats.AttackRange = 100.0f;
            WeaponStats.AttackSpeed = 1.0f;
            WeaponStats.Durability = 30.0f;
            WeaponStats.MaxDurability = 30.0f;
            WeaponStats.Weight = 1.5f;
            WeaponStats.bCanBlock = false;
            WeaponStats.bIsRanged = false;
            AttackCooldown = 1.0f;
            break;
            
        default:
            WeaponStats = FCombat_WeaponStats();
            AttackCooldown = 1.0f;
            break;
    }
}

void UCombat_WeaponSystem::UpdateWeaponMesh()
{
    if (!WeaponMesh)
    {
        return;
    }
    
    if (CurrentWeaponType == ECombat_WeaponType::None)
    {
        WeaponMesh->SetVisibility(false);
        return;
    }
    
    // Try to find mesh for this weapon type
    if (UStaticMesh** FoundMesh = WeaponMeshes.Find(CurrentWeaponType))
    {
        if (*FoundMesh)
        {
            WeaponMesh->SetStaticMesh(*FoundMesh);
            WeaponMesh->SetVisibility(true);
        }
    }
    else
    {
        // Use default cube mesh as placeholder
        WeaponMesh->SetVisibility(true);
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for weapon type: %s"), 
               *UEnum::GetValueAsString(CurrentWeaponType));
    }
}

void UCombat_WeaponSystem::PlayAttackSound()
{
    if (USoundCue** FoundSound = AttackSounds.Find(CurrentWeaponType))
    {
        if (*FoundSound && GetOwner())
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), *FoundSound, GetOwner()->GetActorLocation());
        }
    }
}

void UCombat_WeaponSystem::SpawnHitEffect(FVector HitLocation)
{
    if (UParticleSystem** FoundEffect = HitEffects.Find(CurrentWeaponType))
    {
        if (*FoundEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), *FoundEffect, HitLocation);
        }
    }
}