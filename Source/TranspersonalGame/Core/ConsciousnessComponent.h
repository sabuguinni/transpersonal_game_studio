#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ConsciousnessComponent.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Expanded        UMETA(DisplayName = "Expanded Awareness"),
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Void            UMETA(DisplayName = "Void State")
};

USTRUCT(BlueprintType)
struct FConsciousnessLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState State = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FGameplayTagContainer ActiveStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float TransitionSpeed = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessChanged, EConsciousnessState, NewState, float, Intensity);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UConsciousnessComponent();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Consciousness Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SetConsciousnessState(EConsciousnessState NewState, float TargetIntensity = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ModifyConsciousnessIntensity(float Delta);

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    EConsciousnessState GetCurrentState() const { return CurrentLevel.State; }

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    float GetCurrentIntensity() const { return CurrentLevel.Intensity; }

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TriggerConsciousnessShift(const FGameplayTag& TriggerTag);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Consciousness")
    FOnConsciousnessChanged OnConsciousnessChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FConsciousnessLevel CurrentLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FConsciousnessLevel TargetLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float BaseTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bAutoTransition = true;

    void UpdateConsciousnessTransition(float DeltaTime);
    void BroadcastConsciousnessChange();
};