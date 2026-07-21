// BiomeEnvironmentAudio.cpp
// Agent #5 — Procedural World Generator
// Ambient audio system tied to biome type and weather conditions.

#include "BiomeEnvironmentAudio.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

ABiomeEnvironmentAudio::ABiomeEnvironmentAudio()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create ambient audio component
	AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
	AmbientAudioComponent->SetupAttachment(RootComponent);
	AmbientAudioComponent->bAutoActivate = false;
	AmbientAudioComponent->VolumeMultiplier = 0.0f;

	// Create weather audio component
	WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
	WeatherAudioComponent->SetupAttachment(RootComponent);
	WeatherAudioComponent->bAutoActivate = false;
	WeatherAudioComponent->VolumeMultiplier = 0.0f;

	// Default biome
	BiomeType = EWorld_BiomeType::Jungle;
}

void ABiomeEnvironmentAudio::BeginPlay()
{
	Super::BeginPlay();

	// Cache player pawn reference
	CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Start audio components at zero volume
	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->Play();
		AmbientAudioComponent->SetVolumeMultiplier(0.0f);
	}
	if (WeatherAudioComponent)
	{
		WeatherAudioComponent->Play();
		WeatherAudioComponent->SetVolumeMultiplier(0.0f);
	}
}

void ABiomeEnvironmentAudio::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Refresh player pawn if lost
	if (!CachedPlayerPawn)
	{
		CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	bool bWasInRange = bPlayerInRange;
	bPlayerInRange = CheckPlayerInRange();

	// Fire biome enter/exit events
	if (bPlayerInRange && !bWasInRange)
	{
		OnPlayerEnterBiome();
		TargetVolume = 1.0f;
	}
	else if (!bPlayerInRange && bWasInRange)
	{
		OnPlayerExitBiome();
		TargetVolume = 0.0f;
	}

	UpdateAudioVolume(DeltaTime);
}

void ABiomeEnvironmentAudio::UpdateAudioVolume(float DeltaTime)
{
	float FadeSpeed = (TargetVolume > CurrentVolume) ? (1.0f / FadeInTime) : (1.0f / FadeOutTime);
	CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);

	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
	}
}

bool ABiomeEnvironmentAudio::CheckPlayerInRange() const
{
	if (!CachedPlayerPawn)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
	return Distance <= AudioRadius;
}

float ABiomeEnvironmentAudio::GetDistanceToPlayer() const
{
	if (!CachedPlayerPawn)
	{
		return TNumericLimits<float>::Max();
	}
	return FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
}

void ABiomeEnvironmentAudio::SetBiomeType(EWorld_BiomeType NewBiomeType)
{
	BiomeType = NewBiomeType;
}

void ABiomeEnvironmentAudio::UpdateWeatherAudio(EWorld_WeatherType WeatherType)
{
	if (!WeatherAudioComponent)
	{
		return;
	}

	// Adjust weather audio volume based on weather intensity
	switch (WeatherType)
	{
	case EWorld_WeatherType::Clear:
		WeatherAudioComponent->SetVolumeMultiplier(0.0f);
		break;
	case EWorld_WeatherType::Cloudy:
		WeatherAudioComponent->SetVolumeMultiplier(0.2f);
		break;
	case EWorld_WeatherType::Rain:
		WeatherAudioComponent->SetVolumeMultiplier(0.8f);
		break;
	case EWorld_WeatherType::Storm:
		WeatherAudioComponent->SetVolumeMultiplier(1.0f);
		break;
	default:
		WeatherAudioComponent->SetVolumeMultiplier(0.0f);
		break;
	}
}

void ABiomeEnvironmentAudio::OnPlayerEnterBiome_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("BiomeEnvironmentAudio: Player entered biome zone — %s"), *GetActorLabel());
}

void ABiomeEnvironmentAudio::OnPlayerExitBiome_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("BiomeEnvironmentAudio: Player exited biome zone — %s"), *GetActorLabel());
}

void ABiomeEnvironmentAudio::PreviewAudio()
{
#if WITH_EDITOR
	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->SetVolumeMultiplier(1.0f);
		AmbientAudioComponent->Play();
	}
	UE_LOG(LogTemp, Log, TEXT("BiomeEnvironmentAudio: Preview playing for biome zone — %s"), *GetActorLabel());
#endif
}
