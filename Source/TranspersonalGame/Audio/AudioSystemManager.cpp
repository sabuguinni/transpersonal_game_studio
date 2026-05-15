#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

UAudioSystemManager::UAudioSystemManager()
{
	CurrentBiome = EAudio_BiomeType::Forest;
	CurrentThreatLevel = EAudio_ThreatLevel::Safe;
	PlayerLocation = FVector::ZeroVector;
	MasterVolume = 1.0f;
	EnvironmentalAudioComponent = nullptr;
	ThreatAudioComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing audio subsystem"));
	
	// Initialize biome audio data with default values
	BiomeAudioMap.Empty();
	
	// Forest biome
	FAudio_BiomeAudioData ForestAudio;
	ForestAudio.BaseVolume = 0.8f;
	ForestAudio.FadeDistance = 2500.0f;
	BiomeAudioMap.Add(EAudio_BiomeType::Forest, ForestAudio);
	
	// Savanna biome
	FAudio_BiomeAudioData SavannaAudio;
	SavannaAudio.BaseVolume = 0.7f;
	SavannaAudio.FadeDistance = 3000.0f;
	BiomeAudioMap.Add(EAudio_BiomeType::Savanna, SavannaAudio);
	
	// Swamp biome
	FAudio_BiomeAudioData SwampAudio;
	SwampAudio.BaseVolume = 0.9f;
	SwampAudio.FadeDistance = 2000.0f;
	BiomeAudioMap.Add(EAudio_BiomeType::Swamp, SwampAudio);
	
	// Desert biome
	FAudio_BiomeAudioData DesertAudio;
	DesertAudio.BaseVolume = 0.6f;
	DesertAudio.FadeDistance = 4000.0f;
	BiomeAudioMap.Add(EAudio_BiomeType::Desert, DesertAudio);
	
	// Mountain biome
	FAudio_BiomeAudioData MountainAudio;
	MountainAudio.BaseVolume = 0.5f;
	MountainAudio.FadeDistance = 5000.0f;
	BiomeAudioMap.Add(EAudio_BiomeType::Mountain, MountainAudio);
	
	// Initialize threat audio map (will be populated with actual sound cues later)
	ThreatAudioMap.Empty();
	
	// Start audio update timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AudioUpdateTimerHandle,
			this,
			&UAudioSystemManager::UpdateEnvironmentalAudio,
			AudioUpdateInterval,
			true
		);
		
		World->GetTimerManager().SetTimer(
			ProximityUpdateTimerHandle,
			this,
			&UAudioSystemManager::UpdateProximityAudio,
			ProximityUpdateInterval,
			true
		);
	}
	
	UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialization complete"));
}

void UAudioSystemManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Deinitializing audio subsystem"));
	
	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AudioUpdateTimerHandle);
		World->GetTimerManager().ClearTimer(ProximityUpdateTimerHandle);
	}
	
	// Stop all audio
	StopEnvironmentalAudio();
	StopThreatAudio();
	
	// Clean up audio components
	if (EnvironmentalAudioComponent && IsValid(EnvironmentalAudioComponent))
	{
		EnvironmentalAudioComponent->Stop();
		EnvironmentalAudioComponent = nullptr;
	}
	
	if (ThreatAudioComponent && IsValid(ThreatAudioComponent))
	{
		ThreatAudioComponent->Stop();
		ThreatAudioComponent = nullptr;
	}
	
	// Clean up proximity audio components
	for (UAudioComponent* AudioComp : ProximityAudioComponents)
	{
		if (AudioComp && IsValid(AudioComp))
		{
			AudioComp->Stop();
		}
	}
	ProximityAudioComponents.Empty();
	
	Super::Deinitialize();
}

bool UAudioSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UAudioSystemManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
	if (CurrentBiome != NewBiome)
	{
		CurrentBiome = NewBiome;
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Biome changed to %d"), (int32)NewBiome);
		UpdateEnvironmentalAudio();
	}
}

void UAudioSystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
	if (CurrentThreatLevel != NewThreatLevel)
	{
		EAudio_ThreatLevel PreviousThreatLevel = CurrentThreatLevel;
		CurrentThreatLevel = NewThreatLevel;
		
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat level changed from %d to %d"), 
			(int32)PreviousThreatLevel, (int32)NewThreatLevel);
		
		// Handle threat audio changes
		if (NewThreatLevel == EAudio_ThreatLevel::Safe)
		{
			StopThreatAudio();
		}
		else
		{
			PlayThreatAudio(NewThreatLevel, PlayerLocation);
		}
	}
}

