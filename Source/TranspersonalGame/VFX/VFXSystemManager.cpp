#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UVFX_SystemManager::UVFX_SystemManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for VFX cleanup

	// Default settings
	MaxActiveEffects = 50;
	EffectLifetime = 5.0f;
	bEnableVFXLOD = true;
	LastCleanupTime = 0.0f;

	// Initialize Niagara systems to nullptr - will be loaded at runtime
	FootstepDustSystem = nullptr;
	BloodImpactSystem = nullptr;
	WeaponImpactSystem = nullptr;
	DebrisSystem = nullptr;
}

void UVFX_SystemManager::BeginPlay()
{
	Super::BeginPlay();

	// Try to load default Niagara systems
	FootstepDustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultParticleSystem"));
	if (!FootstepDustSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VFX: Failed to load FootstepDustSystem"));
	}

	// Initialize other systems with fallbacks
	BloodImpactSystem = FootstepDustSystem; // Use same system as fallback
	WeaponImpactSystem = FootstepDustSystem;
	DebrisSystem = FootstepDustSystem;

	UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized with %d max effects"), MaxActiveEffects);
}

void UVFX_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cleanup old effects every 2 seconds
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastCleanupTime > 2.0f)
	{
		CleanupOldEffects();
		LastCleanupTime = CurrentTime;
	}
}

void UVFX_SystemManager::SpawnFootstepEffect(FVector Location, EDinosaurSpecies DinoType, float ImpactForce)
{
	if (!ShouldSpawnEffect(Location))
	{
		return;
	}

	UNiagaraSystem* SystemToUse = GetFootstepSystemForSpecies(DinoType);
	if (!SystemToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("VFX: No footstep system available for species %d"), (int32)DinoType);
		return;
	}

	// Spawn Niagara effect
	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		SystemToUse,
		Location,
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (Effect)
	{
		// Set parameters based on dinosaur type and impact force
		Effect->SetFloatParameter(TEXT("ImpactForce"), ImpactForce);
		Effect->SetFloatParameter(TEXT("Scale"), DinoType == EDinosaurSpecies::TRex ? 3.0f : 1.0f);
		
		RegisterActiveEffect(Effect);
		UE_LOG(LogTemp, Log, TEXT("VFX: Spawned footstep effect for %s at %s"), 
			*UEnum::GetValueAsString(DinoType), *Location.ToString());
	}
}

void UVFX_SystemManager::SpawnPlayerFootstepEffect(FVector Location, ESurfaceType SurfaceType)
{
	if (!ShouldSpawnEffect(Location) || !FootstepDustSystem)
	{
		return;
	}

	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		FootstepDustSystem,
		Location,
		FRotator::ZeroRotator,
		FVector(0.3f), // Smaller scale for player
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (Effect)
	{
		// Adjust effect based on surface type
		float DustIntensity = 1.0f;
		switch (SurfaceType)
		{
		case ESurfaceType::Dirt:
			DustIntensity = 1.5f;
			break;
		case ESurfaceType::Rock:
			DustIntensity = 0.5f;
			break;
		case ESurfaceType::Mud:
			DustIntensity = 2.0f;
			break;
		default:
			DustIntensity = 1.0f;
			break;
		}

		Effect->SetFloatParameter(TEXT("DustIntensity"), DustIntensity);
		RegisterActiveEffect(Effect);
	}
}

void UVFX_SystemManager::SpawnBloodImpactEffect(FVector Location, FVector ImpactDirection, float Damage)
{
	if (!ShouldSpawnEffect(Location) || !BloodImpactSystem)
	{
		return;
	}

	FRotator ImpactRotation = ImpactDirection.Rotation();
	
	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		BloodImpactSystem,
		Location,
		ImpactRotation,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (Effect)
	{
		// Scale effect based on damage
		float BloodScale = FMath::Clamp(Damage / 50.0f, 0.5f, 3.0f);
		Effect->SetFloatParameter(TEXT("BloodScale"), BloodScale);
		Effect->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection);
		
		RegisterActiveEffect(Effect);
		UE_LOG(LogTemp, Log, TEXT("VFX: Spawned blood impact effect with damage %f"), Damage);
	}
}

void UVFX_SystemManager::SpawnWeaponImpactEffect(FVector Location, EWeaponType WeaponType, ESurfaceType SurfaceHit)
{
	if (!ShouldSpawnEffect(Location))
	{
		return;
	}

	UNiagaraSystem* SystemToUse = GetImpactSystemForWeapon(WeaponType);
	if (!SystemToUse)
	{
		return;
	}

	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		SystemToUse,
		Location,
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (Effect)
	{
		RegisterActiveEffect(Effect);
		UE_LOG(LogTemp, Log, TEXT("VFX: Spawned weapon impact effect for weapon %d on surface %d"), 
			(int32)WeaponType, (int32)SurfaceHit);
	}
}

