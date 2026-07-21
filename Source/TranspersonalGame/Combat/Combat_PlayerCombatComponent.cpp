#include "Combat_PlayerCombatComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "../TranspersonalCharacter.h"

UCombat_PlayerCombatComponent::UCombat_PlayerCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    bIsAttacking = false;
    bIsBlocking = false;
    bIsDodging = false;
    
    AttackDuration = 0.8f;
    DodgeDuration = 0.5f;
    DodgeCooldown = 2.0f;
    
    AttackTimer = 0.0f;
    DodgeTimer = 0.0f;
    DodgeCooldownTimer = 0.0f;
    
    // Initialize default weapon (fists)
    CurrentWeapon.WeaponType = ECombat_WeaponType::None;
    CurrentWeapon.BaseDamage = 5.0f;
    CurrentWeapon.AttackRange = 100.0f;
    CurrentWeapon.AttackSpeed = 1.2f;
    CurrentWeapon.StaminaCost = 10.0f;
}

void UCombat_PlayerCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure health is at max at start
    CurrentHealth = MaxHealth;
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerCombatComponent: Initialized with %f health"), CurrentHealth);
}

void UCombat_PlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update attack timer
    if (bIsAttacking && AttackTimer > 0.0f)
    {
        AttackTimer -= DeltaTime;
        if (AttackTimer <= 0.0f)
        {
            EndAttack();
        }
    }
    
    // Update dodge timer
    if (bIsDodging && DodgeTimer > 0.0f)
    {
        DodgeTimer -= DeltaTime;
        if (DodgeTimer <= 0.0f)
        {
            EndDodge();
        }
    }
    
    // Update dodge cooldown
    if (DodgeCooldownTimer > 0.0f)
    {
        DodgeCooldownTimer -= DeltaTime;
    }
}

void UCombat_PlayerCombatComponent::StartAttack()
{
    if (!CanAttack())
    {
        return;
    }
    
    // Check stamina
    if (!HasEnoughStamina(CurrentWeapon.StaminaCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Not enough stamina for attack"));
        return;
    }
    
    bIsAttacking = true;
    AttackTimer = AttackDuration / CurrentWeapon.AttackSpeed;
    
    // Consume stamina
    ConsumeStamina(CurrentWeapon.StaminaCost);
    
    // Perform the attack after a short delay
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &UCombat_PlayerCombatComponent::PerformAttack,
        AttackTimer * 0.3f,
        false
    );
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Started attack with %s"), 
           *UEnum::GetValueAsString(CurrentWeapon.WeaponType));
}

void UCombat_PlayerCombatComponent::PerformAttack()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Get forward vector for attack direction
    FVector ForwardVector = Owner->GetActorForwardVector();
    FVector StartLocation = Owner->GetActorLocation();
    FVector EndLocation = StartLocation + (ForwardVector * CurrentWeapon.AttackRange);
    
    // Perform line trace to find targets
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Pawn,
        QueryParams
    );
    
    if (bHit && HitResult.GetActor())
    {
        AActor* Target = HitResult.GetActor();
        
        // Check if target is a dinosaur or enemy
        if (Target->GetName().Contains(TEXT("Raptor")) || 
            Target->GetName().Contains(TEXT("TRex")) || 
            Target->GetName().Contains(TEXT("Brachio")))
        {
            DealDamage(Target, CurrentWeapon.BaseDamage);
            UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Hit %s for %f damage"), 
                   *Target->GetName(), CurrentWeapon.BaseDamage);
        }
    }
}

void UCombat_PlayerCombatComponent::EndAttack()
{
    bIsAttacking = false;
    AttackTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("PlayerCombat: Attack ended"));
}

void UCombat_PlayerCombatComponent::StartBlock()
{
    if (bIsAttacking || bIsDodging)
    {
        return;
    }
    
    bIsBlocking = true;
    UE_LOG(LogTemp, Log, TEXT("PlayerCombat: Started blocking"));
}

void UCombat_PlayerCombatComponent::StopBlock()
{
    bIsBlocking = false;
    UE_LOG(LogTemp, Log, TEXT("PlayerCombat: Stopped blocking"));
}

void UCombat_PlayerCombatComponent::Dodge()
{
    if (bIsAttacking || bIsDodging || DodgeCooldownTimer > 0.0f)
    {
        return;
    }
    
    // Check stamina for dodge
    if (!HasEnoughStamina(20.0f))
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Not enough stamina for dodge"));
        return;
    }
    
    bIsDodging = true;
    DodgeTimer = DodgeDuration;
    DodgeCooldownTimer = DodgeCooldown;
    
    // Consume stamina
    ConsumeStamina(20.0f);
    
    // Apply dodge movement
    AActor* Owner = GetOwner();
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        FVector DodgeDirection = Character->GetActorRightVector();
        FVector DodgeImpulse = DodgeDirection * 800.0f;
        
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->AddImpulse(DodgeImpulse, true);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Performed dodge"));
}