void UAudioSystemManager::PlayProximityAudio(const FVector& Location, const FAudio_ProximityAudioData& AudioData)
{
	if (!AudioData.ProximitySound)
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No proximity sound provided"));
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// Check if we're within trigger distance
	float Distance = FVector::Dist(PlayerLocation, Location);
	if (Distance > AudioData.TriggerDistance)
	{
		return;
	}
	
	// Create new audio component for proximity sound
	UAudioComponent* ProximityAudioComp = CreateAudioComponent(TEXT("ProximityAudio"));
	if (ProximityAudioComp)
	{
		ProximityAudioComp->SetSound(AudioData.ProximitySound);
		ProximityAudioComp->SetWorldLocation(Location);
		ProximityAudioComp->SetVolumeMultiplier(CalculateVolumeByDistance(Distance, AudioData.TriggerDistance) * AudioData.MaxVolume);
		
		if (AudioData.bLooping)
		{
			ProximityAudioComp->Play();
		}
		else
		{
			ProximityAudioComp->FadeIn(0.5f);
		}
		
		ProximityAudioComponents.Add(ProximityAudioComp);
		
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Started proximity audio at distance %.2f"), Distance);
	}
}

void UAudioSystemManager::StopProximityAudio(const FVector& Location)
{
	// Find and stop proximity audio components near this location
	for (int32 i = ProximityAudioComponents.Num() - 1; i >= 0; i--)
	{
		UAudioComponent* AudioComp = ProximityAudioComponents[i];
		if (AudioComp && IsValid(AudioComp))
		{
			float Distance = FVector::Dist(AudioComp->GetComponentLocation(), Location);
			if (Distance < 100.0f) // Close enough to be the same source
			{
				AudioComp->FadeOut(0.5f, 0.0f);
				ProximityAudioComponents.RemoveAt(i);
			}
		}
	}
}

void UAudioSystemManager::UpdatePlayerLocation(const FVector& NewPlayerLocation)
{
	PlayerLocation = NewPlayerLocation;
}

void UAudioSystemManager::StartEnvironmentalAudio()
{
	if (!EnvironmentalAudioComponent)
	{
		EnvironmentalAudioComponent = CreateAudioComponent(TEXT("EnvironmentalAudio"));
	}
	
	if (EnvironmentalAudioComponent)
	{
		UpdateEnvironmentalAudio();
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Environmental audio started"));
	}
}

