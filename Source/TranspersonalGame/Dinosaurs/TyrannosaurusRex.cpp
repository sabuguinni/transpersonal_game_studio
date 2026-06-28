// TyrannosaurusRex.cpp — Implementation of ATyrannosaurusRex
// Agent #3 — Core Systems Programmer

#include "Dinosaurs/TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Species identity ---
    Species = EEng_DinosaurSpecies::TRex;
    Diet = EEng_DinosaurDiet::Carnivore;
    bIsAggressive = true;
    bIsPackAnimal = false;

    // --- Stats: apex predator tier ---
    DinoStats.MaxHealth = 2500.0f;
    DinoStats.CurrentHealth = 2500.0f;
    DinoStats.AttackDamage = 180.0f;
    DinoStats.AttackRange = 250.0f;
    DinoStats.MovementSpeed = 550.0f;
    DinoStats.SprintSpeed = 900.0f;
    DinoStats.BodyMass = 8000.0f;
    DinoStats.MaxHunger = 200.0f;
    DinoStats.CurrentHunger = 100.0f;

    // --- Sensory: excellent sight, decent hearing, poor smell ---
    SensoryData.SightRange = 4000.0f;
    SensoryData.SightAngle = 120.0f;
    SensoryData.HearingRange = 2500.0f;
    SensoryData.SmellRange = 800.0f;
    SensoryData.bIsNocturnal = false;

    // --- Capsule: large body ---
    GetCapsuleComponent()->SetCapsuleRadius(120.0f);
    GetCapsuleComponent()->SetCapsuleHalfHeight(220.0f);

    // --- Movement ---
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.MovementSpeed;
    GetCharacterMovement()->JumpZVelocity = 0.0f;    // TRex cannot jump
    GetCharacterMovement()->bCanWalkOffLedges = true;
    GetCharacterMovement()->GravityScale = 1.2f;     // Heavy — falls faster

    // --- Mesh offset for large body ---
    if (GetMesh())
    {
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -220.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    // --- Roar / Stomp config ---
    RoarCooldown = 45.0f;
    TimeSinceLastRoar = RoarCooldown; // Ready to roar immediately
    StompRadius = 400.0f;
    StompDamage = 80.0f;
    bIsFeeding = false;
    FeedingDuration = 12.0f;
    FeedingTimer = 0.0f;
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();

    // TRex starts in patrolling state
    SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
}

void ATyrannosaurusRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Roar cooldown tracking
    TimeSinceLastRoar += DeltaTime;

    // Feeding timer
    if (bIsFeeding)
    {
        FeedingTimer += DeltaTime;
        if (FeedingTimer >= FeedingDuration)
        {
            bIsFeeding = false;
            FeedingTimer = 0.0f;
            SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        }
    }
}

void ATyrannosaurusRex::PerformRoar()
{
    if (TimeSinceLastRoar < RoarCooldown)
    {
        return; // Still on cooldown
    }

    TimeSinceLastRoar = 0.0f;

    // Apply fear to all nearby actors (player-facing: reduce stamina, increase fear stat)
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    const float RoarFearRadius = 2000.0f;
    const FVector MyLocation = GetActorLocation();

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= RoarFearRadius)
        {
            // Broadcast roar event — Blueprint/BehaviorTree can respond
            // Player character will receive fear debuff via their SurvivalComponent
            Actor->Tags.AddUnique(FName("TRexRoarAffected"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex [%s] ROAR — affecting radius %.0f cm"), *GetName(), RoarFearRadius);
}

void ATyrannosaurusRex::PerformStompAttack()
{
    const FVector StompOrigin = GetActorLocation();

    // Sphere overlap for stomp damage
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        StompOrigin,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(StompRadius),
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor != this)
            {
                UGameplayStatics::ApplyDamage(
                    HitActor,
                    StompDamage,
                    GetController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
    }

    // Debug visualization in editor
#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), StompOrigin, StompRadius, 16, FColor::Orange, false, 2.0f);
#endif

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex [%s] STOMP — radius %.0f, damage %.0f"), *GetName(), StompRadius, StompDamage);
}

void ATyrannosaurusRex::OnTargetDetected_Implementation(AActor* Target)
{
    // Call parent implementation
    Super::OnTargetDetected_Implementation(Target);

    // Roar on detection if cooldown expired
    if (TimeSinceLastRoar >= RoarCooldown)
    {
        PerformRoar();
    }

    // Immediately switch to hunting
    SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex [%s] detected target [%s] — HUNTING"), *GetName(), *Target->GetName());
}

void ATyrannosaurusRex::OnDeath_Implementation()
{
    // TRex death — enter feeding state briefly before ragdoll
    bIsFeeding = false; // Already dead, no feeding

    // Broadcast death to nearby creatures (they may flee)
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBase::StaticClass(), NearbyActors);

    const FVector MyLocation = GetActorLocation();
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= 3000.0f)
        {
            // Tag nearby dinos — BehaviorTree can pick this up and trigger flee
            Actor->Tags.AddUnique(FName("TRexDeathNearby"));
        }
    }

    // Call parent (handles ragdoll, mesh collision, etc.)
    Super::OnDeath_Implementation();

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex [%s] DIED — nearby creatures alerted"), *GetName());
}
