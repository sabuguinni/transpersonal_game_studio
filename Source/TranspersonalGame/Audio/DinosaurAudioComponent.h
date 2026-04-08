#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "DinosaurAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSoundType : uint8
{
    Idle,
    Movement,
    Feeding,
    Alert,
    Aggressive,
    Pain,
    Death,
    Territorial,
    Mating,
    Social
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex,
    Triceratops,
    Velociraptor,
    Brachiosaurus,
    Stegosaurus,
    Parasaurolophus,
    Compsognathus,
    Pteranodon
};

USTRUCT(BlueprintType)
struct FDinosaurSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EDinosaurSoundType, USoundCue*> SoundCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxAudibleDistance = 5000.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurVocalBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IdleCallFrequency = 30.0f; // seconds between idle calls

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialCallRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialCallRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanMakeDistantCalls = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchVariation = 0.15f;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FDinosaurSoundSet SoundSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FDinosaurVocalBehavior VocalBehavior;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SecondaryAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EDinosaurSoundType LastPlayedSoundType;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TimeSinceLastCall;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsVocalizing;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlaySound(EDinosaurSoundType SoundType, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayIdleCall();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayTerritorialCall();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlaySocialCall();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetSpecies(EDinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    float GetMaxAudibleDistance() const { return SoundSet.MaxAudibleDistance; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    bool IsAudibleToPlayer() const;

private:
    void ProcessIdleBehavior(float DeltaTime);
    void ProcessTerritorialBehavior();
    void ProcessSocialBehavior();
    
    float GetRandomizedVolume() const;
    float GetRandomizedPitch() const;
    
    UPROPERTY()
    class AAudioManager* AudioManager;
    
    FTimerHandle IdleCallTimer;
};