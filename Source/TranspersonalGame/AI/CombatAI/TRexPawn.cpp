#include "TRexPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ATRexPawn::ATRexPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root body mesh — large capsule-like box for T-Rex torso
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMesh;

	// Head mesh offset forward and up
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BodyMesh);
	HeadMesh->SetRelativeLocation(FVector(180.f, 0.f, 80.f));

	// Attach behavior component from Agent #11
	BehaviorComponent = CreateDefaultSubobject<UTRexBehaviorComponent>(TEXT("BehaviorComponent"));

	// Default combat stats
	MaxHealth = 500.f;
	CurrentHealth = 500.f;
	AttackDamage = 80.f;
	AttackRange = 300.f;
	MoveSpeed = 600.f;

	bIsDead = false;
	RoarCooldown = 8.f;
	TimeSinceLastRoar = 0.f;
}

void ATRexPawn::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ATRexPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	TimeSinceLastRoar += DeltaTime;

	// Auto-roar when chasing (every RoarCooldown seconds)
	if (BehaviorComponent && TimeSinceLastRoar >= RoarCooldown)
	{
		if (BehaviorComponent->CurrentState == ENPC_TRexState::Chasing ||
			BehaviorComponent->CurrentState == ENPC_TRexState::Attacking)
		{
			TriggerRoar();
		}
	}
}

float ATRexPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ActualDamage = FMath::Max(0.f, DamageAmount);

	CurrentHealth -= ActualDamage;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
			FString::Printf(TEXT("T-Rex hit! HP: %.0f/%.0f"), CurrentHealth, MaxHealth));
	}

	// Aggro — if hit while patrolling, switch to chasing
	if (BehaviorComponent && BehaviorComponent->CurrentState == ENPC_TRexState::Patrolling)
	{
		BehaviorComponent->CurrentState = ENPC_TRexState::Chasing;
		TriggerRoar();
	}

	if (CurrentHealth <= 0.f)
	{
		OnDeath();
	}

	return ActualDamage;
}

bool ATRexPawn::IsAlive() const
{
	return !bIsDead && CurrentHealth > 0.f;
}

float ATRexPawn::GetHealthPercent() const
{
	if (MaxHealth <= 0.f) return 0.f;
	return FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f);
}

void ATRexPawn::OnDeath_Implementation()
{
	bIsDead = true;

	// Disable behavior
	if (BehaviorComponent)
	{
		BehaviorComponent->Deactivate();
	}

	// Collapse mesh — tilt to simulate falling
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FRotator DeathRot = GetActorRotation();
	DeathRot.Roll = 90.f;
	SetActorRotation(DeathRot);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("T-Rex defeated!"));
	}

	// Destroy after 8 seconds (loot window)
	SetLifeSpan(8.f);
}

void ATRexPawn::TriggerRoar()
{
	TimeSinceLastRoar = 0.f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange, TEXT("T-Rex ROARS!"));
	}
}
