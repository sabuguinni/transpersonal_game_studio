#include "Core_ImpactSystem.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogCoreImpactSystem, Log, All);

UCore_ImpactSystem::UCore_ImpactSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Initialize default impact settings
	DefaultImpactThreshold = 500.0f;
	MaxImpactForce = 10000.0f;
	ImpactDamageMultiplier = 1.0f;
	bEnableImpactEffects = true;
	bEnableImpactSounds = true;
	bEnableScreenShake = true;
	
	// Initialize material impact data
	InitializeMaterialImpactData();
}

void UCore_ImpactSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache world reference
	CachedWorld = GetWorld();
	
	// Register for collision events
	if (AActor* Owner = GetOwner())
	{
		if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
		{
			PrimComp->OnComponentHit.AddDynamic(this, &UCore_ImpactSystem::OnComponentHit);
		}
	}
	
	UE_LOG(LogCoreImpactSystem, Log, TEXT("Core Impact System initialized for actor: %s"), 
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
}

void UCore_ImpactSystem::ProcessImpact(const FHitResult& HitResult, const FVector& ImpactVelocity, float ImpactForce)
{
	if (!IsValid(CachedWorld) || !HitResult.bBlockingHit)
	{
		return;
	}
	
	// Calculate impact magnitude
	float ImpactMagnitude = ImpactVelocity.Size();
	if (ImpactMagnitude < DefaultImpactThreshold)
	{
		return; // Impact too weak to process
	}
	
	// Clamp impact force
	float ClampedForce = FMath::Clamp(ImpactForce, 0.0f, MaxImpactForce);
	
	// Create impact data
	FCore_ImpactData ImpactData;
	ImpactData.ImpactLocation = HitResult.ImpactPoint;
	ImpactData.ImpactNormal = HitResult.ImpactNormal;
	ImpactData.ImpactVelocity = ImpactVelocity;
	ImpactData.ImpactForce = ClampedForce;
	ImpactData.ImpactMagnitude = ImpactMagnitude;
	ImpactData.HitActor = HitResult.GetActor();
	ImpactData.HitComponent = HitResult.GetComponent();
	ImpactData.SurfaceMaterial = GetSurfaceMaterial(HitResult);
	
	// Process different impact effects
	ProcessImpactEffects(ImpactData);
	ProcessImpactSound(ImpactData);
	ProcessImpactDamage(ImpactData);
	ProcessScreenShake(ImpactData);
	
	// Broadcast impact event
	OnImpactProcessed.Broadcast(ImpactData);
	
	UE_LOG(LogCoreImpactSystem, Verbose, TEXT("Processed impact at %s with force %f"), 
		*ImpactData.ImpactLocation.ToString(), ClampedForce);
}

void UCore_ImpactSystem::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || !IsValid(HitComp))
	{
		return;
	}
	
	// Calculate impact velocity and force
	FVector ImpactVelocity = FVector::ZeroVector;
	float ImpactForce = NormalImpulse.Size();
	
	// Get velocity from component if available
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HitComp))
	{
		ImpactVelocity = PrimComp->GetComponentVelocity();
	}
	
	// Process the impact
	ProcessImpact(Hit, ImpactVelocity, ImpactForce);
}

void UCore_ImpactSystem::ProcessImpactEffects(const FCore_ImpactData& ImpactData)
{
	if (!bEnableImpactEffects || !IsValid(CachedWorld))
	{
		return;
	}
	
	// Get material-specific effect
	UNiagaraSystem* EffectToSpawn = GetImpactEffect(ImpactData.SurfaceMaterial);
	if (!EffectToSpawn)
	{
		return;
	}
	
	// Spawn particle effect
	UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		CachedWorld,
		EffectToSpawn,
		ImpactData.ImpactLocation,
		ImpactData.ImpactNormal.Rotation()
	);
	
	if (SpawnedEffect)
	{
		// Set effect parameters based on impact data
		SpawnedEffect->SetFloatParameter(TEXT("ImpactForce"), ImpactData.ImpactForce);
		SpawnedEffect->SetVectorParameter(TEXT("ImpactVelocity"), ImpactData.ImpactVelocity);
	}
}

void UCore_ImpactSystem::ProcessImpactSound(const FCore_ImpactData& ImpactData)
{
	if (!bEnableImpactSounds || !IsValid(CachedWorld))
	{
		return;
	}
	
	// Get material-specific sound
	USoundBase* SoundToPlay = GetImpactSound(ImpactData.SurfaceMaterial);
	if (!SoundToPlay)
	{
		return;
	}
	
	// Calculate volume based on impact force
	float Volume = FMath::GetMappedRangeValueClamped(
		FVector2D(DefaultImpactThreshold, MaxImpactForce),
		FVector2D(0.3f, 1.0f),
		ImpactData.ImpactForce
	);
	
	// Play impact sound
	UGameplayStatics::PlaySoundAtLocation(
		CachedWorld,
		SoundToPlay,
		ImpactData.ImpactLocation,
		Volume
	);
}

void UCore_ImpactSystem::ProcessImpactDamage(const FCore_ImpactData& ImpactData)
{
	if (!IsValid(ImpactData.HitActor) || ImpactData.ImpactForce < DefaultImpactThreshold)
	{
		return;
	}
	
	// Calculate damage based on impact force
	float Damage = (ImpactData.ImpactForce / MaxImpactForce) * 100.0f * ImpactDamageMultiplier;
	
	// Apply damage if the actor can receive it
	if (ImpactData.HitActor->CanBeDamaged())
	{
		UGameplayStatics::ApplyPointDamage(
			ImpactData.HitActor,
			Damage,
			ImpactData.ImpactLocation,
			ImpactData,
			nullptr,
			GetOwner(),
			UDamageType::StaticClass()
		);
	}
}

