// Copyright Transpersonal Game Studio. All Rights Reserved.
// SurvivalComponent.cpp — Full implementation of survival stat tick system
// Agent #04 Performance Optimizer — PROD_CYCLE_AUTO_20260619_009

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — budget: <0.05ms/frame

	// Default stat values
	Health = 100.0f;
	MaxHealth = 100.0f;
	Hunger = 100.0f;
	MaxHunger = 100.0f;
	Thirst = 100.0f;
	MaxThirst = 100.0f;
	Stamina = 100.0f;
	MaxStamina = 100.0f;
	Fear = 0.0f;
	MaxFear = 100.0f;

	// Drain rates per second (realistic survival pacing)
	HungerDrainRate = 0.5f;   // ~200s to empty (3.3 min)
	ThirstDrainRate = 0.8f;   // ~125s to empty (2 min)
	StaminaDrainRate = 5.0f;  // ~20s sprint to empty
	StaminaRegenRate = 8.0f;  // ~12.5s to full regen
	FearDecayRate = 2.0f;     // Fear decays when safe

	bIsSprinting = false;
	bIsDead = false;
}

void USurvivalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead)
	{
		return;
	}

	// Use TickInterval-scaled delta (component ticks at 10Hz, DeltaTime ~0.1s)
	const float ScaledDelta = DeltaTime;

	// --- Hunger drain ---
	const float PrevHunger = Hunger;
	Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * ScaledDelta);
	if (!FMath::IsNearlyEqual(PrevHunger, Hunger))
	{
		OnSurvivalStatChanged.Broadcast(ESurvivalStat::Hunger, Hunger, MaxHunger);
	}

	// --- Thirst drain ---
	const float PrevThirst = Thirst;
	Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * ScaledDelta);
	if (!FMath::IsNearlyEqual(PrevThirst, Thirst))
	{
		OnSurvivalStatChanged.Broadcast(ESurvivalStat::Thirst, Thirst, MaxThirst);
	}

	// --- Stamina: drain when sprinting, regen when not ---
	const float PrevStamina = Stamina;
	if (bIsSprinting)
	{
		Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * ScaledDelta);
		if (Stamina <= 0.0f)
		{
			// Force stop sprint when exhausted
			SetSprinting(false);
		}
	}
	else
	{
		Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * ScaledDelta);
	}
	if (!FMath::IsNearlyEqual(PrevStamina, Stamina))
	{
		OnSurvivalStatChanged.Broadcast(ESurvivalStat::Stamina, Stamina, MaxStamina);
	}

	// --- Fear decay when not in danger ---
	const float PrevFear = Fear;
	if (Fear > 0.0f)
	{
		Fear = FMath::Max(0.0f, Fear - FearDecayRate * ScaledDelta);
		if (!FMath::IsNearlyEqual(PrevFear, Fear))
		{
			OnSurvivalStatChanged.Broadcast(ESurvivalStat::Fear, Fear, MaxFear);
		}
	}

	// --- Starvation/dehydration damage ---
	if (Hunger <= 0.0f || Thirst <= 0.0f)
	{
		// 1 HP/s damage when starving or dehydrated
		ApplyDamage(1.0f * ScaledDelta);
	}
}

// --- Health ---

void USurvivalComponent::ApplyDamage(float Amount)
{
	if (bIsDead || Amount <= 0.0f) return;

	const float PrevHealth = Health;
	Health = FMath::Max(0.0f, Health - Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Health, Health, MaxHealth);

	if (Health <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnPlayerDied.Broadcast();
	}
}

void USurvivalComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.0f) return;
	Health = FMath::Min(MaxHealth, Health + Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Health, Health, MaxHealth);
}

// --- Hunger ---

void USurvivalComponent::ConsumeFood(float Amount)
{
	if (Amount <= 0.0f) return;
	Hunger = FMath::Min(MaxHunger, Hunger + Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Hunger, Hunger, MaxHunger);
}

// --- Thirst ---

void USurvivalComponent::DrinkWater(float Amount)
{
	if (Amount <= 0.0f) return;
	Thirst = FMath::Min(MaxThirst, Thirst + Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Thirst, Thirst, MaxThirst);
}

// --- Stamina ---

void USurvivalComponent::SetSprinting(bool bSprint)
{
	if (bSprint && Stamina <= 10.0f)
	{
		// Cannot start sprinting with < 10 stamina
		bIsSprinting = false;
		return;
	}
	bIsSprinting = bSprint;
}

void USurvivalComponent::RestoreStamina(float Amount)
{
	if (Amount <= 0.0f) return;
	Stamina = FMath::Min(MaxStamina, Stamina + Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Stamina, Stamina, MaxStamina);
}

// --- Fear ---

void USurvivalComponent::AddFear(float Amount)
{
	if (Amount <= 0.0f) return;
	Fear = FMath::Min(MaxFear, Fear + Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Fear, Fear, MaxFear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
	if (Amount <= 0.0f) return;
	Fear = FMath::Max(0.0f, Fear - Amount);
	OnSurvivalStatChanged.Broadcast(ESurvivalStat::Fear, Fear, MaxFear);
}

// --- Getters ---

float USurvivalComponent::GetHealth() const { return Health; }
float USurvivalComponent::GetMaxHealth() const { return MaxHealth; }
float USurvivalComponent::GetHunger() const { return Hunger; }
float USurvivalComponent::GetMaxHunger() const { return MaxHunger; }
float USurvivalComponent::GetThirst() const { return Thirst; }
float USurvivalComponent::GetMaxThirst() const { return MaxThirst; }
float USurvivalComponent::GetStamina() const { return Stamina; }
float USurvivalComponent::GetMaxStamina() const { return MaxStamina; }
float USurvivalComponent::GetFear() const { return Fear; }
float USurvivalComponent::GetMaxFear() const { return MaxFear; }
bool USurvivalComponent::IsDead() const { return bIsDead; }
bool USurvivalComponent::IsSprinting() const { return bIsSprinting; }

float USurvivalComponent::GetHealthPercent() const
{
	return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
	return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
	return (MaxThirst > 0.0f) ? (Thirst / MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
	return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}

float USurvivalComponent::GetFearPercent() const
{
	return (MaxFear > 0.0f) ? (Fear / MaxFear) : 0.0f;
}