void UVFX_SystemManager::SpawnDustCloudEffect(FVector Location, float Intensity)
{
	if (!ShouldSpawnEffect(Location) || !DebrisSystem)
	{
		return;
	}

	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		DebrisSystem,
		Location,
		FRotator::ZeroRotator,
		FVector(Intensity),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (Effect)
	{
		Effect->SetFloatParameter(TEXT("DustIntensity"), Intensity);
		RegisterActiveEffect(Effect);
	}
}

void UVFX_SystemManager::SpawnDebrisEffect(FVector Location, FVector Direction, int32 DebrisCount)
{
	if (!ShouldSpawnEffect(Location) || !DebrisSystem)
	{
		return;
	}

	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		DebrisSystem,
		Location,
		Direction.Rotation(),
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (Effect)
	{
		Effect->SetIntParameter(TEXT("DebrisCount"), DebrisCount);
		Effect->SetVectorParameter(TEXT("Direction"), Direction);
		RegisterActiveEffect(Effect);
	}
}

void UVFX_SystemManager::SetVFXQualityLevel(int32 QualityLevel)
{
	// Adjust max effects based on quality level
	switch (QualityLevel)
	{
	case 0: // Low
		MaxActiveEffects = 20;
		bEnableVFXLOD = true;
		break;
	case 1: // Medium
		MaxActiveEffects = 35;
		bEnableVFXLOD = true;
		break;
	case 2: // High
		MaxActiveEffects = 50;
		bEnableVFXLOD = false;
		break;
	case 3: // Ultra
		MaxActiveEffects = 100;
		bEnableVFXLOD = false;
		break;
	default:
		MaxActiveEffects = 35;
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("VFX: Quality level set to %d, max effects: %d"), QualityLevel, MaxActiveEffects);
}

void UVFX_SystemManager::CleanupOldEffects()
{
	int32 RemovedCount = 0;
	
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
		{
			ActiveEffects.RemoveAt(i);
			RemovedCount++;
		}
	}

	// If we still have too many effects, remove oldest ones
	while (ActiveEffects.Num() > MaxActiveEffects)
	{
		if (IsValid(ActiveEffects[0]))
		{
			ActiveEffects[0]->DestroyComponent();
		}
		ActiveEffects.RemoveAt(0);
		RemovedCount++;
	}

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("VFX: Cleaned up %d old effects, %d active remaining"), 
			RemovedCount, ActiveEffects.Num());
	}
}

UNiagaraSystem* UVFX_SystemManager::GetFootstepSystemForSpecies(EDinosaurSpecies Species)
{
	// For now, return the same system for all species
	// In the future, different species could have different effects
	return FootstepDustSystem;
}

UNiagaraSystem* UVFX_SystemManager::GetImpactSystemForWeapon(EWeaponType WeaponType)
{
	// For now, return the same system for all weapons
	// In the future, different weapons could have different effects
	return WeaponImpactSystem;
}

void UVFX_SystemManager::RegisterActiveEffect(UNiagaraComponent* Effect)
{
	if (IsValid(Effect))
	{
		ActiveEffects.Add(Effect);
		
		// If we're over the limit, remove the oldest effect
		if (ActiveEffects.Num() > MaxActiveEffects)
		{
			if (IsValid(ActiveEffects[0]))
			{
				ActiveEffects[0]->DestroyComponent();
			}
			ActiveEffects.RemoveAt(0);
		}
	}
}

bool UVFX_SystemManager::ShouldSpawnEffect(FVector Location)
{
	// Don't spawn if we're at the effect limit
	if (ActiveEffects.Num() >= MaxActiveEffects)
	{
		return false;
	}

	// Don't spawn if there's already an effect very close to this location
	for (UNiagaraComponent* Effect : ActiveEffects)
	{
		if (IsValid(Effect))
		{
			float Distance = FVector::Dist(Effect->GetComponentLocation(), Location);
			if (Distance < 100.0f) // 1 meter minimum distance
			{
				return false;
			}
		}
	}

	return true;
}

float UVFX_SystemManager::CalculateEffectIntensity(float Distance)
{
	// Reduce effect intensity based on distance from player
	float MaxDistance = 2000.0f; // 20 meters
	float Intensity = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.1f, 1.0f);
	
	return Intensity;
}