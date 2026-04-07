#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "RealityShiftSystem.generated.h"

UENUM(BlueprintType)
enum class ERealityLayer : uint8
{
    Physical        UMETA(DisplayName = "Physical Reality"),
    Etheric         UMETA(DisplayName = "Etheric Layer"),
    Astral          UMETA(DisplayName = "Astral Plane"),
    Mental          UMETA(DisplayName = "Mental Plane"),
    Causal          UMETA(DisplayName = "Causal Plane"),
    Unity           UMETA(DisplayName = "Unity Field")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRealityLayerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    ERealityLayer Layer = ERealityLayer::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    float Visibility = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    float FrequencyRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    FLinearColor EnergyColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    TObjectPtr<UMaterialInterface> LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    TObjectPtr<USoundCue> LayerAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
    TObjectPtr<UParticleSystem> LayerParticleEffect;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRealityShiftParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    UCurveFloat* TransitionCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    bool bAffectLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    bool bAffectPostProcess = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    bool bAffectAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    bool bAffectPhysics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shift")
    float IntensityMultiplier = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URealityShiftComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    URealityShiftComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Layers")
    TArray<FRealityLayerData> RealityLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ERealityLayer CurrentLayer = ERealityLayer::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ERealityLayer TargetLayer = ERealityLayer::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FRealityShiftParameters ShiftParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialParameterCollection> GlobalMaterialParameters;

    // Timer handle for transitions
    FTimerHandle TransitionTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Reality Shift")
    void InitiateRealityShift(ERealityLayer NewLayer, const FRealityShiftParameters& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Reality Shift")
    void SetRealityLayer(ERealityLayer Layer, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Reality Shift")
    ERealityLayer GetCurrentRealityLayer() const { return CurrentLayer; }

    UFUNCTION(BlueprintCallable, Category = "Reality Shift")
    float GetLayerVisibility(ERealityLayer Layer) const;

    UFUNCTION(BlueprintCallable, Category = "Reality Shift")
    void ModifyLayerProperty(ERealityLayer Layer, float Visibility, float Density, float Frequency);

    UFUNCTION(BlueprintCallable, Category = "Reality Shift")
    bool IsTransitioning() const { return bIsTransitioning; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Reality Shift")
    void OnRealityShiftStarted(ERealityLayer FromLayer, ERealityLayer ToLayer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Reality Shift")
    void OnRealityShiftCompleted(ERealityLayer NewLayer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Reality Shift")
    void OnRealityLayerChanged(ERealityLayer OldLayer, ERealityLayer NewLayer, float Progress);

protected:
    UFUNCTION()
    void UpdateTransition();

    UFUNCTION()
    void CompleteTransition();

    void ApplyLayerEffects(const FRealityLayerData& LayerData, float Intensity);
    void UpdatePostProcessEffects(float Progress);
    void UpdateMaterialParameters(float Progress);
    void UpdateAudioEffects(const FRealityLayerData& LayerData, float Intensity);

    FRealityLayerData* GetLayerData(ERealityLayer Layer);
    const FRealityLayerData* GetLayerData(ERealityLayer Layer) const;
};

UCLASS()
class TRANSPERSONALGAME_API ARealityShiftManager : public AActor
{
    GENERATED_BODY()

public:
    ARealityShiftManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Reality")
    ERealityLayer GlobalRealityLayer = ERealityLayer::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Reality")
    float GlobalShiftIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TArray<URealityShiftComponent*> ManagedComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Integration")
    float ConsciousnessThresholds[6] = {0.0f, 20.0f, 40.0f, 60.0f, 80.0f, 95.0f};

public:
    UFUNCTION(BlueprintCallable, Category = "Reality Management")
    void RegisterRealityShiftComponent(URealityShiftComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Reality Management")
    void SetGlobalRealityLayer(ERealityLayer NewLayer, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Reality Management")
    void UpdateRealityBasedOnConsciousness(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Reality Management")
    ERealityLayer DetermineLayerFromConsciousness(float ConsciousnessLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Reality Management")
    void TriggerGlobalRealityWave(ERealityLayer WaveLayer, float Duration, float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Reality Management")
    void OnGlobalRealityChanged(ERealityLayer OldLayer, ERealityLayer NewLayer);
};