void UAudioSystemManager::StopEnvironmentalAudio()
{
	if (EnvironmentalAudioComponent && IsValid(EnvironmentalAudioComponent))
	{
		EnvironmentalAudioComponent->FadeOut(2.0f, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Environmental audio stopped"));
	}
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	
	// Update all active audio components
	if (EnvironmentalAudioComponent && IsValid(EnvironmentalAudioComponent))
	{
		EnvironmentalAudioComponent->SetVolumeMultiplier(MasterVolume);
	}
	
	if (ThreatAudioComponent && IsValid(ThreatAudioComponent))
	{
		ThreatAudioComponent->SetVolumeMultiplier(MasterVolume);
	}
	
	for (UAudioComponent* AudioComp : ProximityAudioComponents)
	{
		if (AudioComp && IsValid(AudioComp))
		{
			AudioComp->SetVolumeMultiplier(MasterVolume);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Master volume set to %.2f"), MasterVolume);
}

float UAudioSystemManager::GetMasterVolume() const
{
	return MasterVolume;
}

void UAudioSystemManager::PlayThreatAudio(EAudio_ThreatLevel ThreatLevel, const FVector& ThreatLocation)
{
	USoundCue** ThreatSoundPtr = ThreatAudioMap.Find(ThreatLevel);
	if (!ThreatSoundPtr || !*ThreatSoundPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No threat audio found for level %d"), (int32)ThreatLevel);
		return;
	}
	
	if (!ThreatAudioComponent)
	{
		ThreatAudioComponent = CreateAudioComponent(TEXT("ThreatAudio"));
	}
	
	if (ThreatAudioComponent)
	{
		ThreatAudioComponent->SetSound(*ThreatSoundPtr);
		ThreatAudioComponent->SetWorldLocation(ThreatLocation);
		ThreatAudioComponent->SetVolumeMultiplier(MasterVolume);
		ThreatAudioComponent->Play();
		
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat audio started for level %d"), (int32)ThreatLevel);
	}
}

void UAudioSystemManager::StopThreatAudio()
{
	if (ThreatAudioComponent && IsValid(ThreatAudioComponent))
	{
		ThreatAudioComponent->FadeOut(1.0f, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat audio stopped"));
	}
}

void UAudioSystemManager::UpdateEnvironmentalAudio()
{
	if (!EnvironmentalAudioComponent || !IsValid(EnvironmentalAudioComponent))
	{
		return;
	}
	
	// Get current biome audio data
	FAudio_BiomeAudioData* BiomeAudio = BiomeAudioMap.Find(CurrentBiome);
	if (!BiomeAudio)
	{
		return;
	}
	
	// Update environmental audio based on current biome
	if (BiomeAudio->AmbientSound)
	{
		EnvironmentalAudioComponent->SetSound(BiomeAudio->AmbientSound);
	}
	else if (BiomeAudio->MetaSoundAmbient)
	{
		EnvironmentalAudioComponent->SetSound(BiomeAudio->MetaSoundAmbient);
	}
	
	// Set volume based on biome and master volume
	float FinalVolume = BiomeAudio->BaseVolume * MasterVolume;
	EnvironmentalAudioComponent->SetVolumeMultiplier(FinalVolume);
	
	// Ensure the audio is playing
	if (!EnvironmentalAudioComponent->IsPlaying())
	{
		EnvironmentalAudioComponent->Play();
	}
}

void UAudioSystemManager::UpdateProximityAudio()
{
	CleanupInactiveAudioComponents();
	
	// Update volume of proximity audio based on player distance
	for (UAudioComponent* AudioComp : ProximityAudioComponents)
	{
		if (AudioComp && IsValid(AudioComp))
		{
			float Distance = FVector::Dist(PlayerLocation, AudioComp->GetComponentLocation());
			float Volume = CalculateVolumeByDistance(Distance, 1000.0f) * MasterVolume;
			AudioComp->SetVolumeMultiplier(Volume);
			
			// Stop audio if too far away
			if (Distance > 1500.0f)
			{
				AudioComp->FadeOut(0.5f, 0.0f);
			}
		}
	}
}

void UAudioSystemManager::CleanupInactiveAudioComponents()
{
	// Remove invalid or stopped audio components
	for (int32 i = ProximityAudioComponents.Num() - 1; i >= 0; i--)
	{
		UAudioComponent* AudioComp = ProximityAudioComponents[i];
		if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
		{
			ProximityAudioComponents.RemoveAt(i);
		}
	}
}

float UAudioSystemManager::CalculateVolumeByDistance(float Distance, float MaxDistance) const
{
	if (Distance >= MaxDistance)
	{
		return 0.0f;
	}
	
	// Linear falloff
	return 1.0f - (Distance / MaxDistance);
}

void UAudioSystemManager::FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime)
{
	if (AudioComp && IsValid(AudioComp))
	{
		if (TargetVolume > 0.0f)
		{
			AudioComp->FadeIn(FadeTime, TargetVolume);
		}
		else
		{
			AudioComp->FadeOut(FadeTime, 0.0f);
		}
	}
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(const FString& ComponentName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	UAudioComponent* NewAudioComponent = NewObject<UAudioComponent>(World);
	if (NewAudioComponent)
	{
		NewAudioComponent->bAutoActivate = false;
		NewAudioComponent->bStopWhenOwnerDestroyed = false;
		NewAudioComponent->SetWorldLocation(PlayerLocation);
		
		UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Created audio component '%s'"), *ComponentName);
	}
	
	return NewAudioComponent;
}

void UAudioSystemManager::RemoveAudioComponent(UAudioComponent* ComponentToRemove)
{
	if (ComponentToRemove && IsValid(ComponentToRemove))
	{
		ComponentToRemove->Stop();
		ProximityAudioComponents.Remove(ComponentToRemove);
	}
}