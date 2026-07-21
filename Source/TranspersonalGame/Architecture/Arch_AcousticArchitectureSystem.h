#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_AcousticArchitectureSystem.generated.h"

/**
 * Acoustic Architecture System - Manages sound propagation and acoustic properties of prehistoric structures
 * Handles reverb zones, echo chambers, and environmental audio integration
 * Based on scientifically accurate acoustic principles for Cretaceous period stone formations
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AcousticZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float ReverbIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float EchoDelay = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float SoundDampening = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float FrequencyResponse = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    FVector AcousticCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float EffectiveRadius = 1000.0f;

    FArch_AcousticZoneData()
    {
        ReverbIntensity = 0.5f;
        EchoDelay = 0.3f;
        SoundDampening = 0.2f;
        FrequencyResponse = 1.0f;
        AcousticCenter = FVector::ZeroVector;
        EffectiveRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralAcoustics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float WallThickness = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float CeilingHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float FloorArea = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    bool bHasOpenings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    int32 NumberOfOpenings = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Acoustics")
    float AcousticReflectivity = 0.8f;

    FArch_StructuralAcoustics()
    {
        MaterialType = EArch_MaterialType::Stone;
        WallThickness = 100.0f;
        CeilingHeight = 300.0f;
        FloorArea = 10000.0f;
        bHasOpenings = true;
        NumberOfOpenings = 2;
        AcousticReflectivity = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_AcousticArchitectureSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_AcousticArchitectureSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CORE COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* AcousticTriggerVolume;

    // === ACOUSTIC PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Architecture")
    FArch_AcousticZoneData AcousticZoneData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Architecture")
    FArch_StructuralAcoustics StructuralAcoustics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Architecture")
    TArray<FArch_AcousticZoneData> MultipleAcousticZones;

    // === SOUND ASSETS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* CaveReverbSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* StoneEchoSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* AmbientStructureSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TArray<USoundCue*> LayeredAmbientSounds;

    // === ACOUSTIC CALCULATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Calculation")
    float BaseReverbTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Calculation")
    float MaterialAbsorptionCoefficient = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Calculation")
    float DistanceAttenuationFactor = 0.8f;

public:
    // === ACOUSTIC SYSTEM METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Acoustic Architecture")
    void InitializeAcousticSystem();

    UFUNCTION(BlueprintCallable, Category = "Acoustic Architecture")
    void UpdateAcousticProperties(const FArch_AcousticZoneData& NewAcousticData);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Architecture")
    float CalculateReverbTime(float RoomVolume, float SurfaceArea, float AbsorptionCoefficient);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Architecture")
    void ApplyAcousticEffects(AActor* SourceActor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Acoustic Architecture")
    void CreateAcousticZone(const FVector& Center, float Radius, const FArch_AcousticZoneData& ZoneData);

    // === STRUCTURAL INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void IntegrateWithStructuralSystem(class AArch_StructuralManager* StructuralManager);

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void AnalyzeStructuralAcoustics();

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void OptimizeAcousticPerformance();

    // === ENVIRONMENTAL INTERACTION ===
    UFUNCTION(BlueprintCallable, Category = "Environmental Interaction")
    void HandleWeatherAcousticEffects(EWeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Interaction")
    void UpdateSeasonalAcoustics(ESeason CurrentSeason);

    // === TRIGGER EVENTS ===
    UFUNCTION()
    void OnAcousticTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                      bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAcousticTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    // === INTERNAL ACOUSTIC CALCULATIONS ===
    float CalculateDistanceAttenuation(float Distance) const;
    float CalculateFrequencyResponse(float Frequency, const FArch_StructuralAcoustics& Acoustics) const;
    void UpdateAmbientAudioLayers();
    void ProcessAcousticReflections(const FVector& SourceLocation, const FVector& ListenerLocation);

private:
    // === PERFORMANCE OPTIMIZATION ===
    float LastAcousticUpdateTime;
    bool bIsPlayerInAcousticZone;
    TArray<AActor*> ActiveAcousticSources;
    
    // === ACOUSTIC STATE TRACKING ===
    float CurrentReverbIntensity;
    float CurrentEchoDelay;
    bool bAcousticSystemInitialized;
};