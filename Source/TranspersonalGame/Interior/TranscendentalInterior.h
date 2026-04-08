// TranscendentalInterior.h
// Sistema de interiores transcendentais para espaços sagrados

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "TranscendentalInterior.generated.h"

UENUM(BlueprintType)
enum class EInteriorMoodType : uint8
{
    Meditative     UMETA(DisplayName = "Meditative"),
    Mystical       UMETA(DisplayName = "Mystical"),
    Ethereal       UMETA(DisplayName = "Ethereal"),
    Cosmic         UMETA(DisplayName = "Cosmic"),
    Grounding      UMETA(DisplayName = "Grounding"),
    Transformative UMETA(DisplayName = "Transformative"),
    Healing        UMETA(DisplayName = "Healing"),
    Awakening      UMETA(DisplayName = "Awakening")
};

USTRUCT(BlueprintType)
struct FInteriorAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarmthFactor = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyFrequency = 432.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SacredResonance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDynamicLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphericDensity = 0.7f;
};

USTRUCT(BlueprintType)
struct FSacredElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* ElementMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* SacredMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyEmission = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInteractive = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranscendentalInterior : public AActor
{
    GENERATED_BODY()

public:
    ATranscendentalInterior();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CeilingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UStaticMeshComponent*> WallMeshes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UPointLightComponent*> SacredLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    ATriggerVolume* InteriorTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    EInteriorMoodType MoodType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    FInteriorAmbience AmbienceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Elements")
    TArray<FSacredElement> SacredElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* FloorMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WallMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* CeilingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* MeditativeAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* SacredChanting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CosmicResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* EnergyParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* LightOrbs;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetMoodType(EInteriorMoodType NewMood);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void UpdateAmbience(const FInteriorAmbience& NewAmbience);

    UFUNCTION(BlueprintCallable, Category = "Sacred Elements")
    void AddSacredElement(const FSacredElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Sacred Elements")
    void RemoveSacredElement(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ActivateSacredLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void DeactivateSacredLighting();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySacredAudio(USoundBase* SoundToPlay);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerTranscendentalEffect();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnMoodChanged(EInteriorMoodType NewMood);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnPlayerEntered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnPlayerExited();

    UFUNCTION()
    void OnTriggerEntered(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnTriggerExited(AActor* OverlappedActor, AActor* OtherActor);

private:
    void SetupInteriorGeometry();
    void ConfigureMoodLighting();
    void PlaceSacredElements();
    void InitializeAmbientSystems();
    void UpdateDynamicLighting(float DeltaTime);
    void UpdateEnergyFlow(float DeltaTime);

    void ApplyMeditativeMood();
    void ApplyMysticalMood();
    void ApplyEtherealMood();
    void ApplyCosmicMood();
    void ApplyGroundingMood();
    void ApplyTransformativeMood();
    void ApplyHealingMood();
    void ApplyAwakeningMood();

    float TimeAccumulator;
    float EnergyFlowCycle;
    bool bSacredLightingActive;
    bool bPlayerPresent;

    TArray<UStaticMeshComponent*> SacredElementMeshes;
};