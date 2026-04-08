#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/DirectionalLight.h"
#include "ConsciousnessEnvironment.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Meditative      UMETA(DisplayName = "Meditative State"),
    Transcendent    UMETA(DisplayName = "Transcendent Experience"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Cosmic          UMETA(DisplayName = "Cosmic Awareness")
};

UENUM(BlueprintType)
enum class EEnvironmentBiome : uint8
{
    SacredForest    UMETA(DisplayName = "Sacred Forest"),
    CrystalCaves    UMETA(DisplayName = "Crystal Caves"),
    FloatingIslands UMETA(DisplayName = "Floating Islands"),
    LightRealm      UMETA(DisplayName = "Realm of Light"),
    VoidSpace       UMETA(DisplayName = "Void Space"),
    MandalaGarden   UMETA(DisplayName = "Mandala Garden")
};

USTRUCT(BlueprintType)
struct FEnvironmentTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCurveFloat* TransitionCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSmoothTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GeometryMorphSpeed = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessEnvironment : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessEnvironment();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    UStaticMeshComponent* TerrainMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    UStaticMeshComponent* VegetationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    UStaticMeshComponent* SacredGeometry;

    // Lighting System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* AmbientLight;

    // Material System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialParameterCollection* EnvironmentParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* VegetationMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SacredGeometryMaterial;

    // Environment State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EEnvironmentBiome CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessLevel = 0.0f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float SpiritualResonance = 0.0f; // 0-1 range

    // Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    FEnvironmentTransition TransitionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionProgress = 0.0f;

    // Sacred Geometry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    TArray<UStaticMesh*> GeometryMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    float GeometryRotationSpeed = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    float GeometryFloatAmplitude = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    float GeometryFloatFrequency = 1.0f;

public:
    // Environment Control Functions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetEnvironmentBiome(EEnvironmentBiome NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateConsciousnessLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateSpiritualResonance(float NewResonance);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerEnvironmentTransition(EConsciousnessState TargetState, EEnvironmentBiome TargetBiome);

    // Visual Effects
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SpawnLightParticles(FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateEnergyWaves(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ManifestSacredSymbols(FVector Location, float Scale);

    // Procedural Generation
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void GenerateTerrainForState(EConsciousnessState State);

    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void PopulateVegetation(float Density, float ConsciousnessInfluence);

    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void PlaceSacredGeometry(int32 Count, float MinDistance);

protected:
    // Internal Functions
    void UpdateMaterialParameters();
    void UpdateLighting();
    void UpdateGeometryAnimation(float DeltaTime);
    void ProcessTransition(float DeltaTime);
    void ApplyConsciousnessEffects();
    void UpdateBiomeCharacteristics();

    // Transition Helpers
    FLinearColor GetStateColor(EConsciousnessState State);
    float GetStateLightIntensity(EConsciousnessState State);
    FVector GetBiomeScale(EEnvironmentBiome Biome);
    UMaterialInterface* GetBiomeMaterial(EEnvironmentBiome Biome);

private:
    float GeometryAnimationTime = 0.0f;
    FVector InitialGeometryLocation;
    FRotator InitialGeometryRotation;
    
    // Cached values for smooth transitions
    FLinearColor PreviousAmbientColor;
    FLinearColor TargetAmbientColor;
    float PreviousLightIntensity;
    float TargetLightIntensity;
};