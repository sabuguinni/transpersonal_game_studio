#include "DinosaurTRex.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ADinosaurTRex::ADinosaurTRex()
{
    // Set T-Rex specific stats
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    MovementSpeed = 400.0f;
    AttackDamage = 75.0f;
    DetectionRange = 4000.0f;
    AttackRange = 300.0f;

    // T-Rex specific properties
    RoarRange = 6000.0f;
    RoarCooldown = 15.0f;
    LastRoarTime = 0.0f;
    ChargeSpeed = 800.0f;
    ChargeDistance = 2000.0f;

    // Initialize behavior states
    bIsRoaring = false;
    bIsCharging = false;
    ChargeTarget = FVector::ZeroVector;

    // Set species and behavior type
    Species = EDinosaurSpecies::TyrannosaurusRex;
    BehaviorType = EDinosaurBehavior::Aggressive;

    // Larger territory for T-Rex
    TerritoryRadius = 8000.0f;

    // Scale up the mesh for T-Rex
    if (DinosaurMesh)
    {
        DinosaurMesh->SetWorldScale3D(FVector(4.0f, 3.0f, 3.5f));
    }

    // Larger collision for T-Rex
    if (CollisionSphere)
    {
        CollisionSphere->SetSphereRadius(400.0f);
    }
}

void ADinosaurTRex::BeginPlay()
{
    Super::BeginPlay();

    // T-Rex never joins packs - always solitary
    bIsPackLeader = false;
    PackLeader = nullptr;
}

void ADinosaurTRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update charge movement
    if (bIsCharging)
    {
        UpdateChargeMovement();
    }

    // Handle roar effects
    if (bIsRoaring)
    {
        HandleRoarEffects();
    }

    // Auto-roar when detecting threats
    if (TargetPawn && CanRoar() && !bIsRoaring)
    {
        float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
        if (DistanceToTarget > AttackRange && DistanceToTarget < RoarRange)
        {
            PerformRoar();
        }
    }
}

void ADinosaurTRex::PerformRoar()
{
    if (!CanRoar())
        return;

    bIsRoaring = true;
    LastRoarTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Warning, TEXT("T-Rex %s performs territorial roar!"), *GetName());

    // Scare nearby dinosaurs
    ScareNearbyDinosaurs();

    // Stop roaring after 3 seconds
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { bIsRoaring = false; },
        3.0f,
        false
    );

    // Update blackboard
    if (DinosaurAI && DinosaurAI->GetBlackboardComponent())
    {
        DinosaurAI->GetBlackboardComponent()->SetValueAsBool(TEXT("IsRoaring"), bIsRoaring);
    }
}

void ADinosaurTRex::StartCharge(FVector TargetLocation)
{
    if (bIsCharging)
        return;

    ChargeTarget = TargetLocation;
    bIsCharging = true;

    UE_LOG(LogTemp, Warning, TEXT("T-Rex %s starts charging towards target!"), *GetName());

    // Update blackboard
    if (DinosaurAI && DinosaurAI->GetBlackboardComponent())
    {
        DinosaurAI->GetBlackboardComponent()->SetValueAsBool(TEXT("IsCharging"), bIsCharging);
        DinosaurAI->GetBlackboardComponent()->SetValueAsVector(TEXT("ChargeTarget"), ChargeTarget);
    }
}

void ADinosaurTRex::StopCharge()
{
    bIsCharging = false;
    ChargeTarget = FVector::ZeroVector;

    // Update blackboard
    if (DinosaurAI && DinosaurAI->GetBlackboardComponent())
    {
        DinosaurAI->GetBlackboardComponent()->SetValueAsBool(TEXT("IsCharging"), bIsCharging);
    }
}

bool ADinosaurTRex::CanRoar()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastRoarTime) >= RoarCooldown && !bIsRoaring;
}

void ADinosaurTRex::ScareNearbyDinosaurs()
{
    // Find all dinosaurs within roar range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBase::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ADinosaurBase* OtherDinosaur = Cast<ADinosaurBase>(Actor);
        if (OtherDinosaur && OtherDinosaur != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), OtherDinosaur->GetActorLocation());
            if (Distance <= RoarRange)
            {
                // Smaller dinosaurs flee from T-Rex roar
                if (OtherDinosaur->Species != EDinosaurSpecies::TyrannosaurusRex &&
                    OtherDinosaur->Species != EDinosaurSpecies::Triceratops)
                {
                    OtherDinosaur->ClearTarget();
                    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s scared by T-Rex roar"), *OtherDinosaur->GetName());
                }
            }
        }
    }
}

void ADinosaurTRex::AttackTarget()
{
    if (!TargetPawn || !IsInAttackRange())
        return;

    // T-Rex has powerful bite attack
    Super::AttackTarget();

    UE_LOG(LogTemp, Warning, TEXT("T-Rex %s delivers devastating bite attack!"), *GetName());

    // Chance to start charging if target is at medium range
    if (!bIsCharging && TargetPawn)
    {
        float Distance = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
        if (Distance > AttackRange && Distance < ChargeDistance && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            StartCharge(TargetPawn->GetActorLocation());
        }
    }
}

void ADinosaurTRex::OnPawnSeen(APawn* SeenPawn)
{
    Super::OnPawnSeen(SeenPawn);

    // T-Rex is very aggressive - immediately roar when seeing threats
    if (SeenPawn && IsPlayerPawn(SeenPawn) && CanRoar())
    {
        PerformRoar();
    }
}

void ADinosaurTRex::UpdateChargeMovement()
{
    if (!bIsCharging)
        return;

    FVector CurrentLocation = GetActorLocation();
    FVector DirectionToTarget = (ChargeTarget - CurrentLocation).GetSafeNormal();
    
    // Move towards charge target at high speed
    FVector NewLocation = CurrentLocation + (DirectionToTarget * ChargeSpeed * GetWorld()->GetDeltaSeconds());
    SetActorLocation(NewLocation);

    // Check if we've reached the target or should stop charging
    float DistanceToTarget = FVector::Dist(CurrentLocation, ChargeTarget);
    if (DistanceToTarget < 200.0f)
    {
        StopCharge();
        UE_LOG(LogTemp, Log, TEXT("T-Rex %s completed charge attack"), *GetName());
    }

    // Stop charging after maximum distance
    float DistanceFromStart = FVector::Dist(TerritoryCenter, CurrentLocation);
    if (DistanceFromStart > TerritoryRadius)
    {
        StopCharge();
    }
}

void ADinosaurTRex::HandleRoarEffects()
{
    if (!bIsRoaring)
        return;

    // Visual/audio effects would go here
    // For now, just log the roaring state
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastLogTime > 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("T-Rex %s is roaring intimidatingly"), *GetName());
        LastLogTime = CurrentTime;
    }
}