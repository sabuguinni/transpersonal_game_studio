#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "AcousticArchitecture.generated.h"

// Acoustic properties for different architectural materials
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AcousticProperties
{
    GENERATED_BODY()

    // Reverb characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float ReverbDecayTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float ReverbWetness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float ReverbDryness = 0.7f;

    // Sound absorption coefficient (0.0 = reflective, 1.0 = absorptive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float AbsorptionCoefficient = 0.2f;

    // Frequency response characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float LowFrequencyGain = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float MidFrequencyGain = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float HighFrequencyGain = 0.8f;

    FArch_AcousticProperties()
    {
        ReverbDecayTime = 2.0f;
        ReverbWetness = 0.3f;
        ReverbDryness = 0.7f;
        AbsorptionCoefficient = 0.2f;
        LowFrequencyGain = 1.0f;
        MidFrequencyGain = 1.0f;
        HighFrequencyGain = 0.8f;
    }
};

// Types of acoustic environments
UENUM(BlueprintType)
enum class EArch_AcousticEnvironment : uint8
{
    OpenAir         UMETA(DisplayName = "Open Air"),
    Cave            UMETA(DisplayName = "Cave"),
    Forest          UMETA(DisplayName = "Forest"),
    StoneStructure  UMETA(DisplayName = "Stone Structure"),
    WoodStructure   UMETA(DisplayName = "Wood Structure"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Underwater      UMETA(DisplayName = "Underwater")
};

/**
 * Acoustic Architecture Actor - Creates environmental audio zones with realistic sound propagation
 * Handles reverb, echo, and environmental audio effects for prehistoric structures
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAcousticArchitecture : public AActor
{
    GENERATED_BODY()

public:
    AAcousticArchitecture();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* AcousticTriggerVolume;

    // Acoustic configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    EArch_AcousticEnvironment EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    FArch_AcousticProperties AcousticProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    class USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    class USoundBase* ReverbImpulseResponse;

    // Environmental audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EchoStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableDistanceAttenuation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxAudibleDistance = 2000.0f;

public:
    // Acoustic system interface
    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    void SetAcousticEnvironment(EArch_AcousticEnvironment NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    void UpdateAcousticProperties(const FArch_AcousticProperties& NewProperties);

    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    void PlayPositionalSound(class USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    float CalculateReverbForDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetWindIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetEchoStrength(float NewStrength);

protected:
    // Internal acoustic processing
    UFUNCTION()
    void OnAcousticTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAcousticTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyAcousticEffects();
    void UpdateEnvironmentalAudio();
    FArch_AcousticProperties GetDefaultPropertiesForEnvironment(EArch_AcousticEnvironment Environment) const;

private:
    // Runtime state
    TArray<AActor*> ActorsInAcousticZone;
    float CurrentReverbLevel;
    float CurrentEchoLevel;
    bool bIsPlayerInZone;
    
    // Audio processing
    void ProcessAudioForActor(AActor* Actor);
    void RemoveAudioEffectsFromActor(AActor* Actor);
};