#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "SharedTypes.h"
#include "Audio_AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
	None = 0,
	Swamp = 1,
	Forest = 2,
	Savanna = 3,
	Desert = 4,
	Mountain = 5
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
	Safe = 0,
	Low = 1,
	Medium = 2,
	High = 3,
	Critical = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundCue> AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float BaseVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float AttenuationRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bUse3DAttenuation = true;

	FAudio_BiomeAudioSettings()
	{
		BaseVolume = 0.7f;
		AttenuationRadius = 5000.0f;
		bUse3DAttenuation = true;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudioData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundCue> ThreatSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float TriggerDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bLooping = false;

	FAudio_ThreatAudioData()
	{
		TriggerDistance = 1000.0f;
		Volume = 1.0f;
		bLooping = false;
	}
};

/**
 * Sistema de áudio adaptativo que gere música e efeitos sonoros baseados no bioma,
 * proximidade de dinossauros, e estado de ameaça do jogador.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAudio_AudioManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Biome Audio Management
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCurrentBiome(EAudio_BiomeType NewBiome);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void UpdateBiomeAudio(const FVector& PlayerLocation);

	// Threat Audio System
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayThreatAudio(EAudio_ThreatLevel ThreatLevel, const FVector& ThreatLocation);

	// Dinosaur Proximity Audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void RegisterDinosaurActor(AActor* DinosaurActor, const FString& DinosaurType);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void UpdateDinosaurProximityAudio(const FVector& PlayerLocation);

	// Master Audio Controls
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAmbientVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetEffectsVolume(float Volume);

	// Audio Component Management
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* PlaySoundAtLocation(USoundCue* Sound, const FVector& Location, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAllAmbientSounds();

protected:
	// Current audio state
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	EAudio_BiomeType CurrentBiome;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	EAudio_ThreatLevel CurrentThreatLevel;

	// Audio settings per biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TMap<EAudio_BiomeType, FAudio_BiomeAudioSettings> BiomeAudioSettings;

	// Threat audio settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TMap<EAudio_ThreatLevel, FAudio_ThreatAudioData> ThreatAudioSettings;

	// Volume controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AmbientVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EffectsVolume;

	// Active audio components
	UPROPERTY()
	TArray<UAudioComponent*> ActiveAmbientComponents;

	UPROPERTY()
	TArray<UAudioComponent*> ActiveEffectComponents;

	// Registered dinosaurs for proximity audio
	UPROPERTY()
	TMap<AActor*, FString> RegisteredDinosaurs;

private:
	// Internal methods
	void InitializeBiomeAudioSettings();
	void InitializeThreatAudioSettings();
	EAudio_BiomeType DetermineBiomeFromLocation(const FVector& Location);
	void TransitionBiomeAudio(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome);
	void CleanupInactiveAudioComponents();
};