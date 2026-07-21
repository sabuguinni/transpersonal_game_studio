#include "DinosaurCombatEncounter.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatEncounter::ADinosaurCombatEncounter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // 10Hz — sufficient for AI decisions
}

void ADinosaurCombatEncounter::BeginPlay()
{
	Super::BeginPlay();

	// Cache the player pawn at start
	CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!CachedPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatEncounter [%s]: No player pawn found at BeginPlay."), *GetName());
	}
}

void ADinosaurCombatEncounter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEncounterStarted || EncounterState.Outcome != ECombat_EncounterOutcome::Ongoing)
	{
		return;
	}

	// Refresh player cache if lost
	if (!CachedPlayer)
	{
		CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	// Update cached distance
	EncounterState.PlayerDistanceCached = GetDistanceToPlayer();
	EncounterState.TimeInPhase += DeltaTime;

	// Melee cooldown tick
	if (MeleeAttackTimer > 0.f)
	{
		MeleeAttackTimer -= DeltaTime;
	}

	UpdatePhase(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatEncounter::StartEncounter()
{
	if (bEncounterStarted)
	{
		return;
	}

	bEncounterStarted = true;
	DinoCurrentHealth = DinoMaxHealth;
	EncounterState = FCombat_EncounterState();
	TransitionToPhase(ECombat_EncounterPhase::Alert);

	UE_LOG(LogTemp, Log, TEXT("DinosaurCombatEncounter [%s]: Encounter started."), *GetName());
}

void ADinosaurCombatEncounter::EndEncounter(ECombat_EncounterOutcome Outcome)
{
	if (EncounterState.Outcome != ECombat_EncounterOutcome::Ongoing)
	{
		return;
	}

	EncounterState.Outcome = Outcome;
	bEncounterStarted = false;

	OnEncounterEnded.Broadcast(Outcome);

	UE_LOG(LogTemp, Log, TEXT("DinosaurCombatEncounter [%s]: Encounter ended — Outcome: %d"),
		*GetName(), static_cast<int32>(Outcome));
}

void ADinosaurCombatEncounter::ApplyDamageToDino(float DamageAmount)
{
	if (DamageAmount <= 0.f || !IsEncounterActive())
	{
		return;
	}

	DinoCurrentHealth = FMath::Max(0.f, DinoCurrentHealth - DamageAmount);

	UE_LOG(LogTemp, Log, TEXT("DinosaurCombatEncounter [%s]: Dino took %.1f damage — HP: %.1f/%.1f"),
		*GetName(), DamageAmount, DinoCurrentHealth, DinoMaxHealth);

	// Check for death
	if (DinoCurrentHealth <= 0.f)
	{
		EndEncounter(ECombat_EncounterOutcome::DinoKilled);
		return;
	}

	// Chance to retreat when heavily damaged
	const float HealthPct = DinoCurrentHealth / DinoMaxHealth;
	if (HealthPct < 0.3f)
	{
		const float Roll = FMath::FRand();
		if (Roll < RetreatChanceOnDamage)
		{
			TransitionToPhase(ECombat_EncounterPhase::Retreat);
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatEncounter::UpdatePhase(float DeltaTime)
{
	const float Dist = EncounterState.PlayerDistanceCached;

	switch (EncounterState.Phase)
	{
	case ECombat_EncounterPhase::Idle:
		if (Dist < DetectionRadius)
		{
			TransitionToPhase(ECombat_EncounterPhase::Alert);
		}
		break;

	case ECombat_EncounterPhase::Alert:
		EncounterState.bPlayerSpotted = (Dist < DetectionRadius);
		if (Dist < ChargeRadius)
		{
			TransitionToPhase(ECombat_EncounterPhase::Stalk);
		}
		else if (Dist > DetectionRadius * 1.2f)
		{
			TransitionToPhase(ECombat_EncounterPhase::Patrol);
		}
		break;

	case ECombat_EncounterPhase::Stalk:
		if (Dist < ChargeRadius * 0.6f)
		{
			TransitionToPhase(ECombat_EncounterPhase::Charge);
		}
		else if (Dist > DetectionRadius)
		{
			TransitionToPhase(ECombat_EncounterPhase::Alert);
		}
		break;

	case ECombat_EncounterPhase::Charge:
		if (Dist < MeleeRadius)
		{
			TransitionToPhase(ECombat_EncounterPhase::Attack);
		}
		else if (Dist > FleeDistance)
		{
			// Player escaped
			EndEncounter(ECombat_EncounterOutcome::PlayerFled);
		}
		break;

	case ECombat_EncounterPhase::Attack:
		// Execute melee if cooldown elapsed
		if (MeleeAttackTimer <= 0.f && CachedPlayer)
		{
			// Apply damage to player via UE damage system
			UGameplayStatics::ApplyDamage(
				CachedPlayer,
				MeleeDamage,
				nullptr,
				this,
				nullptr
			);
			MeleeAttackTimer = MeleeCooldown;
			EncounterState.AttackCount++;

			UE_LOG(LogTemp, Log, TEXT("DinosaurCombatEncounter [%s]: Melee hit #%d — %.1f dmg"),
				*GetName(), EncounterState.AttackCount, MeleeDamage);
		}

		// If player moves away, resume charge
		if (Dist > MeleeRadius * 1.5f)
		{
			TransitionToPhase(ECombat_EncounterPhase::Charge);
		}
		break;

	case ECombat_EncounterPhase::Retreat:
		// After 5 seconds of retreat, check if dino should flee entirely
		if (EncounterState.TimeInPhase > 5.f)
		{
			EndEncounter(ECombat_EncounterOutcome::DinoFled);
		}
		break;

	case ECombat_EncounterPhase::Flee:
		EndEncounter(ECombat_EncounterOutcome::DinoFled);
		break;

	default:
		break;
	}
}

void ADinosaurCombatEncounter::TransitionToPhase(ECombat_EncounterPhase NewPhase)
{
	if (EncounterState.Phase == NewPhase)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("DinosaurCombatEncounter [%s]: Phase %d -> %d"),
		*GetName(), static_cast<int32>(EncounterState.Phase), static_cast<int32>(NewPhase));

	EncounterState.Phase = NewPhase;
	EncounterState.TimeInPhase = 0.f;

	OnPhaseChanged.Broadcast(NewPhase);
}

float ADinosaurCombatEncounter::GetDistanceToPlayer() const
{
	if (!CachedPlayer)
	{
		return 99999.f;
	}
	return FVector::Dist(GetActorLocation(), CachedPlayer->GetActorLocation());
}

bool ADinosaurCombatEncounter::IsPlayerInCover() const
{
	if (!CachedPlayer || !GetWorld())
	{
		return false;
	}

	// Simple line-of-sight check — if there is a blocking hit between dino and player, player is in cover
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(CachedPlayer);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		GetActorLocation(),
		CachedPlayer->GetActorLocation(),
		ECC_Visibility,
		Params
	);

	return bBlocked;
}
