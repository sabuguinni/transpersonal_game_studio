#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "ConsciousnessSystem.h"
#include "RealitySystem.h"
#include "PerceptionSystem.generated.h"

UENUM(BlueprintType)
enum class EPerceptionMode : uint8
{
    Normal          UMETA(DisplayName = "Normal Perception"),
    Enhanced        UMETA(DisplayName = "Enhanced Awareness"),
    Intuitive       UMETA(DisplayName = "Intuitive Sight"),
    Energetic       UMETA(DisplayName = "Energetic Vision"),
    Archetypal      UMETA(DisplayName = "Archetypal Perception"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Void            UMETA(DisplayName = "Void Perception")
};

UENUM(BlueprintType)
enum class EPerceptionFilter : uint8
{
    None            UMETA(DisplayName = "No Filter"),
    Emotional       UMETA(DisplayName = "Emotional Overlay"),
    Mental          UMETA(DisplayName = "Mental Constructs"),
    Energetic       UMETA(DisplayName = "Energy Fields"),
    Symbolic        UMETA(DisplayName = "Symbolic Representation"),
    Geometric       UMETA(DisplayName = "Sacred Geometry"),
    Fractal         UMETA(DisplayName = "Fractal Patterns")
};

USTRUCT(BlueprintType)
struct FPerceptionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerceptionMode Mode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EPerceptionFilter> ActiveFilters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Clarity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Depth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowHiddenObjects = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowEnergyFields = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowSymbolicMeaning = false;

    FPerceptionData()
    {
        Mode = EPerceptionMode::Normal;
        Intensity = 1.0f;
        Clarity = 1.0f;
        Depth = 0.5f;
        bShowHiddenObjects = false;
        bShowEnergyFields = false;
        bShowSymbolicMeaning = false;
    }
};

USTRUCT(BlueprintType)
struct FPerceptualObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EPerceptionLayer> VisibleInLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EPerceptionMode> VisibleInModes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EnergyColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SymbolicMeaning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsArchetypal = false;

    FPerceptualObject()
    {
        Actor = nullptr;
        BaseOpacity = 1.0f;
        EnergyColor = FLinearColor::White;
        bIsArchetypal = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerceptionModeChanged, EPerceptionMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerceptionFilterToggled, EPerceptionFilter, Filter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectPerceptionChanged, AActor*, Actor, bool, bVisible);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerceptionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerceptionSystem();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Core Perception Functions
    UFUNCTION(BlueprintCallable, Category = "Perception")
    void SetPerceptionMode(EPerceptionMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void TogglePerceptionFilter(EPerceptionFilter Filter);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void SetPerceptionIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void SetPerceptionClarity(float NewClarity);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void SetPerceptionDepth(float NewDepth);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void RegisterPerceptualObject(const FPerceptualObject& Object);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void UnregisterPerceptualObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void UpdateObjectVisibility(AActor* Actor);

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Perception")
    void UpdateFromConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void UpdateFromRealityLayer(ERealityLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void UpdateFromPerceptionLayer(EPerceptionLayer Layer, bool bActive);

    // Query Functions
    UFUNCTION(BlueprintPure, Category = "Perception")
    EPerceptionMode GetCurrentMode() const { return CurrentPerception.Mode; }

    UFUNCTION(BlueprintPure, Category = "Perception")
    FPerceptionData GetCurrentPerceptionData() const { return CurrentPerception; }

    UFUNCTION(BlueprintPure, Category = "Perception")
    bool IsObjectVisible(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Perception")
    float GetObjectOpacity(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Perception")
    FLinearColor GetObjectEnergyColor(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "Perception")
    TArray<AActor*> GetVisibleObjects() const;

    UFUNCTION(BlueprintPure, Category = "Perception")
    TArray<AActor*> GetObjectsWithSymbolicMeaning() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Perception Events")
    FOnPerceptionModeChanged OnPerceptionModeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Perception Events")
    FOnPerceptionFilterToggled OnPerceptionFilterToggled;

    UPROPERTY(BlueprintAssignable, Category = "Perception Events")
    FOnObjectPerceptionChanged OnObjectPerceptionChanged;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FPerceptionData CurrentPerception;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FPerceptualObject> RegisteredObjects;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<EPerceptionLayer> ActivePerceptionLayers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    float PerceptionUpdateRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    TMap<EPerceptionMode, FPerceptionData> ModePresets;

private:
    class UConsciousnessSystem* ConsciousnessSystem;
    class URealitySystem* RealitySystem;
    FTimerHandle PerceptionUpdateTimer;

    void InitializeModePresets();
    void UpdateAllObjectsVisibility();
    void ApplyPerceptionEffects();
    bool ShouldObjectBeVisible(const FPerceptualObject& Object) const;
    float CalculateObjectOpacity(const FPerceptualObject& Object) const;
    void ApplyVisualFilters();
};