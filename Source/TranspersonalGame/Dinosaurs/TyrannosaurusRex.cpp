// TyrannosaurusRex.cpp
// Apex predator implementation — inherits from ADinosaurBase
// Stats: 500hp, 80 damage, 1800 detection, 350/650 move/sprint speed

#include "Dinosaurs/TyrannosaurusRex.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    // ---- Identity ----
    Species = EEng_DinoSpecies::TyrannosaurusRex;
    SpeciesDisplayName = TEXT("Tyrannosaurus Rex");
    bIsCarnivore = true;
    bIsPackHunter = false;

    // ---- Stats override ----
    DinoStats.MaxHealth       = 500.0f;
    DinoStats.CurrentHealth   = 500.0f;
    DinoStats.MaxHunger       = 100.0f;
    DinoStats.CurrentHunger   = 75.0f;
    DinoStats.AttackDamage    = 80.0f;
    DinoStats.AttackRange     = 350.0f;
    DinoStats.DetectionRadius = 1800.0f;
    DinoStats.MoveSpeed       = 350.0f;
    DinoStats.SprintSpeed     = 650.0f;
    DinoStats.Mass            = 8000.0f;

    // ---- Capsule sizing for large biped ----
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 180.0f);

    // ---- Movement ----
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.MoveSpeed;
    GetCharacterMovement()->JumpZVelocity = 0.0f;   // TRex cannot jump
    GetCharacterMovement()->bCanWalkOffLedges = true;
    GetCharacterMovement()->Mass = DinoStats.Mass;

    // ---- Hunger decay — slower for large predator ----
    HungerDecayRate = 1.2f;

    // ---- AI update interval — slightly slower for performance ----
    AIUpdateInterval = 0.6f;
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[TRex] %s spawned — Health: %.0f, Detection: %.0f cm"),
        *GetName(), DinoStats.CurrentHealth, DinoStats.DetectionRadius);
}

void ATyrannosaurusRex::TriggerRoar()
{
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastRoarTime < RoarCooldown)
    {
        return; // Cooldown not elapsed
    }
    LastRoarTime = Now;

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s ROARS — applying fear in %.0f cm radius"), *GetName(), DinoStats.DetectionRadius);

    // Apply fear to player if in range
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        DinoStats.DetectionRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>{ this },
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->ActorHasTag(FName("Player")))
        {
            UE_LOG(LogTemp, Log, TEXT("[TRex] Roar reached player: %s"), *Actor->GetName());
            // Fear mechanic hook — TranspersonalCharacter SurvivalComponent handles fear stat
            // Actor->TakeDamage(0, FDamageEvent(), nullptr, this);  // placeholder for fear event
        }
    }
}

void ATyrannosaurusRex::PerformStomp()
{
    UE_LOG(LogTemp, Log, TEXT("[TRex] %s STOMPS — radius: %.0f cm, damage: %.0f"),
        *GetName(), StompRadius, StompDamage);

    TArray<AActor*> HitActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        StompRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>{ this },
        HitActors
    );

    for (AActor* Actor : HitActors)
    {
        if (Actor && Actor != this)
        {
            UGameplayStatics::ApplyDamage(Actor, StompDamage, GetController(), this, nullptr);
            UE_LOG(LogTemp, Log, TEXT("[TRex] Stomp hit: %s for %.0f damage"), *Actor->GetName(), StompDamage);
        }
    }

#if WITH_EDITOR
    // Debug sphere in editor
    DrawDebugSphere(GetWorld(), GetActorLocation(), StompRadius, 12, FColor::Orange, false, 2.0f);
#endif
}

void ATyrannosaurusRex::OnDinoDeath_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("[TRex] %s has died — collapsing"), *GetName());

    // Disable collision and movement (base class handles this too, but explicit for TRex)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll mesh
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Set dead state
    BehaviorState = EEng_DinoBehaviorState::Dead;
}

void ATyrannosaurusRex::OnDinoSpotTarget_Implementation(AActor* SpottedTarget)
{
    if (!SpottedTarget) return;

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s spotted target: %s"), *GetName(), *SpottedTarget->GetName());

    // Trigger roar when first spotting prey
    TriggerRoar();

    // Transition to hunting
    SetBehaviorState(EEng_DinoBehaviorState::Hunting);
    CurrentTarget = SpottedTarget;
}
