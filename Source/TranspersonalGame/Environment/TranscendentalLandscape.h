// TranscendentalLandscape.h
// Sistema de paisagens transcendentais para estados alterados de consciência
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TranscendentalLandscape.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    Meditative      UMETA(DisplayName = "Meditative"),
    Psychedelic     UMETA(DisplayName = "Psychedelic"),
    Mystical        UMETA(DisplayName = "Mystical"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct FLandscapeLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightMax = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SlopeMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SlopMax = 90.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranscendentalLandscape : public AActor
{
    GENERATED_BODY()

public:
    ATranscendentalLandscape();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TerrainMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* VegetationInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* CrystalInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* EnergyNodeInstances;

    // Configurações de paisagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector2D LandscapeSize = FVector2D(10000.0f, 10000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 TerrainResolution = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float MaxHeight = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    TArray<FLandscapeLayer> VegetationLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    TArray<FLandscapeLayer> CrystalLayers;

    // Estados de consciência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState CurrentState = EConsciousnessState::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    UMaterialParameterCollection* EnvironmentParameters;

    // Efeitos visuais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float EnergyFlowIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FLinearColor AuraColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float GeometryMorphing = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float FractalComplexity = 1.0f;

public:
    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void GenerateLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void PopulateVegetation();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void PlaceCrystals();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void CreateEnergyNodes();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TransitionToState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void UpdateVisualEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void PulseEnergyField(float Intensity);

private:
    // Funções internas
    void GenerateHeightmap();
    void ApplyFractalNoise(TArray<float>& Heights, float Frequency, float Amplitude);
    void SmoothTerrain(TArray<float>& Heights, int32 Iterations);
    
    void PlaceInstancesOnTerrain(UInstancedStaticMeshComponent* InstanceComponent, 
                                const FLandscapeLayer& Layer, int32 Count);
    
    bool IsValidPlacement(const FVector& Location, const FLandscapeLayer& Layer);
    float GetHeightAtLocation(const FVector2D& Location);
    float GetSlopeAtLocation(const FVector2D& Location);
    
    void UpdateMaterialParameters();
    void AnimateGeometry(float DeltaTime);
    void ProcessConsciousnessTransition(float DeltaTime);

    // Variáveis internas
    TArray<float> HeightmapData;
    float CurrentTransitionTime;
    float TargetTransitionTime;
    EConsciousnessState PreviousState;
    
    // Cache de performance
    TArray<FVector> CachedPositions;
    TArray<FTransform> InstanceTransforms;
    bool bNeedsRegeneration;
    
    // Parâmetros de ruído
    float NoiseScale = 0.001f;
    int32 NoiseOctaves = 6;
    float NoisePersistence = 0.5f;
    float NoiseLacunarity = 2.0f;
};