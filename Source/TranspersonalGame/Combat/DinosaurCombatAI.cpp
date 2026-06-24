#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// UDinosaurCombatAIComponent
// ============================================================

UDinosaurCombatAIComponent::UDinosaurCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz AI tick — performance friendly
}

void UDinosaurCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeFromSpecies();
    SetState(ECombat_DinoState::Idle);
}

void UDinosaurCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        UpdateIdleState(DeltaTime);
        break;
    case ECombat_DinoState::Alerted:
        UpdateAlertedState(DeltaTime);
        break;
    case ECombat_DinoState::Chasing:
        UpdateChasingState(DeltaTime);
        break;
    case ECombat_DinoState::Attacking:
        UpdateAttackingState(DeltaTime);
        break;
    case ECombat_DinoState::Fleeing:
        UpdateFleeingState(DeltaTime);
        break;
    default:
        break;
    }
}

void UDinosaurCombatAIComponent::InitializeFromSpecies()
{
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.MaxHealth = 2000.0f;
        Stats.CurrentHealth = 2000.0f;
        Stats.AttackDamage = 300.0f;
        Stats.AttackRange = 400.0f;
        Stats.DetectionRadius = 3000.0f;
        Stats.ChaseSpeed = 700.0f;
        Stats.PatrolSpeed = 250.0f;
        Stats.AttackCooldown = 3.0f;
        Stats.bIsPredator = true;
        Stats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.MaxHealth = 400.0f;
        Stats.CurrentHealth = 400.0f;
        Stats.AttackDamage = 80.0f;
        Stats.AttackRange = 150.0f;
        Stats.DetectionRadius = 2500.0f;
        Stats.ChaseSpeed = 1200.0f;
        Stats.PatrolSpeed = 500.0f;
        Stats.AttackCooldown = 1.0f;
        Stats.bIsPredator = true;
        Stats.bIsPackHunter = true;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.MaxHealth = 1500.0f;
        Stats.CurrentHealth = 1500.0f;
        Stats.AttackDamage = 200.0f;
        Stats.AttackRange = 350.0f;
        Stats.DetectionRadius = 1500.0f;
        Stats.ChaseSpeed = 600.0f;
        Stats.PatrolSpeed = 200.0f;
        Stats.AttackCooldown = 4.0f;
        Stats.bIsPredator = false;
        Stats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        Stats.MaxHealth = 1800.0f;
        Stats.CurrentHealth = 1800.0f;
        Stats.AttackDamage = 250.0f;
        Stats.AttackRange = 300.0f;
        Stats.DetectionRadius = 1200.0f;
        Stats.ChaseSpeed = 400.0f;
        Stats.PatrolSpeed = 150.0f;
        Stats.AttackCooldown = 5.0f;
        Stats.bIsPredator = false;
        Stats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.MaxHealth = 3000.0f;
        Stats.CurrentHealth = 3000.0f;
        Stats.AttackDamage = 100.0f;
        Stats.AttackRange = 500.0f;
        Stats.DetectionRadius = 800.0f;
        Stats.ChaseSpeed = 350.0f;
        Stats.PatrolSpeed = 150.0f;
        Stats.AttackCooldown = 6.0f;
        Stats.bIsPredator = false;
        Stats.bIsPackHunter = false;
        break;

    default:
        // Default herbivore
        Stats.MaxHealth = 600.0f;
        Stats.CurrentHealth = 600.0f;
        Stats.AttackDamage = 50.0f;
        Stats.AttackRange = 200.0f;
        Stats.DetectionRadius = 1000.0f;
        Stats.ChaseSpeed = 500.0f;
        Stats.PatrolSpeed = 200.0f;
        Stats.AttackCooldown = 3.0f;
        Stats.bIsPredator = false;
        Stats.bIsPackHunter = false;
        break;
    }
}

void UDinosaurCombatAIComponent::OnPlayerDetected(AActor* Player, float Distance)
{
    if (!Player) return;

    CurrentTarget = Player;

    if (!Stats.bIsPredator)
    {
        // Herbivores flee or charge if cornered
        if (Distance < Stats.AttackRange * 1.5f)
        {
            SetState(ECombat_DinoState::Attacking);
        }
        else
        {
            SetState(ECombat_DinoState::Fleeing);
        }
        return;
    }

    // Predators: alert first, then chase
    if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
    {
        SetState(ECombat_DinoState::Alerted);
        AlertTimer = AlertDecayTime;
    }
}

