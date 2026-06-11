#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Audio_SoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeDistance;

    FAudio_SoundZone()
    {
        ZoneName = TEXT("DefaultZone");
        AmbientSound = nullptr;
        VolumeMultiplier = 1.0f;
        FadeDistance = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxHearingDistance;

    FAudio_DinosaurSoundSet()
    {
        IdleSound = nullptr;
        AlertSound = nullptr;
        AttackSound = nullptr;
        FootstepSound = nullptr;
        MaxHearingDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_SoundZone> SoundZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EBiomeType, USoundCue*> BiomeAmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EDinosaurSpecies, FAudio_DinosaurSoundSet> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    USoundCue* PlayerFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    USoundCue* CraftingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    USoundCue* DangerStinger;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FVector& Location, bool bIsPlayer = true);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayCraftingSound(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDangerStinger();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiomeAmbient(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdatePlayerPosition(const FVector& PlayerLocation);

private:
    FVector LastPlayerPosition;
    EBiomeType CurrentBiome;
    
    void UpdateAmbientAudio();
    void CheckSoundZones(const FVector& PlayerLocation);
    float CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance);
};