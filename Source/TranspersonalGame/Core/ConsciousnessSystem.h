#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ConsciousnessSystem.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Meditative      UMETA(DisplayName = "Meditative State"),
    Lucid           UMETA(DisplayName = "Lucid Awareness"),
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Void            UMETA(DisplayName = "Void State")
};

UENUM(BlueprintType)
enum class ERealityLayer : uint8
{
    Physical        UMETA(DisplayName = "Physical Reality"),
    Emotional       UMETA(DisplayName = "Emotional Layer"),
    Mental          UMETA(DisplayName = "Mental Constructs"),
    Archetypal      UMETA(DisplayName = "Archetypal Realm"),
    Causal          UMETA(DisplayName = "Causal Dimension"),
    Absolute        UMETA(DisplayName = "Absolute Reality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FConsciousnessMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Awareness = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Clarity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Compassion = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Wisdom = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Integration = 50.0f;

    FConsciousnessMetrics()
    {
        Awareness = 50.0f;
        Clarity = 50.0f;
        Compassion = 50.0f;
        Wisdom = 50.0f;
        Integration = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRealityShiftData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    ERealityLayer SourceLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    ERealityLayer TargetLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    float RequiredAwareness = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    TSoftObjectPtr<class UMaterialInterface> LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality Shift")
    TSoftObjectPtr<class USoundBase> TransitionSound;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessStateChanged, EConsciousnessState, OldState, EConsciousnessState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRealityLayerChanged, ERealityLayer, OldLayer, ERealityLayer, NewLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsciousnessMetricsUpdated, const FConsciousnessMetrics&, NewMetrics);

/**
 * Core consciousness simulation system for transpersonal experiences
 * Manages consciousness states, reality layers, and awareness metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UConsciousnessSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UConsciousnessSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Consciousness State Management
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SetConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    EConsciousnessState GetCurrentConsciousnessState() const { return CurrentConsciousnessState; }

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool CanTransitionToState(EConsciousnessState TargetState) const;

    // Reality Layer Management
    UFUNCTION(BlueprintCallable, Category = "Reality")
    void ShiftToRealityLayer(ERealityLayer NewLayer);

    UFUNCTION(BlueprintPure, Category = "Reality")
    ERealityLayer GetCurrentRealityLayer() const { return CurrentRealityLayer; }

    UFUNCTION(BlueprintCallable, Category = "Reality")
    bool CanAccessRealityLayer(ERealityLayer TargetLayer) const;

    // Consciousness Metrics
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessMetric(const FString& MetricName, float Delta);

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    FConsciousnessMetrics GetConsciousnessMetrics() const { return ConsciousnessMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SetConsciousnessMetrics(const FConsciousnessMetrics& NewMetrics);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnConsciousnessStateChanged OnConsciousnessStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRealityLayerChanged OnRealityLayerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnConsciousnessMetricsUpdated OnConsciousnessMetricsUpdated;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void LoadRealityShiftData(UDataTable* DataTable);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState CurrentConsciousnessState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ERealityLayer CurrentRealityLayer;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FConsciousnessMetrics ConsciousnessMetrics;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    TObjectPtr<UDataTable> RealityShiftDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float MetricDecayRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float StateTransitionCooldown = 2.0f;

private:
    void UpdateConsciousnessMetrics(float DeltaTime);
    void ProcessStateTransitions();
    
    FTimerHandle MetricsUpdateTimer;
    float LastStateTransitionTime;
};