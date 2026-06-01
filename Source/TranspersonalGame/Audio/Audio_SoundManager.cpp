#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

UAudio_SoundManager::UAudio_SoundManager()
{
	MasterVolume = 1.0f;
	SFXVolume = 1.0f;
	MusicVolume = 1.0f;
	AmbienceVolume = 1.0f;
	CurrentBiome = EBiomeType::Savanna;
	CurrentTimeOfDay = 12.0f;
	CurrentWeather = EWeatherType::Clear;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Initializing audio system"));
	
	InitializeSoundRegistry();
	InitializeAmbienceLayers();
	
	// Set default biome ambience
	SetBiomeAmbience(EBiomeType::Savanna, 0.0f);
}

void UAudio_SoundManager::Deinitialize()
{
	StopAllAmbience(0.0f);
	
	// Clean up all active components
	for (UAudioComponent* Component : ActiveAudioComponents)
	{
		if (IsValid(Component))
		{
			Component->Stop();
			Component->DestroyComponent();
		}
	}
	ActiveAudioComponents.Empty();
	ActiveAmbienceComponents.Empty();
	
	Super::Deinitialize();
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
	if (!SoundRegistry.Contains(SoundID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound ID '%s' not found in registry"), *SoundID);
		return;
	}

	const FAudio_SoundEntry& SoundEntry = SoundRegistry[SoundID];
	if (!SoundEntry.Sound.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound asset for ID '%s' is not valid"), *SoundID);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float FinalVolume = CalculateFinalVolume(SoundEntry.Volume, VolumeMultiplier);
	
	UGameplayStatics::PlaySoundAtLocation(
		World,
		SoundEntry.Sound.Get(),
		Location,
		FinalVolume,
		SoundEntry.Pitch,
		0.0f,
		nullptr,
		nullptr,
		true
	);
}

void UAudio_SoundManager::PlaySound2D(const FString& SoundID, float VolumeMultiplier)
{
	if (!SoundRegistry.Contains(SoundID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound ID '%s' not found in registry"), *SoundID);
		return;
	}

	const FAudio_SoundEntry& SoundEntry = SoundRegistry[SoundID];
	if (!SoundEntry.Sound.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound asset for ID '%s' is not valid"), *SoundID);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float FinalVolume = CalculateFinalVolume(SoundEntry.Volume, VolumeMultiplier);
	
	UGameplayStatics::PlaySound2D(
		World,
		SoundEntry.Sound.Get(),
		FinalVolume,
		SoundEntry.Pitch,
		0.0f,
		nullptr,
		true
	);
}

UAudioComponent* UAudio_SoundManager::PlaySoundAttached(const FString& SoundID, USceneComponent* AttachComponent, float VolumeMultiplier)
{
	if (!SoundRegistry.Contains(SoundID) || !IsValid(AttachComponent))
	{
		return nullptr;
	}

	const FAudio_SoundEntry& SoundEntry = SoundRegistry[SoundID];
	if (!SoundEntry.Sound.IsValid())
	{
		return nullptr;
	}

	UAudioComponent* AudioComp = CreateAudioComponent();
	if (!AudioComp)
	{
		return nullptr;
	}

	AudioComp->SetSound(SoundEntry.Sound.Get());
	AudioComp->SetVolumeMultiplier(CalculateFinalVolume(SoundEntry.Volume, VolumeMultiplier));
	AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
	AudioComp->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepWorldTransform);
	AudioComp->Play();

	ActiveAudioComponents.Add(AudioComp);
	return AudioComp;
}

void UAudio_SoundManager::SetBiomeAmbience(EBiomeType BiomeType, float FadeTime)
{
	if (CurrentBiome == BiomeType)
	{
		return;
	}

	// Fade out current ambience
	if (ActiveAmbienceComponents.Contains(CurrentBiome))
	{
		UAudioComponent* CurrentAmbience = ActiveAmbienceComponents[CurrentBiome];
		if (IsValid(CurrentAmbience))
		{
			CurrentAmbience->FadeOut(FadeTime, 0.0f);
		}
	}

	// Fade in new ambience
	if (AmbienceLayers.Contains(BiomeType))
	{
		const FAudio_AmbienceLayer& Layer = AmbienceLayers[BiomeType];
		if (Layer.AmbienceSound.IsValid())
		{
			UAudioComponent* NewAmbience = CreateAudioComponent();
			if (NewAmbience)
			{
				NewAmbience->SetSound(Layer.AmbienceSound.Get());
				NewAmbience->SetVolumeMultiplier(0.0f);
				NewAmbience->bAutoActivate = false;
				NewAmbience->Play();
				NewAmbience->FadeIn(FadeTime, CalculateFinalVolume(Layer.BaseVolume, 1.0f));
				
				ActiveAmbienceComponents.Add(BiomeType, NewAmbience);
				ActiveAudioComponents.Add(NewAmbience);
			}
		}
	}

	CurrentBiome = BiomeType;
	UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Changed biome ambience to %d"), (int32)BiomeType);
}

void UAudio_SoundManager::StopAllAmbience(float FadeTime)
{
	for (auto& Pair : ActiveAmbienceComponents)
	{
		UAudioComponent* Component = Pair.Value;
		if (IsValid(Component))
		{
			if (FadeTime > 0.0f)
			{
				Component->FadeOut(FadeTime, 0.0f);
			}
			else
			{
				Component->Stop();
			}
		}
	}
	
	if (FadeTime <= 0.0f)
	{
		ActiveAmbienceComponents.Empty();
	}
}

void UAudio_SoundManager::SetTimeOfDayAudioMix(float TimeOfDay)
{
	CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 24.0f);
	
	// Adjust ambience volume based on time of day
	float NightFactor = 1.0f;
	if (TimeOfDay >= 20.0f || TimeOfDay <= 6.0f)
	{
		// Night time - reduce volume and change mix
		NightFactor = 0.7f;
	}
	else if (TimeOfDay >= 6.0f && TimeOfDay <= 8.0f)
	{
		// Dawn
		NightFactor = FMath::Lerp(0.7f, 1.0f, (TimeOfDay - 6.0f) / 2.0f);
	}
	else if (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f)
	{
		// Dusk
		NightFactor = FMath::Lerp(1.0f, 0.7f, (TimeOfDay - 18.0f) / 2.0f);
	}

	// Apply time-based volume adjustment to active ambience
	for (auto& Pair : ActiveAmbienceComponents)
	{
		UAudioComponent* Component = Pair.Value;
		if (IsValid(Component) && AmbienceLayers.Contains(Pair.Key))
		{
			const FAudio_AmbienceLayer& Layer = AmbienceLayers[Pair.Key];
			float NewVolume = CalculateFinalVolume(Layer.BaseVolume * NightFactor, 1.0f);
			Component->SetVolumeMultiplier(NewVolume);
		}
	}
}

void UAudio_SoundManager::SetWeatherAudioMix(EWeatherType WeatherType, float Intensity)
{
	CurrentWeather = WeatherType;
	
	// Weather-specific audio adjustments
	switch (WeatherType)
	{
		case EWeatherType::Rain:
			// Add rain sounds, muffle distant sounds
			PlaySound2D("Weather_Rain", Intensity);
			break;
		case EWeatherType::Storm:
			// Add thunder, heavy rain, wind
			PlaySound2D("Weather_Storm", Intensity);
			break;
		case EWeatherType::Fog:
			// Muffle all sounds, add eerie ambience
			SetAmbienceVolume(AmbienceVolume * 0.6f);
			break;
		default:
			break;
	}
}

void UAudio_SoundManager::PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, FVector Location)
{
	FString SoundID = FString::Printf(TEXT("Dino_%s_%s"), 
		*UEnum::GetValueAsString(Species), *SoundType);
	
	PlaySoundAtLocation(SoundID, Location, 1.0f);
}

void UAudio_SoundManager::SetDinosaurProximityAudio(float Distance, EDinosaurSpecies Species)
{
	// Adjust audio mix based on dangerous dinosaur proximity
	if (Species == EDinosaurSpecies::TRex && Distance < 2000.0f)
	{
		// T-Rex nearby - increase tension
		float TensionFactor = 1.0f - (Distance / 2000.0f);
		SetAmbienceVolume(AmbienceVolume * (1.0f - TensionFactor * 0.5f));
		
		if (Distance < 500.0f)
		{
			// Very close - play heartbeat or tension sound
			PlaySound2D("Player_Heartbeat", TensionFactor);
		}
	}
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	
	// Update all active components
	for (UAudioComponent* Component : ActiveAudioComponents)
	{
		if (IsValid(Component))
		{
			// Recalculate volume with new master volume
			float CurrentVolume = Component->GetVolumeMultiplier();
			Component->SetVolumeMultiplier(CurrentVolume * MasterVolume);
		}
	}
}

void UAudio_SoundManager::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_SoundManager::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_SoundManager::SetAmbienceVolume(float Volume)
{
	AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	
	// Update active ambience components
	for (auto& Pair : ActiveAmbienceComponents)
	{
		UAudioComponent* Component = Pair.Value;
		if (IsValid(Component) && AmbienceLayers.Contains(Pair.Key))
		{
			const FAudio_AmbienceLayer& Layer = AmbienceLayers[Pair.Key];
			float NewVolume = CalculateFinalVolume(Layer.BaseVolume, 1.0f);
			Component->SetVolumeMultiplier(NewVolume);
		}
	}
}

void UAudio_SoundManager::InitializeSoundRegistry()
{
	// Initialize basic sound registry with placeholder entries
	SoundRegistry.Empty();
	
	// Player sounds
	SoundRegistry.Add("Player_Footstep", FAudio_SoundEntry());
	SoundRegistry.Add("Player_Heartbeat", FAudio_SoundEntry());
	SoundRegistry.Add("Player_Breathing", FAudio_SoundEntry());
	
	// Dinosaur sounds
	SoundRegistry.Add("Dino_TRex_Roar", FAudio_SoundEntry());
	SoundRegistry.Add("Dino_TRex_Footstep", FAudio_SoundEntry());
	SoundRegistry.Add("Dino_Velociraptor_Screech", FAudio_SoundEntry());
	SoundRegistry.Add("Dino_Triceratops_Bellow", FAudio_SoundEntry());
	SoundRegistry.Add("Dino_Brachiosaurus_Call", FAudio_SoundEntry());
	
	// Environment sounds
	SoundRegistry.Add("Environment_Wind", FAudio_SoundEntry());
	SoundRegistry.Add("Environment_Water", FAudio_SoundEntry());
	SoundRegistry.Add("Environment_Fire", FAudio_SoundEntry());
	
	// Weather sounds
	SoundRegistry.Add("Weather_Rain", FAudio_SoundEntry());
	SoundRegistry.Add("Weather_Storm", FAudio_SoundEntry());
	SoundRegistry.Add("Weather_Thunder", FAudio_SoundEntry());
	
	// Crafting sounds
	SoundRegistry.Add("Craft_StoneHit", FAudio_SoundEntry());
	SoundRegistry.Add("Craft_WoodChop", FAudio_SoundEntry());
	SoundRegistry.Add("Craft_FireStart", FAudio_SoundEntry());
	
	UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Initialized %d sound entries"), SoundRegistry.Num());
}

void UAudio_SoundManager::InitializeAmbienceLayers()
{
	AmbienceLayers.Empty();
	
	// Savanna ambience
	FAudio_AmbienceLayer SavannaLayer;
	SavannaLayer.BaseVolume = 0.6f;
	SavannaLayer.BiomeType = EBiomeType::Savanna;
	AmbienceLayers.Add(EBiomeType::Savanna, SavannaLayer);
	
	// Forest ambience
	FAudio_AmbienceLayer ForestLayer;
	ForestLayer.BaseVolume = 0.7f;
	ForestLayer.BiomeType = EBiomeType::Forest;
	AmbienceLayers.Add(EBiomeType::Forest, ForestLayer);
	
	// Swamp ambience
	FAudio_AmbienceLayer SwampLayer;
	SwampLayer.BaseVolume = 0.5f;
	SwampLayer.BiomeType = EBiomeType::Swamp;
	AmbienceLayers.Add(EBiomeType::Swamp, SwampLayer);
	
	// Desert ambience
	FAudio_AmbienceLayer DesertLayer;
	DesertLayer.BaseVolume = 0.4f;
	DesertLayer.BiomeType = EBiomeType::Desert;
	AmbienceLayers.Add(EBiomeType::Desert, DesertLayer);
	
	// Mountain ambience
	FAudio_AmbienceLayer MountainLayer;
	MountainLayer.BaseVolume = 0.5f;
	MountainLayer.BiomeType = EBiomeType::Mountain;
	AmbienceLayers.Add(EBiomeType::Mountain, MountainLayer);
	
	UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Initialized %d ambience layers"), AmbienceLayers.Num());
}

void UAudio_SoundManager::CleanupInactiveComponents()
{
	ActiveAudioComponents.RemoveAll([](UAudioComponent* Component)
	{
		return !IsValid(Component) || !Component->IsPlaying();
	});
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
	if (AudioComp)
	{
		AudioComp->bAutoActivate = false;
		AudioComp->bStopWhenOwnerDestroyed = false;
	}
	
	return AudioComp;
}

float UAudio_SoundManager::CalculateFinalVolume(float BaseVolume, float Multiplier) const
{
	return BaseVolume * Multiplier * MasterVolume * SFXVolume;
}