// SurvivalComponent.cpp — Core Systems Programmer #03
// Implements survival stats: health, hunger, thirst, stamina, temperature, fear.
// Ticks every second to drain stats and apply starvation/dehydration damage.

#include "SurvivalComponent.h"
#include "Math/UnrealMathUtility.h"

USurvivalComponent::USurvivalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz tick, drain applied per second via accumulator
}

void USurvivalComponent::BeginPlay()
{
	Super::BeginPlay();

	// Reset all stats to full on spawn
	Health = 100.f;
	Hunger = 100.f;
	Thirst = 100.f;
	Stamina = 100.f;
	Temperature = 37.f;
	Fear = 0.f;
	DrainAccumulator = 0.f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsAlive())
	{
		return;
	}

	// Accumulate time — apply drain every 1 second
	DrainAccumulator += DeltaTime;
	if (DrainAccumulator >= 1.0f)
	{
		ApplySurvivalDrain(DrainAccumulator);
		DrainAccumulator = 0.f;
	}

	// Stamina regeneration (continuous, not per-second)
	if (Stamina < 100.f)
	{
		Stamina = FMath::Min(100.f, Stamina + StaminaRegenRate * DeltaTime);
	}

	// Fear decay — fear fades over time when no threat is present
	if (Fear > 0.f)
	{
		Fear = FMath::Max(0.f, Fear - 5.f * DeltaTime);
	}

	ClampStats();
}

void USurvivalComponent::ApplySurvivalDrain(float DeltaSeconds)
{
	// Hunger drain
	Hunger = FMath::Max(0.f, Hunger - HungerDrainRate * DeltaSeconds);

	// Thirst drain
	Thirst = FMath::Max(0.f, Thirst - ThirstDrainRate * DeltaSeconds);

	// Starvation damage — health drains when hunger or thirst hits critical
	if (Hunger <= CriticalHungerThreshold)
	{
		Health = FMath::Max(0.f, Health - HealthDrainWhenStarving * DeltaSeconds);
	}
	if (Thirst <= CriticalThirstThreshold)
	{
		// Dehydration is more dangerous than starvation
		Health = FMath::Max(0.f, Health - HealthDrainWhenStarving * 2.f * DeltaSeconds);
	}

	// Temperature effects — hypothermia/hyperthermia
	if (Temperature < 35.f || Temperature > 40.f)
	{
		Health = FMath::Max(0.f, Health - 0.5f * DeltaSeconds);
	}
}

bool USurvivalComponent::ApplyDamage(float DamageAmount)
{
	if (!IsAlive() || DamageAmount <= 0.f)
	{
		return false;
	}

	Health = FMath::Max(0.f, Health - DamageAmount);
	return Health <= 0.f; // Returns true if this damage killed the character
}

void USurvivalComponent::EatFood(float Amount)
{
	if (Amount <= 0.f) return;
	Hunger = FMath::Min(100.f, Hunger + Amount);
}

void USurvivalComponent::DrinkWater(float Amount)
{
	if (Amount <= 0.f) return;
	Thirst = FMath::Min(100.f, Thirst + Amount);
}

bool USurvivalComponent::DrainStamina(float Amount)
{
	if (Amount <= 0.f) return true;
	if (Stamina < Amount)
	{
		Stamina = 0.f;
		return false; // Not enough stamina
	}
	Stamina -= Amount;
	return true;
}

void USurvivalComponent::SetFear(float FearLevel)
{
	Fear = FMath::Clamp(FearLevel, 0.f, 100.f);
}

FSurvivalStats USurvivalComponent::GetStats() const
{
	FSurvivalStats Stats;
	Stats.Health = Health;
	Stats.Hunger = Hunger;
	Stats.Thirst = Thirst;
	Stats.Stamina = Stamina;
	Stats.Temperature = Temperature;
	Stats.Fear = Fear;
	return Stats;
}

void USurvivalComponent::ClampStats()
{
	Health = FMath::Clamp(Health, 0.f, 100.f);
	Hunger = FMath::Clamp(Hunger, 0.f, 100.f);
	Thirst = FMath::Clamp(Thirst, 0.f, 100.f);
	Stamina = FMath::Clamp(Stamina, 0.f, 100.f);
	Temperature = FMath::Clamp(Temperature, 20.f, 45.f);
	Fear = FMath::Clamp(Fear, 0.f, 100.f);
}
