#include "TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Species stats (override DinosaurBase defaults) ---
    DinoStats.MaxHealth        = 2500.0f;
    DinoStats.CurrentHealth    = 2500.0f;
    DinoStats.AttackDamage     = 120.0f;
    DinoStats.AttackRange      = 350.0f;
    DinoStats.DetectionRadius  = 3000.0f;
    DinoStats.MoveSpeed        = 550.0f;   // ~20 km/h — realistic TRex sprint
    DinoStats.TurnRate         = 60.0f;    // Slow turning — large animal
    DinoStats.Mass             = 8000.0f;  // kg
    DinoStats.HungerDecayRate  = 0.8f;     // Large appetite

    // Movement configuration
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed        = DinoStats.MoveSpeed;
        GetCharacterMovement()->RotationRate        = FRotator(0.0f, DinoStats.TurnRate, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale        = 1.0f;
        GetCharacterMovement()->MaxStepHeight       = 80.0f;  // Can step over large obstacles
        GetCharacterMovement()->SetWalkableFloorAngle(40.0f);
    }

    // Capsule size for TRex scale
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // TRex is an apex predator — no natural predators
    bIsApexPredator = true;
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();

    // TRex starts patrolling its territory immediately
    CurrentBehaviorState = EEng_DinosaurBehaviorState::Patrolling;

    // Scale actor to TRex size (placeholder until proper mesh)
    SetActorScale3D(FVector(3.5f, 3.5f, 3.5f));
}

void ATyrannosaurusRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track roar cooldown
    TimeSinceLastRoar += DeltaTime;

    // Auto-roar when hunting and cooldown expired
    if (CurrentBehaviorState == EEng_DinosaurBehaviorState::Hunting &&
        TimeSinceLastRoar >= RoarCooldown)
    {
        PerformRoar();
    }
}

void ATyrannosaurusRex::PerformRoar()
{
    if (TimeSinceLastRoar < RoarCooldown)
    {
        return;
    }

    TimeSinceLastRoar = 0.0f;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find all actors in roar radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(RoarFearRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );

    int32 FrightenedCount = 0;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        ADinosaurBase* NearbyDino = Cast<ADinosaurBase>(Overlap.GetActor());
        if (NearbyDino && NearbyDino != this && !NearbyDino->bIsApexPredator)
        {
            // Force flee state on smaller dinos
            NearbyDino->CurrentBehaviorState = EEng_DinosaurBehaviorState::Fleeing;
            ++FrightenedCount;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROAR — frightened %d nearby dinosaurs"), *GetName(), FrightenedCount);

#if WITH_EDITOR
    // Debug sphere in editor
    DrawDebugSphere(World, GetActorLocation(), RoarFearRadius, 16, FColor::Orange, false, 2.0f);
#endif
}

void ATyrannosaurusRex::StompAttack()
{
    if (bStompOnCooldown)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    bStompOnCooldown = true;

    // Area damage in front of TRex
    FVector StompCenter = GetActorLocation() + GetActorForwardVector() * (StompRadius * 0.5f);

    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(StompRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        StompCenter,
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );

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

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] STOMP — hit %d actors"), *GetName(), Overlaps.Num());

#if WITH_EDITOR
    DrawDebugSphere(World, StompCenter, StompRadius, 12, FColor::Red, false, 1.5f);
#endif

    // Reset stomp cooldown after 3 seconds
    World->GetTimerManager().SetTimer(
        StompCooldownTimer,
        this,
        &ATyrannosaurusRex::ResetStompCooldown,
        3.0f,
        false
    );
}

void ATyrannosaurusRex::ResetStompCooldown()
{
    bStompOnCooldown = false;
}
