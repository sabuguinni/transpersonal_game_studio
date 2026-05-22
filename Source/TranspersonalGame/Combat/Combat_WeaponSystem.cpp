#include "Combat_WeaponSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UCombat_WeaponSystem::UCombat_WeaponSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize weapon stats
    WeaponStats.BaseDamage = 15.0f;
    WeaponStats.Range = 250.0f;
    WeaponStats.AttackSpeed = 1.2f;
    WeaponStats.Durability = 100.0f;
    WeaponStats.WeaponType = EWeaponType::Spear;
    
    CurrentDurability = WeaponStats.Durability;
    bIsEquipped = false;
}

void UCombat_WeaponSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to find weapon mesh component on owner
    if (AActor* Owner = GetOwner())
    {
        WeaponMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (WeaponMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("Combat_WeaponSystem: Found weapon mesh on %s"), *Owner->GetName());
        }
    }
    
    CurrentDurability = WeaponStats.Durability;
}

void UCombat_WeaponSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Weapon degradation over time when equipped
    if (bIsEquipped && CurrentDurability > 0.0f)
    {
        float DegradationRate = 0.1f; // Durability loss per second
        CurrentDurability = FMath::Max(0.0f, CurrentDurability - (DegradationRate * DeltaTime));
        
        if (IsWeaponBroken())
        {
            UnequipWeapon();
            UE_LOG(LogTemp, Warning, TEXT("Weapon broken due to wear!"));
        }
    }
}

void UCombat_WeaponSystem::EquipWeapon()
{
    if (IsWeaponBroken())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot equip broken weapon"));
        return;
    }
    
    bIsEquipped = true;
    
    if (WeaponMesh)
    {
        WeaponMesh->SetVisibility(true);
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weapon equipped: %s"), 
           *UEnum::GetValueAsString(WeaponStats.WeaponType));
}

void UCombat_WeaponSystem::UnequipWeapon()
{
    bIsEquipped = false;
    
    if (WeaponMesh)
    {
        WeaponMesh->SetVisibility(false);
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weapon unequipped"));
}

float UCombat_WeaponSystem::PerformAttack(AActor* Target)
{
    if (!bIsEquipped || IsWeaponBroken() || !Target)
    {
        return 0.0f;
    }
    
    // Calculate distance to target
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    if (Distance > WeaponStats.Range)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target out of weapon range: %f > %f"), Distance, WeaponStats.Range);
        return 0.0f;
    }
    
    // Calculate damage with some randomization
    float DamageMultiplier = FMath::RandRange(0.8f, 1.2f);
    float FinalDamage = WeaponStats.BaseDamage * DamageMultiplier;
    
    // Apply durability damage from use
    TakeDurabilityDamage(2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Attack performed: %f damage to %s"), FinalDamage, *Target->GetName());
    
    return FinalDamage;
}

void UCombat_WeaponSystem::TakeDurabilityDamage(float Damage)
{
    CurrentDurability = FMath::Max(0.0f, CurrentDurability - Damage);
    
    if (IsWeaponBroken() && bIsEquipped)
    {
        UnequipWeapon();
        UE_LOG(LogTemp, Warning, TEXT("Weapon broke from damage!"));
    }
}

bool UCombat_WeaponSystem::IsWeaponBroken() const
{
    return CurrentDurability <= 0.0f;
}

void UCombat_WeaponSystem::RepairWeapon(float RepairAmount)
{
    CurrentDurability = FMath::Min(WeaponStats.Durability, CurrentDurability + RepairAmount);
    UE_LOG(LogTemp, Log, TEXT("Weapon repaired: %f durability"), CurrentDurability);
}

float UCombat_WeaponSystem::GetDurabilityPercentage() const
{
    if (WeaponStats.Durability <= 0.0f)
    {
        return 0.0f;
    }
    
    return (CurrentDurability / WeaponStats.Durability) * 100.0f;
}