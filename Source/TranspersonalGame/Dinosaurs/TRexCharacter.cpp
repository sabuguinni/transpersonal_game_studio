// TRexCharacter.cpp
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260624_001
// Tyrannosaurus Rex — full implementation.

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ATRexCharacter::ATRexCharacter()
{
    // Lock species — ApplySpeciesStats() in BeginPlay will read this
    DinosaurSpecies = EDinosaurSpecies::TyrannosaurusRex;

    // T-Rex capsule: large predator
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // Movement — heavy but fast in bursts
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = 600.0f;       // ~21 km/h — realistic T-Rex estimate
        MoveComp->MaxAcceleration = 800.0f;
        MoveComp->BrakingDecelerationWalking = 600.0f;
        MoveComp->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->Mass = 8000.0f;              // ~8 tonnes
        MoveComp->GravityScale = 1.2f;         // heavier feel
    }

    // Actor tag for AI queries
    Tags.Add(FName("Dinosaur"));
    Tags.Add(FName("Predator"));
    Tags.Add(FName("TRex"));
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void ATRexCharacter::BeginPlay()
{
    // Species must be set BEFORE Super::BeginPlay so ApplySpeciesStats() reads it
    DinosaurSpecies = EDinosaurSpecies::TyrannosaurusRex;

    Super::BeginPlay();

    // Record territory center at spawn
    TerritoryCenter = GetActorLocation();
    bIsDefendingTerritory = true;

    UE_LOG(LogTemp, Log,
           TEXT("[TRex] %s spawned at (%.0f, %.0f, %.0f) | HP=%.0f SPD=%.0f AGG=%.2f"),
           *GetActorLabel(),
           TerritoryCenter.X, TerritoryCenter.Y, TerritoryCenter.Z,
           CurrentHealth, MovementSpeed, AggressionLevel);
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformRoar
// ─────────────────────────────────────────────────────────────────────────────
void ATRexCharacter::PerformRoar_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s ROARS — fear radius=%.0f intensity=%.2f"),
           *GetActorLabel(), RoarFearRadius, RoarFearIntensity);

    // Debug sphere in editor
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), RoarFearRadius, 16,
                    FColor::Orange, false, 2.0f);
#endif

    // Apply fear to all pawns in radius
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Player"), OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarFearRadius)
        {
            // Notify the actor — Blueprint/C++ can listen for this event
            Actor->Tags.AddUnique(FName("Frightened"));
            UE_LOG(LogTemp, Log, TEXT("[TRex] %s frightened by roar (dist=%.0f)"),
                   *Actor->GetActorLabel(), Dist);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformStomp
// ─────────────────────────────────────────────────────────────────────────────
void ATRexCharacter::PerformStomp_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    float StompDamage = AttackDamage * StompDamageMultiplier;

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s STOMPS — radius=%.0f dmg=%.0f"),
           *GetActorLabel(), StompRadius, StompDamage);

#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), StompRadius, 12,
                    FColor::Red, false, 1.5f);
#endif

    // Radial damage
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        World,
        StompDamage,
        GetActorLocation(),
        StompRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetInstigatorController(),
        true  // full damage at epicentre
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformBite
// ─────────────────────────────────────────────────────────────────────────────
void ATRexCharacter::PerformBite_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s BITES — range=%.0f dmg=%.0f"),
           *GetActorLabel(), BiteRange, AttackDamage);

    // Line trace from mouth (forward + slight down)
    FVector Start = GetActorLocation() + FVector(0, 0, 150.0f); // approx mouth height
    FVector End   = Start + GetActorForwardVector() * BiteRange;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End,
                                                ECC_Pawn, Params);

#if WITH_EDITOR
    DrawDebugLine(World, Start, End, bHit ? FColor::Red : FColor::Yellow,
                  false, 1.0f, 0, 3.0f);
#endif

    if (bHit && HitResult.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            HitResult.GetActor(),
            AttackDamage,
            GetActorForwardVector(),
            HitResult,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );

        UE_LOG(LogTemp, Log, TEXT("[TRex] Bite HIT %s for %.0f dmg"),
               *HitResult.GetActor()->GetActorLabel(), AttackDamage);
    }
}
