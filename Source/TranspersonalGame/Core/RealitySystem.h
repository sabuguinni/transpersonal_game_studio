#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "ConsciousnessSystem.h"
#include "RealitySystem.generated.h"

UENUM(BlueprintType)
enum class ERealityLayer : uint8
{
    Consensus       UMETA(DisplayName = "Consensus Reality"),
    Personal        UMETA(DisplayName = "Personal Reality"),
    Collective      UMETA(DisplayName = "Collective Unconscious"),
    Archetypal      UMETA(DisplayName = "Archetypal Realm"),
    Causal          UMETA(DisplayName = "Causal Plane"),
    Absolute        UMETA(DisplayName = "Absolute Reality")
};

UENUM(BlueprintType)
enum class ERealityStability : uint8
{
    Solid           UMETA(DisplayName = "Solid - Unchanging"),
    Stable          UMETA(DisplayName = "Stable - Minor fluctuations"),
    Fluid           UMETA(DisplayName = "Fluid - Moderate changes"),
    Malleable       UMETA(DisplayName = "Malleable - Responsive to consciousness"),
    Chaotic         UMETA(DisplayName = "Chaotic - Unpredictable shifts"),
    Void            UMETA(DisplayName = "Void - No stable form")
};

USTRUCT(BlueprintType)
struct FRealityLayerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERealityLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERealityStability Stability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Opacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Influence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EConsciousnessState> RequiredStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMaterialInterface* LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundBase* AmbientSound;

    FRealityLayerData()
    {
        Layer = ERealityLayer::Consensus;
        Stability = ERealityStability::Solid;
        Opacity = 1.0f;
        Influence = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FRealityDistortion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERealityLayer AffectedLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPermanent = false;

    FRealityDistortion()
    {
        Location = FVector::ZeroVector;
        Radius = 100.0f;
        Intensity = 0.5f;
        AffectedLayer = ERealityLayer::Consensus;
        Duration = 10.0f;
        bIsPermanent = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRealityLayerChanged, ERealityLayer, NewLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRealityDistortionCreated, FVector, Location, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRealityStabilityChanged, ERealityStability, NewStability);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API URealitySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    URealitySystem();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Core Reality Functions
    UFUNCTION(BlueprintCallable, Category = "Reality")
    void SetActiveRealityLayer(ERealityLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void BlendRealityLayers(const TArray<ERealityLayer>& Layers, const TArray<float>& Weights);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void CreateRealityDistortion(const FRealityDistortion& Distortion);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void RemoveRealityDistortion(int32 DistortionID);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void SetRealityStability(ERealityStability NewStability);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void ModifyLayerOpacity(ERealityLayer Layer, float NewOpacity);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void ModifyLayerInfluence(ERealityLayer Layer, float NewInfluence);

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Reality")
    void UpdateFromConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    TArray<ERealityLayer> GetAvailableLayersForState(EConsciousnessState State) const;

    // Query Functions
    UFUNCTION(BlueprintPure, Category = "Reality")
    ERealityLayer GetCurrentRealityLayer() const { return CurrentLayer; }

    UFUNCTION(BlueprintPure, Category = "Reality")
    ERealityStability GetCurrentStability() const { return CurrentStability; }

    UFUNCTION(BlueprintPure, Category = "Reality")
    TArray<FRealityLayerData> GetActiveLayerData() const { return ActiveLayers; }

    UFUNCTION(BlueprintPure, Category = "Reality")
    float GetRealityCoherence() const;

    UFUNCTION(BlueprintPure, Category = "Reality")
    bool IsLocationDistorted(FVector Location, float& DistortionIntensity) const;

    UFUNCTION(BlueprintPure, Category = "Reality")
    TArray<FRealityDistortion> GetActiveDistortions() const { return ActiveDistortions; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Reality Events")
    FOnRealityLayerChanged OnRealityLayerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Reality Events")
    FOnRealityDistortionCreated OnRealityDistortionCreated;

    UPROPERTY(BlueprintAssignable, Category = "Reality Events")
    FOnRealityStabilityChanged OnRealityStabilityChanged;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ERealityLayer CurrentLayer;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ERealityStability CurrentStability;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FRealityLayerData> ActiveLayers;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FRealityDistortion> ActiveDistortions;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    TMap<ERealityLayer, FRealityLayerData> LayerDefinitions;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    float DistortionUpdateRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    float MaxDistortions = 50;

private:
    class UConsciousnessSystem* ConsciousnessSystem;
    FTimerHandle DistortionUpdateTimer;
    int32 NextDistortionID;

    void InitializeLayerDefinitions();
    void UpdateDistortions();
    void ApplyLayerEffects();
    FRealityLayerData GetLayerData(ERealityLayer Layer) const;
    void CleanupExpiredDistortions();
};