#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "DinosaurAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSoundType : uint8
{
    Idle,
    Walking,
    Running,
    Eating,
    Drinking,
    Calling,
    Aggressive,
    Pain,
    Death,
    Breathing,
    Sniffing,
    Territorial
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Triceratops,
    TRex,
    Velociraptor,
    Brachiosaurus,
    Stegosaurus,
    Parasaurolophus,
    Compsognathus,
    Pteranodon
};

USTRUCT(BlueprintType)
struct FDinosaurSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSpatialize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanInterrupt = false;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(EDinosaurSoundType SoundType, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopDinosaurSound(EDinosaurSoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurSpecies(EDinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetEmotionalState(float Aggression, float Fear, float Alertness);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetPhysicalState(float Health, float Energy, float Size);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    bool IsPlayingSound(EDinosaurSoundType SoundType) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EDinosaurSpecies DinosaurSpecies = EDinosaurSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EDinosaurSoundType, FDinosaurSoundData> SoundDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundAttenuation* DinosaurAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float GlobalPitchMultiplier = 1.0f;

    // Emotional state affects sound characteristics
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    float Aggression = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    float Alertness = 0.0f;

    // Physical state affects sound characteristics
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    float Health = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    float Energy = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    float Size = 1.0f;

private:
    UPROPERTY()
    TMap<EDinosaurSoundType, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<EDinosaurSoundType, float> SoundCooldowns;

    void LoadSoundDatabase();
    void UpdateSoundParameters(UAudioComponent* AudioComponent, EDinosaurSoundType SoundType);
    float CalculateVolumeModifier(EDinosaurSoundType SoundType) const;
    float CalculatePitchModifier(EDinosaurSoundType SoundType) const;
    
    UFUNCTION()
    void OnAudioFinished(UAudioComponent* AudioComponent);
};