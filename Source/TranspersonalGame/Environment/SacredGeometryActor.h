#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SacredGeometryActor.generated.h"

UENUM(BlueprintType)
enum class ESacredGeometryType : uint8
{
    FlowerOfLife     UMETA(DisplayName = "Flower of Life"),
    Merkaba          UMETA(DisplayName = "Merkaba"),
    SriYantra        UMETA(DisplayName = "Sri Yantra"),
    Mandala          UMETA(DisplayName = "Mandala"),
    PlatonicSolids   UMETA(DisplayName = "Platonic Solids"),
    TorusField       UMETA(DisplayName = "Torus Field"),
    GoldenRatio      UMETA(DisplayName = "Golden Ratio Spiral"),
    ChakraSymbol     UMETA(DisplayName = "Chakra Symbol")
};

USTRUCT(BlueprintType)
struct FSacredGeometryProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESacredGeometryType GeometryType = ESacredGeometryType::FlowerOfLife;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RotationSpeed = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PulseAmplitude = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PulseFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EnergyColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bResonateWithConsciousness = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessMultiplier = 2.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASacredGeometryActor : public AActor
{
    GENERATED_BODY()

public:
    ASacredGeometryActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sacred Geometry")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sacred Geometry")
    UStaticMeshComponent* GeometryMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sacred Geometry")
    UPointLightComponent* EnergyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sacred Geometry")
    UParticleSystemComponent* EnergyParticles;

    // Sacred Geometry Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    FSacredGeometryProperties GeometryProperties;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessResonance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float SpiritualEnergy = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bIsActivated = false;

    // Animation State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AnimationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector BaseLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FRotator BaseRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector BaseScale;

    // Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bPlayerInRange = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    class ATranspersonalCharacter* NearbyPlayer;

public:
    // Geometry Control
    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    void SetGeometryType(ESacredGeometryType NewType);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    void ActivateGeometry();

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    void DeactivateGeometry();

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    void SetConsciousnessResonance(float NewResonance);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    void SetSpiritualEnergy(float NewEnergy);

    // Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartGeometryAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopGeometryAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationSpeed(float Speed);

    // Interaction
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerEnterRange(class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerExitRange(class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void TriggerConsciousnessResonance();

    // Effects
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SpawnEnergyBurst();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateHealingField();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ManifestWisdomSymbols();

protected:
    // Internal Functions
    void UpdateGeometryAnimation(float DeltaTime);
    void UpdateConsciousnessEffects();
    void UpdateEnergyVisualization();
    void CheckPlayerProximity();
    void ApplyGeometryProperties();

    // Geometry Helpers
    UStaticMesh* GetMeshForGeometryType(ESacredGeometryType Type);
    FLinearColor GetColorForGeometryType(ESacredGeometryType Type);
    float GetBaseScaleForGeometryType(ESacredGeometryType Type);
    float GetRotationSpeedForGeometryType(ESacredGeometryType Type);

    // Animation Helpers
    FVector CalculateFloatingOffset(float Time);
    FRotator CalculateRotationOffset(float Time);
    FVector CalculateScaleOffset(float Time);

    // Consciousness Helpers
    float CalculateConsciousnessInfluence();
    void ApplyConsciousnessModifiers();
    void TriggerConsciousnessEvent();

private:
    // Cached values
    FVector InitialLocation;
    FRotator InitialRotation;
    FVector InitialScale;
    
    // Animation state
    bool bAnimationActive = true;
    float AnimationSpeed = 1.0f;
    
    // Interaction state
    float LastPlayerCheckTime = 0.0f;
    float PlayerCheckInterval = 0.1f; // Check every 0.1 seconds
};