#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
	Forest		UMETA(DisplayName = "Forest"),
	Savanna		UMETA(DisplayName = "Savanna"),
	Swamp		UMETA(DisplayName = "Swamp"),
	Desert		UMETA(DisplayName = "Desert"),
	Mountain	UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
	Safe		UMETA(DisplayName = "Safe"),
	Caution		UMETA(DisplayName = "Caution"),
	Danger		UMETA(DisplayName = "Danger"),
	Combat		UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundCue> AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<UMetaSoundSource> MetaSoundAmbient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float BaseVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float FadeDistance = 2000.0f;

	FAudio_BiomeAudioData()
	{
		AmbientSound = nullptr;
		MetaSoundAmbient = nullptr;
		BaseVolume = 0.7f;
		FadeDistance = 2000.0f;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityAudioData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundCue> ProximitySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float TriggerDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MaxVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bLooping = true;

	FAudio_ProximityAudioData()
	{
		ProximitySound = nullptr;
		TriggerDistance = 1000.0f;
		MaxVolume = 1.0f;
		bLooping = true;
	}
};

/**
 * Audio System Manager - Handles environmental audio, proximity-based sounds, and adaptive music
 * Integrates with MetaSound for real-time audio processing and biome-specific soundscapes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UAudioSystemManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Audio Management
	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void SetCurrentBiome(EAudio_BiomeType NewBiome);

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void PlayProximityAudio(const FVector& Location, const FAudio_ProximityAudioData& AudioData);

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void StopProximityAudio(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void UpdatePlayerLocation(const FVector& PlayerLocation);

	// Environmental Audio
	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void StartEnvironmentalAudio();

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void StopEnvironmentalAudio();

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	float GetMasterVolume() const;

	// Threat-based Audio
	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void PlayThreatAudio(EAudio_ThreatLevel ThreatLevel, const FVector& ThreatLocation);

	UFUNCTION(BlueprintCallable, Category = "Audio System")
	void StopThreatAudio();

protected:
	// Current audio state
	UPROPERTY(BlueprintReadOnly, Category = "Audio State")
	EAudio_BiomeType CurrentBiome;

	UPROPERTY(BlueprintReadOnly, Category = "Audio State")
	EAudio_ThreatLevel CurrentThreatLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Audio State")
	FVector PlayerLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Audio State")
	float MasterVolume;

	// Audio components
	UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
	TObjectPtr<UAudioComponent> EnvironmentalAudioComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
	TObjectPtr<UAudioComponent> ThreatAudioComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
	TArray<TObjectPtr<UAudioComponent>> ProximityAudioComponents;

	// Audio data maps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data", meta = (AllowPrivateAccess = "true"))
	TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data", meta = (AllowPrivateAccess = "true"))
	TMap<EAudio_ThreatLevel, TObjectPtr<USoundCue>> ThreatAudioMap;

private:
	// Internal audio management
	void UpdateEnvironmentalAudio();
	void UpdateProximityAudio();
	void CleanupInactiveAudioComponents();
	
	// Audio calculation helpers
	float CalculateVolumeByDistance(float Distance, float MaxDistance) const;
	void FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime = 1.0f);

	// Component management
	UAudioComponent* CreateAudioComponent(const FString& ComponentName);
	void RemoveAudioComponent(UAudioComponent* ComponentToRemove);

	// Timer handles for audio updates
	FTimerHandle AudioUpdateTimerHandle;
	FTimerHandle ProximityUpdateTimerHandle;

	// Audio update frequency (in seconds)
	static constexpr float AudioUpdateInterval = 0.1f;
	static constexpr float ProximityUpdateInterval = 0.2f;
};