#include "Audio_AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_AudioManager::UAudio_AudioManager()
{
	CurrentBiome = EAudio_BiomeType::None;
	CurrentThreatLevel = EAudio_ThreatLevel::Safe;
	MasterVolume = 1.0f;
	AmbientVolume = 0.7f;
	EffectsVolume = 0.8f;
}

void UAudio_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Sistema de áudio inicializado"));
	
	InitializeBiomeAudioSettings();
	InitializeThreatAudioSettings();
}

void UAudio_AudioManager::Deinitialize()
{
	StopAllAmbientSounds();
	
	ActiveAmbientComponents.Empty();
	ActiveEffectComponents.Empty();
	RegisteredDinosaurs.Empty();
	
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Sistema de áudio desinicializado"));
}

void UAudio_AudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
	if (CurrentBiome != NewBiome)
	{
		EAudio_BiomeType OldBiome = CurrentBiome;
		CurrentBiome = NewBiome;
		
		UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Transição de bioma de %d para %d"), 
			   (int32)OldBiome, (int32)NewBiome);
		
		TransitionBiomeAudio(OldBiome, NewBiome);
	}
}

void UAudio_AudioManager::UpdateBiomeAudio(const FVector& PlayerLocation)
{
	EAudio_BiomeType DetectedBiome = DetermineBiomeFromLocation(PlayerLocation);
	SetCurrentBiome(DetectedBiome);
}

void UAudio_AudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
	if (CurrentThreatLevel != NewThreatLevel)
	{
		EAudio_ThreatLevel OldThreatLevel = CurrentThreatLevel;
		CurrentThreatLevel = NewThreatLevel;
		
		UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Nível de ameaça mudou de %d para %d"), 
			   (int32)OldThreatLevel, (int32)NewThreatLevel);
		
		// Tocar áudio de ameaça se necessário
		if (NewThreatLevel > EAudio_ThreatLevel::Safe)
		{
			PlayThreatAudio(NewThreatLevel, FVector::ZeroVector);
		}
	}
}

void UAudio_AudioManager::PlayThreatAudio(EAudio_ThreatLevel ThreatLevel, const FVector& ThreatLocation)
{
	if (ThreatAudioSettings.Contains(ThreatLevel))
	{
		const FAudio_ThreatAudioData& ThreatData = ThreatAudioSettings[ThreatLevel];
		
		if (ThreatData.ThreatSound.IsValid())
		{
			USoundCue* SoundToPlay = ThreatData.ThreatSound.LoadSynchronous();
			if (SoundToPlay)
			{
				UAudioComponent* AudioComp = PlaySoundAtLocation(
					SoundToPlay, 
					ThreatLocation, 
					ThreatData.Volume * EffectsVolume * MasterVolume
				);
				
				if (AudioComp && ThreatData.bLooping)
				{
					ActiveEffectComponents.Add(AudioComp);
				}
				
				UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Áudio de ameaça reproduzido (nível %d)"), 
					   (int32)ThreatLevel);
			}
		}
	}
}

void UAudio_AudioManager::RegisterDinosaurActor(AActor* DinosaurActor, const FString& DinosaurType)
{
	if (DinosaurActor)
	{
		RegisteredDinosaurs.Add(DinosaurActor, DinosaurType);
		UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Dinossauro registado - %s (%s)"), 
			   *DinosaurActor->GetName(), *DinosaurType);
	}
}

