#include "VFX_ParticleManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Configurar root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VFXRoot"));
	
	// Inicializar propriedades
	MaxPoolSize = 50;
	CurrentEffectIntensity = 1.0f;
}

void AVFX_ParticleManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Sistema iniciado"));
	
	// Pré-carregar sistemas Niagara (paths de exemplo - devem ser configurados no Blueprint)
	if (!FootstepDustSystem)
	{
		FootstepDustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_GPUSprites"));
	}
	
	// Inicializar pool de partículas
	ParticlePool.Reserve(MaxPoolSize);
	for (int32 i = 0; i < MaxPoolSize; i++)
	{
		UNiagaraComponent* PooledParticle = CreateDefaultSubobject<UNiagaraComponent>(*FString::Printf(TEXT("PooledParticle_%d"), i));
		if (PooledParticle)
		{
			PooledParticle->SetupAttachment(RootComponent);
			PooledParticle->SetAutoDestroy(false);
			PooledParticle->SetVisibility(false);
			ParticlePool.Add(PooledParticle);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Pool de %d partículas criado"), ParticlePool.Num());
}

void AVFX_ParticleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Lógica de update para efeitos contínuos (se necessário)
}

void AVFX_ParticleManager::SpawnFootstepEffect(FVector Location, float DinosaurSize)
{
	if (!FootstepDustSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: FootstepDustSystem não configurado"));
		return;
	}
	
	// Usar pool ou spawnar diretamente
	UNiagaraComponent* Effect = GetPooledParticle();
	if (Effect)
	{
		Effect->SetAsset(FootstepDustSystem);
		Effect->SetWorldLocation(Location);
		Effect->SetVisibility(true);
		
		// Configurar parâmetros baseados no tamanho do dinossauro
		Effect->SetFloatParameter(TEXT("Size"), DinosaurSize * CurrentEffectIntensity);
		Effect->SetFloatParameter(TEXT("Intensity"), CurrentEffectIntensity);
		
		Effect->Activate();
		
		UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Pegada spawned em %s (tamanho: %.2f)"), 
			*Location.ToString(), DinosaurSize);
		
		// Auto-retornar ao pool após 3 segundos
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Effect]()
		{
			ReturnParticleToPool(Effect);
		}, 3.0f, false);
	}
	else
	{
		// Fallback: spawnar diretamente no mundo
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FootstepDustSystem,
			Location,
			FRotator::ZeroRotator,
			FVector(DinosaurSize * CurrentEffectIntensity)
		);
		
		UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Pegada spawned diretamente (pool cheio)"));
	}
}

void AVFX_ParticleManager::SpawnCampfireEffect(FVector Location)
{
	if (!CampfireSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: CampfireSystem não configurado"));
		return;
	}
	
	UNiagaraComponent* Effect = GetPooledParticle();
	if (Effect)
	{
		Effect->SetAsset(CampfireSystem);
		Effect->SetWorldLocation(Location);
		Effect->SetVisibility(true);
		Effect->SetFloatParameter(TEXT("Intensity"), CurrentEffectIntensity);
		Effect->Activate();
		
		UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Fogueira spawned em %s"), *Location.ToString());
	}
}

void AVFX_ParticleManager::SpawnBloodImpact(FVector Location, FVector ImpactDirection)
{
	if (!BloodImpactSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: BloodImpactSystem não configurado"));
		return;
	}
	
	UNiagaraComponent* Effect = GetPooledParticle();
	if (Effect)
	{
		Effect->SetAsset(BloodImpactSystem);
		Effect->SetWorldLocation(Location);
		Effect->SetWorldRotation(ImpactDirection.Rotation());
		Effect->SetVisibility(true);
		Effect->SetFloatParameter(TEXT("Intensity"), CurrentEffectIntensity);
		Effect->Activate();
		
		UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Impacto de sangue spawned em %s"), *Location.ToString());
		
		// Auto-retornar ao pool após 2 segundos
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Effect]()
		{
			ReturnParticleToPool(Effect);
		}, 2.0f, false);
	}
}

void AVFX_ParticleManager::SpawnWeatherEffect(EBiomeType BiomeType, FVector Location)
{
	if (!WeatherRainSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: WeatherRainSystem não configurado"));
		return;
	}
	
	UNiagaraComponent* Effect = GetPooledParticle();
	if (Effect)
	{
		Effect->SetAsset(WeatherRainSystem);
		Effect->SetWorldLocation(Location);
		Effect->SetVisibility(true);
		
		// Configurar efeito baseado no bioma
		ConfigureEffectForBiome(Effect, BiomeType);
		
		Effect->Activate();
		
		UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Efeito climático spawned para bioma %d em %s"), 
			(int32)BiomeType, *Location.ToString());
	}
}

void AVFX_ParticleManager::StopAllEffects()
{
	for (UNiagaraComponent* Particle : ParticlePool)
	{
		if (Particle && Particle->IsActive())
		{
			Particle->Deactivate();
			Particle->SetVisibility(false);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Todos os efeitos parados"));
}

void AVFX_ParticleManager::SetEffectIntensity(float Intensity)
{
	CurrentEffectIntensity = FMath::Clamp(Intensity, 0.1f, 3.0f);
	UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Intensidade definida para %.2f"), CurrentEffectIntensity);
}

UNiagaraComponent* AVFX_ParticleManager::GetPooledParticle()
{
	for (UNiagaraComponent* Particle : ParticlePool)
	{
		if (Particle && !Particle->IsActive() && !Particle->IsVisible())
		{
			return Particle;
		}
	}
	
	// Pool cheio
	return nullptr;
}

void AVFX_ParticleManager::ReturnParticleToPool(UNiagaraComponent* Particle)
{
	if (Particle)
	{
		Particle->Deactivate();
		Particle->SetVisibility(false);
		Particle->SetAsset(nullptr);
		
		UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Partícula retornada ao pool"));
	}
}

void AVFX_ParticleManager::ConfigureEffectForBiome(UNiagaraComponent* Effect, EBiomeType BiomeType)
{
	if (!Effect) return;
	
	switch (BiomeType)
	{
		case EBiomeType::Swamp:
			Effect->SetFloatParameter(TEXT("Humidity"), 0.9f);
			Effect->SetFloatParameter(TEXT("Temperature"), 25.0f);
			break;
			
		case EBiomeType::Forest:
			Effect->SetFloatParameter(TEXT("Humidity"), 0.7f);
			Effect->SetFloatParameter(TEXT("Temperature"), 20.0f);
			break;
			
		case EBiomeType::Savanna:
			Effect->SetFloatParameter(TEXT("Humidity"), 0.3f);
			Effect->SetFloatParameter(TEXT("Temperature"), 30.0f);
			break;
			
		case EBiomeType::Desert:
			Effect->SetFloatParameter(TEXT("Humidity"), 0.1f);
			Effect->SetFloatParameter(TEXT("Temperature"), 40.0f);
			break;
			
		case EBiomeType::SnowyMountain:
			Effect->SetFloatParameter(TEXT("Humidity"), 0.8f);
			Effect->SetFloatParameter(TEXT("Temperature"), -5.0f);
			break;
			
		default:
			Effect->SetFloatParameter(TEXT("Humidity"), 0.5f);
			Effect->SetFloatParameter(TEXT("Temperature"), 20.0f);
			break;
	}
	
	Effect->SetFloatParameter(TEXT("Intensity"), CurrentEffectIntensity);
}