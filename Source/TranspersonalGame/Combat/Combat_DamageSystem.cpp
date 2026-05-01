#include "Combat_DamageSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerSphere.h"
#include "../Characters/TranspersonalCharacter.h"
#include "Kismet/GameplayStatics.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check damage every 100ms
    
    // Default damage values
    BaseDamagePerSecond = 10.0f;
    DamageRadius = 300.0f;
    bIsActive = true;
    bCanDealDamage = true;
    
    // Damage scaling by dinosaur type
    TRexDamageMultiplier = 3.0f;
    RaptorDamageMultiplier = 1.5f;
    BrachiosaurusDamageMultiplier = 0.5f; // Herbivore, less aggressive
    
    LastDamageTime = 0.0f;
    DamageCooldown = 1.0f; // 1 second between damage applications
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize damage system
    SetupDamageTrigger();
    
    // Determine dinosaur type from owner name
    if (AActor* Owner = GetOwner())
    {
        FString OwnerName = Owner->GetName().ToLower();
        
        if (OwnerName.Contains(TEXT("rex")))
        {
            DinosaurType = ECombat_DinosaurType::TRex;
            DamageRadius = 500.0f;
            BaseDamagePerSecond = BaseDamagePerSecond * TRexDamageMultiplier;
        }
        else if (OwnerName.Contains(TEXT("raptor")))
        {
            DinosaurType = ECombat_DinosaurType::Raptor;
            DamageRadius = 300.0f;
            BaseDamagePerSecond = BaseDamagePerSecond * RaptorDamageMultiplier;
        }
        else if (OwnerName.Contains(TEXT("brachio")))
        {
            DinosaurType = ECombat_DinosaurType::Brachiosaurus;
            DamageRadius = 400.0f;
            BaseDamagePerSecond = BaseDamagePerSecond * BrachiosaurusDamageMultiplier;
        }
        else
        {
            DinosaurType = ECombat_DinosaurType::Generic;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Combat_DamageSystem: Initialized for %s with damage %.1f"), 
               *Owner->GetName(), BaseDamagePerSecond);
    }
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsActive || !bCanDealDamage)
    {
        return;
    }
    
    // Check for nearby players and apply damage
    CheckAndApplyDamage(DeltaTime);
}

void UCombat_DamageSystem::SetupDamageTrigger()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Create a sphere trigger component for damage detection
    DamageTrigger = NewObject<USphereComponent>(GetOwner(), TEXT("DamageTrigger"));
    if (DamageTrigger)
    {
        DamageTrigger->SetSphereRadius(DamageRadius);
        DamageTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        DamageTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
        DamageTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        // Attach to owner's root component
        if (USceneComponent* RootComp = GetOwner()->GetRootComponent())
        {
            DamageTrigger->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);
        }
        
        // Bind overlap events
        DamageTrigger->OnComponentBeginOverlap.AddDynamic(this, &UCombat_DamageSystem::OnDamageTriggerBeginOverlap);
        DamageTrigger->OnComponentEndOverlap.AddDynamic(this, &UCombat_DamageSystem::OnDamageTriggerEndOverlap);
        
        UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem: Damage trigger created with radius %.1f"), DamageRadius);
    }
}

void UCombat_DamageSystem::CheckAndApplyDamage(float DeltaTime)
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check cooldown
    if (CurrentTime - LastDamageTime < DamageCooldown)
    {
        return;
    }
    
    // Find all players within damage radius
    TArray<AActor*> OverlappingActors;
    if (DamageTrigger)
    {
        DamageTrigger->GetOverlappingActors(OverlappingActors, ATranspersonalCharacter::StaticClass());
    }
    
    for (AActor* Actor : OverlappingActors)
    {
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(Actor))
        {
            ApplyDamageToPlayer(Player, DeltaTime);
            LastDamageTime = CurrentTime;
        }
    }
}

void UCombat_DamageSystem::ApplyDamageToPlayer(ATranspersonalCharacter* Player, float DeltaTime)
{
    if (!Player || !GetOwner())
    {
        return;
    }
    
    // Calculate distance-based damage
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    float DamageMultiplier = FMath::Clamp(1.0f - (Distance / DamageRadius), 0.1f, 1.0f);
    
    float FinalDamage = BaseDamagePerSecond * DamageMultiplier * DamageCooldown;
    
    // Apply damage
    Player->TakeDamage(FinalDamage, FDamageEvent(), nullptr, GetOwner());
    
    // Increase fear based on dinosaur type
    float FearIncrease = 0.0f;
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::TRex:
            FearIncrease = 25.0f;
            break;
        case ECombat_DinosaurType::Raptor:
            FearIncrease = 15.0f;
            break;
        case ECombat_DinosaurType::Brachiosaurus:
            FearIncrease = 5.0f;
            break;
        default:
            FearIncrease = 10.0f;
            break;
    }
    
    // Apply fear (assuming TranspersonalCharacter has fear system)
    // Player->ModifyFear(FearIncrease);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_DamageSystem: Applied %.1f damage to %s (distance: %.1f)"), 
           FinalDamage, *Player->GetName(), Distance);
    
    // Broadcast damage event
    OnDamageApplied.Broadcast(Player, FinalDamage, GetOwner());
}

void UCombat_DamageSystem::OnDamageTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_DamageSystem: Player entered danger zone of %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
        
        // Broadcast player entered danger zone
        OnPlayerEnteredDangerZone.Broadcast(Player, GetOwner());
    }
}

void UCombat_DamageSystem::OnDamageTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem: Player left danger zone of %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
        
        // Broadcast player left danger zone
        OnPlayerLeftDangerZone.Broadcast(Player, GetOwner());
    }
}

void UCombat_DamageSystem::SetDamageActive(bool bActive)
{
    bIsActive = bActive;
    bCanDealDamage = bActive;
    
    if (DamageTrigger)
    {
        DamageTrigger->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem: Damage system %s"), 
           bActive ? TEXT("activated") : TEXT("deactivated"));
}

void UCombat_DamageSystem::SetDamageRadius(float NewRadius)
{
    DamageRadius = FMath::Clamp(NewRadius, 50.0f, 1000.0f);
    
    if (DamageTrigger)
    {
        DamageTrigger->SetSphereRadius(DamageRadius);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem: Damage radius set to %.1f"), DamageRadius);
}

void UCombat_DamageSystem::SetBaseDamage(float NewDamage)
{
    BaseDamagePerSecond = FMath::Clamp(NewDamage, 1.0f, 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Combat_DamageSystem: Base damage set to %.1f"), BaseDamagePerSecond);
}

float UCombat_DamageSystem::GetDamageRadius() const
{
    return DamageRadius;
}

float UCombat_DamageSystem::GetBaseDamage() const
{
    return BaseDamagePerSecond;
}

bool UCombat_DamageSystem::IsPlayerInDangerZone(ATranspersonalCharacter* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= DamageRadius;
}