void UCore_ImpactSystem::ProcessScreenShake(const FCore_ImpactData& ImpactData)
{
	if (!bEnableScreenShake || !IsValid(CachedWorld))
	{
		return;
	}
	
	// Calculate shake intensity based on impact force
	float ShakeIntensity = FMath::GetMappedRangeValueClamped(
		FVector2D(DefaultImpactThreshold, MaxImpactForce),
		FVector2D(0.1f, 1.0f),
		ImpactData.ImpactForce
	);
	
	// Apply screen shake to nearby players
	for (FConstPlayerControllerIterator Iterator = CachedWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Iterator->Get())
		{
			// Calculate distance to impact
			if (APawn* Pawn = PC->GetPawn())
			{
				float Distance = FVector::Dist(Pawn->GetActorLocation(), ImpactData.ImpactLocation);
				float MaxShakeDistance = 2000.0f; // 20 meters
				
				if (Distance < MaxShakeDistance)
				{
					float DistanceMultiplier = 1.0f - (Distance / MaxShakeDistance);
					float FinalIntensity = ShakeIntensity * DistanceMultiplier;
					
					// Apply camera shake (would need a proper shake class in a real implementation)
					// PC->ClientStartCameraShake(ImpactShakeClass, FinalIntensity);
				}
			}
		}
	}
}

ECore_SurfaceMaterial UCore_ImpactSystem::GetSurfaceMaterial(const FHitResult& HitResult)
{
	if (!HitResult.PhysMaterial.IsValid())
	{
		return ECore_SurfaceMaterial::Generic;
	}
	
	// Map physical material to our surface material enum
	// This would be expanded with proper material mapping in a real implementation
	FString MaterialName = HitResult.PhysMaterial->GetName();
	
	if (MaterialName.Contains(TEXT("Rock")) || MaterialName.Contains(TEXT("Stone")))
	{
		return ECore_SurfaceMaterial::Rock;
	}
	else if (MaterialName.Contains(TEXT("Wood")))
	{
		return ECore_SurfaceMaterial::Wood;
	}
	else if (MaterialName.Contains(TEXT("Metal")))
	{
		return ECore_SurfaceMaterial::Metal;
	}
	else if (MaterialName.Contains(TEXT("Flesh")) || MaterialName.Contains(TEXT("Organic")))
	{
		return ECore_SurfaceMaterial::Flesh;
	}
	else if (MaterialName.Contains(TEXT("Water")))
	{
		return ECore_SurfaceMaterial::Water;
	}
	else if (MaterialName.Contains(TEXT("Dirt")) || MaterialName.Contains(TEXT("Ground")))
	{
		return ECore_SurfaceMaterial::Dirt;
	}
	
	return ECore_SurfaceMaterial::Generic;
}

UNiagaraSystem* UCore_ImpactSystem::GetImpactEffect(ECore_SurfaceMaterial SurfaceMaterial)
{
	if (MaterialImpactData.Contains(SurfaceMaterial))
	{
		return MaterialImpactData[SurfaceMaterial].ImpactEffect;
	}
	
	return nullptr;
}

USoundBase* UCore_ImpactSystem::GetImpactSound(ECore_SurfaceMaterial SurfaceMaterial)
{
	if (MaterialImpactData.Contains(SurfaceMaterial))
	{
		return MaterialImpactData[SurfaceMaterial].ImpactSound;
	}
	
	return nullptr;
}

void UCore_ImpactSystem::InitializeMaterialImpactData()
{
	// Initialize default material impact data
	// In a real implementation, these would be loaded from data assets
	
	FCore_MaterialImpactData GenericData;
	GenericData.ImpactEffect = nullptr; // Would load from content
	GenericData.ImpactSound = nullptr;  // Would load from content
	MaterialImpactData.Add(ECore_SurfaceMaterial::Generic, GenericData);
	
	// Add other materials as needed
	MaterialImpactData.Add(ECore_SurfaceMaterial::Rock, GenericData);
	MaterialImpactData.Add(ECore_SurfaceMaterial::Wood, GenericData);
	MaterialImpactData.Add(ECore_SurfaceMaterial::Metal, GenericData);
	MaterialImpactData.Add(ECore_SurfaceMaterial::Flesh, GenericData);
	MaterialImpactData.Add(ECore_SurfaceMaterial::Water, GenericData);
	MaterialImpactData.Add(ECore_SurfaceMaterial::Dirt, GenericData);
}

void UCore_ImpactSystem::SetImpactThreshold(float NewThreshold)
{
	DefaultImpactThreshold = FMath::Max(0.0f, NewThreshold);
}

void UCore_ImpactSystem::SetMaxImpactForce(float NewMaxForce)
{
	MaxImpactForce = FMath::Max(DefaultImpactThreshold, NewMaxForce);
}

void UCore_ImpactSystem::SetDamageMultiplier(float NewMultiplier)
{
	ImpactDamageMultiplier = FMath::Max(0.0f, NewMultiplier);
}

void UCore_ImpactSystem::EnableImpactEffects(bool bEnable)
{
	bEnableImpactEffects = bEnable;
}

void UCore_ImpactSystem::EnableImpactSounds(bool bEnable)
{
	bEnableImpactSounds = bEnable;
}

void UCore_ImpactSystem::EnableScreenShake(bool bEnable)
{
	bEnableScreenShake = bEnable;
}