#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "ConsciousnessSystem.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Expanded        UMETA(DisplayName = "Expanded Awareness"),
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    Unity           UMETA(DisplayName = "Unity Consciousness")
};

UENUM(BlueprintType)
enum class EAwarenessLevel : uint8
{
    Unconscious     UMETA(DisplayName = "Unconscious"),
    Subconscious    UMETA(DisplayName = "Subconscious"),
    Conscious       UMETA(DisplayName = "Conscious"),
    Superconscious  UMETA(DisplayName = "Superconscious"),
    Cosmic          UMETA(DisplayName = "Cosmic Consciousness")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FConsciousnessMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float AwarenessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float IntegrationLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float TranscendenceProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float UnityConnection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState CurrentState = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EAwarenessLevel CurrentAwareness = EAwarenessLevel::Conscious;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FConsciousnessMetrics ConsciousnessData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float StateTransitionThreshold = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float AwarenessGrowthRate = 1.0f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ModifyAwareness(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TriggerConsciousnessShift(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    FConsciousnessMetrics GetConsciousnessMetrics() const { return ConsciousnessData; }

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool CanTranscend() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Consciousness")
    void OnConsciousnessStateChanged(EConsciousnessState OldState, EConsciousnessState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Consciousness")
    void OnAwarenessLevelChanged(EAwarenessLevel OldLevel, EAwarenessLevel NewLevel);
};

UCLASS()
class TRANSPERSONALGAME_API AConsciousnessGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AConsciousnessGameMode();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Systems")
    float GlobalConsciousnessField = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Systems")
    TArray<AActor*> ConsciousEntities;

public:
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void RegisterConsciousEntity(AActor* Entity);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateGlobalField();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    float GetGlobalConsciousnessLevel() const { return GlobalConsciousnessField; }
};