void UAudio_AudioManager::UpdateDinosaurProximityAudio(const FVector& PlayerLocation)
{
	// Limpar dinossauros inválidos
	TArray<AActor*> InvalidActors;
	for (auto& DinosaurPair : RegisteredDinosaurs)
	{
		if (!IsValid(DinosaurPair.Key))
		{
			InvalidActors.Add(DinosaurPair.Key);
		}
	}
	
	for (AActor* InvalidActor : InvalidActors)
	{
		RegisteredDinosaurs.Remove(InvalidActor);
	}
	
	// Verificar proximidade dos dinossauros válidos
	EAudio_ThreatLevel HighestThreat = EAudio_ThreatLevel::Safe;
	
	for (const auto& DinosaurPair : RegisteredDinosaurs)
	{
		AActor* DinosaurActor = DinosaurPair.Key;
		const FString& DinosaurType = DinosaurPair.Value;
		
		if (IsValid(DinosaurActor))
		{
			float Distance = FVector::Dist(PlayerLocation, DinosaurActor->GetActorLocation());
			
			// Determinar nível de ameaça baseado na distância e tipo de dinossauro
			EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;
			
			if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Tyrannosaurus")))
			{
				if (Distance < 500.0f) ThreatLevel = EAudio_ThreatLevel::Critical;
				else if (Distance < 1000.0f) ThreatLevel = EAudio_ThreatLevel::High;
				else if (Distance < 2000.0f) ThreatLevel = EAudio_ThreatLevel::Medium;
			}
			else if (DinosaurType.Contains(TEXT("Raptor")))
			{
				if (Distance < 300.0f) ThreatLevel = EAudio_ThreatLevel::High;
				else if (Distance < 800.0f) ThreatLevel = EAudio_ThreatLevel::Medium;
				else if (Distance < 1500.0f) ThreatLevel = EAudio_ThreatLevel::Low;
			}
			else // Herbívoros
			{
				if (Distance < 200.0f) ThreatLevel = EAudio_ThreatLevel::Medium;
				else if (Distance < 500.0f) ThreatLevel = EAudio_ThreatLevel::Low;
			}
			
			if (ThreatLevel > HighestThreat)
			{
				HighestThreat = ThreatLevel;
			}
		}
	}
	
	SetThreatLevel(HighestThreat);
}

void UAudio_AudioManager::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	
	// Actualizar volume de todos os componentes activos
	for (UAudioComponent* AudioComp : ActiveAmbientComponents)
	{
		if (IsValid(AudioComp))
		{
			AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * MasterVolume);
		}
	}
	
	for (UAudioComponent* AudioComp : ActiveEffectComponents)
	{
		if (IsValid(AudioComp))
		{
			AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * MasterVolume);
		}
	}
}

void UAudio_AudioManager::SetAmbientVolume(float Volume)
{
	AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	
	// Actualizar volume dos componentes ambientais
	for (UAudioComponent* AudioComp : ActiveAmbientComponents)
	{
		if (IsValid(AudioComp))
		{
			AudioComp->SetVolumeMultiplier(AmbientVolume * MasterVolume);
		}
	}
}

void UAudio_AudioManager::SetEffectsVolume(float Volume)
{
	EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	
	// Actualizar volume dos componentes de efeitos
	for (UAudioComponent* AudioComp : ActiveEffectComponents)
	{
		if (IsValid(AudioComp))
		{
			AudioComp->SetVolumeMultiplier(EffectsVolume * MasterVolume);
		}
	}
}

UAudioComponent* UAudio_AudioManager::PlaySoundAtLocation(USoundCue* Sound, const FVector& Location, float Volume)
{
	if (!Sound)
	{
		return nullptr;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
		World, 
		Sound, 
		Location, 
		FRotator::ZeroRotator, 
		Volume
	);
	
	return AudioComp;
}

void UAudio_AudioManager::StopAllAmbientSounds()
{
	for (UAudioComponent* AudioComp : ActiveAmbientComponents)
	{
		if (IsValid(AudioComp))
		{
			AudioComp->Stop();
		}
	}
	
	ActiveAmbientComponents.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Todos os sons ambientais parados"));
}

void UAudio_AudioManager::InitializeBiomeAudioSettings()
{
	// Configurações de áudio por bioma
	FAudio_BiomeAudioSettings SwampSettings;
	SwampSettings.BaseVolume = 0.6f;
	SwampSettings.AttenuationRadius = 8000.0f;
	BiomeAudioSettings.Add(EAudio_BiomeType::Swamp, SwampSettings);
	
	FAudio_BiomeAudioSettings ForestSettings;
	ForestSettings.BaseVolume = 0.7f;
	ForestSettings.AttenuationRadius = 6000.0f;
	BiomeAudioSettings.Add(EAudio_BiomeType::Forest, ForestSettings);
	
	FAudio_BiomeAudioSettings SavannaSettings;
	SavannaSettings.BaseVolume = 0.8f;
	SavannaSettings.AttenuationRadius = 10000.0f;
	BiomeAudioSettings.Add(EAudio_BiomeType::Savanna, SavannaSettings);
	
	FAudio_BiomeAudioSettings DesertSettings;
	DesertSettings.BaseVolume = 0.5f;
	DesertSettings.AttenuationRadius = 12000.0f;
	BiomeAudioSettings.Add(EAudio_BiomeType::Desert, DesertSettings);
	
	FAudio_BiomeAudioSettings MountainSettings;
	MountainSettings.BaseVolume = 0.4f;
	MountainSettings.AttenuationRadius = 15000.0f;
	BiomeAudioSettings.Add(EAudio_BiomeType::Mountain, MountainSettings);
	
	UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Configurações de áudio por bioma inicializadas"));
}

