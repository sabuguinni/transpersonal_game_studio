#include "TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex capsule — large predator
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // Override base stats with T-Rex specific values
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 150.0f;
    AttackRange = 250.0f;
    DetectionRadius = 5000.0f;
    TerritoryRadius = 8000.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    SprintSpeed = 800.0f;
    Diet = EEng_DinoDiet::Carnivore;
    Species = EEng_DinoSpecies::TyrannosaurusRex;

    // T-Rex movement — heavy, powerful
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    GetCharacterMovement()->Mass = 8000.0f;
    GetCharacterMovement()->GroundFriction = 4.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 800.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // Scale — T-Rex is massive
    SetActorScale3D(FVector(3.0f, 3.0f, 3.0f));

    // Combat properties
    StompRadius = 400.0f;
    StompDamage = 25.0f;
    ChargeSpeedMultiplier = 1.5f;
    ChargeCooldown = 8.0f;
    bIsCharging = false;

    // Audio
    RoarSound = nullptr;

    // Internal timers
    LastChargeTime = -999.0f;
    LastStompTime = 0.0f;
    StompInterval = 2.0f; // stomp every 2 seconds while walking
}

void ATyrannosaurusRex::BeginPlay()
{
    // Call parent BeginPlay — initialises health, hunger, movement speed
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex spawned: Health=%.0f, Speed=%.0f, DetectionRadius=%.0f"),
        CurrentHealth, RunSpeed, DetectionRadius);
}

void ATyrannosaurusRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply stomp damage periodically while moving
    if (BehaviorState == EEng_DinoBehaviorState::Hunt || BehaviorState == EEng_DinoBehaviorState::Aggressive)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastStompTime >= StompInterval)
        {
            ApplyStompDamage();
            LastStompTime = CurrentTime;
        }
    }

    // Reset charge flag if not moving fast enough
    if (bIsCharging)
    {
        float CurrentSpeed = GetVelocity().Size();
        if (CurrentSpeed < RunSpeed * 0.5f)
        {
            bIsCharging = false;
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        }
    }
}

void ATyrannosaurusRex::OnPlayerDetected(APawn* DetectedPlayer)
{
    // T-Rex always hunts — it's an apex carnivore
    SetBehaviorState(EEng_DinoBehaviorState::Aggressive);

    // Play roar sound at detection
    if (RoarSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), RoarSound, GetActorLocation(), 1.5f);
    }

    UE_LOG(LogTemp, Warning, TEXT("TyrannosaurusRex detected player %s — HUNTING!"),
        *DetectedPlayer->GetName());

    // Attempt charge if cooldown has elapsed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastChargeTime >= ChargeCooldown)
    {
        StartCharge(DetectedPlayer);
    }
}

void ATyrannosaurusRex::StartCharge(AActor* Target)
{
    if (!Target || !GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastChargeTime < ChargeCooldown)
    {
        return; // Still on cooldown
    }

    bIsCharging = true;
    LastChargeTime = CurrentTime;

    // Boost speed during charge
    float ChargeSpeed = RunSpeed * ChargeSpeedMultiplier;
    GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;

    UE_LOG(LogTemp, Warning, TEXT("TyrannosaurusRex CHARGING at %s! Speed=%.0f"),
        *Target->GetName(), ChargeSpeed);

    // Reset charge after 3 seconds
    FTimerHandle ChargeTimer;
    GetWorld()->GetTimerManager().SetTimer(ChargeTimer, [this]()
    {
        bIsCharging = false;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex charge ended"));
    }, 3.0f, false);
}

void ATyrannosaurusRex::ApplyStompDamage()
{
    if (!GetWorld())
    {
        return;
    }

    // Find all pawns within stomp radius
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), OverlappingActors);

    FVector MyLocation = GetActorLocation();

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == this)
        {
            continue;
        }

        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= StompRadius)
        {
            // Apply stomp damage — falls off with distance
            float DamageFalloff = 1.0f - (Distance / StompRadius);
            float ActualDamage = StompDamage * DamageFalloff;

            UGameplayStatics::ApplyDamage(Actor, ActualDamage, GetController(),
                this, UDamageType::StaticClass());

            UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex stomp hit %s for %.1f damage (dist=%.0f)"),
                *Actor->GetName(), ActualDamage, Distance);
        }
    }
}

void ATyrannosaurusRex::Die()
{
    UE_LOG(LogTemp, Warning, TEXT("TyrannosaurusRex DIED — the apex predator has fallen!"));

    // Call parent die — enables ragdoll, disables movement
    Super::Die();

    // T-Rex death causes a ground shake (screen shake would be added via camera manager)
    // Apply final stomp damage as death throes
    ApplyStompDamage();
}
