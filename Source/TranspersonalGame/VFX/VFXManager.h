#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Meditative      UMETA(DisplayName = "Meditative State"),
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Cosmic          UMETA(DisplayName = "Cosmic Consciousness")
};

UENUM(BlueprintType)
enum class EChakraType : uint8
{
    Root            UMETA(DisplayName = "Root Chakra"),
    Sacral          UMETA(DisplayName = "Sacral Chakra"),
    SolarPlexus     UMETA(DisplayName = "Solar Plexus Chakra"),
    Heart           UMETA(DisplayName = "Heart Chakra"),
    Throat          UMETA(DisplayName = "Throat Chakra"),
    ThirdEye        UMETA(DisplayName = "Third Eye Chakra"),
    Crown           UMETA(DisplayName = "Crown Chakra")
};

USTRUCT(BlueprintType)
struct FEnergyFieldData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Frequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPulsating = true;
};

UCLASS()
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Consciousness State VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Consciousness")
    void SetConsciousnessState(EConsciousnessState NewState, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Consciousness")
    void TriggerTranscendentMoment(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Consciousness")
    void CreateUnityField(FVector Location, float Radius = 500.0f);

    // Chakra System VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Chakras")
    void ActivateChakra(EChakraType ChakraType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Chakras")
    void AlignAllChakras(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Chakras")
    void CreateChakraBeam(EChakraType FromChakra, EChakraType ToChakra);

    // Energy Field VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Energy")
    void CreateEnergyField(FVector Location, const FEnergyFieldData& FieldData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Energy")
    void SpawnEnergyOrb(FVector Location, FLinearColor Color, float Size = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Energy")
    void CreateEnergyFlow(FVector StartLocation, FVector EndLocation, FLinearColor Color);

    // Meditation VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Meditation")
    void StartMeditationAura(AActor* MeditatingActor);

    UFUNCTION(BlueprintCallable, Category = "VFX|Meditation")
    void StopMeditationAura();

    UFUNCTION(BlueprintCallable, Category = "VFX|Meditation")
    void CreateBreathingVisualization(float BreathRate = 0.2f);

    // Spiritual Transformation VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Transformation")
    void TriggerSpiritualAwakening(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "VFX|Transformation")
    void CreateLightBody(AActor* Character, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Transformation")
    void SpawnDivineLight(FVector Location, float Intensity = 2.0f);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SetSacredGeometryPattern(int32 PatternType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreatePortalEffect(FVector Location, float Size = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnCrystalResonance(FVector Location, FLinearColor Color);

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* MainParticleSystem;

    // VFX Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UParticleSystem* ChakraParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UParticleSystem* EnergyFieldParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UParticleSystem* TranscendentParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UParticleSystem* MeditationAuraParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UParticleSystem* DivineLight ParticleSystem;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* EnergyFieldMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ChakraMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* LightBodyMaterial;

    // Material Parameter Collection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialParameterCollection* ConsciousnessParameters;

    // State Variables
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState CurrentConsciousnessState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<bool> ChakraActivationStates;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float ConsciousnessIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bInTranscendentState;

private:
    // Internal VFX Management
    void UpdateConsciousnessVFX(float DeltaTime);
    void UpdateChakraVFX(float DeltaTime);
    void UpdateEnergyFields(float DeltaTime);
    
    FLinearColor GetChakraColor(EChakraType ChakraType);
    FVector GetChakraLocation(EChakraType ChakraType, AActor* Character);
    
    // Active VFX tracking
    TArray<UParticleSystemComponent*> ActiveParticleSystems;
    TMap<EChakraType, UParticleSystemComponent*> ChakraParticles;
    
    // Timers
    FTimerHandle TranscendentTimerHandle;
    FTimerHandle ChakraAlignmentTimerHandle;
};