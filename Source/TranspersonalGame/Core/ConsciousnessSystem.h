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
    Meditative      UMETA(DisplayName = "Meditative State"),
    Flow            UMETA(DisplayName = "Flow State"),
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
struct FConsciousnessMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Awareness = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Presence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Coherence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Transcendence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float Unity = 0.0f;
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EAwarenessLevel AwarenessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FConsciousnessMetrics Metrics;

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TriggerTranscendentExperience();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void EnterMeditativeState();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ExitMeditativeState();

    UFUNCTION(BlueprintPure, Category = "Consciousness")
    float GetOverallConsciousnessLevel() const;

private:
    float StateTransitionTimer;
    float MeditationDuration;
    bool bInTranscendentExperience;

    void CalculateAwarenessLevel();
    void ProcessStateTransitions(float DeltaTime);
};

UCLASS()
class TRANSPERSONALGAME_API AConsciousnessGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AConsciousnessGameMode();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float GlobalConsciousnessField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    int32 MaxPlayers;

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateGlobalField();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SynchronizePlayerStates();

protected:
    virtual void BeginPlay() override;

private:
    TArray<UConsciousnessComponent*> PlayerConsciousnessComponents;
};