void UAudio_AudioManager::InitializeThreatAudioSettings()
{
	// Configurações de áudio por nível de ameaça
	FAudio_ThreatAudioData SafeData;
	SafeData.Volume = 0.0f;
	ThreatAudioSettings.Add(EAudio_ThreatLevel::Safe, SafeData);
	
	FAudio_ThreatAudioData LowData;
	LowData.Volume = 0.3f;
	LowData.TriggerDistance = 1500.0f;
	ThreatAudioSettings.Add(EAudio_ThreatLevel::Low, LowData);
	
	FAudio_ThreatAudioData MediumData;
	MediumData.Volume = 0.6f;
	MediumData.TriggerDistance = 1000.0f;
	ThreatAudioSettings.Add(EAudio_ThreatLevel::Medium, MediumData);
	
	FAudio_ThreatAudioData HighData;
	HighData.Volume = 0.8f;
	HighData.TriggerDistance = 500.0f;
	ThreatAudioSettings.Add(EAudio_ThreatLevel::High, HighData);
	
	FAudio_ThreatAudioData CriticalData;
	CriticalData.Volume = 1.0f;
	CriticalData.TriggerDistance = 200.0f;
	CriticalData.bLooping = true;
	ThreatAudioSettings.Add(EAudio_ThreatLevel::Critical, CriticalData);
	
	UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Configurações de áudio de ameaça inicializadas"));
}

EAudio_BiomeType UAudio_AudioManager::DetermineBiomeFromLocation(const FVector& Location)
{
	// Coordenadas dos biomas (baseado no brain memory)
	float X = Location.X;
	float Y = Location.Y;
	
	// Pantano (sudoeste)
	if (X >= -77500.0f && X <= -25000.0f && Y >= -76500.0f && Y <= -15000.0f)
	{
		return EAudio_BiomeType::Swamp;
	}
	
	// Floresta (noroeste)
	if (X >= -77500.0f && X <= -15000.0f && Y >= 15000.0f && Y <= 76500.0f)
	{
		return EAudio_BiomeType::Forest;
	}
	
	// Savana (centro)
	if (X >= -20000.0f && X <= 20000.0f && Y >= -20000.0f && Y <= 20000.0f)
	{
		return EAudio_BiomeType::Savanna;
	}
	
	// Deserto (leste)
	if (X >= 25000.0f && X <= 79500.0f && Y >= -30000.0f && Y <= 30000.0f)
	{
		return EAudio_BiomeType::Desert;
	}
	
	// Montanha Nevada (nordeste)
	if (X >= 15000.0f && X <= 79500.0f && Y >= 20000.0f && Y <= 76500.0f)
	{
		return EAudio_BiomeType::Mountain;
	}
	
	return EAudio_BiomeType::None;
}

void UAudio_AudioManager::TransitionBiomeAudio(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome)
{
	// Parar áudio do bioma anterior
	StopAllAmbientSounds();
	
	// Iniciar áudio do novo bioma
	if (BiomeAudioSettings.Contains(ToBiome))
	{
		const FAudio_BiomeAudioSettings& NewBiomeSettings = BiomeAudioSettings[ToBiome];
		
		// Aqui seria onde carregaríamos e reproduziríamos o som ambiental do novo bioma
		// Por agora, apenas logamos a transição
		UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Transição de áudio de bioma %d para %d"), 
			   (int32)FromBiome, (int32)ToBiome);
	}
}

void UAudio_AudioManager::CleanupInactiveAudioComponents()
{
	// Remover componentes de áudio inválidos ou parados
	ActiveAmbientComponents.RemoveAll([](UAudioComponent* AudioComp) {
		return !IsValid(AudioComp) || !AudioComp->IsPlaying();
	});
	
	ActiveEffectComponents.RemoveAll([](UAudioComponent* AudioComp) {
		return !IsValid(AudioComp) || !AudioComp->IsPlaying();
	});
}