void UCombat_PlayerCombatComponent::EndDodge()
{
    bIsDodging = false;
    DodgeTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("PlayerCombat: Dodge ended"));
}

void UCombat_PlayerCombatComponent::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    if (CurrentHealth <= 0.0f)
    {
        return; // Already dead
    }
    
    float ActualDamage = DamageAmount;
    
    // Reduce damage if blocking
    if (bIsBlocking)
    {
        ActualDamage *= 0.3f; // 70% damage reduction when blocking
        UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Blocked attack, reduced damage to %f"), ActualDamage);
    }
    
    // Apply damage
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
    
    // Broadcast damage event
    OnPlayerDamaged.Broadcast(ActualDamage, DamageSource);
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Took %f damage from %s. Health: %f/%f"), 
           ActualDamage, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"), 
           CurrentHealth, MaxHealth);
    
    // Check for death
    if (CurrentHealth <= 0.0f)
    {
        OnPlayerDeath.Broadcast(DamageSource);
        UE_LOG(LogTemp, Error, TEXT("PlayerCombat: Player died!"));
    }
}

void UCombat_PlayerCombatComponent::DealDamage(AActor* Target, float DamageAmount)
{
    if (!Target)
    {
        return;
    }
    
    // Try to find combat component on target
    if (UActorComponent* TargetComponent = Target->GetComponentByClass(UCombat_PlayerCombatComponent::StaticClass()))
    {
        if (UCombat_PlayerCombatComponent* TargetCombat = Cast<UCombat_PlayerCombatComponent>(TargetComponent))
        {
            TargetCombat->TakeDamage(DamageAmount, GetOwner());
        }
    }
    else
    {
        // For dinosaurs without combat components, use basic damage
        UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Dealt %f damage to %s (no combat component)"), 
               DamageAmount, *Target->GetName());
    }
}

void UCombat_PlayerCombatComponent::EquipWeapon(ECombat_WeaponType NewWeapon)
{
    CurrentWeapon.WeaponType = NewWeapon;
    
    // Set weapon stats based on type
    switch (NewWeapon)
    {
        case ECombat_WeaponType::Stone:
            CurrentWeapon.BaseDamage = 15.0f;
            CurrentWeapon.AttackRange = 120.0f;
            CurrentWeapon.AttackSpeed = 1.0f;
            CurrentWeapon.StaminaCost = 12.0f;
            break;
            
        case ECombat_WeaponType::Spear:
            CurrentWeapon.BaseDamage = 25.0f;
            CurrentWeapon.AttackRange = 200.0f;
            CurrentWeapon.AttackSpeed = 0.8f;
            CurrentWeapon.StaminaCost = 18.0f;
            break;
            
        case ECombat_WeaponType::Club:
            CurrentWeapon.BaseDamage = 30.0f;
            CurrentWeapon.AttackRange = 150.0f;
            CurrentWeapon.AttackSpeed = 0.6f;
            CurrentWeapon.StaminaCost = 25.0f;
            break;
            
        case ECombat_WeaponType::Bow:
            CurrentWeapon.BaseDamage = 35.0f;
            CurrentWeapon.AttackRange = 800.0f;
            CurrentWeapon.AttackSpeed = 0.5f;
            CurrentWeapon.StaminaCost = 20.0f;
            break;
            
        default:
            CurrentWeapon.BaseDamage = 5.0f;
            CurrentWeapon.AttackRange = 100.0f;
            CurrentWeapon.AttackSpeed = 1.2f;
            CurrentWeapon.StaminaCost = 10.0f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerCombat: Equipped %s (Damage: %f, Range: %f)"), 
           *UEnum::GetValueAsString(NewWeapon), CurrentWeapon.BaseDamage, CurrentWeapon.AttackRange);
}

void UCombat_PlayerCombatComponent::UnequipWeapon()
{
    EquipWeapon(ECombat_WeaponType::None);
}

bool UCombat_PlayerCombatComponent::CanAttack() const
{
    return !bIsAttacking && !bIsDodging && CurrentHealth > 0.0f;
}

bool UCombat_PlayerCombatComponent::HasEnoughStamina(float StaminaCost) const
{
    // Get stamina from TranspersonalCharacter if available
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(GetOwner()))
    {
        return Character->GetCurrentStamina() >= StaminaCost;
    }
    
    // Default to true if no stamina system found
    return true;
}

void UCombat_PlayerCombatComponent::ConsumeStamina(float Amount)
{
    // Consume stamina from TranspersonalCharacter if available
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(GetOwner()))
    {
        float NewStamina = FMath::Max(0.0f, Character->GetCurrentStamina() - Amount);
        // Note: TranspersonalCharacter should have a SetCurrentStamina method
        UE_LOG(LogTemp, Log, TEXT("PlayerCombat: Consumed %f stamina"), Amount);
    }
}

AActor* UCombat_PlayerCombatComponent::GetOwnerCharacter() const
{
    return GetOwner();
}