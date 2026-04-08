#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "ConsciousnessSystem.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Waking          UMETA(DisplayName = "Waking Consciousness"),
    Dreaming        UMETA(DisplayName = "Dream State"),
    DeepMeditation  UMETA(DisplayName = "Deep Meditation"),
    Transpersonal   UMETA(DisplayName = "Transpersonal Experience"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Void            UMETA(DisplayName = "Void State")
};

UENUM(BlueprintType)
enum class EPerceptionLayer : uint8
{
    Physical        UMETA(DisplayName = "Physical Reality"),
    Emotional       UMETA(DisplayName = "Emotional Layer"),
    Mental          UMETA(DisplayName = "Mental Constructs"),
    Intuitive       UMETA(DisplayName = "Intuitive Insights"),
    Archetypal      UMETA(DisplayName = "Archetypal Realm"),
    Cosmic          UMETA(DisplayName = "Cosmic Consciousness")
};

USTRUCT(BlueprintType)
struct FConsciousnessMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Awareness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Clarity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Equanimity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Integration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Transcendence = 0.0f;

    FConsciousnessMetrics()
    {
        Awareness = 0.5f;
        Clarity = 0.5f;
        Equanimity = 0.5f;
        Integration = 0.5f;
        Transcendence = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FConsciousnessStateData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FConsciousnessMetrics RequiredMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EPerceptionLayer> AvailablePerceptionLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundBase* TransitionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMaterialInterface* VisualFilter;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessStateChanged, EConsciousnessState, OldState, EConsciousnessState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerceptionLayerActivated, EPerceptionLayer, Layer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsciousnessMetricsUpdated, const FConsciousnessMetrics&, NewMetrics);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UConsciousnessSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UConsciousnessSystem();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Consciousness Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool TransitionToState(EConsciousnessState NewState, bool bForceTransition = false);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateMetrics(const FConsciousnessMetrics& NewMetrics);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ModifyMetric(const FString& MetricName, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool CanAccessPerceptionLayer(EPerceptionLayer Layer) const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ActivatePerceptionLayer(EPerceptionLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void DeactivatePerceptionLayer(EPerceptionLayer Layer);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Consciousness")
    EConsciousnessState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    const FConsciousnessMetrics& GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    TArray<EPerceptionLayer> GetActivePerceptionLayers() const { return ActivePerceptionLayers; }

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    float GetTranscendenceLevel() const;

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    bool IsInTransition() const { return bIsTransitioning; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Consciousness Events")
    FOnConsciousnessStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Consciousness Events")
    FOnPerceptionLayerActivated OnPerceptionLayerActivated;

    UPROPERTY(BlueprintAssignable, Category = "Consciousness Events")
    FOnConsciousnessMetricsUpdated OnMetricsUpdated;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    class UDataTable* ConsciousnessStatesTable;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState PreviousState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FConsciousnessMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<EPerceptionLayer> ActivePerceptionLayers;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransitionProgress;

private:
    FTimerHandle TransitionTimerHandle;
    FConsciousnessStateData* GetStateData(EConsciousnessState State) const;
    void CompleteTransition();
    void UpdateTransitionProgress();
    bool ValidateStateTransition(EConsciousnessState FromState, EConsciousnessState ToState) const;
    void ApplyStateEffects(EConsciousnessState State);
};