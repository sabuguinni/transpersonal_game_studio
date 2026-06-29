#include "TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule sizing for a T-Rex: ~6m tall, ~2m radius
    GetCapsuleComponent()->InitCapsuleSize(120.0f, 300.0f);

    // Movement defaults — T-Rex is powerful but not fast
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = 600.0f;         // ~6 m/s base walk
        MoveComp->MaxAcceleration = 800.0f;
        MoveComp->BrakingDecelerationWalking = 400.0f;
        MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale = 1.2f;           // Heavy animal
    }
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();
    // Species defaults are initialized in InitializeSpeciesDefaults via Super::BeginPlay
}

void ATyrannosaurusRex::InitializeSpeciesDefaults()
{
    // T-Rex species stats — apex predator profile
    DinoStats.MaxHealth         = 2500.0f;
    DinoStats.CurrentHealth     = 2500.0f;
    DinoStats.AttackDamage      = 150.0f;
    DinoStats.AttackRange       = 250.0f;       // Bite reach in cm
    DinoStats.DetectionRange    = 3000.0f;      // Large sensory range
    DinoStats.MoveSpeed         = 600.0f;
    DinoStats.SprintSpeed       = 1100.0f;      // Short burst sprint
    DinoStats.TerritoryRadius   = 8000.0f;      // Very large territory
    DinoStats.MaxHunger         = 100.0f;
    DinoStats.CurrentHunger     = 80.0f;
    DinoStats.HungerDecayRate   = 0.5f;         // Slow decay — large metabolism

    SpeciesData.Species         = EEng_DinoSpecies::TyrannosaurusRex;
    SpeciesData.Diet            = EEng_DinoDiet::Carnivore;
    SpeciesData.bIsPack         = false;         // Solitary hunter
    SpeciesData.PackSize        = 1;
    SpeciesData.bIsNocturnal    = false;
    SpeciesData.bIsTerritorial  = true;
    SpeciesData.ThreatLevel     = 10;            // Maximum threat

    // Sync movement speed
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex: InitializeSpeciesDefaults complete. HP=%.0f, DMG=%.0f, Territory=%.0fm"),
        DinoStats.MaxHealth, DinoStats.AttackDamage, DinoStats.TerritoryRadius / 100.0f);
}

void ATyrannosaurusRex::PerformRoar()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoarTime < RoarCooldown)
    {
        UE_LOG(LogTemp, Verbose, TEXT("TRex Roar on cooldown. %.1fs remaining."),
            RoarCooldown - (CurrentTime - LastRoarTime));
        return;
    }

    LastRoarTime = CurrentTime;
    UE_LOG(LogTemp, Log, TEXT("TRex %s ROARS! Stun radius=%.0fcm, duration=%.1fs"),
        *GetName(), RoarStunRadius, RoarStunDuration);

    ApplyRoarStun();

#if WITH_EDITOR
    // Debug visualization in editor
    DrawDebugSphere(GetWorld(), GetActorLocation(), RoarStunRadius, 16, FColor::Orange, false, 2.0f);
#endif
}

void ATyrannosaurusRex::PerformStomp()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStompTime < StompCooldown)
    {
        UE_LOG(LogTemp, Verbose, TEXT("TRex Stomp on cooldown. %.1fs remaining."),
            StompCooldown - (CurrentTime - LastStompTime));
        return;
    }

    LastStompTime = CurrentTime;
    UE_LOG(LogTemp, Log, TEXT("TRex %s STOMPS! Radius=%.0fcm, damage=%.0f"),
        *GetName(), StompRadius, StompDamage);

    ApplyStompDamage();

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation(), StompRadius, 12, FColor::Red, false, 1.5f);
#endif
}

void ATyrannosaurusRex::ApplyRoarStun()
{
    if (!GetWorld()) return;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        RoarStunRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != this)
        {
            // Apply stun via gameplay tag or movement disable
            // For now, log the affected actor — full stun requires GameplayAbilities
            UE_LOG(LogTemp, Log, TEXT("TRex Roar: Stunned %s for %.1fs"), *Actor->GetName(), RoarStunDuration);
        }
    }
}

void ATyrannosaurusRex::ApplyStompDamage()
{
    if (!GetWorld()) return;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        StompRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != this)
        {
            // Apply damage using UE5 damage system
            UGameplayStatics::ApplyDamage(
                Actor,
                StompDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            UE_LOG(LogTemp, Log, TEXT("TRex Stomp: Applied %.0f damage to %s"), StompDamage, *Actor->GetName());
        }
    }
}
