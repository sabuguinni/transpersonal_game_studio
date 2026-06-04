#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Audio_EffectSystem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EffectSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EffectSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* EffectAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundCue* ForestAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundCue* TRexFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundCue* DinosaurRoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectVolumeMultiplier;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayTRexFootstep(FVector Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayDinosaurRoar(FVector Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StartForestAmbient();

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StopForestAmbient();

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetAmbientVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlaySpatialEffect(class USoundCue* SoundToPlay, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

private:
    void InitializeAudioComponents();
};