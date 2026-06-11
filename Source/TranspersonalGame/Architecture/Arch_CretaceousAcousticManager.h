#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Arch_CretaceousAcousticManager.generated.h"

/**
 * Cretaceous Acoustic Manager
 * Manages environmental acoustics for prehistoric architectural structures
 * Handles reverb zones, sound occlusion, and atmospheric audio integration
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AcousticZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    float ReverbIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    float EchoDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    float SoundOcclusion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    float ZoneRadius;

    FArch_AcousticZone()
    {
        ZoneName = TEXT("DefaultZone");
        ReverbIntensity = 0.5f;
        EchoDelay = 0.2f;
        SoundOcclusion = 0.3f;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralAcoustics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float WallThickness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float MaterialDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float SoundAbsorption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float ResonanceFrequency;

    FArch_StructuralAcoustics()
    {
        StructureType = EArch_StructureType::Cave;
        WallThickness = 100.0f;
        MaterialDensity = 2.5f;
        SoundAbsorption = 0.4f;
        ResonanceFrequency = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousAcousticManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousAcousticManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zones")
    TArray<FArch_AcousticZone> AcousticZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    TArray<FArch_StructuralAcoustics> StructuralAcoustics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    USoundCue* CaveAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    USoundCue* ForestAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    USoundCue* RockFormationSound;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Acoustic Management")
    void InitializeAcousticZones();

    UFUNCTION(BlueprintCallable, Category = "Acoustic Management")
    void UpdatePlayerAcoustics(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Management")
    FArch_AcousticZone GetAcousticZoneAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Management")
    void SetStructuralAcoustics(EArch_StructureType StructureType, float Absorption);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Management")
    float CalculateSoundOcclusion(FVector SourceLocation, FVector ListenerLocation);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Management")
    void ApplyEnvironmentalReverb(float ReverbIntensity, float EchoDelay);

private:
    float CurrentReverbIntensity;
    float CurrentEchoDelay;
    FVector LastPlayerLocation;
    
    void UpdateAmbientSounds(const FArch_AcousticZone& CurrentZone);
    void CalculateStructuralResonance(const FArch_StructuralAcoustics& Acoustics);
};