void UDinosaurCombatAIComponent::TakeDamage_Dino(float DamageAmount, AActor* DamageSource)
{
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    if (DamageSource)
    {
        CurrentTarget = DamageSource;
    }

    // Flee if health critically low
    if (GetHealthPercent() <= FleeHealthThreshold)
    {
        SetState(ECombat_DinoState::Fleeing);
        return;
    }

    // Enrage: switch to chasing if hit
    if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
    {
        SetState(ECombat_DinoState::Chasing);
    }
}

void UDinosaurCombatAIComponent::ExecuteAttack(AActor* Target)
{
    if (!Target) return;

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastAttackTime < Stats.AttackCooldown) return;

    LastAttackTime = CurrentTime;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, nullptr, GetOwner(), nullptr);

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s attacked %s for %.0f damage"),
        *GetOwner()->GetActorLabel(), *Target->GetActorLabel(), Stats.AttackDamage);
}

void UDinosaurCombatAIComponent::SetState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s state -> %d"),
        *GetOwner()->GetActorLabel(), (int32)NewState);
}

bool UDinosaurCombatAIComponent::IsHostile() const
{
    return CurrentState == ECombat_DinoState::Chasing ||
           CurrentState == ECombat_DinoState::Attacking ||
           CurrentState == ECombat_DinoState::Alerted;
}

float UDinosaurCombatAIComponent::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void UDinosaurCombatAIComponent::UpdateIdleState(float DeltaTime)
{
    // Idle: do nothing, wait for detection sphere to trigger
}

void UDinosaurCombatAIComponent::UpdateAlertedState(float DeltaTime)
{
    AlertTimer -= DeltaTime;

    if (!CurrentTarget.IsValid())
    {
        SetState(ECombat_DinoState::Idle);
        return;
    }

    // Check distance — if close enough, start chasing
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Dist < Stats.DetectionRadius)
    {
        SetState(ECombat_DinoState::Chasing);
    }
    else if (AlertTimer <= 0.0f)
    {
        SetState(ECombat_DinoState::Idle);
    }
}

void UDinosaurCombatAIComponent::UpdateChasingState(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        SetState(ECombat_DinoState::Idle);
        return;
    }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist <= Stats.AttackRange)
    {
        SetState(ECombat_DinoState::Attacking);
    }
    else if (Dist > Stats.DetectionRadius * 1.5f)
    {
        // Lost target
        CurrentTarget = nullptr;
        SetState(ECombat_DinoState::Idle);
    }
}

void UDinosaurCombatAIComponent::UpdateAttackingState(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        SetState(ECombat_DinoState::Idle);
        return;
    }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist <= Stats.AttackRange)
    {
        ExecuteAttack(CurrentTarget.Get());
    }
    else
    {
        SetState(ECombat_DinoState::Chasing);
    }
}

void UDinosaurCombatAIComponent::UpdateFleeingState(float DeltaTime)
{
    // Flee logic: move away from threat — handled by BT or movement component
    // After sufficient distance, return to idle
    if (!CurrentTarget.IsValid())
    {
        SetState(ECombat_DinoState::Idle);
        return;
    }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Dist > Stats.DetectionRadius * 2.0f)
    {
        CurrentTarget = nullptr;
        SetState(ECombat_DinoState::Idle);
    }
}

// ============================================================
// ADinosaurCombatActor
// ============================================================

ADinosaurCombatActor::ADinosaurCombatActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    CombatAI = CreateDefaultSubobject<UDinosaurCombatAIComponent>(TEXT("CombatAI"));
}

void ADinosaurCombatActor::BeginPlay()
{
    Super::BeginPlay();

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurCombatActor::OnDetectionOverlapBegin);
    DetectionSphere->SetSphereRadius(CombatAI->Stats.DetectionRadius);
}

void ADinosaurCombatActor::OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // Check if it's a player pawn
    if (OtherActor->ActorHasTag(TEXT("Player")))
    {
        float Dist = FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation());
        CombatAI->OnPlayerDetected(OtherActor, Dist);

        // Alert pack members if pack hunter
        if (CombatAI->Stats.bIsPackHunter)
        {
            AlertNearbyPackMembers(3000.0f);
        }
    }
}

void ADinosaurCombatActor::AlertNearbyPackMembers(float AlertRadius)
{
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCombatActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= AlertRadius)
        {
            ADinosaurCombatActor* PackMember = Cast<ADinosaurCombatActor>(Actor);
            if (PackMember && PackMember->CombatAI->Stats.bIsPackHunter)
            {
                if (CombatAI->CurrentTarget.IsValid())
                {
                    PackMember->CombatAI->OnPlayerDetected(CombatAI->CurrentTarget.Get(), Dist);
                }
            }
        }
    }
}
